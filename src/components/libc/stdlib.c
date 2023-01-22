/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-02-15     Meco Man     first version
 */

#include <rtthread.h>
#include <stddef.h>
#include "cmsis_os2.h"

#define DBG_TAG    "stdlib"
#define DBG_LVL    DBG_INFO
#include <rtdbg.h>

void __rt_libc_exit(int status)
{
    osThreadId_t tid = osThreadGetId();
    osRtxThread_t *self = (osRtxThread_t *)tid;

    if (tid != NULL)
    {
        LOG_E("thread:%s exit:%d!", self->name, status);
        osThreadExit();
    }
}

#ifdef RT_USING_MSH
int system(const char *command)
{
    extern int msh_exec(char *cmd, rt_size_t length);

    int ret = -RT_ENOMEM;
    char *cmd = rt_strdup(command);

    if (cmd)
    {
        ret = msh_exec(cmd, rt_strlen(cmd));
        rt_free(cmd);
    }

    return ret;
}
RTM_EXPORT(system);
#endif
