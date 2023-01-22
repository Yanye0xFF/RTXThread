/*
 * Copyright (c) 2006-2022, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2006-04-30     Bernard      first implementation
 * 2006-05-04     Bernard      add list_thread,
 *                                 list_sem,
 *                                 list_timer
 * 2006-05-20     Bernard      add list_mutex,
 *                                 list_mailbox,
 *                                 list_msgqueue,
 *                                 list_event,
 *                                 list_fevent,
 *                                 list_mempool
 * 2006-06-03     Bernard      display stack information in list_thread
 * 2006-08-10     Bernard      change version to invoke rt_show_version
 * 2008-09-10     Bernard      update the list function for finsh syscall
 *                                 list and sysvar list
 * 2009-05-30     Bernard      add list_device
 * 2010-04-21     yi.qiu       add list_module
 * 2012-04-29     goprife      improve the command line auto-complete feature.
 * 2012-06-02     lgnq         add list_memheap
 * 2012-10-22     Bernard      add MS VC++ patch.
 * 2016-06-02     armink       beautify the list_thread command
 * 2018-11-22     Jesven       list_thread add smp support
 * 2018-12-27     Jesven       Fix the problem that disable interrupt too long in list_thread
 *                             Provide protection for the "first layer of objects" when list_*
 * 2020-04-07     chenhui      add clear
 * 2022-07-02     Stanley Lwin add list command
 * 2023-01-02     yanye        add rtx5 wrapper
 */

#include <rthw.h>
#include <rtthread.h>
#include <string.h>
#include "cmsis_os2.h"
#include "rtx_os.h"
#include "core_cm4.h"
#include "gd32f30x.h"

#ifdef RT_USING_FINSH
#include <finsh.h>

#define LIST_FIND_OBJ_NR 8
#define LIST_THREAD_NR   32
static const int maxlen = RT_NAME_MAX;

static long clear(void)
{
    rt_kprintf("\x1b[2J\x1b[H");

    return 0;
}
MSH_CMD_EXPORT(clear, clear the terminal screen);

extern void rt_show_version(void);
long version(void)
{
    rt_show_version();

    return 0;
}
MSH_CMD_EXPORT(version, show RTXThread version information);

rt_inline void object_split(int len)
{
    while (len--) rt_kprintf("-");
}

typedef struct
{
    rt_list_t *list;
    rt_list_t **array;
    rt_uint8_t type;
    int nr;             /* input: max nr, can't be 0 */
    int nr_out;         /* out: got nr */
} list_get_next_t;

static void list_find_init(list_get_next_t *p, rt_uint8_t type, rt_list_t **array, int nr)
{
    struct rt_object_information *info;
    rt_list_t *list;

    info = rt_object_get_information((enum rt_object_class_type)type);
    list = &info->object_list;

    p->list = list;
    p->type = type;
    p->array = array;
    p->nr = nr;
    p->nr_out = 0;
}

static rt_list_t *list_get_next(rt_list_t *current, list_get_next_t *arg)
{
    int first_flag = 0;
    rt_list_t *node, *list;
    rt_list_t **array;
    int nr;

    arg->nr_out = 0;

    if (!arg->nr || !arg->type)
    {
        return (rt_list_t *)RT_NULL;
    }

    list = arg->list;

    if (!current) /* find first */
    {
        node = list;
        first_flag = 1;
    }
    else
    {
        node = current;
    }


    if (!first_flag)
    {
        struct rt_object *obj;
        /* The node in the list? */
        obj = rt_list_entry(node, struct rt_object, list);
        if ((obj->type & ~RT_Object_Class_Static) != arg->type)
        {
            return (rt_list_t *)RT_NULL;
        }
    }

    nr = 0;
    array = arg->array;
    while (1)
    {
        node = node->next;

        if (node == list)
        {
            node = (rt_list_t *)RT_NULL;
            break;
        }
        nr++;
        *array++ = node;
        if (nr == arg->nr)
        {
            break;
        }
    }

    arg->nr_out = nr;
    return node;
}

