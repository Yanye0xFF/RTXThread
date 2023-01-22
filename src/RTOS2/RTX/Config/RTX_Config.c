/*
 * Copyright (c) 2013-2021 Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * -----------------------------------------------------------------------------
 *
 * $Revision:   V5.1.1
 *
 * Project:     CMSIS-RTOS RTX
 * Title:       RTX Configuration
 *
 * -----------------------------------------------------------------------------
 */
 
#include "cmsis_compiler.h"
#include "core_cmInstr.h"
#include "rtx_os.h"
#include "rtthread.h"
 
// OS Idle Thread
__NO_RETURN void osRtxIdleThread (void *argument) {
  (void)argument;
  __WFI();
  for (;;) {}
}
 
// OS Error Callback function
uint32_t osRtxErrorNotify (uint32_t code, void *object_id) {
  (void)object_id;

  switch (code) {
    case osRtxErrorStackOverflow:
      // Stack overflow detected for thread (thread_id=object_id)
        rt_kprintf("StackOverflow\n");
      break;
    case osRtxErrorISRQueueOverflow:
      // ISR Queue overflow detected when inserting object (object_id)
        rt_kprintf("ISRQueueOverflow\n");
      break;
    case osRtxErrorTimerQueueOverflow:
      // User Timer Callback Queue overflow detected for timer (timer_id=object_id)
        rt_kprintf("TimerQueueOverflow\n");
      break;
    case osRtxErrorClibSpace:
      // Standard C/C++ library libspace not available: increase OS_THREAD_LIBSPACE_NUM
        rt_kprintf("ErrorClibSpace\n");
      break;
    case osRtxErrorClibMutex:
      // Standard C/C++ library mutex initialization failed
        rt_kprintf("ErrorClibMutex\n");
      break;
    default:
      // Reserved
      break;
  }
  for (;;) {}
  return 0U;
}
