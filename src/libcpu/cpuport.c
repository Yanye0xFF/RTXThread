/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2011-10-21     Bernard      the first version.
 * 2011-10-27     aozima       update for cortex-M4 FPU.
 * 2011-12-31     aozima       fixed stack align issues.
 * 2012-01-01     aozima       support context switch load/store FPU register.
 * 2012-12-11     lgnq         fixed the coding style.
 * 2012-12-23     aozima       stack addr align to 8byte.
 * 2012-12-29     Bernard      Add exception hook.
 * 2013-06-23     aozima       support lazy stack optimized.
 * 2018-07-24     aozima       enhancement hard fault exception handler.
 * 2019-07-03     yangjie      add __rt_ffs() for armclang.
 * 2022-06-12     jonas        fixed __rt_ffs() for armclang.
 * 2023-01-08     yanye        add human readable fault message refered by cm_backtrace.
 */

#include <rtthread.h>
#include <rthw.h>
#include "core_cm4.h"
#include "rtx_os.h"
#include "drv_dwt.h"
#include "rtconfig.h"

#if               /* ARMCC */ (  (defined ( __CC_ARM ) && defined ( __TARGET_FPU_VFP ))    \
                  /* Clang */ || (defined ( __clang__ ) && defined ( __VFP_FP__ ) && !defined(__SOFTFP__)) \
                  /* IAR */   || (defined ( __ICCARM__ ) && defined ( __ARMVFP__ ))        \
                  /* GNU */   || (defined ( __GNUC__ ) && defined ( __VFP_FP__ ) && !defined(__SOFTFP__)) )
#define USE_FPU   1
#else
#define USE_FPU   0
#endif

#define SCB_CFSR        (*(volatile const unsigned *)0xE000ED28) /* Configurable Fault Status Register */
#define SCB_HFSR        (*(volatile const unsigned *)0xE000ED2C) /* HardFault Status Register */
#define SCB_MMAR        (*(volatile const unsigned *)0xE000ED34) /* MemManage Fault Address register */
#define SCB_BFAR        (*(volatile const unsigned *)0xE000ED38) /* Bus Fault Address Register */
#define SCB_AIRCR       (*(volatile unsigned long *)0xE000ED0C)  /* Reset control Address Register */
#define SCB_RESET_VALUE 0x05FA0004                               /* Reset value, write to SCB_AIRCR can reset cpu */

#define SCB_CFSR_MFSR   (*(volatile const unsigned char*)0xE000ED28)  /* Memory-management Fault Status Register */
#define SCB_CFSR_BFSR   (*(volatile const unsigned char*)0xE000ED29)  /* Bus Fault Status Register */
#define SCB_CFSR_UFSR   (*(volatile const unsigned short*)0xE000ED2A) /* Usage Fault Status Register */

#define CMB_CPU_ARM_CORTEX_M0             0
#define CMB_CPU_ARM_CORTEX_M3             1
#define CMB_CPU_ARM_CORTEX_M4             2
#define CMB_CPU_ARM_CORTEX_M7             3
#define CMB_CPU_ARM_CORTEX_M33            4
#define CMB_CPU_PLATFORM_TYPE             CMB_CPU_ARM_CORTEX_M4

/* system handler control and state register */
#ifndef CMB_SYSHND_CTRL
#define CMB_SYSHND_CTRL                (*(volatile unsigned int*)  (0xE000ED24u))
#endif

/* memory management fault status register */
#ifndef CMB_NVIC_MFSR
#define CMB_NVIC_MFSR                  (*(volatile unsigned char*) (0xE000ED28u))
#endif

/* bus fault status register */
#ifndef CMB_NVIC_BFSR
#define CMB_NVIC_BFSR                  (*(volatile unsigned char*) (0xE000ED29u))
#endif

/* usage fault status register */
#ifndef CMB_NVIC_UFSR
#define CMB_NVIC_UFSR                  (*(volatile unsigned short*)(0xE000ED2Au))
#endif

/* hard fault status register */
#ifndef CMB_NVIC_HFSR
#define CMB_NVIC_HFSR                  (*(volatile unsigned int*)  (0xE000ED2Cu))
#endif

