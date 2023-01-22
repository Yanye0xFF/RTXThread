/*
 * drv_dwt.h
 * @brief 
 * Created on: Nov 24, 2022
 * Author: Yanye
 */

#pragma once

#include <stdint.h>
#include "core_cm4.h"

#ifdef __cplusplus
extern "C" {
#endif

static inline __attribute__((always_inline)) void dwt_cnt_clear(void) {
    DWT->CYCCNT = 0x0;
}

static inline __attribute__((always_inline)) uint32_t dwt_cnt_get(void) {
    return DWT->CYCCNT;
}

void dwt_delay_us(uint32_t us);

void dwt_delay_ms(uint32_t ms);

#ifdef __cplusplus
}
#endif
