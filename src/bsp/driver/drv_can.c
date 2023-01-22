/*
 * drv_can.c
 * @brief GD32F303 CAN0
 * Created on: Apr 18, 2022
 * Author: Yanye
 */

#include "drv_can.h"
#include "rtthread.h"
#include "can.h"
#include "gd32f30x_can.h"
#include "gd32f30x_gpio.h"
#include "bspconfig.h"
#include "bspconfig.h"
#include "pin.h"

#define CAN0_GPIO_PORT    GPIOA
#define CAN0_RX_PIN       GPIO_PIN_11
#define CAN0_TX_PIN       GPIO_PIN_12

#define CAN_PHY_ENABLE    GPIO_PIN_LOW
#define CAN_PHY_DISABLE   GPIO_PIN_HIGH
static rt_base_t can_phy_ctrl;

static struct rt_can_device can_dev;
static can_receive_message_struct rx_message;

// 1M (800K)x 500K 250K 125K 100K 50K 20K (10K)x
// x: not support
static const uint16_t CAN0_PRESCALER[] = {6, 0, 12, 24, 48, 60, 120, 300, 0};

void USBD_LP_CAN0_RX0_IRQHandler(void) {
    if(SET == can_interrupt_flag_get(CAN0, CAN_INT_FLAG_RFL0)) {
        can_message_receive(CAN0, CAN_FIFO0, &rx_message);
        rt_hw_can_isr(&can_dev, RT_CAN_EVENT_RX_IND);
        can_interrupt_flag_clear(CAN0, CAN_INT_FLAG_RFL0);
    }
}

void USBD_HP_CAN0_TX_IRQHandler(void) {
    if(SET == can_interrupt_flag_get(CAN0, CAN_INT_FLAG_MTF0)) {
        can_interrupt_flag_clear(CAN0, CAN_INT_FLAG_MTF0);
        rt_hw_can_isr(&can_dev, (0x0000 | RT_CAN_EVENT_TX_DONE));
    }
    if(SET == can_interrupt_flag_get(CAN0, CAN_INT_FLAG_MTF1)) {
        can_interrupt_flag_clear(CAN0, CAN_INT_FLAG_MTF1);
        rt_hw_can_isr(&can_dev, (0x0100 | RT_CAN_EVENT_TX_DONE));
    }
    if(SET == can_interrupt_flag_get(CAN0, CAN_INT_FLAG_MTF2)) {
        can_interrupt_flag_clear(CAN0, CAN_INT_FLAG_MTF2);
        rt_hw_can_isr(&can_dev, (0x0200 | RT_CAN_EVENT_TX_DONE));
    }
}

rt_err_t (can_configure)(struct rt_can_device *can, struct can_configure *cfg) {
    can_parameter_struct can_param;
    can_filter_parameter_struct filter_param;

    /* enable CAN clock */
    rcu_periph_clock_enable(RCU_CAN0);

    /* can0 phy enable out-OD */
    can_phy_ctrl = rt_pin_get(CAN_PHY_EN_PIN);
    rt_pin_mode(can_phy_ctrl, PIN_MODE_OUTPUT_OD, PIN_SLEWRATE_LOW);
    // low level enable.
    rt_pin_write(can_phy_ctrl, CAN_PHY_ENABLE);

    /* configure CAN0 GPIO */
    gpio_init(CAN0_GPIO_PORT, GPIO_MODE_IPU, GPIO_OSPEED_50MHZ, CAN0_RX_PIN);
    gpio_init(CAN0_GPIO_PORT, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, CAN0_TX_PIN);

    can_struct_para_init(CAN_INIT_STRUCT, &can_param);
    can_struct_para_init(CAN_FILTER_STRUCT, &filter_param);

    /* initialize CAN register */
    can_deinit(CAN0);

    /* initialize CAN parameters */
    can_param.time_triggered = DISABLE;
    can_param.auto_bus_off_recovery = ENABLE;
    can_param.auto_wake_up = DISABLE;
    can_param.auto_retrans = ENABLE;
    can_param.rec_fifo_overwrite = DISABLE;
    can_param.trans_fifo_order = DISABLE;
    can_param.working_mode = CAN_NORMAL_MODE;
    can_param.resync_jump_width = CAN_BT_SJW_1TQ;
    can_param.time_segment_1 = CAN_BT_BS1_5TQ;
    can_param.time_segment_2 = CAN_BT_BS2_4TQ;

    if(cfg->baud_rate > CAN10kBaud) {
        return -RT_EINVAL;
    }
    can_param.prescaler = CAN0_PRESCALER[cfg->baud_rate];
    if(can_param.prescaler == 0) {
        return -RT_EINVAL;
    }
    can_init(CAN0, &can_param);

    /* initialize filter */
    filter_param.filter_number = 0;
    filter_param.filter_mode = CAN_FILTERMODE_MASK;
    filter_param.filter_bits = CAN_FILTERBITS_32BIT;
    filter_param.filter_list_high = 0x0000;
    filter_param.filter_list_low = 0x0000;
    filter_param.filter_mask_high = 0x0000;
    filter_param.filter_mask_low = 0x0000;
    filter_param.filter_fifo_number = CAN_FIFO0;
    filter_param.filter_enable = ENABLE;
    can_filter_init(&filter_param);

    return RT_EOK;
}