/* debug fault status register */
#ifndef CMB_NVIC_DFSR
#define CMB_NVIC_DFSR                  (*(volatile unsigned short*)(0xE000ED30u))
#endif

/* memory management fault address register */
#ifndef CMB_NVIC_MMAR
#define CMB_NVIC_MMAR                  (*(volatile unsigned int*)  (0xE000ED34u))
#endif

/* bus fault manage address register */
#ifndef CMB_NVIC_BFAR
#define CMB_NVIC_BFAR                  (*(volatile unsigned int*)  (0xE000ED38u))
#endif

/* auxiliary fault status register */
#ifndef CMB_NVIC_AFSR
#define CMB_NVIC_AFSR                  (*(volatile unsigned short*)(0xE000ED3Cu))
#endif

enum {
    PRINT_HFSR_VECTBL = 0,
    PRINT_MFSR_IACCVIOL,
    PRINT_MFSR_DACCVIOL,
    PRINT_MFSR_MUNSTKERR,
    PRINT_MFSR_MSTKERR,
    PRINT_MFSR_MLSPERR,
    PRINT_BFSR_IBUSERR,
    PRINT_BFSR_PRECISERR,
    PRINT_BFSR_IMPREISERR,
    PRINT_BFSR_UNSTKERR,
    PRINT_BFSR_STKERR,
    PRINT_BFSR_LSPERR,
    PRINT_UFSR_UNDEFINSTR,
    PRINT_UFSR_INVSTATE,
    PRINT_UFSR_INVPC,
    PRINT_UFSR_NOCP,
#if (CMB_CPU_PLATFORM_TYPE == CMB_CPU_ARM_CORTEX_M33)
    PRINT_UFSR_STKOF,
#endif
    PRINT_UFSR_UNALIGNED,
    PRINT_UFSR_DIVBYZERO0,
    PRINT_DFSR_HALTED,
    PRINT_DFSR_BKPT,
    PRINT_DFSR_DWTTRAP,
    PRINT_DFSR_VCATCH,
    PRINT_DFSR_EXTERNAL,
    PRINT_MMAR,
    PRINT_BFAR,
};

static const char * const print_info[] = {
    [PRINT_HFSR_VECTBL]           = "Hard fault is caused by failed vector fetch",
    [PRINT_MFSR_IACCVIOL]         = "instruction access violation",
    [PRINT_MFSR_DACCVIOL]         = "data access violation",
    [PRINT_MFSR_MUNSTKERR]        = "unstacking error",
    [PRINT_MFSR_MSTKERR]          = "stacking error",
    [PRINT_MFSR_MLSPERR]          = "floating-point lazy state preservation",
    [PRINT_BFSR_IBUSERR]          = "instruction access violation",
    [PRINT_BFSR_PRECISERR]        = "precise data access violation",
    [PRINT_BFSR_IMPREISERR]       = "imprecise data access violation",
    [PRINT_BFSR_UNSTKERR]         = "unstacking error",
    [PRINT_BFSR_STKERR]           = "stacking error",
    [PRINT_BFSR_LSPERR]           = "floating-point lazy state preservation",
    [PRINT_UFSR_UNDEFINSTR]       = "attempts to execute an undefined instruction",
    [PRINT_UFSR_INVSTATE]         = "attempts to switch to an invalid state (e.g., ARM)",
    [PRINT_UFSR_INVPC]            = "attempts to do an exception with a bad value in the EXC_RETURN number",
    [PRINT_UFSR_NOCP]             = "attempts to execute a coprocessor instruction",
    #if (CMB_CPU_PLATFORM_TYPE == CMB_CPU_ARM_CORTEX_M33)
        [PRINT_UFSR_STKOF]        = "Usage fault is caused by indicates that a stack overflow (hardware check) has taken place",
    #endif
    [PRINT_UFSR_UNALIGNED]        = "indicates that an unaligned access fault has taken place",
    [PRINT_UFSR_DIVBYZERO0]       = "Indicates a divide by zero has taken place (can be set only if DIV_0_TRP is set)",
    [PRINT_DFSR_HALTED]           = "halt requested in NVIC",
    [PRINT_DFSR_BKPT]             = "BKPT instruction executed",
    [PRINT_DFSR_DWTTRAP]          = "DWT match occurred",
    [PRINT_DFSR_VCATCH]           = "Vector fetch occurred",
    [PRINT_DFSR_EXTERNAL]         = "EDBGRQ signal asserted",
    [PRINT_MMAR]                  = "The memory management fault occurred address is %08x",
    [PRINT_BFAR]                  = "The bus fault occurred address is %08x",
};

