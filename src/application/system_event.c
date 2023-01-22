/*
 * sys_event.c
 * @brief 
 * Created on: Jan 3, 2023
 * Author: Yanye
 */

#include "system_event.h"
#include "cmsis_os2.h"

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>

// private:
typedef struct {
    int32_t event_id;
    const void *event_data;
}event_message_t;

typedef struct handler_item_s {
    int32_t event_id;
    event_handler_t event_handler;
    void *event_handler_arg;
    struct handler_item_s *next;
}handler_item_t;

static osThreadId_t sys_evt_tid = NULL;
static osMessageQueueId_t event_queue = NULL;
static handler_item_t *head_node = NULL;
static osMutexId_t *node_mutex = NULL;

static void system_event_loop_run_thread(void* args);

/**
 * @brief Create system event loop.
 * @note 无锁的单例模式，初始化过程非线程安全
 * @param[in] event_queue_count size of the event loop queue
 *
 * @return
 *  - RT_EOK: Success
 *  - -RT_ENOMEM: Cannot allocate memory for event loops list
 */
rt_err_t system_event_loop_create(uint32_t event_queue_count) {

    // prevent duplicate calls.
    if(sys_evt_tid != NULL) {
        return RT_EOK;
    }

    const osMessageQueueAttr_t queue_attr = {
        .name = "sys_queue",
        .attr_bits = 0U,
        .cb_mem = NULL,
        .cb_size = 0U,
        .mq_mem = NULL,
        .mq_size = 0U,
    };
    event_queue = osMessageQueueNew(event_queue_count, sizeof(event_message_t), &queue_attr);
    if(event_queue == NULL) {
        return -RT_ENOMEM;
    }

    const osMutexAttr_t node_mutex_attr = {
        .name = "sq_mutex",
        .attr_bits = (osMutexRecursive|osMutexPrioInherit|osMutexRobust),
        .cb_mem = NULL,
        .cb_size = 0U
    };
    node_mutex = osMutexNew(&node_mutex_attr);
    if(node_mutex == NULL) {
        goto __CREATE_LOOP_ERROR2;
    }

    const osThreadAttr_t event_thread_attr = {
        .name = "sys_evt",
        .attr_bits = osThreadDetached,
        .cb_mem = NULL,
        .cb_size = 0u,
        .stack_mem = NULL,
        .stack_size = 1024u,
        .priority = osPriorityBelowNormal7,
        .tz_module = 0u,
        .reserved= 0u
    };
    sys_evt_tid = osThreadNew(system_event_loop_run_thread, NULL, &event_thread_attr);
    if(sys_evt_tid == NULL) {
        goto __CREATE_LOOP_ERROR1;
    }

    return RT_EOK;

__CREATE_LOOP_ERROR1:
    osMutexDelete(node_mutex);

__CREATE_LOOP_ERROR2:
    osMessageQueueDelete(event_queue);

    return -RT_ENOMEM;
}

/**
 * @brief Register an event handler to the system event loop.
 *
 * This function can be used to register a handler for specific events,
 *
 * Registering a single handler to multiple events is possible. \\
 * However, registering the same event multiple times would throws error code.
 *
 * @param[in] event_id the ID of the event to register the handler for.
 * @param[in] event_handler the handler function which gets called when the event is dispatched.
 * @param[in] event_handler_arg data, aside from event data, that is passed to the handler when it is called.
 *
 * @note this function is thread safe.
 * the event loop library does not maintain a copy of event_handler_arg(not a value copy !!!), therefore the user should
 * ensure that event_handler_arg still points to a valid location by the time the handler gets called.
 *
 * @return
 *  - RT_EOK: Success
 *  - -RT_EFULL: the handler already registered.
 *  - -RT_ENOMEM: Cannot allocate memory for the handler
 */
