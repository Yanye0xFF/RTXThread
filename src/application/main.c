/*
 * main.c
 * @brief app entry
 * Created on: May 1, 2022
 * Author: Yanye
 */

// standard C library
#include <stdio.h>
#include <stdbool.h>
#include "stdint.h"

// GD32F30x standard peripheral
#include "system_gd32f30x.h"

// rtx-thread core
#include "rtthread.h"
#include "cmsis_os2.h"
#include "bspconfig.h"
#include "system_event.h"

#pragma GCC diagnostic ignored "-Wformat="

static const uint32_t APP_VERSION = 12;

int user_main(void) {
#if USE_UART0_AS_CONSOLE
    rt_console_set_device("uart0");
#endif

    printf("app version:%d, fcpu:%dHz\n", APP_VERSION, SystemCoreClock);

    system_event_loop_create(SYSTEM_EVENT_QUEUE_COUNT);

    while(true) {
        osDelay(5000);
        printf("hello rtx-thread\n");
    }

    return 0;
}

int test_busfault(int argc, char **argv) {
    float a = 100.0f;
    float b = 2.0f;
    float c = a / b;
    printf("a:%f\n", c);

    int *ptr = (int *)0x9f0000000;
    *ptr = 100;
    return 0;
}
MSH_CMD_EXPORT(test_busfault, exception bus fault);

int test_div0(int argc, char **argv) {
    int a = 100;
    int b = 0;
    int c = a / b;
    printf("c:%d\n", c);

    return 0;
}
MSH_CMD_EXPORT(test_div0, divided by zero);