/**
 * Cortex-M fault registers
 */
struct cmb_hard_fault_regs{
  union {
    unsigned int value;
    struct {
      unsigned int MEMFAULTACT    : 1;   // Read as 1 if memory management fault is active
      unsigned int BUSFAULTACT    : 1;   // Read as 1 if bus fault exception is active
#if (CMB_CPU_PLATFORM_TYPE == CMB_CPU_ARM_CORTEX_M33)
      unsigned int HARDFAULTACT   : 1;   // Read as 1 if hardfault is active
#else
      unsigned int UnusedBits1    : 1;
#endif
      unsigned int USGFAULTACT    : 1;   // Read as 1 if usage fault exception is active
#if (CMB_CPU_PLATFORM_TYPE == CMB_CPU_ARM_CORTEX_M33)
      unsigned int SECUREFAULTACT : 1;   // Read as 1 if secure fault exception is active
      unsigned int NMIACT         : 1;   // Read as 1 if NMI exception is active
      unsigned int UnusedBits2    : 1;
#else
      unsigned int UnusedBits2    : 3;
#endif
      unsigned int SVCALLACT      : 1;   // Read as 1 if SVC exception is active
      unsigned int MONITORACT     : 1;   // Read as 1 if debug monitor exception is active
      unsigned int UnusedBits3    : 1;
      unsigned int PENDSVACT      : 1;   // Read as 1 if PendSV exception is active
      unsigned int SYSTICKACT     : 1;   // Read as 1 if SYSTICK exception is active
      unsigned int USGFAULTPENDED : 1;   // Usage fault pended; usage fault started but was replaced by a higher-priority exception
      unsigned int MEMFAULTPENDED : 1;   // Memory management fault pended; memory management fault started but was replaced by a higher-priority exception
      unsigned int BUSFAULTPENDED : 1;   // Bus fault pended; bus fault handler was started but was replaced by a higher-priority exception
      unsigned int SVCALLPENDED   : 1;   // SVC pended; SVC was started but was replaced by a higher-priority exception
      unsigned int MEMFAULTENA    : 1;   // Memory management fault handler enable
      unsigned int BUSFAULTENA    : 1;   // Bus fault handler enable
      unsigned int USGFAULTENA    : 1;   // Usage fault handler enable
#if (CMB_CPU_PLATFORM_TYPE == CMB_CPU_ARM_CORTEX_M33)
      unsigned int SECUREFAULTENA : 1;   // Secure fault handler enable
      unsigned int SECUREFAULTPENDED : 1;   // Secure fault pended; Secure fault handler was started but was replaced by a higher-priority exception
      unsigned int HARDFAULTPENDED   : 1;   // Hard fault pended; Hard fault handler was started but was replaced by a higher-priority exception
#else
      // None
#endif
    } bits;
  } syshndctrl;                          // System Handler Control and State Register (0xE000ED24)

  union {
    unsigned char value;
    struct {
      unsigned char IACCVIOL    : 1;     // Instruction access violation
      unsigned char DACCVIOL    : 1;     // Data access violation
      unsigned char UnusedBits  : 1;
      unsigned char MUNSTKERR   : 1;     // Unstacking error
      unsigned char MSTKERR     : 1;     // Stacking error
      unsigned char MLSPERR     : 1;     // Floating-point lazy state preservation (M4/M7)
      unsigned char UnusedBits2 : 1;
      unsigned char MMARVALID   : 1;     // Indicates the MMAR is valid
    } bits;
  } mfsr;                                // Memory Management Fault Status Register (0xE000ED28)
  unsigned int mmar;                     // Memory Management Fault Address Register (0xE000ED34)

