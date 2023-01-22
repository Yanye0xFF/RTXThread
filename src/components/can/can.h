/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author            Notes
 * 2015-05-14     aubrcool@qq.com   first version
 * 2015-07-06     Bernard           remove RT_CAN_USING_LED.
 */

#ifndef CAN_H_
#define CAN_H_

#include <rtthread.h>
#include "cmsis_os2.h"

#ifndef RT_CANMSG_BOX_SZ
#define RT_CANMSG_BOX_SZ    16
#endif
#ifndef RT_CANSND_BOX_NUM
// 硬件邮箱数量，决定信号量数量
#define RT_CANSND_BOX_NUM   3
#endif

enum CANBAUD
{
    CAN1MBaud = 0,
    CAN800kBaud,
    CAN500kBaud,
    CAN250kBaud,
    CAN125kBaud,
    CAN100kBaud,
    CAN50kBaud,
    CAN20kBaud,
    CAN10kBaud
};

#define RT_CAN_MODE_NORMAL              0
#define RT_CAN_MODE_LISEN               1
#define RT_CAN_MODE_LOOPBACK            2
#define RT_CAN_MODE_LOOPBACKANLISEN     3

#define RT_CAN_MODE_PRIV                0x01
#define RT_CAN_MODE_NOPRIV              0x00

struct rt_can_filter_item
{
    /* 报文 ID*/
    rt_uint32_t id  : 29;
    /* 扩展帧标识位 */
    rt_uint32_t ide : 1;
    /* 远程帧标识位 */
    rt_uint32_t rtr : 1;
    /* 过滤表模式 */
    rt_uint32_t mode : 1;
    /* ID 掩码，0 表示对应的位不关心，1 表示对应的位必须匹配 */
    rt_uint32_t mask;
    /* -1 表示不指定过滤表号，对应的过滤表控制块也不会被初始化，正数为过滤表号，对应的过滤表控制块会被初始化 */
    rt_int32_t hdr;
#ifdef RT_CAN_USING_HDR
    rt_err_t (*ind)(rt_device_t dev, void *args , rt_int32_t hdr, rt_size_t size);
    void *args;
#endif /*RT_CAN_USING_HDR*/
};

#ifdef RT_CAN_USING_HDR
#define RT_CAN_FILTER_ITEM_INIT(id,ide,rtr,mode,mask,ind,args) \
     {(id), (ide), (rtr), (mode), (mask), -1, (ind), (args)}
#define RT_CAN_FILTER_STD_INIT(id,ind,args) \
     RT_CAN_FILTER_ITEM_INIT(id,0,0,0,0xFFFFFFFF,ind,args)
#define RT_CAN_FILTER_EXT_INIT(id,ind,args) \
     RT_CAN_FILTER_ITEM_INIT(id,1,0,0,0xFFFFFFFF,ind,args)
#define RT_CAN_STD_RMT_FILTER_INIT(id,ind,args) \
     RT_CAN_FILTER_ITEM_INIT(id,0,1,0,0xFFFFFFFF,ind,args)
#define RT_CAN_EXT_RMT_FILTER_INIT(id,ind,args) \
     RT_CAN_FILTER_ITEM_INIT(id,1,1,0,0xFFFFFFFF,ind,args)
#define RT_CAN_STD_RMT_DATA_FILTER_INIT(id,ind,args) \
     RT_CAN_FILTER_ITEM_INIT(id,0,0,1,0xFFFFFFFF,ind,args)
#define RT_CAN_EXT_RMT_DATA_FILTER_INIT(id,ind,args) \
     RT_CAN_FILTER_ITEM_INIT(id,1,0,1,0xFFFFFFFF,ind,args)
#else

#define RT_CAN_FILTER_ITEM_INIT(id,ide,rtr,mode,mask) \
     {(id), (ide), (rtr), (mode), (mask), -1, }
#define RT_CAN_FILTER_STD_INIT(id) \
     RT_CAN_FILTER_ITEM_INIT(id,0,0,0,0xFFFFFFFF)
