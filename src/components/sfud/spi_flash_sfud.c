/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2016-09-28     armink       first version.
 */

#include <stdint.h>
#include <string.h>
#include <rtthread.h>
#include "cmsis_os2.h"
#include <rtdevice.h>
#include "rt_spi.h"
#include "spi_flash.h"
#include "spi_flash_sfud.h"
#include "sfud.h"
#include "../../bsp/bspconfig.h"
#include "../../bsp/driver/drv_spi.h"

#ifdef RT_USING_SFUD

#ifndef RT_SFUD_DEFAULT_SPI_CFG
/* read the JEDEC SFDP command must run at 50 MHz or less */
#define RT_SFUD_DEFAULT_SPI_CFG                  \
{                                                \
    .mode = (RT_SPI_MODE_0 | RT_SPI_MASTER | RT_SPI_MSB),          \
    .data_width = 8,                             \
    .max_hz = FLASH_SPI_SPEED,                   \
}
#endif /* RT_SFUD_DEFAULT_SPI_CFG */

static rt_err_t rt_sfud_control(rt_device_t dev, int cmd, void *args) {
    RT_ASSERT(dev);

    switch (cmd) {
        case RT_DEVICE_CTRL_BLK_GETGEOME: {
            struct rt_device_blk_geometry *geometry = (struct rt_device_blk_geometry *) args;
            struct spi_flash_device *rtt_dev = (struct spi_flash_device *) (dev->user_data);

            if (rtt_dev == RT_NULL || geometry == RT_NULL) {
                return -RT_ERROR;
            }

            geometry->bytes_per_sector = rtt_dev->geometry.bytes_per_sector;
            geometry->sector_count = rtt_dev->geometry.sector_count;
            geometry->block_size = rtt_dev->geometry.block_size;
            break;
        }
        case RT_DEVICE_CTRL_BLK_ERASE: {
            rt_uint32_t *addrs = (rt_uint32_t *) args, start_addr = addrs[0], end_addr = addrs[1], phy_start_addr;
            struct spi_flash_device *rtt_dev = (struct spi_flash_device *) (dev->user_data);
            sfud_flash *sfud_dev = (sfud_flash *) (rtt_dev->user_data);
            rt_size_t phy_size;

            if (addrs == RT_NULL || start_addr > end_addr || rtt_dev == RT_NULL || sfud_dev == RT_NULL) {
                return -RT_ERROR;
            }

            if (end_addr == start_addr) {
                end_addr ++;
            }

            phy_start_addr = start_addr * rtt_dev->geometry.bytes_per_sector;
            phy_size = (end_addr - start_addr) * rtt_dev->geometry.bytes_per_sector;

            if (sfud_erase(sfud_dev, phy_start_addr, phy_size) != SFUD_SUCCESS) {
                return -RT_ERROR;
            }
            break;
        }
    }

    return RT_EOK;
}


static rt_size_t rt_sfud_read(rt_device_t dev, rt_off_t pos, void* buffer, rt_size_t size) {
    struct spi_flash_device *rtt_dev = (struct spi_flash_device *) (dev->user_data);
    sfud_flash *sfud_dev = (sfud_flash *) (rtt_dev->user_data);

    RT_ASSERT(dev);
    RT_ASSERT(rtt_dev);
    RT_ASSERT(sfud_dev);
    /* change the block device's logic address to physical address */
    rt_off_t phy_pos = pos * rtt_dev->geometry.bytes_per_sector;
    rt_size_t phy_size = size * rtt_dev->geometry.bytes_per_sector;

    if (sfud_read(sfud_dev, phy_pos, phy_size, buffer) != SFUD_SUCCESS) {
        return 0;
    } else {
        return size;
    }
}