  union {
    unsigned char value;
    struct {
      unsigned char IBUSERR    : 1;      // Instruction access violation
      unsigned char PRECISERR  : 1;      // Precise data access violation
      unsigned char IMPREISERR : 1;      // Imprecise data access violation
      unsigned char UNSTKERR   : 1;      // Unstacking error
      unsigned char STKERR     : 1;      // Stacking error
      unsigned char LSPERR     : 1;      // Floating-point lazy state preservation (M4/M7)
      unsigned char UnusedBits : 1;
      unsigned char BFARVALID  : 1;      // Indicates BFAR is valid
    } bits;
  } bfsr;                                // Bus Fault Status Register (0xE000ED29)
  unsigned int bfar;                     // Bus Fault Manage Address Register (0xE000ED38)

  union {
    unsigned short value;
    struct {
      unsigned short UNDEFINSTR : 1;     // Attempts to execute an undefined instruction
      unsigned short INVSTATE   : 1;     // Attempts to switch to an invalid state (e.g., ARM)
      unsigned short INVPC      : 1;     // Attempts to do an exception with a bad value in the EXC_RETURN number
      unsigned short NOCP       : 1;     // Attempts to execute a coprocessor instruction
#if (CMB_CPU_PLATFORM_TYPE == CMB_CPU_ARM_CORTEX_M33)
      unsigned short STKOF      : 1;     // Indicates a stack overflow error has occured
      unsigned short UnusedBits : 3;
#else
      unsigned short UnusedBits : 4;
#endif
      unsigned short UNALIGNED  : 1;     // Indicates that an unaligned access fault has taken place
      unsigned short DIVBYZERO0 : 1;     // Indicates a divide by zero has taken place (can be set only if DIV_0_TRP is set)
    } bits;
  } ufsr;                                // Usage Fault Status Register (0xE000ED2A)

  union {
    unsigned int value;
    struct {
      unsigned int UnusedBits  : 1;
      unsigned int VECTBL      : 1;      // Indicates hard fault is caused by failed vector fetch
      unsigned int UnusedBits2 : 28;
      unsigned int FORCED      : 1;      // Indicates hard fault is taken because of bus fault/memory management fault/usage fault
      unsigned int DEBUGEVT    : 1;      // Indicates hard fault is triggered by debug event
    } bits;
  } hfsr;                                // Hard Fault Status Register (0xE000ED2C)

  union {
    unsigned int value;
    struct {
      unsigned int HALTED   : 1;         // Halt requested in NVIC
      unsigned int BKPT     : 1;         // BKPT instruction executed
      unsigned int DWTTRAP  : 1;         // DWT match occurred
      unsigned int VCATCH   : 1;         // Vector fetch occurred
      unsigned int EXTERNAL : 1;         // EDBGRQ signal asserted
    } bits;
  } dfsr;                                // Debug Fault Status Register (0xE000ED30)

  unsigned int afsr;                     // Auxiliary Fault Status Register (0xE000ED3C), Vendor controlled (optional)
};

/* exception hook */
static rt_err_t (*rt_exception_hook)(void *context) = RT_NULL;
static struct cmb_hard_fault_regs regs;

struct exception_stack_frame
{
    rt_uint32_t r0;
    rt_uint32_t r1;
    rt_uint32_t r2;
    rt_uint32_t r3;
    rt_uint32_t r12;
    rt_uint32_t lr;
    rt_uint32_t pc;
    rt_uint32_t psr;
};

struct stack_frame
{
#if USE_FPU
    rt_uint32_t flag;
#endif /* USE_FPU */

    /* r4 ~ r11 register */
    rt_uint32_t r4;
    rt_uint32_t r5;
    rt_uint32_t r6;
    rt_uint32_t r7;
    rt_uint32_t r8;
    rt_uint32_t r9;
    rt_uint32_t r10;
    rt_uint32_t r11;

    struct exception_stack_frame exception_stack_frame;
};

