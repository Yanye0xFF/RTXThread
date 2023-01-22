/*
 * drv_dac.c
 * @brief 
 * Created on: Jun 13, 2022
 * Author: Yanye
 */
#include "rt_dac.h"

#include "rtthread.h"
#include "gd32f30x_rcu.h"
#include "gd32f30x_dac.h"

// private:
#define DAC0_GPIO_PORT    GPIOA
#define DAC0_GPIO_PIN     GPIO_PIN_4

static struct rt_dac_device dac0_dev;

static rt_err_t (adc_disabled)(struct rt_dac_device *device, rt_uint32_t channel) {
    rcu_periph_clock_disable(RCU_DAC);
    return RT_EOK;
}

static rt_err_t (adc_enabled)(struct rt_dac_device *device, rt_uint32_t channel) {
    rcu_periph_clock_enable(RCU_DAC);
    /* once enabled the DAC, the corresponding GPIO pin is connected to the DAC converter automatically */
    gpio_init(DAC0_GPIO_PORT, GPIO_MODE_AIN, GPIO_OSPEED_50MHZ, DAC0_GPIO_PIN);

    dac_deinit();
    /* configure the DAC0 */
    dac_trigger_disable(DAC0);
    dac_concurrent_disable();
    dac_wave_mode_config(DAC0, DAC_WAVE_DISABLE);
    dac_output_buffer_enable(DAC0);

    dac_enable(DAC0);
    dac_data_set(DAC0, DAC_ALIGN_12B_R, 0);

    return RT_EOK;
}

static rt_err_t (adc_convert)(struct rt_dac_device *device, rt_uint32_t channel, rt_uint32_t *value) {
    uint16_t data = (uint16_t)(*value & 0xFFFF);
    dac_data_set(DAC0, DAC_ALIGN_12B_R, data);
    return RT_EOK;
}

static const struct rt_dac_ops dac_ops = {
    .disabled = adc_disabled,
    .enabled = adc_enabled,
    .convert = adc_convert
};

int rt_hw_dac_init(void) {
    rt_err_t err;
    err = rt_hw_dac_register(&dac0_dev, "dac0", &dac_ops, RT_NULL);
    return (int)err;
}
INIT_DEVICE_EXPORT(rt_hw_dac_init);
