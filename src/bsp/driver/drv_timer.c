/*
 * drv_timer.c
 * @brief 
 * Created on: Dec 13, 2022
 * Author: Yanye
 */
#include "rtthread.h"
#include "rthw.h"
#include "cmsis_os2.h"

#include "gd32f30x_rcu.h"
#include "gd32f30x_timer.h"
#include "gd32f30x_misc.h"

#include "bspconfig.h"

static osSemaphoreId_t timer_sem;
ALIGN(4) static osRtxSemaphore_t timer_sem_entity;
static const osSemaphoreAttr_t timer_sem_attr = {
    .name = "tm_sem",
    .attr_bits = 0u,
    .cb_mem = (void *)&timer_sem_entity,
    .cb_size = sizeof(osRtxSemaphore_t),
};

/**
 * @brief timer1中断向量入口
 * */
void TIMER1_IRQHandler(void) {
    // clear and disable timer interrupt
    TIMER_INTF(TIMER1) = (~TIMER_INT_FLAG_UP);
    TIMER_DMAINTEN(TIMER1) &= (~TIMER_INT_FLAG_UP);
    // disable timer
    TIMER_CTL0(TIMER1) &= (~TIMER_CTL0_CEN);

    osSemaphoreRelease(timer_sem);
}

/**
 * @brief 硬件定时器初始化，自动在main_thread_entry(线程环境)调用
 * */
int rt_hw_timer_init(void) {
    timer_parameter_struct param;

    rcu_periph_clock_enable(RCU_TIMER1);
    timer_deinit(TIMER1);
    timer_disable(TIMER1);
    // update event generate only by counter overflow/underflow
    timer_update_source_config(TIMER1, TIMER_UPDATE_SRC_REGULAR);

    /* TIMER1 configuration */
    param.prescaler         = 119;
    param.alignedmode       = TIMER_COUNTER_EDGE;
    param.counterdirection  = TIMER_COUNTER_UP;
    param.period            = 0xFFFF;
    param.clockdivision     = TIMER_CKDIV_DIV1;
    param.repetitioncounter = 0;
    timer_init(TIMER1, &param);

    timer_interrupt_flag_clear(TIMER1, TIMER_INT_UP);
    timer_update_event_enable(TIMER1);
    // 关闭reload shadow register，对TIMER_CAR寄存器的修改立即生效(不用等到下一个update事件之后)
    timer_auto_reload_shadow_disable(TIMER1);

    nvic_irq_enable(TIMER1_IRQn, HW_TIMER_PRIORITY, 0);

    timer_sem = osSemaphoreNew(1, 0, &timer_sem_attr);

    return RT_EOK;
}
INIT_DEVICE_EXPORT(rt_hw_timer_init);

/**
 * @brief 微秒级异步延时，只能在线程环境调用
 * @param us 1 ~ 65536
 * */
void rt_hw_us_delay(rt_uint32_t us) {
    TIMER_CNT(TIMER1) = 0;
    TIMER_CAR(TIMER1) = (us - 1);

    TIMER_DMAINTEN(TIMER1) |= TIMER_INT_UP;
    TIMER_CTL0(TIMER1) |= TIMER_CTL0_CEN;

    osSemaphoreAcquire(timer_sem, osWaitForever);
}