struct exception_stack_frame_fpu
{
    rt_uint32_t r0;
    rt_uint32_t r1;
    rt_uint32_t r2;
    rt_uint32_t r3;
    rt_uint32_t r12;
    rt_uint32_t lr;
    rt_uint32_t pc;
    rt_uint32_t psr;

#if USE_FPU
    /* FPU register */
    rt_uint32_t S0;
    rt_uint32_t S1;
    rt_uint32_t S2;
    rt_uint32_t S3;
    rt_uint32_t S4;
    rt_uint32_t S5;
    rt_uint32_t S6;
    rt_uint32_t S7;
    rt_uint32_t S8;
    rt_uint32_t S9;
    rt_uint32_t S10;
    rt_uint32_t S11;
    rt_uint32_t S12;
    rt_uint32_t S13;
    rt_uint32_t S14;
    rt_uint32_t S15;
    rt_uint32_t FPSCR;
    rt_uint32_t NO_NAME;
#endif
};

struct stack_frame_fpu
{
    rt_uint32_t flag;

    /* r4 ~ r11 register */
    rt_uint32_t r4;
    rt_uint32_t r5;
    rt_uint32_t r6;
    rt_uint32_t r7;
    rt_uint32_t r8;
    rt_uint32_t r9;
    rt_uint32_t r10;
    rt_uint32_t r11;

#if USE_FPU
    /* FPU register s16 ~ s31 */
    rt_uint32_t s16;
    rt_uint32_t s17;
    rt_uint32_t s18;
    rt_uint32_t s19;
    rt_uint32_t s20;
    rt_uint32_t s21;
    rt_uint32_t s22;
    rt_uint32_t s23;
    rt_uint32_t s24;
    rt_uint32_t s25;
    rt_uint32_t s26;
    rt_uint32_t s27;
    rt_uint32_t s28;
    rt_uint32_t s29;
    rt_uint32_t s30;
    rt_uint32_t s31;
#endif

    struct exception_stack_frame_fpu exception_stack_frame;
};


#if (CMB_CPU_PLATFORM_TYPE != CMB_CPU_ARM_CORTEX_M0)
/**
 * fault diagnosis then print cause of fault
 */
