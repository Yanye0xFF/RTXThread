/*
 * drv_pin.c
 * @brief
 * Created on: Mar 27, 2022
 * Author: Yanye
 */

#include "drv_pin.h"
#include "pin.h"
#include "stdlib.h"
#include "gd32f30x_rcu.h"
#include "gd32f30x_gpio.h"
#include "rtthread.h"

static const uint32_t PORT_LUT[] = {GPIOA, GPIOB, GPIOC, GPIOD};
static const uint8_t GPIO_SPEED[] = {GPIO_OSPEED_2MHZ, GPIO_OSPEED_10MHZ, GPIO_OSPEED_50MHZ};

static void pin_mode(struct rt_device *device, rt_base_t pin, rt_base_t mode, rt_base_t strength) {
	uint32_t port = PORT_LUT[(pin >> 4)];
	uint32_t pin_sub = (pin & 0xF);
	uint32_t pin_mode = GPIO_MODE_IN_FLOATING, pin_speed;

	pin_sub = ((uint32_t)1 << pin_sub);
	pin_speed = GPIO_SPEED[strength];

	if(mode == PIN_MODE_OUTPUT) {
	    pin_mode = GPIO_MODE_OUT_PP;
	}else if(mode == PIN_MODE_INPUT) {
		pin_mode = GPIO_MODE_IN_FLOATING;
	}else if(mode == PIN_MODE_INPUT_PULLUP) {
		pin_mode = GPIO_MODE_IPU;
	}else if(mode == PIN_MODE_INPUT_PULLDOWN) {
		pin_mode = GPIO_MODE_IPD;
	}else if(mode == PIN_MODE_OUTPUT_OD) {
		pin_mode = GPIO_MODE_OUT_OD;
	}

	gpio_init(port, pin_mode, pin_speed, pin_sub);
}

/**
 * @param name gpio's name, for example: "PA.0"，"PB.15"...
 * @return enum GPIO_PIN_NUM, if error occurs return value < 0
 * */
#define GPIO_NAME_MIN    4
#define GPIO_NAME_MAX    5
rt_base_t pin_get(const char *name) {
    char buffer[8];
    rt_size_t len =  rt_strlen(name);
    rt_base_t pin_num;
    int pin_id;

    if((len < GPIO_NAME_MIN) || (len > GPIO_NAME_MAX)) {
        return -RT_ERROR;
    }
    for(rt_size_t i = 0; i < len; i++) {
        buffer[i] = (name[i] | 0x20);
    }
    if((buffer[1] < 'a') || (buffer[1] > 'd')) {
        return -RT_ERROR;
    }
    pin_id = atoi(name + 3);
    pin_num = ((buffer[1] - 'a') * 16 + pin_id);
    return pin_num;
}

void pin_write(struct rt_device *device, rt_base_t pin, rt_base_t value) {
    uint32_t port = PORT_LUT[(pin >> 4)];
	uint32_t p = (pin & 0xF);
	if(value & 0x1) {
	    GPIO_BOP(port) = (uint32_t)1 << p;
	}else {
        GPIO_BC(port) = (uint32_t)1 << p;
	}
}

void pin_toggle(struct rt_device *device, rt_base_t pin) {
    uint32_t port = PORT_LUT[(pin >> 4)];
    uint32_t p = (pin & 0xF);
    uint32_t status = (GPIO_OCTL(port) >> p) & 0x1;
    // reserve status
    if(status) {
        GPIO_BC(port) = (uint32_t)1 << p;
    }else {
        GPIO_BOP(port) = (uint32_t)1 << p;
    }
}

int pin_read(struct rt_device *device, rt_base_t pin) {
    uint32_t port = PORT_LUT[(pin >> 4)];
    uint32_t p = (pin & 0xF);
    uint32_t in = (GPIO_ISTAT(port) >> p) & 0x1;
    return in;
}

static const struct rt_pin_ops ops = {
    pin_mode,
    pin_write,
    pin_toggle,
    pin_read,
    RT_NULL,
    RT_NULL,
    RT_NULL,
    pin_get
};

int rt_hw_gpio_init(void) {
    // GPIO clock enable in 'pre_init.c'
    rt_device_pin_register("gpio", &ops, RT_NULL);
    return 0;
}
INIT_DEVICE_EXPORT(rt_hw_gpio_init);

#ifdef RT_USING_FINSH
#include <finsh.h>
#include <stdlib.h>

static int gpio(int argc, char **argv)
{
    int result = 0, level;

    if (argc < 3) {
        rt_kprintf("Usage: gpio PA.1 write 0\n");
        rt_kprintf("       gpio PA.0 read\n");
        result = -RT_ERROR;
        goto _exit;
    }

    rt_base_t pin = rt_pin_get(argv[1]);
    rt_kprintf("pin:%d\n", pin);
    if(rt_strcmp(argv[2], "write") == 0) {
        level = atoi(argv[3]);
        rt_pin_mode(pin, PIN_MODE_OUTPUT, PIN_SLEWRATE_LOW);
        rt_pin_write(pin, level);
        rt_kprintf("write:%s %d\n", argv[1], level);
    }else {
        rt_pin_mode(pin, PIN_MODE_INPUT, PIN_SLEWRATE_LOW);
        level = rt_pin_read(pin);
        rt_kprintf("read:%s %d\n", argv[1], level);
    }

_exit:
    return result;
}
MSH_CMD_EXPORT(gpio, test gpio);
#endif
