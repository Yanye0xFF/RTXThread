/*
 * drv_adc.c
 * @brief 
 * Created on: Jun 13, 2022
 * Author: Yanye
 */
#include "rtthread.h"
#include "cmsis_os2.h"
#include "gd32f30x_rcu.h"
#include "bspconfig.h"
#include "rt_adc.h"

static struct rt_adc_device adc0_dev;

/**
 * @param device adc device
 * @param channel parameter not used, enable or disable all channels which support.
 * @param enabled RT_TRUE: enable, RT_FALSE: disable.
 * */
static rt_err_t (adc_enabled)(struct rt_adc_device *device, rt_uint32_t channel, rt_bool_t enabled) {
    if(enabled) {
        /* enable ADC clock */
        rcu_periph_clock_enable(RCU_ADC0);
        /* config ADC clock, apb2 is 60M, adc clock is 10M */
        rcu_adc_clock_config(RCU_CKADC_CKAPB2_DIV6);

        /* config the GPIO as analog mode */
        gpio_init(GPIOA, GPIO_MODE_AIN, GPIO_OSPEED_10MHZ, GPIO_PIN_1);
        gpio_init(GPIOA, GPIO_MODE_AIN, GPIO_OSPEED_10MHZ, GPIO_PIN_2); // *
        gpio_init(GPIOA, GPIO_MODE_AIN, GPIO_OSPEED_10MHZ, GPIO_PIN_3);
        gpio_init(GPIOA, GPIO_MODE_AIN, GPIO_OSPEED_10MHZ, GPIO_PIN_5); // *
        gpio_init(GPIOA, GPIO_MODE_AIN, GPIO_OSPEED_10MHZ, GPIO_PIN_6); // *

        gpio_init(GPIOB, GPIO_MODE_AIN, GPIO_OSPEED_10MHZ, GPIO_PIN_0);
        gpio_init(GPIOB, GPIO_MODE_AIN, GPIO_OSPEED_10MHZ, GPIO_PIN_1);

        gpio_init(GPIOC, GPIO_MODE_AIN, GPIO_OSPEED_10MHZ, GPIO_PIN_0);
        gpio_init(GPIOC, GPIO_MODE_AIN, GPIO_OSPEED_10MHZ, GPIO_PIN_1);
        gpio_init(GPIOC, GPIO_MODE_AIN, GPIO_OSPEED_10MHZ, GPIO_PIN_2);
        gpio_init(GPIOC, GPIO_MODE_AIN, GPIO_OSPEED_10MHZ, GPIO_PIN_3);
        gpio_init(GPIOC, GPIO_MODE_AIN, GPIO_OSPEED_10MHZ, GPIO_PIN_4);
        gpio_init(GPIOC, GPIO_MODE_AIN, GPIO_OSPEED_10MHZ, GPIO_PIN_5);

        /* ADC mode config */
        adc_mode_config(ADC_MODE_FREE);
        /* ADC data alignment config */
        adc_data_alignment_config(ADC0, ADC_DATAALIGN_RIGHT);
        /* ADC channel length config */
        adc_channel_length_config(ADC0, ADC_REGULAR_CHANNEL, 1);
        adc_resolution_config(ADC0, ADC_RESOLUTION_12B);

        /* ADC trigger config */
        adc_external_trigger_source_config(ADC0, ADC_REGULAR_CHANNEL, ADC0_1_2_EXTTRIG_REGULAR_NONE);
        adc_external_trigger_config(ADC0, ADC_REGULAR_CHANNEL, ENABLE);

        /* enable ADC interface */
        adc_enable(ADC0);

        /**
         * @see Device_limitations_of_GD32F30x_Rev1.0.pdf
         * When application programme executes the adc calibration function after power up,
         * VDDA voltage generates fluctuation which results in the ADC sampling value distortion.
         *
         * Add 1ms delay after ADC is enabled and before calibrating.
         * */
        osDelay(1);

        /* ADC calibration and reset calibration */
        adc_calibration_enable(ADC0);

    }else {
        adc_disable(ADC0);
        /* disable ADC clock */
        rcu_periph_clock_disable(RCU_ADC0);
    }

    return RT_EOK;
}

/**
 * @brief read adc value
 * @param device adc device
 * @param channel support channel @see drv_adc.h
 * */
static rt_err_t (adc_convert)(struct rt_adc_device *device, rt_uint32_t channel, rt_uint32_t *value) {
    /* clear the end of conversion flag */
    adc_flag_clear(ADC0, ADC_FLAG_EOC);
    /* ADC regular channel config */
    adc_regular_channel_config(ADC0, 0, channel, ADC_SAMPLETIME_239POINT5);
    /* ADC software trigger enable */
    adc_software_trigger_enable(ADC0, ADC_REGULAR_CHANNEL);

    /* wait the end of conversion flag */
    while(!adc_flag_get(ADC0, ADC_FLAG_EOC)) {};
    // convert time less than 400ns.
    adc_flag_clear(ADC0, ADC_FLAG_EOC);
    *value = adc_regular_data_read(ADC0);

    return RT_EOK;
}

static const struct rt_adc_ops adc_ops = {
    .enabled = adc_enabled,
    .convert = adc_convert,
};

int rt_hw_adc_init(void) {
    rt_err_t err;
    err = rt_hw_adc_register(&adc0_dev, "adc0", &adc_ops, RT_NULL);
    return (int)err;
}
INIT_DEVICE_EXPORT(rt_hw_adc_init);