static void fault_diagnosis(void) {
    if (regs.hfsr.bits.VECTBL) {
        rt_kprintf(print_info[PRINT_HFSR_VECTBL]);
    }
    if (regs.hfsr.bits.FORCED) {
        /* Memory Management Fault */
        if (regs.mfsr.value) {
            rt_kprintf("mem manage fault:\n");
            rt_kprintf("SCB_CFSR_MFSR:0x%02X, ", SCB_CFSR_MFSR);
            rt_kprintf("Memory management fault is caused by ");

            if (regs.mfsr.bits.IACCVIOL) {
                rt_kprintf(print_info[PRINT_MFSR_IACCVIOL]);
            }
            if (regs.mfsr.bits.DACCVIOL) {
                rt_kprintf(print_info[PRINT_MFSR_DACCVIOL]);
            }
            if (regs.mfsr.bits.MUNSTKERR) {
                rt_kprintf(print_info[PRINT_MFSR_MUNSTKERR]);
            }
            if (regs.mfsr.bits.MSTKERR) {
                rt_kprintf(print_info[PRINT_MFSR_MSTKERR]);
            }

#if (CMB_CPU_PLATFORM_TYPE == CMB_CPU_ARM_CORTEX_M4) || (CMB_CPU_PLATFORM_TYPE == CMB_CPU_ARM_CORTEX_M7) || \
    (CMB_CPU_PLATFORM_TYPE == CMB_CPU_ARM_CORTEX_M33)
            if (regs.mfsr.bits.MLSPERR) {
                rt_kprintf(print_info[PRINT_MFSR_MLSPERR]);
            }
#endif

            if (regs.mfsr.bits.MMARVALID) {
                if (regs.mfsr.bits.IACCVIOL || regs.mfsr.bits.DACCVIOL) {
                    rt_kprintf(print_info[PRINT_MMAR], regs.mmar);
                }
            }
        }
        /* Bus Fault */
        if (regs.bfsr.value) {
            rt_kprintf("bus fault:\n");
            rt_kprintf("SCB_CFSR_BFSR:0x%02X, ", SCB_CFSR_BFSR);
            rt_kprintf("Bus fault is caused by ");

            if (regs.bfsr.bits.IBUSERR) {
                rt_kprintf(print_info[PRINT_BFSR_IBUSERR]);
            }
            if (regs.bfsr.bits.PRECISERR) {
                rt_kprintf(print_info[PRINT_BFSR_PRECISERR]);
            }
            if (regs.bfsr.bits.IMPREISERR) {
                rt_kprintf(print_info[PRINT_BFSR_IMPREISERR]);
            }
            if (regs.bfsr.bits.UNSTKERR) {
                rt_kprintf(print_info[PRINT_BFSR_UNSTKERR]);
            }
            if (regs.bfsr.bits.STKERR) {
                rt_kprintf(print_info[PRINT_BFSR_STKERR]);
            }

#if (CMB_CPU_PLATFORM_TYPE == CMB_CPU_ARM_CORTEX_M4) || (CMB_CPU_PLATFORM_TYPE == CMB_CPU_ARM_CORTEX_M7) || \
    (CMB_CPU_PLATFORM_TYPE == CMB_CPU_ARM_CORTEX_M33)
            if (regs.bfsr.bits.LSPERR) {
                rt_kprintf(print_info[PRINT_BFSR_LSPERR]);
            }
#endif

            if (regs.bfsr.bits.BFARVALID) {
                if (regs.bfsr.bits.PRECISERR) {
                    rt_kprintf(print_info[PRINT_BFAR], regs.bfar);
                }
            }

        }
        /* Usage Fault */
        if (regs.ufsr.value) {
            rt_kprintf("usage fault:\n");
            rt_kprintf("SCB_CFSR_UFSR:0x%02X, ", SCB_CFSR_UFSR);
            rt_kprintf("Usage fault is caused by ");

            if (regs.ufsr.bits.UNDEFINSTR) {
                rt_kprintf(print_info[PRINT_UFSR_UNDEFINSTR]);
            }
            if (regs.ufsr.bits.INVSTATE) {
                rt_kprintf(print_info[PRINT_UFSR_INVSTATE]);
            }
            if (regs.ufsr.bits.INVPC) {
                rt_kprintf(print_info[PRINT_UFSR_INVPC]);
            }
            if (regs.ufsr.bits.NOCP) {
                rt_kprintf(print_info[PRINT_UFSR_NOCP]);
            }
#if (CMB_CPU_PLATFORM_TYPE == CMB_CPU_ARM_CORTEX_M33)
            if (regs.ufsr.bits.STKOF) {
                rt_kprintf(print_info[PRINT_UFSR_STKOF]);
            }
#endif
            if (regs.ufsr.bits.UNALIGNED) {
                rt_kprintf(print_info[PRINT_UFSR_UNALIGNED]);
            }
            if (regs.ufsr.bits.DIVBYZERO0) {
                rt_kprintf(print_info[PRINT_UFSR_DIVBYZERO0]);
            }
        }
    }
    /* Debug Fault */
    if (regs.hfsr.bits.DEBUGEVT) {
        rt_kprintf("debug event:\n");
        rt_kprintf("Debug fault is caused by \n");
        if (regs.dfsr.value) {
            if (regs.dfsr.bits.HALTED) {
                rt_kprintf(print_info[PRINT_DFSR_HALTED]);
            }
            if (regs.dfsr.bits.BKPT) {
                rt_kprintf(print_info[PRINT_DFSR_BKPT]);
            }
            if (regs.dfsr.bits.DWTTRAP) {
                rt_kprintf(print_info[PRINT_DFSR_DWTTRAP]);
            }
            if (regs.dfsr.bits.VCATCH) {
                rt_kprintf(print_info[PRINT_DFSR_VCATCH]);
            }
            if (regs.dfsr.bits.EXTERNAL) {
                rt_kprintf(print_info[PRINT_DFSR_EXTERNAL]);
            }
        }
    }
}
#endif /* (CMB_CPU_PLATFORM_TYPE != CMB_CPU_ARM_CORTEX_M0) */