#define RT_CAN_FILTER_EXT_INIT(id) \
     RT_CAN_FILTER_ITEM_INIT(id,1,0,0,0xFFFFFFFF)
#define RT_CAN_STD_RMT_FILTER_INIT(id) \
     RT_CAN_FILTER_ITEM_INIT(id,0,1,0,0xFFFFFFFF)
#define RT_CAN_EXT_RMT_FILTER_INIT(id) \
     RT_CAN_FILTER_ITEM_INIT(id,1,1,0,0xFFFFFFFF)
#define RT_CAN_STD_RMT_DATA_FILTER_INIT(id) \
     RT_CAN_FILTER_ITEM_INIT(id,0,0,1,0xFFFFFFFF)
#define RT_CAN_EXT_RMT_DATA_FILTER_INIT(id) \
     RT_CAN_FILTER_ITEM_INIT(id,1,0,1,0xFFFFFFFF)
#endif

struct rt_can_filter_config
{
    rt_uint32_t count;
    rt_uint32_t actived;
    struct rt_can_filter_item *items;
};

struct can_configure
{
    rt_uint32_t baud_rate;
    rt_uint32_t msgboxsz;
    rt_uint32_t sndboxnumber;
    rt_uint32_t mode      : 8;
    rt_uint32_t privmode  : 8;
    rt_uint32_t reserved  : 16;
#ifdef RT_CAN_USING_HDR
    rt_uint32_t maxhdr;
#endif
};

#define CANDEFAULTCONFIG \
{\
        CAN500kBaud,\
        RT_CANMSG_BOX_SZ,\
        RT_CANSND_BOX_NUM,\
        RT_CAN_MODE_NORMAL,\
        RT_CAN_MODE_NOPRIV,\
        0,\
};

struct rt_can_ops;
#define RT_CAN_CMD_SET_FILTER       0x13
#define RT_CAN_CMD_SET_BAUD         0x14
#define RT_CAN_CMD_SET_MODE         0x15
#define RT_CAN_CMD_SET_PRIV         0x16
#define RT_CAN_CMD_GET_STATUS       0x17
#define RT_CAN_CMD_SET_STATUS_IND   0x18
#define RT_CAN_CMD_SET_BUS_HOOK     0x19

#define RT_DEVICE_CAN_INT_ERR       0x1000

enum RT_CAN_STATUS_MODE
{
    NORMAL = 0,
    ERRWARNING = 1,
    ERRPASSIVE = 2,
    BUSOFF = 4,
};
enum RT_CAN_BUS_ERR
{
    RT_CAN_BUS_NO_ERR = 0,
    RT_CAN_BUS_BIT_PAD_ERR = 1,
    RT_CAN_BUS_FORMAT_ERR = 2,
    RT_CAN_BUS_ACK_ERR = 3,
    RT_CAN_BUS_IMPLICIT_BIT_ERR = 4,
    RT_CAN_BUS_EXPLICIT_BIT_ERR = 5,
    RT_CAN_BUS_CRC_ERR = 6,
};

struct rt_can_status
{
    rt_uint32_t rcverrcnt;
    rt_uint32_t snderrcnt;
    rt_uint32_t errcode;
    rt_uint32_t rcvpkg;
    rt_uint32_t dropedrcvpkg;
    rt_uint32_t sndpkg;
    rt_uint32_t dropedsndpkg;
    rt_uint32_t bitpaderrcnt;
    rt_uint32_t formaterrcnt;
    rt_uint32_t ackerrcnt;
    rt_uint32_t biterrcnt;
    rt_uint32_t crcerrcnt;
    rt_uint32_t rcvchange;
    rt_uint32_t sndchange;
    rt_uint32_t lasterrtype;
};

