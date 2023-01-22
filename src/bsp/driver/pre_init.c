/*
 * pre_init.c
 * @brief 
 * Created on: May 9, 2022
 * Author: Yanye
 */
#include "rtthread.h"
#include "gd32f30x_rcu.h"

// attention: os env not setup now !
int hw_ccu_init(void) {
    rcu_periph_clock_enable(RCU_GPIOA);
    rcu_periph_clock_enable(RCU_GPIOB);
    rcu_periph_clock_enable(RCU_GPIOC);
    rcu_periph_clock_enable(RCU_GPIOD);

    rcu_periph_clock_enable(RCU_AF);

    rcu_periph_clock_enable(RCU_DMA0);
    rcu_periph_clock_enable(RCU_DMA1);
    return 0;
}
INIT_BOARD_EXPORT(hw_ccu_init);

