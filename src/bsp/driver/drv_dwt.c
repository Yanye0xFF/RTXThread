/*
 * drv_dwt.c
 * @brief 
 * Created on: Nov 24, 2022
 * Author: Yanye
 */
#include "drv_dwt.h"
#include "rtthread.h"
#include "system_gd32f30x.h"
#include "core_cm4.h"

int hw_dwt_init(void) {
    // DWT timer enable
    CoreDebug->DEMCR = 0x1000000;
    // clear Cycle Count Register
    DWT->CYCCNT = 0x0;
    // enable  Data Watchpoint and Trace Register
    DWT->CTRL = 0x1;
    return 0;
}
INIT_BOARD_EXPORT(hw_dwt_init);

/**
 * @param us 0 ~ 35791394us
 * */
void dwt_delay_us(uint32_t us) {
    uint32_t timeout = (SystemCoreClock / 1000000) * us;

    DWT->CYCCNT = 0x0;

    while(DWT->CYCCNT < timeout);
}

/**
 * @param us 0 ~ 35791ms
 * */
void dwt_delay_ms(uint32_t ms) {
    uint32_t timeout = (SystemCoreClock / 1000) * ms;

    DWT->CYCCNT = 0x0;

    while(DWT->CYCCNT < timeout);
}