long list_thread_isr(void) {
    const char *item_title = "thread";
    const int maxlen = RT_NAME_MAX;
    const uint32_t THREAD_MAX = 32;

    uint32_t thread_count, count;
    osRtxThread_t *thread;
    uint8_t thread_state;
    uint32_t stack_size;
    uint32_t *ptr;
    osThreadId_t buffer[THREAD_MAX];

    // Running Thread
    thread_count = 1U;
    // Ready List
    for (thread = osRtxInfo.thread.ready.thread_list;
         thread != NULL; thread = thread->thread_next) {
        thread_count++;
    }
    // Delay List
    for (thread = osRtxInfo.thread.delay_list;
         thread != NULL; thread = thread->delay_next) {
        thread_count++;
    }
    // Wait List
    for (thread = osRtxInfo.thread.wait_list;
         thread != NULL; thread = thread->delay_next) {
        thread_count++;
    }
    if(thread_count > THREAD_MAX) {
        thread_count = THREAD_MAX;
        rt_kprintf("thread count larger than buffer size !\n");
    }

    // Running Thread
    count = 0;
    buffer[count] = osRtxInfo.thread.run.curr;
    count = 1;
    // Ready List
    for (thread = osRtxInfo.thread.ready.thread_list;
         (thread != NULL) && (count < thread_count); thread = thread->thread_next) {
        buffer[count]= thread;
        count++;
    }
    // Delay List
    for (thread = osRtxInfo.thread.delay_list;
         (thread != NULL) && (count < thread_count); thread = thread->delay_next) {
        buffer[count]= thread;
        count++;
    }
    // Wait List
    for (thread = osRtxInfo.thread.wait_list;
         (thread != NULL) && (count < thread_count); thread = thread->delay_next) {
        buffer[count]= thread;
        count++;
    }


    rt_kprintf("%-*.s pri  status      sp     stack size stack base max used left tick  error\n", maxlen, item_title);
    for(uint32_t i = 0; i < maxlen; i++) {
        rt_kprintf("-");
    }
    rt_kprintf(" ---  ------- ---------- ---------- ----------  ------  ---------- ---\n");

    for(uint32_t i = 0; i < count; i++) {
        thread = (osRtxThread_t *)buffer[i];
        thread_state = (thread->state & osRtxThreadStateMask);
        rt_kprintf("%-*.*s %3d ", maxlen, RT_NAME_MAX, thread->name, thread->priority);

        if (thread_state == osRtxThreadInactive)        rt_kprintf(" inactive  ");
        else if (thread_state == osRtxThreadReady)      rt_kprintf(" ready  ");
        else if (thread_state == osRtxThreadRunning)    rt_kprintf(" running");
        else if (thread_state == osRtxThreadBlocked)    rt_kprintf(" suspend");
        else if (thread_state == osRtxThreadTerminated) rt_kprintf(" close  ");

        // 1 Magic Word + 16 Regs
        stack_size = (thread->stack_size - (17 << 2));
        ptr = (uint32_t *)thread->stack_mem;

        for (uint32_t n = (thread->stack_size / 4U) - (16U + 1U); n != 0U; n--) {
            // skip magic word.
            ptr++;
            // check watermark
            if(osRtxStackFillPattern != *ptr) {
                break;
            }
        }

        rt_kprintf(" 0x%08x 0x%08x 0x%08x    %02d%%     %8d %s\n",
                   thread->sp, thread->stack_size,
                   (uint32_t)thread->stack_mem,
                   (stack_size- ((uint32_t)ptr - (uint32_t)thread->stack_mem)) * 100 / stack_size,
                   thread->delay,
                   rt_strerror(thread->error));
    }
    rt_kprintf("\n");
    return 0;
}

/**
 * This function set the hook, which is invoked on fault exception handling.
 *
 * @param exception_handle the exception handling hook function.
 */
void rt_hw_exception_install(rt_err_t (*exception_handle)(void *context))
{
    rt_exception_hook = exception_handle;
}

struct exception_info
{
    rt_uint32_t exc_return;
    struct stack_frame stack_frame;
};

