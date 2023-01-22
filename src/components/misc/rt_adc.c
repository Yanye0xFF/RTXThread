/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-05-07     aozima       the first version
 * 2018-11-16     Ernest Chen  add finsh command and update adc function
 */

#include <rtthread.h>
#include <rtdevice.h>
#include "rt_adc.h"

#include <string.h>
#include <stdlib.h>

static rt_size_t _adc_read(rt_device_t dev, rt_off_t pos, void *buffer, rt_size_t size)
{
    rt_err_t result = RT_EOK;
    rt_size_t i;
    struct rt_adc_device *adc = (struct rt_adc_device *)dev;
    rt_uint32_t *value = (rt_uint32_t *)buffer;

    for (i = 0; i < size; i += sizeof(int))
    {
        result = adc->ops->convert(adc, pos + i, value);
        if (result != RT_EOK)
        {
            return 0;
        }
        value++;
    }

    return i;
}

static rt_err_t _adc_control(rt_device_t dev, int cmd, void *args)
{
    rt_err_t result = RT_EOK;
    rt_adc_device_t adc = (struct rt_adc_device *)dev;

    if (adc->ops->enabled == RT_NULL)
    {
        return -RT_ENOSYS;
    }
    if (cmd == RT_ADC_CMD_ENABLE)
    {
        result = adc->ops->enabled(adc, (rt_uint32_t)args, RT_TRUE);
    }
    else if (cmd == RT_ADC_CMD_DISABLE)
    {
        result = adc->ops->enabled(adc, (rt_uint32_t)args, RT_FALSE);
    }

    return result;
}

#ifdef RT_USING_DEVICE_OPS
const static struct rt_device_ops adc_ops =
{
    RT_NULL,
    RT_NULL,
    RT_NULL,
    _adc_read,
    RT_NULL,
    _adc_control,
};
#endif

rt_err_t rt_hw_adc_register(rt_adc_device_t device, const char *name, const struct rt_adc_ops *ops, const void *user_data)
{
    rt_err_t result = RT_EOK;
    RT_ASSERT(ops != RT_NULL && ops->convert != RT_NULL);

    device->parent.type = RT_Device_Class_ADC;
    device->parent.rx_indicate = RT_NULL;
    device->parent.tx_complete = RT_NULL;

#ifdef RT_USING_DEVICE_OPS
    device->parent.ops         = &adc_ops;
#else
    device->parent.init        = RT_NULL;
    device->parent.open        = RT_NULL;
    device->parent.close       = RT_NULL;
    device->parent.read        = _adc_read;
    device->parent.write       = RT_NULL;
    device->parent.control     = _adc_control;
#endif
    device->ops = ops;
    device->parent.user_data = (void *)user_data;

    result = rt_device_register(&device->parent, name, RT_DEVICE_FLAG_RDWR);

    return result;
}

rt_uint32_t rt_adc_read(rt_adc_device_t dev, rt_uint16_t channel)
{
    rt_uint32_t value;

    RT_ASSERT(dev);

    dev->ops->convert(dev, channel, &value);

    return value;
}

rt_err_t rt_adc_enable(rt_adc_device_t dev, rt_uint16_t channel)
{
    rt_err_t result = RT_EOK;
    rt_uint16_t enable_mask = ((rt_uint16_t)0x1 << channel);

    RT_ASSERT(dev);

    if (dev->ops->enabled != RT_NULL)
    {
        if(dev->parent.open_flag & enable_mask) {
            // we already enable this channel.
            return RT_EOK;
        }else {
            // set enable flag.
            dev->parent.open_flag |= enable_mask;
            result = dev->ops->enabled(dev, channel, RT_TRUE);
        }
    }
    else
    {
        result = -RT_ENOSYS;
    }

    return result;
}

rt_err_t rt_adc_disable(rt_adc_device_t dev, rt_uint16_t channel)
{
    rt_err_t result = RT_EOK;
    rt_uint16_t enable_mask = ((rt_uint16_t)0x1 << channel);
    rt_uint16_t clear_mask;

    RT_ASSERT(dev);
    if (dev->ops->enabled != RT_NULL)
    {
        if(dev->parent.open_flag & enable_mask) {
            result = dev->ops->enabled(dev, channel, RT_FALSE);
            // clear enable flag.
            clear_mask = (~enable_mask);
            dev->parent.open_flag &= clear_mask;
        }else {
            return RT_EOK;
        }
    }
    else
    {
        result = -RT_ENOSYS;
    }

    return result;
}
