/*
 * drv_mem.h
 * @brief These occupy the SRAM and Peripherals respectively
 * Created on: Jul 15, 2022
 * Author: Yanye
 */
#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void mem_atomic_set(uint32_t base_addr, uint32_t index, uint32_t value);

uint32_t mem_atomic_get(uint32_t base_addr, uint32_t index);

#ifdef __cplusplus
}
#endif