void rt_hw_hard_fault_exception(struct exception_info *exception_info)
{
    struct exception_stack_frame *exception_stack = &exception_info->stack_frame.exception_stack_frame;
    struct stack_frame *context = &exception_info->stack_frame;
    const char *regs_name[] = {
            "R0 ", "R1 ", "R2 ", "R3 ",
            "R4 ","R5 ","R6 ","R7 ",
            "R8 ","R9 ","R10","R11",
            "R12", "LR ", "PC ", "PSR"
    };

    if (rt_exception_hook != RT_NULL)
    {
        rt_err_t result;

        result = rt_exception_hook(exception_stack);
        if (result == RT_EOK) return;
    }

    if (exception_info->exc_return & (1 << 2))
    {
        osRtxThread_t *tid = osRtxInfo.thread.run.curr;

        rt_kprintf("hard fault on thread: \"%s\"\n", tid->name);

        list_thread_isr();
    }
    else
    {
        rt_kprintf("hard fault on handler\r\n\r\n");
    }

    rt_kprintf("=================== Registers information ====================\n");

    rt_kprintf("  %s: %08x  %s: %08x  %s: %08x  %s: %08x\n", regs_name[0], context->exception_stack_frame.r0,
                                                             regs_name[1], context->exception_stack_frame.r1,
                                                             regs_name[2], context->exception_stack_frame.r2,
                                                             regs_name[3], context->exception_stack_frame.r3);
    rt_kprintf("  %s: %08x  %s: %08x  %s: %08x  %s: %08x\n", regs_name[4], context->r4,
                                                             regs_name[5], context->r5,
                                                             regs_name[6], context->r6,
                                                             regs_name[7], context->r7);
    rt_kprintf("  %s: %08x  %s: %08x  %s: %08x  %s: %08x\n", regs_name[8], context->r8,
                                                             regs_name[9], context->r9,
                                                             regs_name[10], context->r10,
                                                             regs_name[11], context->r11);
    rt_kprintf("  %s: %08x  %s: %08x  %s: %08x  %s: %08x\n", regs_name[12], context->exception_stack_frame.r12,
                                                             regs_name[13], context->exception_stack_frame.lr,
                                                             regs_name[14], context->exception_stack_frame.pc,
                                                             regs_name[15], context->exception_stack_frame.psr);
    rt_kprintf("==============================================================\n\n");


    if ( (exception_info->exc_return & 0x10) == 0)
    {
        rt_kprintf("FPU is active!\r\n");
    }

#if (CMB_CPU_PLATFORM_TYPE != CMB_CPU_ARM_CORTEX_M0)
    regs.syshndctrl.value = CMB_SYSHND_CTRL;  // System Handler Control and State Register
    regs.mfsr.value       = CMB_NVIC_MFSR;    // Memory Fault Status Register
    regs.mmar             = CMB_NVIC_MMAR;    // Memory Management Fault Address Register
    regs.bfsr.value       = CMB_NVIC_BFSR;    // Bus Fault Status Register
    regs.bfar             = CMB_NVIC_BFAR;    // Bus Fault Manage Address Register
    regs.ufsr.value       = CMB_NVIC_UFSR;    // Usage Fault Status Register
    regs.hfsr.value       = CMB_NVIC_HFSR;    // Hard Fault Status Register
    regs.dfsr.value       = CMB_NVIC_DFSR;    // Debug Fault Status Register
    regs.afsr             = CMB_NVIC_AFSR;    // Auxiliary Fault Status Register

    fault_diagnosis();
#endif
#if DEBUG_FAULT_RESET
    rt_kprintf("\nSystem will reboot after 1 second.\n\n");

    __disable_irq();
    dwt_delay_ms(1000);
    rt_hw_cpu_reset();
#else
    while(1);
#endif
}

/**
 * shutdown CPU
 */
void rt_hw_cpu_shutdown(void)
{
    rt_kprintf("shutdown...\n");

    RT_ASSERT(0);
}

/**
 * reset CPU
 */
void rt_hw_cpu_reset(void)
{
    __DSB();

    SCB_AIRCR  = SCB_RESET_VALUE;// ((0x5FAUL << SCB_AIRCR_VECTKEY_Pos) |SCB_AIRCR_SYSRESETREQ_Msk);

    __DSB();

    while(1);
}

void machine_reset(void) {
    __disable_irq();
    rt_hw_cpu_reset();
}
#ifdef RT_USING_FINSH
    MSH_CMD_EXPORT_ALIAS(machine_reset, reboot, reboot system);
#endif