rt_err_t system_event_handler_register(int32_t event_id,
                                     event_handler_t event_handler,
                                     void *event_handler_arg) {
    handler_item_t *tail_node;

    osMutexAcquire(node_mutex, osWaitForever);

    tail_node = head_node;
    while(tail_node != NULL) {
        if(tail_node->event_id == event_id) {
            return -RT_EFULL;
        }
        tail_node = tail_node->next;
    }

    osMutexRelease(node_mutex);

    handler_item_t *node = (handler_item_t *)rt_malloc(sizeof(handler_item_t));
    if(node == NULL) {
        return -RT_ENOMEM;
    }

    node->event_id = event_id;
    node->event_handler = event_handler;
    node->event_handler_arg = event_handler_arg;
    node->next = NULL;

    osMutexAcquire(node_mutex, osWaitForever);

    if(head_node == NULL) {
        head_node = node;

    }else {
        tail_node = head_node;
        while(tail_node->next != NULL) {
            tail_node = tail_node->next;
        }
        tail_node->next = node;

    }

    osMutexRelease(node_mutex);

    return RT_EOK;
}

/**
 * @brief Unregister a handler with the system event loop.
 *
 * Unregisters a handler, so it will no longer be called during dispatch.
 * Handlers can be unregistered for any  event_id which were previously registered.
 * To unregister a handler, the event_id arguments must match exactly the arguments passed to
 * abs_event_handler_register() when that handler was registered.
 *
 * @note this function is thread safe.
 *
 * @param[in] event_id the ID of the event with which to unregister the handler
 * @param[in] event_handler the handler to unregister
 *
 * @return RT_EOK success
 */
rt_err_t system_event_handler_unregister(int32_t event_id,
                                       event_handler_t event_handler) {
    handler_item_t *prev;
    handler_item_t *cur;

    osMutexAcquire(node_mutex, osWaitForever);

    // -Wmaybe-uninitialized
    prev = head_node;

    cur = head_node;

    while(cur != NULL) {
        if((cur->event_id == event_id) && (cur->event_handler == event_handler)) {
            if(cur == head_node) {
                head_node = cur->next;
            }else {
                prev->next = cur->next;
            }
            rt_free(cur);
            break;
        }
        prev = cur;
        cur = cur->next;
    }

    osMutexRelease(node_mutex);

    return RT_EOK;
}

/**
 * @brief Posts an event to the system event loop.
 * the event loop library does not maintain a copy of event_handler_arg(not a value copy !!!), therefore the user should
 * ensure that event_handler_arg still points to a valid location by the time the handler gets called.
 *
 * @param[in] event_id the event ID that identifies the event
 * @param[in] event_data the data, specific to the event occurrence, that gets passed to the handler
 * @param[in] timeout number of ticks to block on a full event queue
 *
 * @return
 *  - RT_EOK: Success
 *  - -RT_ETIMEOUT: Time to wait for event queue to unblock expired,
 */
rt_err_t system_event_post(int32_t event_id,
                         const void *event_data,
                         uint32_t timeout) {

    event_message_t msg;
    msg.event_id = event_id;
    msg.event_data = event_data;

    osStatus_t status = osMessageQueuePut(event_queue, (const void *)&msg, 0, timeout);

    return (status == osOK) ? RT_EOK : (-RT_ETIMEOUT);
}

static void system_event_loop_run_thread(void* args) {
    (void)args;

    osStatus_t status;
    event_message_t msg;
    handler_item_t *node;
    event_handler_t callback;
    void *arg;
    bool mutex_hold;

    while(true) {
        status = osMessageQueueGet(event_queue, (void *)&msg, NULL, osWaitForever);
        if(status != osOK) {
            break;
        }

        osMutexAcquire(node_mutex, osWaitForever);

        node = head_node;
        mutex_hold = true;

        while(node != NULL) {
            if(node->event_id == msg.event_id) {
                // 这里用一次值拷贝作为代价换取提前释放锁
                callback = node->event_handler;
                arg = node->event_handler_arg;
                osMutexRelease(node_mutex);
                mutex_hold = false;

                // Execute id level handlers
                callback(arg, msg.event_id, msg.event_data);
                break;
            }
            node = node->next;
        }

        if(mutex_hold) {
            osMutexRelease(node_mutex);
        }
        // to next loop:
    }
    // message queue read error:
    osMessageQueueDelete(event_queue);
    osMutexDelete(node_mutex);
}