rt_err_t (can_control)(struct rt_can_device *can, int cmd, void *arg) {
    const can_flag_enum CAN_FLAG_LUT[3] = {CAN_INT_FLAG_MTF0, CAN_INT_FLAG_MTF1, CAN_INT_FLAG_MTF2};
    if(cmd == RT_DEVICE_CTRL_OPEN) {
        /* configure CAN0 NVIC */
        can_interrupt_enable(CAN0, CAN_INT_TME);
        can_interrupt_enable(CAN0, CAN_INT_RFNE0);
        nvic_irq_enable(USBD_LP_CAN0_RX0_IRQn, CAN0_RX_PRIORITY, 0);
        nvic_irq_enable(USBD_HP_CAN0_TX_IRQn, CAN0_RX_PRIORITY, 0);

    }else if(cmd == RT_DEVICE_CTRL_CLOSE) {
        can_deinit(CAN0);
        /* configure CAN0 NVIC */
        can_interrupt_disable(CAN0, CAN_INT_TME);
        can_interrupt_disable(CAN0, CAN_INT_RFNE0);
        nvic_irq_disable(USBD_LP_CAN0_RX0_IRQn);
        nvic_irq_disable(USBD_HP_CAN0_TX_IRQn);
        rcu_periph_clock_disable(RCU_CAN0);
        // close can phy
        rt_pin_write(can_phy_ctrl, CAN_PHY_DISABLE);

    }else if(cmd == RT_DEVICE_CTRL_CLR_INT) {
        uint32_t boxn = (uint32_t)arg;
        nvic_irq_disable(USBD_HP_CAN0_TX_IRQn);
        can_transmission_stop(CAN0, (uint8_t)(boxn & 0x3));
        can_interrupt_flag_clear(CAN0, CAN_FLAG_LUT[(boxn & 0x3)]);
        nvic_irq_enable(USBD_HP_CAN0_TX_IRQn, CAN0_RX_PRIORITY, 0);

    }
    return RT_EOK;
}

int (can_sendmsg)(struct rt_can_device *can, const void *buf, rt_uint32_t boxno) {
    const struct rt_can_msg *msg = (const struct rt_can_msg *)buf;
    can_trasnmit_message_struct tx_message;
    uint8_t res;

    if(msg->ide) {
        tx_message.tx_ff = CAN_FF_EXTENDED;
        tx_message.tx_sfid = 0;
        // 29 bit extended id
        tx_message.tx_efid = (msg->id & 0x1FFFFFFF);
    }else {
        tx_message.tx_ff = CAN_FF_STANDARD;
        tx_message.tx_efid = 0;
        // 11 bit standard id
        tx_message.tx_sfid = (msg->id & 0x7FFF);
    }
    tx_message.tx_ft = (msg->rtr) ? CAN_FT_REMOTE : CAN_FT_DATA;
    tx_message.tx_dlen = msg->len;
    rt_memcpy(tx_message.tx_data, msg->data, 8);

    res = can_message_transmit(CAN0, boxno, &tx_message);

    return (res == CAN_NOMAILBOX) ? (-RT_EBUSY) : RT_EOK;
}

int (can_recvmsg)(struct rt_can_device *can, void *buf, rt_uint32_t boxno) {
    struct rt_can_msg *msg = (struct rt_can_msg *)buf;
    if(rx_message.rx_ff == CAN_FF_EXTENDED) {
        msg->id = rx_message.rx_efid;
        msg->ide = 1;
    }else {
        msg->id = rx_message.rx_sfid;
        msg->ide = 0;
    }
    msg->rtr = (rx_message.rx_ft) ? 1 : 0;
    msg->len = rx_message.rx_dlen;
    rt_memcpy(msg->data, rx_message.rx_data, 8);
    return boxno;
}

static const struct rt_can_ops can_ops = {
    .configure = can_configure,
    .control = can_control,
    .sendmsg = can_sendmsg,
    .recvmsg = can_recvmsg
};

static const struct can_configure config = CANDEFAULTCONFIG;

int rt_can_init(void) {
    rt_memcpy(&(can_dev.config), &config, sizeof(struct can_configure));

    rt_err_t err = rt_hw_can_register(&can_dev, "can0", &can_ops, RT_NULL);
    return (int)err;
}
INIT_DEVICE_EXPORT(rt_can_init);
