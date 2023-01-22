/*
 * spi0.c
 * @brief
 * Created on: Nov 4, 2021
 * Author: Yanye
 * Change Logs:
 * Date           Author       Notes
 * 2022-10-12     Yanye        remove rt_complete, change to fast spi implementation.
 * 2022-12-30     Yanye        remove SPI2, add SPI0, support DMA@8bit mode only.
 */
#include <driver/drv_spi.h>
#include "gd32f30x.h"
#include "gd32f30x_spi.h"
#include "gd32f30x_rcu.h"
#include "gd32f30x_gpio.h"

#include "drv_pin.h"
#include "rtthread.h"
#include "rt_spi.h"
#include "pin.h"
#include "bspconfig.h"

// private:
#define SPI0_GPIO_PORT   GPIOA
#define SPI0_SCLK_PIN    GPIO_PIN_5
#define SPI0_MISO_PIN    GPIO_PIN_6
#define SPI0_MOSI_PIN    GPIO_PIN_7

//@see drv_pin.h
// GD25Q32CSIG: 120MHz Max
#define SPI_FLASH_CS_PIN          GPIO_PB11
// LSM6DS3TR: 10MHz Max
#define SPI_GSENSOR_CS_PIN        GPIO_PB10

typedef struct _spi_info {
    enum GPIO_PIN_NUM cs_pin;
    uint32_t spi_base;
} spi_info_t;

static const uint32_t RX_DUMMY_BYTE_ADDR = 0x0803F000;

static volatile uint8_t dma_rx_done;
static volatile uint8_t dma_tx_done;

// DMA burst length: 65535 bytes max.

// SPI0_RX
void DMA0_Channel1_IRQHandler(void) {
    // receive done
    dma_interrupt_flag_clear(DMA0, DMA_CH1, DMA_INT_FLAG_FTF);
    dma_rx_done = 1;
}

// SPI0_TX
void DMA0_Channel2_IRQHandler(void) {
    // transmit done
    dma_interrupt_flag_clear(DMA0, DMA_CH2, DMA_INT_FLAG_FTF);
    dma_tx_done = 1;
}

/**
 * @brief SPI控制器初始化配置
 * @param device spi设备实例，由设备驱动层传入
 * @param configuration::mode 只支持master, mode0~mode4, msb~lsb
 * @param configuration::data_width 只支持8bit
 * */