static rt_size_t rt_sfud_write(rt_device_t dev, rt_off_t pos, const void* buffer, rt_size_t size) {
    struct spi_flash_device *rtt_dev = (struct spi_flash_device *) (dev->user_data);
    sfud_flash *sfud_dev = (sfud_flash *) (rtt_dev->user_data);

    RT_ASSERT(dev);
    RT_ASSERT(rtt_dev);
    RT_ASSERT(sfud_dev);
    /* change the block device's logic address to physical address */
    rt_off_t phy_pos = pos * rtt_dev->geometry.bytes_per_sector;
    rt_size_t phy_size = size * rtt_dev->geometry.bytes_per_sector;

    if (sfud_erase_write(sfud_dev, phy_pos, phy_size, buffer) != SFUD_SUCCESS) {
        return 0;
    } else {
        return size;
    }
}

/**
 * SPI write data then read data
 */
static sfud_err spi_write_read(const sfud_spi *spi, const uint8_t *write_buf, size_t write_size, uint8_t *read_buf,
        size_t read_size) {
    sfud_err result = SFUD_SUCCESS;
    sfud_flash *sfud_dev = (sfud_flash *) (spi->user_data);
    struct spi_flash_device *rtt_dev = (struct spi_flash_device *) (sfud_dev->user_data);

    RT_ASSERT(spi);
    RT_ASSERT(sfud_dev);
    RT_ASSERT(rtt_dev);

    if (write_size) {
        RT_ASSERT(write_buf);
    }
    if (read_size) {
        RT_ASSERT(read_buf);
    }

    if (write_size && read_size) {
        if (rt_spi_send_then_recv(rtt_dev->rt_spi_device, write_buf, write_size, read_buf, read_size) != RT_EOK) {
            result = SFUD_ERR_TIMEOUT;
        }
    } else if (write_size) {
        if (rt_spi_send(rtt_dev->rt_spi_device, write_buf, write_size) <= 0) {
            result = SFUD_ERR_TIMEOUT;
        }
    } else {
        if (rt_spi_recv(rtt_dev->rt_spi_device, read_buf, read_size) <= 0) {
            result = SFUD_ERR_TIMEOUT;
        }
    }


    return result;
}

static void spi_lock(const sfud_spi *spi) {
    sfud_flash *sfud_dev = (sfud_flash *) (spi->user_data);
    struct spi_flash_device *rtt_dev = (struct spi_flash_device *) (sfud_dev->user_data);

    RT_ASSERT(spi);
    RT_ASSERT(sfud_dev);
    RT_ASSERT(rtt_dev);

    osMutexAcquire(rtt_dev->lock, osWaitForever);
}

static void spi_unlock(const sfud_spi *spi) {
    sfud_flash *sfud_dev = (sfud_flash *) (spi->user_data);
    struct spi_flash_device *rtt_dev = (struct spi_flash_device *) (sfud_dev->user_data);

    RT_ASSERT(spi);
    RT_ASSERT(sfud_dev);
    RT_ASSERT(rtt_dev);

    osMutexRelease(rtt_dev->lock);
}

static void retry_delay_1ms(void) {
    osDelay(1);
}

sfud_err sfud_spi_port_init(sfud_flash *flash) {
    sfud_err result = SFUD_SUCCESS;

    RT_ASSERT(flash);

    /* port SPI device interface */
    flash->spi.wr = spi_write_read;
    flash->spi.lock = spi_lock;
    flash->spi.unlock = spi_unlock;
    flash->spi.user_data = flash;
    /* 1 millisecond delay */
    flash->retry.delay = retry_delay_1ms;
    /* 60 seconds timeout */
    flash->retry.times = (60 * 1000);

    return result;
}

#ifdef RT_USING_DEVICE_OPS
const static struct rt_device_ops flash_device_ops =
{
    RT_NULL,
    RT_NULL,
    RT_NULL,
    rt_sfud_read,
    rt_sfud_write,
    rt_sfud_control
};
#endif

