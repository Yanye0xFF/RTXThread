/*
 * Copyright (c) 2006-2022, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2009-10-11     Bernard      first version
 * 2012-01-01     aozima       support context switch load/store FPU register.
 * 2013-06-18     aozima       add restore MSP feature.
 * 2013-06-23     aozima       support lazy stack optimized.
 * 2018-07-24     aozima       enhancement hard fault exception handler.
 */

/**
 * @addtogroup cortex-m4
 */
/*@{*/

.cpu cortex-m4
.syntax unified
.thumb
.text

.global rt_hw_hard_fault_exception

.global HardFault_Handler
.type HardFault_Handler, %function
HardFault_Handler:
    /* get current context */
    MRS     r0, msp                 /* get fault context from handler. */
    TST     lr, #0x04               /* if(!EXC_RETURN[2]) */
    BEQ     _get_sp_done
    MRS     r0, psp                 /* get fault context from thread. */
_get_sp_done:
    /* xpsr pc lr r12 r3~r0 push by hardware */
    STMFD   r0!, {r4 - r11}         /* push r4 - r11 register */
#if defined (__VFP_FP__) && !defined(__SOFTFP__)
    STMFD   r0!, {lr}               /* push dummy for flag */
#endif
    STMFD   r0!, {lr}               /* push exec_return register */

    TST     lr, #0x04               /* if(!EXC_RETURN[2]) */
    BEQ     _update_msp
    MSR     psp, r0                 /* update stack pointer to PSP. */
    B       _update_done
_update_msp:
    MSR     msp, r0                 /* update stack pointer to MSP. */
_update_done:

    PUSH    {LR}
    BL      rt_hw_hard_fault_exception
    POP     {LR}

    ORR     lr, lr, #0x04
    BX      lr