static rt_err_t spi_configure(struct rt_spi_device *device, struct rt_spi_configuration *configuration) {
    spi_parameter_struct spi_param;
    dma_parameter_struct dma_param;

    rt_uint16_t spi_mode = (configuration->mode & 0x3); // BIT0 & BIT1
    rt_uint16_t spi_msb = (configuration->mode >> 2) & 0x1; // BIT2
    rt_uint16_t spi_slave = (configuration->mode >> 3) & 0x1; // BIT3

    if(configuration->data_width == 8) {
        spi_param.frame_size = SPI_FRAMESIZE_8BIT;
    }else {
        return -RT_EINVAL;
    }

    rcu_periph_clock_enable(RCU_SPI0);

    gpio_init(GPIOA, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, SPI0_SCLK_PIN);
    gpio_init(GPIOA, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, SPI0_MOSI_PIN);
    gpio_init(GPIOA, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, SPI0_MISO_PIN);

    // DMA clock enable @see pre_init.c
    // SPI0 RX
    dma_deinit(DMA0, DMA_CH1);
    dma_param.periph_addr  = (uint32_t)&SPI_DATA(SPI0);
    dma_param.periph_width = DMA_PERIPHERAL_WIDTH_8BIT;
    // edit 'DMA_CHxMADDR' register directly when DMA transmit
    dma_param.memory_addr  = 0;
    dma_param.memory_width = DMA_MEMORY_WIDTH_8BIT;
    // edit 'DMA_CHCNT' register directly when DMA transmit
    dma_param.number = 0;
    dma_param.priority     = DMA_PRIORITY_MEDIUM;
    dma_param.periph_inc   = DMA_PERIPH_INCREASE_DISABLE;
    dma_param.memory_inc   = DMA_MEMORY_INCREASE_ENABLE;
    dma_param.direction    = DMA_PERIPHERAL_TO_MEMORY;
    dma_init(DMA0, DMA_CH1, &dma_param);
    dma_circulation_disable(DMA0, DMA_CH1);
    dma_memory_to_memory_disable(DMA0, DMA_CH1);

    dma_interrupt_flag_clear(DMA0, DMA_CH1, DMA_INT_FLAG_FTF);
    dma_interrupt_enable(DMA0, DMA_CH1, DMA_INT_FTF);
    nvic_irq_enable(DMA0_Channel1_IRQn, SPI0_RX_DMA_PRIORITY, 0);

    // SPI0 TX
    dma_deinit(DMA0, DMA_CH2);
    dma_param.periph_addr  = (uint32_t)&SPI_DATA(SPI0);
    dma_param.periph_width = DMA_PERIPHERAL_WIDTH_8BIT;
    // edit 'DMA_CHxMADDR' register directly when DMA transmit
    dma_param.memory_addr  = 0;
    dma_param.memory_width = DMA_MEMORY_WIDTH_8BIT;
    // edit 'DMA_CHCNT' register directly when DMA receive
    dma_param.number = 0;
    dma_param.priority     = DMA_PRIORITY_MEDIUM;
    dma_param.periph_inc   = DMA_PERIPH_INCREASE_DISABLE;
    dma_param.memory_inc   = DMA_MEMORY_INCREASE_ENABLE;
    dma_param.direction    = DMA_MEMORY_TO_PERIPHERAL;
    dma_init(DMA0, DMA_CH2, &dma_param);
    dma_circulation_disable(DMA0, DMA_CH2);
    dma_memory_to_memory_disable(DMA0, DMA_CH2);

    dma_interrupt_flag_clear(DMA0, DMA_CH2, DMA_INT_FLAG_FTF);
    dma_interrupt_enable(DMA0, DMA_CH2, DMA_INT_FTF);
    nvic_irq_enable(DMA0_Channel2_IRQn, SPI0_TX_DMA_PRIORITY, 0);

    spi_disable(SPI0);

    spi_param.device_mode = (spi_slave ? SPI_SLAVE : SPI_MASTER);
    spi_param.trans_mode = SPI_TRANSMODE_FULLDUPLEX;
    spi_param.nss = SPI_NSS_SOFT;
    spi_param.endian = (spi_msb ? SPI_ENDIAN_MSB : SPI_ENDIAN_LSB);
    spi_param.prescale = CTL0_PSC(configuration->max_hz);
    spi_param.clock_polarity_phase = spi_mode;

    spi_init(SPI0, &spi_param);
    spi_enable(SPI0);

    return RT_EOK;
}