/**
 * Probe SPI flash by SFUD (Serial Flash Universal Driver) driver library and though SPI device by specified configuration.
 *
 * @param spi_flash_dev_name the name which will create SPI flash device, len <= 15 chars
 * @param spi_dev_name using SPI device name, len <= 15 chars
 * @param spi_cfg SPI device configuration
 * @param qspi_cfg QSPI device configuration
 *
 * @return probed SPI flash device, probe failed will return RT_NULL
 */

static struct spi_flash_device spi_flash_device;
static sfud_flash sfud_dev;
static char spi_flash_dev_name_bak[RT_NAME_MAX];
static char spi_dev_name_bak[RT_NAME_MAX];

rt_spi_flash_device_t rt_sfud_flash_probe_ex(const char *spi_flash_dev_name, const char *spi_dev_name,
        struct rt_spi_configuration *spi_cfg) {

    extern sfud_err sfud_device_init(sfud_flash *flash);

    RT_ASSERT(spi_flash_dev_name);
    RT_ASSERT(spi_dev_name);

    rt_memset(&spi_flash_device, 0, sizeof(struct spi_flash_device));
    /* initialize lock */
    const osMutexAttr_t sfud_mutex_attr = {
        .name = "sfud_mutex",
        .attr_bits = (osMutexRecursive|osMutexPrioInherit|osMutexRobust),
        .cb_mem = NULL,
        .cb_size = 0U
    };
    spi_flash_device.lock = osMutexNew(&sfud_mutex_attr);

    rt_memset(&sfud_dev, 0, sizeof(sfud_flash));
    rt_memset(spi_flash_dev_name_bak, 0, RT_NAME_MAX);
    rt_memset(spi_dev_name_bak, 0, RT_NAME_MAX);

    rt_strncpy(spi_flash_dev_name_bak, spi_flash_dev_name, rt_strlen(spi_flash_dev_name));
    rt_strncpy(spi_dev_name_bak, spi_dev_name, rt_strlen(spi_dev_name));

    /* SPI configure */
    {
        /* RT-Thread SPI device initialize */
        spi_flash_device.rt_spi_device = (struct rt_spi_device *) rt_device_find(spi_dev_name);
        if (spi_flash_device.rt_spi_device == RT_NULL || spi_flash_device.rt_spi_device->parent.type != RT_Device_Class_SPIDevice) {
            LOG_E("ERROR: SPI device %s not found!", spi_dev_name);
            goto error;
        }
        sfud_dev.spi.name = spi_dev_name_bak;
        // add rt_spi_device reference count
        rt_device_open((rt_device_t)spi_flash_device.rt_spi_device, RT_DEVICE_FLAG_RDWR);
        rt_spi_configure(spi_flash_device.rt_spi_device, spi_cfg);
    }

    /* SFUD flash device initialize */
    {
        sfud_dev.name = spi_flash_dev_name_bak;
        /* accessed each other */
        spi_flash_device.user_data = &sfud_dev;
        spi_flash_device.rt_spi_device->user_data = &spi_flash_device;
        spi_flash_device.flash_device.user_data = &spi_flash_device;
        sfud_dev.user_data = &spi_flash_device;
        /* initialize SFUD device */
        if (sfud_device_init(&sfud_dev) != SFUD_SUCCESS) {
            LOG_E("ERROR: SPI flash probe failed by SPI device %s.", spi_dev_name);
            goto error;
        }
        /* when initialize success, then copy SFUD flash device's geometry to RT-Thread SPI flash device */
        spi_flash_device.geometry.sector_count = sfud_dev.chip.capacity / sfud_dev.chip.erase_gran;
        spi_flash_device.geometry.bytes_per_sector = sfud_dev.chip.erase_gran;
        spi_flash_device.geometry.block_size = sfud_dev.chip.erase_gran;
    }

    /* register device */
    spi_flash_device.flash_device.type = RT_Device_Class_Block;
#ifdef RT_USING_DEVICE_OPS
    rtt_dev->flash_device.ops  = &flash_device_ops;
#else
    spi_flash_device.flash_device.init = RT_NULL;
    spi_flash_device.flash_device.open = RT_NULL;
    spi_flash_device.flash_device.close = RT_NULL;
    spi_flash_device.flash_device.read = rt_sfud_read;
    spi_flash_device.flash_device.write = rt_sfud_write;
    spi_flash_device.flash_device.control = rt_sfud_control;
#endif

    rt_device_register(&(spi_flash_device.flash_device), spi_flash_dev_name, RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_STANDALONE);

    LOG_I("Probe SPI flash %s by SPI device %s success.",spi_flash_dev_name, spi_dev_name);
    return &spi_flash_device;

error:
    osMutexDelete(spi_flash_device.lock);

    return RT_NULL;
}

