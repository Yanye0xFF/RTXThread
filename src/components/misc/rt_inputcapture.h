/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019-08-13     balanceTWK   first version.
 */

#ifndef __RT_INPUT_CAPTURE_H__
#define __RT_INPUT_CAPTURE_H__

#include <rtthread.h>
#include <rtdevice.h>
#include <stdint.h>
#include "ringbuffer.h"

#ifdef __cplusplus
extern "C" {
#endif

/* capture control command */
#define INPUTCAPTURE_CMD_CLEAR_BUF          0
#define INPUTCAPTURE_CMD_SET_WATERMARK      1
#define INPUTCAPTURE_CMD_GET_WATERMARK      2
#define INPUTCAPTURE_CMD_SET_PRESCALER      3
#define INPUTCAPTURE_CMD_GET_BLOCK_COUNT    4

#define RT_INPUT_CAPTURE_RB_SIZE             96

struct rt_inputcapture_data {
    // positive plus width, unit: timer tick
    uint32_t ppw_tick;

    // duty plus with, unit: timer tick
    uint32_t duty_tick;

    // event time, 400us/LSB
    uint32_t event_time;
};

struct rt_inputcapture_device {
    struct rt_device parent;

    const struct rt_inputcapture_ops *ops;
    rt_uint32_t clock_prescaler;

    struct rt_ringbuffer *ringbuff;

    rt_size_t watermark;
};

/**
 * capture operators
 */
struct rt_inputcapture_ops {
    rt_err_t (*init)(struct rt_inputcapture_device *inputcapture);
    rt_err_t (*open)(struct rt_inputcapture_device *inputcapture);
    rt_err_t (*close)(struct rt_inputcapture_device *inputcapture);
    rt_err_t (*get_pulsewidth)(struct rt_inputcapture_device *inputcapture, struct rt_inputcapture_data *data);
};

void rt_hw_inputcapture_isr(struct rt_inputcapture_device *inputcapture);

rt_err_t rt_device_inputcapture_register(struct rt_inputcapture_device *inputcapture, const char *name, void *data);

#ifdef __cplusplus
}
#endif

#endif /* __RT_INPUT_CAPTURE_H__ */