static rt_uint32_t spi_xfer(struct rt_spi_device *device, struct rt_spi_message *message) {
    spi_info_t *info = (spi_info_t *)(device->parent.user_data);

    /* take CS */
    if (message->cs_take) {
        rt_pin_write((rt_base_t)info->cs_pin, GPIO_PIN_LOW);
    }

    // disable DMA channel to update 'counter register', see below.
    DMA_CHCTL(DMA0, DMA_CH2) &= ~DMA_CHXCTL_CHEN;
    DMA_CHCTL(DMA0, DMA_CH1) &= ~DMA_CHXCTL_CHEN;

    dma_rx_done = 0;
    dma_tx_done = 0;

    // update DMA transfer count register
    DMA_CHCNT(DMA0, DMA_CH2) = message->length;
    DMA_CHCNT(DMA0, DMA_CH1) = message->length;

    if((message->send_buf != RT_NULL) && (message->recv_buf != RT_NULL)) {
        // full-duplex
        // dummy read to clear 'RBNE' flag
        *(uint8_t *)message->recv_buf = (uint8_t)SPI_DATA(info->spi_base);

        // TX DMA
        DMA_CHMADDR(DMA0, DMA_CH2) = (uint32_t)message->send_buf;
        DMA_CHCTL(DMA0, DMA_CH2) |= DMA_CHXCTL_CHEN;
        // RX DMA
        DMA_CHMADDR(DMA0, DMA_CH1) = (uint32_t)message->recv_buf;
        DMA_CHCTL(DMA0, DMA_CH1) |= DMA_CHXCTL_CHEN;

        // When the TBE or RBNE bit in SPI_STAT is set, it will
        // generate a DMA request at corresponding DMA channel.
        SPI_CTL1(info->spi_base) |= (SPI_CTL1_DMATEN | SPI_CTL1_DMAREN);

        // TODO add rt_completion

        while(!dma_tx_done) {};

        while(SPI_STAT(info->spi_base) & SPI_FLAG_TRANS){};

        while(!dma_rx_done) {};

        DMA_CHCTL(DMA0, DMA_CH2) &= ~DMA_CHXCTL_CHEN;
        DMA_CHCTL(DMA0, DMA_CH1) &= ~DMA_CHXCTL_CHEN;

    }else if(message->send_buf != RT_NULL) {
        // tx only
        // TX DMA
        DMA_CHMADDR(DMA0, DMA_CH2) = (uint32_t)message->send_buf;
        DMA_CHCTL(DMA0, DMA_CH2) |= DMA_CHXCTL_CHEN;

        // When the TBE or RBNE bit in SPI_STAT is set, it will
        // generate a DMA request at corresponding DMA channel.
        SPI_CTL1(info->spi_base) |= (SPI_CTL1_DMATEN);

        // TODO add rt_completion

        while(!dma_tx_done) {};

        while(SPI_STAT(info->spi_base) & SPI_FLAG_TRANS){};

        DMA_CHCTL(DMA0, DMA_CH2) &= ~DMA_CHXCTL_CHEN;

    }else if(message->recv_buf != RT_NULL) {
        // rx only
        // dummy read to clear 'RBNE' flag
        *(uint8_t *)message->recv_buf = (uint8_t)SPI_DATA(info->spi_base);

        // TX DMA
        DMA_CHMADDR(DMA0, DMA_CH2) = RX_DUMMY_BYTE_ADDR;
        DMA_CHCTL(DMA0, DMA_CH2) |= DMA_CHXCTL_CHEN;
        // RX DMA
        DMA_CHMADDR(DMA0, DMA_CH1) = (uint32_t)message->recv_buf;
        DMA_CHCTL(DMA0, DMA_CH1) |= DMA_CHXCTL_CHEN;

        // When the TBE or RBNE bit in SPI_STAT is set, it will
        // generate a DMA request at corresponding DMA channel.
        SPI_CTL1(info->spi_base) |= (SPI_CTL1_DMATEN | SPI_CTL1_DMAREN);

        // TODO add rt_completion

        while(!dma_tx_done) {};

        while(SPI_STAT(info->spi_base) & SPI_FLAG_TRANS){};

        while(!dma_rx_done) {};

        DMA_CHCTL(DMA0, DMA_CH2) &= ~DMA_CHXCTL_CHEN;
        DMA_CHCTL(DMA0, DMA_CH1) &= ~DMA_CHXCTL_CHEN;
    }

	/* release CS */
	if (message->cs_release) {
        rt_pin_write((rt_base_t)info->cs_pin, GPIO_PIN_HIGH);
	}

	return message->length;
}

rt_err_t spi_cs_init(rt_device_t dev) {
    spi_info_t *info = (spi_info_t *)dev->user_data;
    rt_pin_mode((rt_base_t)info->cs_pin, PIN_MODE_OUTPUT, PIN_SLEWRATE_MEDIUM);
    rt_pin_write((rt_base_t)info->cs_pin, GPIO_PIN_HIGH);
    return RT_EOK;
}

static const struct rt_spi_ops spi_ops = {spi_configure, spi_xfer};

static const spi_info_t gsensor_info = {
    .cs_pin = SPI_GSENSOR_CS_PIN,
    .spi_base = SPI0
};
static struct rt_spi_device gsensor_device;

static const spi_info_t spiflash_info = {
    .cs_pin = SPI_FLASH_CS_PIN,
    .spi_base = SPI0
};
static struct rt_spi_device flash_device;

static struct rt_spi_bus spi_bus0;

int rt_hw_spi0_init(void) {
	rt_spi_bus_register(&spi_bus0, "spi0", &spi_ops);

    rt_spi_bus_attach_device(&gsensor_device, "g-sensor", "spi0", (void *)&gsensor_info);
    gsensor_device.parent.init = spi_cs_init;

    rt_spi_bus_attach_device(&flash_device, "spi-flash", "spi0", (void *)&spiflash_info);
    flash_device.parent.init = spi_cs_init;

	return RT_EOK;
}
INIT_DEVICE_EXPORT(rt_hw_spi0_init);