/**
 * Probe SPI flash by SFUD(Serial Flash Universal Driver) driver library and though SPI device.
 *
 * @param spi_flash_dev_name the name which will create SPI flash device
 * @param spi_dev_name using SPI device name
 *
 * @return probed SPI flash device, probe failed will return RT_NULL
 */
rt_spi_flash_device_t rt_sfud_flash_probe(const char *spi_flash_dev_name, const char *spi_dev_name)
{
    struct rt_spi_configuration cfg = RT_SFUD_DEFAULT_SPI_CFG;

    return rt_sfud_flash_probe_ex(spi_flash_dev_name, spi_dev_name, &cfg);
}

/**
 * Delete SPI flash device
 *
 * @param spi_flash_dev SPI flash device
 *
 * @return the operation status, RT_EOK on successful
 */
rt_err_t rt_sfud_flash_delete(rt_spi_flash_device_t spi_flash_dev) {
    sfud_flash *sfud_flash_dev = (sfud_flash *) (spi_flash_dev->user_data);

    RT_ASSERT(spi_flash_dev);
    RT_ASSERT(sfud_flash_dev);

    rt_device_unregister(&(spi_flash_dev->flash_device));
    osMutexDelete(spi_flash_device.lock);

    return RT_EOK;
}

sfud_flash_t rt_sfud_flash_find(const char *spi_dev_name)
{
    rt_spi_flash_device_t  rtt_dev       = RT_NULL;
    struct rt_spi_device  *rt_spi_device = RT_NULL;
    sfud_flash_t           sfud_dev2      = RT_NULL;

    rt_spi_device = (struct rt_spi_device *) rt_device_find(spi_dev_name);
    if (rt_spi_device == RT_NULL || rt_spi_device->parent.type != RT_Device_Class_SPIDevice) {
        LOG_E("ERROR: SPI device %s not found!", spi_dev_name);
        goto __error;
    }

    rtt_dev = (rt_spi_flash_device_t) (rt_spi_device->user_data);
    if (rtt_dev && rtt_dev->user_data) {
        sfud_dev2 = (sfud_flash_t) (rtt_dev->user_data);
        return sfud_dev2;
    } else {
        LOG_E("ERROR: SFUD flash device not found!");
        goto __error;
    }

__error:
    return RT_NULL;
}

sfud_flash_t rt_sfud_flash_find_by_dev_name(const char *flash_dev_name)
{
    rt_spi_flash_device_t  rtt_dev       = RT_NULL;
    sfud_flash_t           sfud_dev2      = RT_NULL;

    rtt_dev = (rt_spi_flash_device_t) rt_device_find(flash_dev_name);
    if (rtt_dev == RT_NULL || rtt_dev->flash_device.type != RT_Device_Class_Block) {
        LOG_E("ERROR: Flash device %s not found!", flash_dev_name);
        goto __error;
    }

    if (rtt_dev->user_data) {
        sfud_dev2 = (sfud_flash_t) (rtt_dev->user_data);
        return sfud_dev2;
    } else {
        LOG_E("ERROR: SFUD flash device not found!");
        goto __error;
    }

__error:
    return RT_NULL;
}

#endif /* RT_USING_SFUD */
