/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2016/5/20      bernard      the first version
 * 2020/1/7       redoc        add include
 */

#ifndef SPI_FLASH_H__
#define SPI_FLASH_H__

#include <rtdevice.h>
#include "cmsis_os2.h"

struct spi_flash_device
{
    struct rt_device                flash_device;
    struct rt_device_blk_geometry   geometry;
    struct rt_spi_device *          rt_spi_device;
    osMutexId_t                     lock;
    void *                          user_data;
};

typedef struct spi_flash_device *rt_spi_flash_device_t;

#endif
