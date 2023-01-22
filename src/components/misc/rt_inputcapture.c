/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019-08-13     balanceTWK   the first version
 */

#include <rtthread.h>
#include <rtdevice.h>
#include <rtdbg.h>
#include "rt_inputcapture.h"

static rt_err_t rt_inputcapture_init(struct rt_device *dev)
{
    rt_err_t ret;
    struct rt_inputcapture_device *inputcapture;

    RT_ASSERT(dev != RT_NULL);

    ret = RT_EOK;
    inputcapture = (struct rt_inputcapture_device *)dev;
    inputcapture->watermark = (RT_INPUT_CAPTURE_RB_SIZE / 2);
    if (inputcapture->ops->init)
    {
        ret = inputcapture->ops->init(inputcapture);
    }

    return ret;
}

static rt_err_t rt_inputcapture_open(struct rt_device *dev, rt_uint16_t oflag)
{
    rt_err_t ret;
    struct rt_inputcapture_device *inputcapture;

    RT_ASSERT(dev != RT_NULL);

    ret = RT_EOK;
    inputcapture = (struct rt_inputcapture_device *)dev;
    if (inputcapture->ringbuff == RT_NULL)
    {
        inputcapture->ringbuff = rt_ringbuffer_create(sizeof(struct rt_inputcapture_data) * RT_INPUT_CAPTURE_RB_SIZE);
    }
    if (inputcapture->ops->open)
    {
        ret = inputcapture->ops->open(inputcapture);
    }

    return ret;
}

static rt_err_t rt_inputcapture_close(struct rt_device *dev)
{
    rt_err_t ret;
    struct rt_inputcapture_device *inputcapture;

    RT_ASSERT(dev != RT_NULL);

    ret = -RT_ERROR;
    inputcapture = (struct rt_inputcapture_device *)dev;

    if (inputcapture->ops->close)
    {
        ret = inputcapture->ops->close(inputcapture);
    }

    if (ret != RT_EOK)
    {
        return ret;
    }
    if (inputcapture->ringbuff)
    {
        rt_ringbuffer_destroy(inputcapture->ringbuff);
        inputcapture->ringbuff = RT_NULL;
    }
    return ret;
}

/**
 * @brief read data from ring-block buffer(use value copy to compatible with POSIX API)
 * @param dev
 * @param pos not use
 * @param buffer type of 'struct rt_inputcapture_data *'
 * @param size block nums
 * @return
 * */
static rt_size_t rt_inputcapture_read(struct rt_device *dev,
                                 rt_off_t          pos,
                                 void             *buffer,
                                 rt_size_t         size)
{
    struct rt_inputcapture_device *capture = (struct rt_inputcapture_device *)dev;

    rt_size_t receive_size = rt_ringbuffer_get(capture->ringbuff,
            (rt_uint8_t *)buffer, (size * sizeof(struct rt_inputcapture_data)));

    return (receive_size /sizeof(struct rt_inputcapture_data));
}

static rt_err_t rt_inputcapture_control(struct rt_device *dev, int cmd, void *args)
{
    rt_err_t result;
    rt_size_t data_len;
    rt_size_t *blk_count;
    struct rt_inputcapture_device *inputcapture;

    result = RT_EOK;
    inputcapture = (struct rt_inputcapture_device *)dev;

    switch (cmd) {
        case INPUTCAPTURE_CMD_CLEAR_BUF:
            if (inputcapture->ringbuff)
            {
                rt_ringbuffer_reset(inputcapture->ringbuff);
            }
            break;
        case INPUTCAPTURE_CMD_SET_WATERMARK:
            inputcapture->watermark = *(rt_size_t *)args;
            break;
        case INPUTCAPTURE_CMD_GET_WATERMARK:
            *((rt_size_t *)args) = inputcapture->watermark;
            break;
        case INPUTCAPTURE_CMD_SET_PRESCALER:
            inputcapture->clock_prescaler = *((rt_uint32_t *)args);
            inputcapture->ops->close(inputcapture);
            inputcapture->ops->open(inputcapture);
            break;
        case INPUTCAPTURE_CMD_GET_BLOCK_COUNT:
            blk_count = (rt_size_t *)args;
            data_len = rt_ringbuffer_data_len(inputcapture->ringbuff);
            *blk_count = (data_len / sizeof(struct rt_inputcapture_data));
            break;
        default:
            result = -RT_ENOSYS;
            break;
    }

    return result;
}

#ifdef RT_USING_DEVICE_OPS
const static struct rt_device_ops inputcapture_ops =
{
    rt_inputcapture_init,
    rt_inputcapture_open,
    rt_inputcapture_close,
    rt_inputcapture_read,
    RT_NULL,
    rt_inputcapture_control
};
#endif

rt_err_t rt_device_inputcapture_register(struct rt_inputcapture_device *inputcapture, const char *name, void *user_data)
{
    struct rt_device *device;

    RT_ASSERT(inputcapture != RT_NULL);
    RT_ASSERT(inputcapture->ops != RT_NULL);
    RT_ASSERT(inputcapture->ops->get_pulsewidth != RT_NULL);

    device = &(inputcapture->parent);

    device->type        = RT_Device_Class_Miscellaneous;
    device->rx_indicate = RT_NULL;
    device->tx_complete = RT_NULL;

#ifdef RT_USING_DEVICE_OPS
    device->ops         = &inputcapture_ops;
#else
    device->init        = rt_inputcapture_init;
    device->open        = rt_inputcapture_open;
    device->close       = rt_inputcapture_close;
    device->read        = rt_inputcapture_read;
    device->write       = RT_NULL;
    device->control     = rt_inputcapture_control;
#endif
    device->user_data   = user_data;

    return rt_device_register(device, name, RT_DEVICE_FLAG_RDONLY | RT_DEVICE_FLAG_STANDALONE);
}

/**
 * This function is ISR for inputcapture interrupt.
 */
void rt_hw_inputcapture_isr(struct rt_inputcapture_device *inputcapture){
    struct rt_inputcapture_data data;
    if (inputcapture->ops->get_pulsewidth(inputcapture, &data) == RT_EOK) {
        rt_ringbuffer_put(inputcapture->ringbuff, (const rt_uint8_t *)(&data), sizeof(struct rt_inputcapture_data));
    }
}
