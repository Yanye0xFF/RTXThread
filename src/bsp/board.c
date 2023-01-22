/*
 * Copyright (c) 2006-2019, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2017-07-24     Tanek        the first version
 * 2018-11-12     Ernest Chen  modify copyright
 */
#include <stdint.h>

#include <rthw.h>
#include <rtthread.h>
#include <rtconfig.h>
#include "bspconfig.h"

#include "system_gd32f30x.h"
#include "gd32f30x_misc.h"

#if defined(RT_USING_USER_MAIN) && defined(RT_USING_HEAP)

extern unsigned int __bss_start;
extern unsigned int __bss_end;

#define RT_HW_HEAP_BEGIN    (void*)&__bss_end
#define RT_HW_HEAP_END      (void*)(0x20000000 + 48 * 1024)

#endif

/**
 * This function will initial your board.
 */
void rt_hw_board_init(void) {
    /* System Clock Update */
    SystemCoreClockUpdate();

    nvic_priority_group_set(NVIC_PRIGROUP_PRE4_SUB0);

    /* System Tick Configuration */
    // @see ../RTOS2/Source/os_systick.c

    /* Call components board initial (use INIT_BOARD_EXPORT()) */
#ifdef RT_USING_COMPONENTS_INIT
    rt_components_board_init();
#endif

#if defined(RT_USING_USER_MAIN) && defined(RT_USING_HEAP)
    rt_system_heap_init(RT_HW_HEAP_BEGIN, RT_HW_HEAP_END);
#endif
}
