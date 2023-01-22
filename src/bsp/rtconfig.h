/* RT-Thread config file */

#ifndef __RTTHREAD_CFG_H__
#define __RTTHREAD_CFG_H__


/* RT-Thread Kernel */
#define RT_NAME_MAX     12
#define RT_ALIGN_SIZE   4


/* kservice optimization */
#define RT_DEBUG
#define RT_DEBUG_INIT 0
//#define RT_DEBUG_COLOR


/* Memory Management */
// after rtthread 4.1.0
#define RT_USING_SMALL_MEM_AS_HEAP
#define RT_USING_SMALL_MEM
#define RT_USING_HEAP
//#define RT_KSERVICE_USING_TINY_SIZE


/* Kernel Device Object */
#define RT_USING_DEVICE
#define RT_USING_CONSOLE
#define RT_CONSOLEBUF_SIZE          128
#define RT_CONSOLE_DEVICE_NAME      "uart0"
#define RT_VER_NUM                  0x40101


/* RT-Thread Components */
#define RT_USING_COMPONENTS_INIT
#define RT_USING_USER_MAIN
#define RT_MAIN_THREAD_STACK_SIZE     2048
#define RT_MAIN_THREAD_PRIORITY       24

#define RT_USING_MSH
#define RT_USING_FINSH

#if defined(RT_USING_FINSH)

    #define FINSH_USING_MSH
    #define FINSH_USING_MSH_ONLY
    #define FINSH_THREAD_NAME "tshell"
    //#define FINSH_ECHO_DISABLE_DEFAULT

    #define FINSH_THREAD_PRIORITY       16
    #define FINSH_THREAD_STACK_SIZE     1024

    //#define FINSH_USING_HISTORY
    //#define FINSH_HISTORY_LINES         1
    #define FINSH_CMD_SIZE 80
    #define FINSH_USING_SYMTAB
    #define MSH_USING_BUILT_IN_COMMANDS
    #define FINSH_USING_DESCRIPTION
    #define FINSH_ARG_MAX 10
#endif


/* Device Drivers */

#define RT_USING_TIMER_SOFT         1
#if RT_USING_TIMER_SOFT == 0
    #undef RT_USING_TIMER_SOFT
#endif
#define RT_TIMER_THREAD_PRIO        4
#define RT_TIMER_THREAD_STACK_SIZE  512

#define RT_USING_RTC
#define RT_USING_RTT

// spi-flash with sfud driver
#define RT_USING_SFUD
#define RT_SFUD_USING_SFDP
#define RT_SFUD_USING_FLASH_INFO_TABLE

#define RT_LIBC_DEFAULT_TIMEZONE   8

// 量定义DEBUG_FAULT_RESET为1，陷入异常/未实现中断后自动重启
#define DEBUG_FAULT_RESET    1
// 启用主栈(MSP)水印
#define USE_MAIN_STACK_WATERMARK    1

#endif