long list_thread(void) {
    const char *item_title = "thread";

    uint32_t thread_count, enum_count;
    osRtxThread_t *thread;
    uint8_t thread_state;
    uint32_t stack_size;
    uint32_t *ptr;
    osThreadId_t buffer[LIST_THREAD_NR];

    thread_count = osThreadGetCount();

    if(thread_count > LIST_THREAD_NR) {
        thread_count = LIST_THREAD_NR;
        rt_kprintf("thread count larger than buffer size !\n");
    }

    enum_count = osThreadEnumerate(buffer, thread_count);

    rt_kprintf("%-*.s pri  status      sp     stack size stack base max used left tick  error\n", maxlen, item_title);
    object_split(maxlen);
    rt_kprintf(" ---  ------- ---------- ---------- ----------  ------  ---------- ---\n");

    for(uint32_t i = 0; i < enum_count; i++) {
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

    return 0;
}
MSH_CMD_EXPORT(list_thread, list thread);

long list_timer(void) {
    const char *item_title = "timer";
    osRtxTimer_t  *timer = osRtxInfo.timer.list;

    if (timer == NULL) {
        rt_kprintf("no soft timer.\n");
        return 0;
    }

    rt_kprintf("%-*.s  periodic   timeout    activated     mode\n", maxlen, item_title);
    object_split(maxlen);
    rt_kprintf(" ---------- ---------- ----------- ---------\n");

    do{
        rt_kprintf("%-*.*s   %8d   %8d ",
                  maxlen, RT_NAME_MAX,
                  timer->name, timer->load, timer->tick);

        if (timer->state == osRtxTimerInactive) {
            rt_kprintf("Inactive    ");
        }else if(timer->state == osRtxTimerStopped) {
            rt_kprintf("Stopped     ");
        }else if(timer->state == osRtxTimerRunning) {
            rt_kprintf("Running     ");
        }else {
            rt_kprintf("Unknown     ");
        }

        if (timer->type == osTimerOnce) {
            rt_kprintf("one shot\n");
        }else {
            rt_kprintf("periodic\n");
        }

        timer = timer->next;
    }while(timer != NULL);

    return 0;
}
MSH_CMD_EXPORT(list_timer, list timer in system);

// startup_gd32f30x_hd.S
extern void Default_Handler(void);
// gd32f30x_flash.ld
extern unsigned int __link_addr;

// 15 arm core exception + 60 periph interrupt
#define PERIPH_INTERRUPT_NUMS    60
// some core exception not defined in arm cortex-m4
#define CORE_EXCEPTION_NUMS      10

long list_isr(void) {
    const char *item_title = "interrupt";
    const uint8_t core_exception_offset[] = {4, 4, 4, 4, 4, 4, 20, 4, 8, 4};
    const int8_t core_exception_num[] = {0, 0, 0, -12, -11, -10, -5, -4, -2, -1};
    const char * const exception_name[] = {
        "Reset", "NMI", "HardFault", "MemManage", "BusFault",
        "UsageFault", "SVC", "DebugMon", "PendSV", "SysTick"
    };

    const char * const interrupt_name[] = {
        "WWDGT","LVD","TAMPER","RTC","FMC",
        "RCU","EXTI0","EXTI1","EXTI2","EXTI3",
        "EXTI4","DMA0_CH0","DMA0_CH1","DMA0_CH2","DMA0_CH3",
        "DMA0_CH4","DMA0_CH5","DMA0_CH6","ADC0_1","USBD_HP",
        "USBD_LP","CAN0_RX1","CAN0_EWMC","EXTI5_9","TIMER0_BRK",
        "TIMER0_UP","TIMER0_TRG","TIMER0_Ch","TIMER1","TIMER2",
        "TIMER3","I2C0_EV","I2C0_ER","I2C1_EV","I2C1_ER",
        "SPI0","SPI1","USART0","USART1","USART2",
        "EXTI10_15","RTC_Alarm","USBD_WKUP","TIMER7_BRK","TIMER7_UP",
        "TIMER7_TRG","TIMER7_Ch","ADC2","EXMC","SDIO",
        "TIMER4","SPI2","UART3","UART4","TIMER5",
        "TIMER6","DMA1_CH0","DMA1_CH1","DMA1_CH2","DMA1_CH3"
    };

    uint32_t grouping = NVIC_GetPriorityGrouping();
    uint32_t pre_bit = (7u - grouping);
    uint32_t sub_bit = (grouping - 3u);
    rt_kprintf("%d bits for pre-emption priority %d bits for subpriority.\n", pre_bit, sub_bit);

    uint32_t addr = (uint32_t)&__link_addr;
    const uint32_t addr_default = (const uint32_t)Default_Handler;
    uint32_t func_ptr;
    uint32_t priority;

    rt_kprintf("link address:0x%08x\n", addr);
    rt_kprintf("default handler:0x%08x\n",addr_default);

    rt_kprintf("%-*.s  pri   handler\n", maxlen, item_title);
    object_split(maxlen);
    rt_kprintf(" ---- ----------\n");

    for(size_t i = 0; i < CORE_EXCEPTION_NUMS; i++) {
        addr += core_exception_offset[i];
        func_ptr = REG32(addr) & 0xFFFFFFFEu;

        if(core_exception_num[i] == 0) {
            rt_kprintf("%-*.*s    - ",
                          maxlen, RT_NAME_MAX, exception_name[i]);

        }else {
            priority = NVIC_GetPriority((IRQn_Type)core_exception_num[i]);
            rt_kprintf("%-*.*s %4d ",
                         maxlen, RT_NAME_MAX,
                         exception_name[i], priority);

        }
        if(func_ptr == addr_default) {
            rt_kprintf("   default\n");
        }else {
            rt_kprintf("0x%08x\n", func_ptr);
        }
    }

    object_split(maxlen);
    rt_kprintf("----------------\n");
    // periph interrupt
    addr += sizeof(size_t);

    for(size_t i = 0; i < PERIPH_INTERRUPT_NUMS; i++) {
        func_ptr = REG32(addr) & 0xFFFFFFFEu;
        priority = NVIC_GetPriority((IRQn_Type)i);

        rt_kprintf("%-*.*s %4d ", maxlen, RT_NAME_MAX, interrupt_name[i], priority);
        if(func_ptr == addr_default) {
            rt_kprintf("   default\n");
        }else {
            rt_kprintf("0x%08x\n", func_ptr);
        }
        addr += sizeof(size_t);
    }


    return 0;
}
MSH_CMD_EXPORT(list_isr, list interrupt handlers);

#if (USE_MAIN_STACK_WATERMARK)
// gd32f30x_flash.ld
extern unsigned int _sstack;
extern unsigned int _estack;
extern unsigned int _system_stack_size;

long show_mstack(void) {
    uint32_t *msp_bottom = (uint32_t *)&_sstack;
    uint32_t *msp_top = (uint32_t *)&_estack;
    uint32_t stack_size = (uint32_t)&_system_stack_size;
    uint32_t *msp_ptr = msp_bottom;

    while(msp_ptr < msp_top) {
        // watermark set by /libcpu/startup_gd32f30x_hd.S
        if(*msp_ptr != 0xCCCCCCCC) {
            break;
        }
        msp_ptr++;
    }

    uint32_t percent = ((uint32_t)msp_top - (uint32_t)msp_ptr) * 100 / stack_size;

    rt_kprintf("main stack info:\n");
    rt_kprintf("stack size : %d Bytes\n", stack_size);
    rt_kprintf("stack base : 0x%08x\n", (uint32_t)msp_bottom);
    rt_kprintf("stack top  : 0x%08x\n", (uint32_t)msp_top);
    rt_kprintf("watermark  : 0x%08x\n", (uint32_t)msp_ptr);
    rt_kprintf("max used   : %d%%\n", percent);

    return 0;
}
MSH_CMD_EXPORT(show_mstack, show main stack usage);
#endif

#ifdef RT_USING_DEVICE
static char *const device_type_str[RT_Device_Class_Unknown] =
{
    "Character Device",
    "Block Device",
    "Network Interface",
    "MTD Device",
    "CAN Device",
    "RTC",
    "Sound Device",
    "Graphic Device",
    "I2C Bus",
    "USB Slave Device",
    "USB Host Bus",
    "USB OTG Bus",
    "SPI Bus",
    "SPI Device",
    "SDIO Bus",
    "PM Pseudo Device",
    "Pipe",
    "Portal Device",
    "Timer Device",
    "Miscellaneous Device",
    "Sensor Device",
    "Touch Device",
    "Phy Device",
    "Security Device",
    "WLAN Device",
    "Pin Device",
    "ADC Device",
    "DAC Device",
    "WDT Device",
    "PWM Device",
};

long list_device(void)
{
    list_get_next_t find_arg;
    rt_list_t *obj_list[LIST_FIND_OBJ_NR];
    rt_list_t *next = (rt_list_t *)RT_NULL;
    const char *device_type;

    const char *item_title = "device";

    list_find_init(&find_arg, RT_Object_Class_Device, obj_list, sizeof(obj_list) / sizeof(obj_list[0]));


    rt_kprintf("%-*.s         type         ref count\n", maxlen, item_title);
    object_split(maxlen);
    rt_kprintf(" -------------------- ----------\n");
    do
    {
        next = list_get_next(next, &find_arg);
        {
            int i;
            for (i = 0; i < find_arg.nr_out; i++)
            {
                struct rt_object *obj;
                struct rt_device *device;

                obj = rt_list_entry(obj_list[i], struct rt_object, list);
                if ((obj->type & ~RT_Object_Class_Static) != find_arg.type)
                {
                    continue;
                }

                device = (struct rt_device *)obj;
                device_type = "Unknown";
                if (device->type < RT_Device_Class_Unknown &&
                    device_type_str[device->type] != RT_NULL)
                {
                    device_type = device_type_str[device->type];
                }
                rt_kprintf("%-*.*s %-20s %-8d\n",
                           maxlen, RT_NAME_MAX,
                           device->parent.name,
                           device_type,
                           device->ref_count);

            }
        }
    }
    while (next != (rt_list_t *)RT_NULL);

    return 0;
}
MSH_CMD_EXPORT(list_device, list device in system);
#endif

int cmd_list(int argc, char **argv)
{
    if(argc == 2)
    {
        if(strcmp(argv[1], "thread") == 0)
        {
            list_thread();
        }
        else if(strcmp(argv[1], "timer") == 0)
        {
            list_timer();
        }
        else if(strcmp(argv[1], "isr") == 0)
        {
            list_isr();
        }
#ifdef RT_USING_MEMPOOL
        else if(strcmp(argv[1], "mempool") == 0)
        {
            list_mempool();
        }
#endif /* RT_USING_MEMPOOL */
#ifdef RT_USING_DEVICE
        else if(strcmp(argv[1], "device") == 0)
        {
            list_device();
        }
#endif /* RT_USING_DEVICE */

#ifdef RT_USING_DFS
        else if(strcmp(argv[1], "fd") == 0)
        {
            extern int list_fd(void);
            list_fd();
        }
#endif /* RT_USING_DFS */
        else
        {
            goto _usage;
        }

        return 0;
    }

_usage:
    rt_kprintf("Usage: list [options]\n");
    rt_kprintf("[options]:\n");
    rt_kprintf("    thread - list threads\n");
    rt_kprintf("    timer  - list timers\n");
    rt_kprintf("    isr    - list interrupt handlers\n");
#ifdef RT_USING_DEVICE
    rt_kprintf("    device - list devices\n");
#endif /* RT_USING_DEVICE */
#ifdef RT_USING_DFS
    rt_kprintf("    fd - list file descriptors\n");
#endif /* RT_USING_DFS */

    return 0;
}
MSH_CMD_EXPORT_ALIAS(cmd_list, list, list objects);

#endif /* RT_USING_FINSH */
