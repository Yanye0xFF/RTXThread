/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 */

#include "finsh_port.h"
#include <rthw.h>
#include <rtconfig.h>
#include <rtt.h>

#ifdef RT_USING_FINSH

int rt_hw_console_getchar(void) {
    unsigned char c;
    int r;
    rt_size_t len;

    rt_device_t console;
    console = rt_console_get_device();
    if (console) {
        len = rt_device_read(console, 0, &c, 1);
        r = (len == 1) ? ((int)c) : (-1);
    }else {
        len = rtt_read(0u, &c, 1u);
        if (len == 1) {
          r = (int)c;
        } else {
          r = -1;
        }
    }
    return r;
}

#endif /* RT_USING_FINSH */

