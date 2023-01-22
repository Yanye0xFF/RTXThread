/*
 * drv_rtc.c
 * @brief 
 * Created on: Apr 24, 2022
 * Author: Yanye
 */

#include "rtc.h"
#include "rtthread.h"
#include "rthw.h"
#include <sys/time.h>

#include "gd32f30x_rcu.h"

#define TIME_TO_SECOND_FACTOR    2500

static rt_rtc_dev_t rtc_dev;

rt_err_t rtc_init(void) {
    /* enable PMU and BKPI clocks */
    rcu_periph_clock_enable(RCU_BKPI);
    rcu_periph_clock_enable(RCU_PMU);

    /* allow access to BKP domain */
    pmu_backup_write_enable();

    /* select RCU_RTCSRC_HXTAL_DIV_128 as RTC clock source */
    rcu_rtc_clock_config(RCU_RTCSRC_HXTAL_DIV_128);
    /* enable RTC Clock */
    rcu_periph_clock_enable(RCU_RTC);

    /* wait for RTC registers synchronization */
    rtc_register_sync_wait();
    /* wait until last write operation on RTC registers has finished */
    rtc_lwoff_wait();

    /* enable the RTC second interrupt*/
    //rtc_interrupt_enable(RTC_INT_SECOND);
    /* wait until last write operation on RTC registers has finished */
    //rtc_lwoff_wait();

    /* set RTC prescaler: finally: RTC clock is 2500Hz, period is 0.4ms, 19 days max of 32bit register */
    rtc_prescaler_set(25);
    /* wait until last write operation on RTC registers has finished */
    rtc_lwoff_wait();

    rtc_counter_set(0);
    rtc_lwoff_wait();

    return RT_EOK;
}

rt_err_t rtc_get_secs(void *arg) {
    time_t *t = (time_t *)arg;
    *t = rtc_counter_get() / TIME_TO_SECOND_FACTOR;
    return RT_EOK;
}

rt_err_t rtc_set_secs(void *arg) {
    uint32_t cnt = *((uint32_t *)arg);
    rtc_counter_set(cnt * TIME_TO_SECOND_FACTOR);
    rtc_lwoff_wait();
    return RT_EOK;
}

rt_err_t rtc_get_alarm(void *arg) {
    return RT_EOK;
}

rt_err_t rtc_set_alarm(void *arg) {
    return RT_EOK;
}

rt_err_t rtc_get_timeval(void *arg) {
    // get microseconds
    struct timeval *val = (struct timeval *)arg;
    val->tv_usec = (rtc_counter_get() * 400);
    return RT_EOK;
}

rt_err_t rtc_set_timeval(void *arg) {
    return RT_EOK;
}

const struct rt_rtc_ops rtc_ops = {
    .init = rtc_init,
    .get_secs = rtc_get_secs,
    .set_secs = rtc_set_secs,
    .get_alarm = rtc_get_alarm,
    .set_alarm = rtc_set_alarm,
    .get_timeval = rtc_get_timeval,
    .set_timeval = rtc_set_timeval
};

/**
 * @brief RTC计数器值作为系统时钟
 * @return 计数器值
 * */
rt_uint32_t rt_hw_get_time(void) {
    uint32_t temp;

    temp = RTC_CNTL;
    temp |= (RTC_CNTH << 16);

    return temp;
}

int rt_hw_rtc_init(void) {
    rtc_dev.ops = &rtc_ops;
    rt_hw_rtc_register(&rtc_dev, "rtc", RT_DEVICE_FLAG_RDWR, RT_NULL);
    return RT_EOK;
}
INIT_DEVICE_EXPORT(rt_hw_rtc_init);
