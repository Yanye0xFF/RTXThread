/*
 * drv_mem.c
 * @brief 
 * Created on: Jul 8, 2022
 * Author: Yanye
 */

#include "rtthread.h"
#include <stdlib.h>
#include <stdint.h>
#include "gd32f30x.h"
#include "drv_mem.h"

#ifdef RT_USING_FINSH
#include <finsh.h>

/**
 * @brief General Programming-Syntax: peek <Memory address>
 * @note The PEEK function returns the memory contents of the specified address,
 *       which must be in the range 0 through 0xFFFFFFFF.
 *       The byte value returned will be in the range from 0 thru 0xFFFFFFFF.
 * */
static int peek(int argc, char **argv) {
    if (argc != 2) {
        rt_kprintf("Usage: peek <memory> <address>\n");
        return 0;
    }

    unsigned long i = strtoul(argv[1], NULL, 16);
    uint32_t val = REG32(i);

    rt_kprintf("mem:0x%08X, val:0x%08X\n", i, val);
    return 0;
}
MSH_CMD_EXPORT(peek, read memory);

/**
 * @brief General Programming-Syntax: poke <memory address>, <number>
 * @note The command POKE changes the content of any address in the memory address,
 *        ranging from 0 to 0xFFFFFFFF, to the given byte value in the range 0 through 0xFFFFFFFF.
 * */
static int poke(int argc, char **argv) {
    if (argc != 3) {
        rt_kprintf("Usage: poke <memory> <address> <number>\n");
        return 0;
    }

    unsigned long addr = strtoul(argv[1], NULL, 16);
    unsigned long value = strtoul(argv[2], NULL, 16);

    REG32(addr) = value;
    uint32_t val = REG32(addr);

    rt_kprintf("mem:0x%08X, val:0x%08X\n", addr, val);

    return 0;
}
MSH_CMD_EXPORT(poke, write memory);

#endif

/**
 * @brief bit-banding function to perform a single atomic bit operation
 * @param base_addr the number of the byte(byte address) in the bit-band region that contains the targeted bit.
 * @param index byte offset [7,0]
 * @param value bit value '0' or '1'.
 * */
void mem_atomic_set(uint32_t base_addr, uint32_t index, uint32_t value) {
    uint32_t bit_band_base = (base_addr & 0xF0000000) | 0x02000000;
    uint32_t mask_addr = (base_addr & 0x000FFFFF);
    uint32_t bit_word_addr = bit_band_base + (mask_addr << 5) + (index << 2);

    REG32(bit_word_addr) = (value & 0x1);
}

/**
 * @param base_addr same as above.
 * @param index same as above.
 * */
uint32_t mem_atomic_get(uint32_t base_addr, uint32_t index) {
    uint32_t bit_band_base = (base_addr & 0xF0000000) | 0x02000000;
    uint32_t mask_addr = (base_addr & 0x000FFFFF);
    uint32_t bit_word_addr = bit_band_base + (mask_addr << 5) + (index << 2);

    return REG32(bit_word_addr);
}
