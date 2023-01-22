/*
 * bspconfig.h
 * @brief 
 * Created on: Dec 9, 2022
 * Author: Yanye
 */

#pragma once

// 4 bits for pre-emption priority, 0 bits for subpriority
// 0: highest, 15: lowest

#define CAN0_RX_PRIORITY          15
#define CAN0_TX_PRIORITY          15
#define UART0_RX_PRIORITY         15
#define UART0_TX_DMA_PRIORITY     15
#define SPI0_TX_DMA_PRIORITY      15
#define SPI0_RX_DMA_PRIORITY      15

#define HW_TIMER_PRIORITY         1

#define CAN_BUS_DEVICE_NAME    "can0"
#define CAN_PHY_EN_PIN         "PC.12"

#define SPI_FLASH_DEVICE_NAME   "spi-flash"
// add flash chip info into 'SFUD_FLASH_CHIP_TABLE', @see sfud_flash_def.h, if device not support JEDEC SFDP.
#define SPI_FLASH_NAME          "GD25Q32C"
// @see driver/drv_spi.h
#define FLASH_SPI_SPEED          0

#define RS485_RTS_PIN          "PA.12"
#define RS485_USE_SERIAL       "uart0"
#define RS485_OVERLAP_UART0    0
#define USE_UART0_AS_CONSOLE   0
