/*
 * system_event.h
 * @brief 
 * Created on: Jan 3, 2023
 * Author: Yanye
 */

#pragma once

/* includes */

#include <stdint.h>

#include "rtthread.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    // add user event id here.


    EVENT_ID_INVALID = 0x7fffffff
}event_id_t;

#define SYSTEM_EVENT_QUEUE_COUNT    16

// function called when an event is posted to the queue
typedef void (*event_handler_t)(void* event_handler_arg, int32_t event_id, const void* event_data);

rt_err_t system_event_loop_create(uint32_t event_queue_count);

rt_err_t system_event_handler_register(int32_t event_id,
                                     event_handler_t event_handler,
                                     void *event_handler_arg);

rt_err_t system_event_handler_unregister(int32_t event_id,
                                       event_handler_t event_handler);

rt_err_t system_event_post(int32_t event_id,
                         const void *event_data,
                         uint32_t timeout);

#ifdef __cplusplus
}
#endif
