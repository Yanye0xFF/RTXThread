/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2006-03-18     Bernard      the first version
 * 2006-04-26     Bernard      add semaphore APIs
 * 2006-08-10     Bernard      add version information
 * 2007-01-28     Bernard      rename RT_OBJECT_Class_Static to RT_Object_Class_Static
 * 2007-03-03     Bernard      clean up the definitions to rtdef.h
 * 2010-04-11     yi.qiu       add module feature
 * 2013-06-24     Bernard      add rt_kprintf re-define when not use RT_USING_CONSOLE.
 * 2016-08-09     ArdaFu       add new thread and interrupt hook.
 * 2018-11-22     Jesven       add all cpu's lock and ipi handler
 * 2021-02-28     Meco Man     add RT_KSERVICE_USING_STDLIB
 * 2021-11-14     Meco Man     add rtlegacy.h for compatibility
 * 2022-06-04     Meco Man     remove strnlen
 */

#ifndef __RT_THREAD_H__
#define __RT_THREAD_H__

#include <rtconfig.h>
#include <rtdebug.h>
#include <rtdef.h>
#include <rtservice.h>
#include <rtm.h>
#ifdef RT_USING_LEGACY
#include <rtlegacy.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup KernelObject
 */

/**@{*/

/*
 * kernel object interface
 */
struct rt_object_information *
rt_object_get_information(enum rt_object_class_type type);
int rt_object_get_length(enum rt_object_class_type type);
int rt_object_get_pointers(enum rt_object_class_type type, rt_object_t *pointers, int maxlen);

void rt_object_init(struct rt_object         *object,
                    enum rt_object_class_type type,
                    const char               *name);
void rt_object_detach(rt_object_t object);
#ifdef RT_USING_HEAP
rt_object_t rt_object_allocate(enum rt_object_class_type type,
                               const char               *name);
void rt_object_delete(rt_object_t object);
#endif
rt_bool_t rt_object_is_systemobject(rt_object_t object);
rt_uint8_t rt_object_get_type(rt_object_t object);
rt_object_t rt_object_find(const char *name, rt_uint8_t type);

#ifdef RT_USING_HOOK
void rt_object_attach_sethook(void (*hook)(struct rt_object *object));
void rt_object_detach_sethook(void (*hook)(struct rt_object *object));
void rt_object_trytake_sethook(void (*hook)(struct rt_object *object));
void rt_object_take_sethook(void (*hook)(struct rt_object *object));
void rt_object_put_sethook(void (*hook)(struct rt_object *object));
#endif

/**@}*/


/**
 * @addtogroup MM
 */

/**@{*/

/*
 * memory management interface
 */
#ifdef RT_USING_HEAP
/*
 * heap memory interface
 */
void rt_system_heap_init(void *begin_addr, void *end_addr);

void *rt_malloc(rt_size_t nbytes);
void rt_free(void *ptr);
void *rt_realloc(void *ptr, rt_size_t nbytes);
void *rt_calloc(rt_size_t count, rt_size_t size);
void *rt_malloc_align(rt_size_t size, rt_size_t align);
void rt_free_align(void *ptr);

void rt_memory_info(rt_size_t *total,
                    rt_size_t *used,
                    rt_size_t *max_used);

#if defined(RT_USING_SLAB) && defined(RT_USING_SLAB_AS_HEAP)
void *rt_page_alloc(rt_size_t npages);
void rt_page_free(void *addr, rt_size_t npages);
#endif

#ifdef RT_USING_HOOK
void rt_malloc_sethook(void (*hook)(void *ptr, rt_size_t size));
void rt_free_sethook(void (*hook)(void *ptr));
#endif

#endif

#ifdef RT_USING_SMALL_MEM
/**
 * small memory object interface
 */
rt_smem_t rt_smem_init(const char    *name,
                     void          *begin_addr,
                     rt_size_t      size);
rt_err_t rt_smem_detach(rt_smem_t m);
void *rt_smem_alloc(rt_smem_t m, rt_size_t size);
void *rt_smem_realloc(rt_smem_t m, void *rmem, rt_size_t newsize);
void rt_smem_free(void *rmem);
#endif

#ifdef RT_USING_MEMHEAP
/**
 * memory heap object interface
 */
rt_err_t rt_memheap_init(struct rt_memheap *memheap,
                         const char        *name,
                         void              *start_addr,
                         rt_size_t         size);
rt_err_t rt_memheap_detach(struct rt_memheap *heap);
void *rt_memheap_alloc(struct rt_memheap *heap, rt_size_t size);
void *rt_memheap_realloc(struct rt_memheap *heap, void *ptr, rt_size_t newsize);
void rt_memheap_free(void *ptr);
void rt_memheap_info(struct rt_memheap *heap,
                     rt_size_t *total,
                     rt_size_t *used,
                     rt_size_t *max_used);
#endif

#ifdef RT_USING_SLAB
/**
 * slab object interface
 */
rt_slab_t rt_slab_init(const char *name, void *begin_addr, rt_size_t size);
rt_err_t rt_slab_detach(rt_slab_t m);
void *rt_slab_page_alloc(rt_slab_t m, rt_size_t npages);
void rt_slab_page_free(rt_slab_t m, void *addr, rt_size_t npages);
void *rt_slab_alloc(rt_slab_t m, rt_size_t size);
void *rt_slab_realloc(rt_slab_t m, void *ptr, rt_size_t size);
void rt_slab_free(rt_slab_t m, void *ptr);
#endif

/**@}*/


#ifdef RT_USING_DEVICE
/**
 * @addtogroup Device
 */

/**@{*/

/*
 * device (I/O) system interface
 */
rt_device_t rt_device_find(const char *name);