#ifdef RT_CAN_USING_HDR
struct rt_can_hdr
{
    rt_uint32_t connected;
    rt_uint32_t msgs;
    struct rt_can_filter_item filter;
    struct rt_list_node list;
};
#endif
struct rt_can_device;
typedef rt_err_t (*rt_canstatus_ind)(struct rt_can_device *, void *);
typedef struct rt_can_status_ind_type
{
    rt_canstatus_ind ind;
    void *args;
} *rt_can_status_ind_type_t;
typedef void (*rt_can_bus_hook)(struct rt_can_device *);
struct rt_can_device
{
    struct rt_device parent;

    const struct rt_can_ops *ops;
    struct can_configure config;
    struct rt_can_status status;

    struct rt_can_status_ind_type status_indicate;
#ifdef RT_CAN_USING_HDR
    struct rt_can_hdr *hdr;
#endif
#ifdef RT_CAN_USING_BUS_HOOK
    rt_can_bus_hook bus_hook;
#endif /*RT_CAN_USING_BUS_HOOK*/
    osMutexId_t lock;
    void *can_rx;
    void *can_tx;
};
typedef struct rt_can_device *rt_can_t;

#define RT_CAN_STDID 0
#define RT_CAN_EXTID 1
#define RT_CAN_DTR   0
#define RT_CAN_RTR   1

typedef struct rt_can_status *rt_can_status_t;

struct rt_can_msg {
    rt_uint32_t id : 29; /* CAN ID, 标志格式 11 位，扩展格式 29 位 */
    rt_uint32_t ide : 1; /* 扩展帧标识位 */
    rt_uint32_t rtr : 1; /* 远程帧标识位 */
    rt_uint32_t rsv : 1; /* 保留位 */
    rt_uint32_t len : 8; /* 数据段长度 */
    rt_uint32_t priv : 8; /* 报文发送优先级 */
    rt_uint32_t hdr : 8; /* 硬件过滤表号 */
    rt_uint32_t reserved : 8;
    rt_uint8_t data[8]; /* 数据段 */
};

typedef struct rt_can_msg *rt_can_msg_t;

struct rt_can_msg_list
{
    struct rt_list_node list;
#ifdef RT_CAN_USING_HDR
    struct rt_list_node hdrlist;
    struct rt_can_hdr *owner;
#endif
    struct rt_can_msg data;
};

struct rt_can_rx_fifo
{
    /* software fifo */
    struct rt_can_msg_list *buffer;
    rt_uint32_t freenumbers;
    struct rt_list_node freelist;
    struct rt_list_node uselist;
};

#define RT_CAN_SND_RESULT_OK        0
#define RT_CAN_SND_RESULT_ERR       1
#define RT_CAN_SND_RESULT_WAIT      2
#define RT_CAN_SND_TIMEOUT          1000

#define RT_CAN_EVENT_RX_IND         0x01    /* Rx indication */
#define RT_CAN_EVENT_TX_DONE        0x02    /* Tx complete   */
#define RT_CAN_EVENT_TX_FAIL        0x03    /* Tx fail   */
#define RT_CAN_EVENT_RX_TIMEOUT     0x05    /* Rx timeout    */
#define RT_CAN_EVENT_RXOF_IND       0x06    /* Rx overflow */

struct rt_can_sndbxinx_list
{
    struct rt_list_node list;
    osSemaphoreId_t completion;
    rt_uint32_t result;
};

struct rt_can_tx_fifo
{
    struct rt_can_sndbxinx_list *buffer;
    osSemaphoreId_t sem;
    struct rt_list_node freelist;
};

struct rt_can_ops
{
    rt_err_t (*configure)(struct rt_can_device *can, struct can_configure *cfg);
    rt_err_t (*control)(struct rt_can_device *can, int cmd, void *arg);
    int (*sendmsg)(struct rt_can_device *can, const void *buf, rt_uint32_t boxno);
    int (*recvmsg)(struct rt_can_device *can, void *buf, rt_uint32_t boxno);
};

rt_err_t rt_hw_can_register(struct rt_can_device    *can,
                            const char              *name,
                            const struct rt_can_ops *ops,
                            void                    *data);
void rt_hw_can_isr(struct rt_can_device *can, int event);
#endif /*_CAN_H*/