rt_err_t rt_device_register(rt_device_t dev,
                            const char *name,
                            rt_uint16_t flags);
rt_err_t rt_device_unregister(rt_device_t dev);

#ifdef RT_USING_HEAP
rt_device_t rt_device_create(int type, int attach_size);
void rt_device_destroy(rt_device_t device);
#endif

rt_err_t
rt_device_set_rx_indicate(rt_device_t dev,
                          rt_err_t (*rx_ind)(rt_device_t dev, rt_size_t size));
rt_err_t
rt_device_set_tx_complete(rt_device_t dev,
                          rt_err_t (*tx_done)(rt_device_t dev, void *buffer));

rt_err_t  rt_device_init (rt_device_t dev);
rt_err_t  rt_device_open (rt_device_t dev, rt_uint16_t oflag);
rt_err_t  rt_device_close(rt_device_t dev);
rt_size_t rt_device_read (rt_device_t dev,
                          rt_off_t    pos,
                          void       *buffer,
                          rt_size_t   size);
rt_size_t rt_device_write(rt_device_t dev,
                          rt_off_t    pos,
                          const void *buffer,
                          rt_size_t   size);
rt_err_t  rt_device_control(rt_device_t dev, int cmd, void *arg);

/**@}*/
#endif


#ifdef RT_USING_SMP

/*
 * smp cpus lock service
 */

rt_base_t rt_cpus_lock(void);
void rt_cpus_unlock(rt_base_t level);

struct rt_cpu *rt_cpu_self(void);
struct rt_cpu *rt_cpu_index(int index);

#endif


#ifdef RT_USING_HOOK
void rt_interrupt_enter_sethook(void (*hook)(void));
void rt_interrupt_leave_sethook(void (*hook)(void));
#endif

#ifdef RT_USING_COMPONENTS_INIT
void rt_components_init(void);
void rt_components_board_init(void);
#endif

/**
 * @addtogroup KernelService
 */

/**@{*/

/*
 * general kernel service
 */
#ifndef RT_USING_CONSOLE
#define rt_kprintf(...)
#define rt_kputs(str)
#else
int rt_kprintf(const char *fmt, ...);
void rt_kputs(const char *str);
#endif

int rt_vsprintf(char *dest, const char *format, va_list arg_ptr);
int rt_vsnprintf(char *buf, rt_size_t size, const char *fmt, va_list args);
int rt_sprintf(char *buf, const char *format, ...);
int rt_snprintf(char *buf, rt_size_t size, const char *format, ...);

#if defined(RT_USING_DEVICE) && defined(RT_USING_CONSOLE)
rt_device_t rt_console_set_device(const char *name);
rt_device_t rt_console_get_device(void);
#endif

rt_err_t rt_get_errno(void);
void rt_set_errno(rt_err_t no);
int *_rt_errno(void);
const char *rt_strerror(rt_err_t error);
#if !defined(RT_USING_NEWLIB) && !defined(_WIN32)
#ifndef errno
#define errno    *_rt_errno()
#endif
#endif

int __rt_ffs(int value);

#ifndef RT_KSERVICE_USING_STDLIB_MEMORY
void *rt_memset(void *src, int c, rt_ubase_t n);
void *rt_memcpy(void *dest, const void *src, rt_ubase_t n);
void *rt_memmove(void *dest, const void *src, rt_size_t n);
rt_int32_t rt_memcmp(const void *cs, const void *ct, rt_size_t count);
#endif /* RT_KSERVICE_USING_STDLIB_MEMORY */
char *rt_strdup(const char *s);
rt_size_t rt_strnlen(const char *s, rt_ubase_t maxlen);
#ifndef RT_KSERVICE_USING_STDLIB
char *rt_strstr(const char *str1, const char *str2);
rt_int32_t rt_strcasecmp(const char *a, const char *b);
char *rt_strcpy(char *dst, const char *src);
char *rt_strncpy(char *dest, const char *src, rt_size_t n);
rt_int32_t rt_strncmp(const char *cs, const char *ct, rt_size_t count);
rt_int32_t rt_strcmp(const char *cs, const char *ct);
rt_size_t rt_strlen(const char *src);
#else
#include <string.h>
#ifdef RT_KSERVICE_USING_STDLIB_MEMORY
#define rt_memset(s, c, count)      memset(s, c, count)
#define rt_memcpy(dst, src, count)  memcpy(dst, src, count)
#define rt_memmove(dest, src, n)    memmove(dest, src, n)
#define rt_memcmp(cs, ct, count)    memcmp(cs, ct, count)
#endif /* RT_KSERVICE_USING_STDLIB_MEMORY */
#define rt_strstr(str1, str2)       strstr(str1, str2)
#define rt_strcasecmp(a, b)         strcasecmp(a, b)
#define rt_strcpy(dest, src)        strcpy(dest, src)
#define rt_strncpy(dest, src, n)    strncpy(dest, src, n)
#define rt_strncmp(cs, ct, count)   strncmp(cs, ct, count)
#define rt_strcmp(cs, ct)           strcmp(cs, ct)
#define rt_strlen(src)              strlen(src)
#endif /*RT_KSERVICE_USING_STDLIB*/
void rt_show_version(void);

#ifdef RT_DEBUG
extern void (*rt_assert_hook)(const char *ex, const char *func, rt_size_t line);
void rt_assert_set_hook(void (*hook)(const char *ex, const char *func, rt_size_t line));

void rt_assert_handler(const char *ex, const char *func, rt_size_t line);
#endif /* RT_DEBUG */

#ifdef RT_USING_FINSH
#include <finsh.h>
#endif

/**@}*/

#ifdef __cplusplus
}
#endif

#endif
