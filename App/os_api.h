#ifndef TX_API_H
#define TX_API_H

#include "gd32f450i_eval.h"
#include "systick.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "event_groups.h"
#include "timers.h"
#include "semphr.h"

#include "logging.h"

typedef void                          VOID;
typedef char                          CHAR;
typedef unsigned char                 UCHAR;
typedef int                           INT;
typedef unsigned int                  UINT;
typedef long                          LONG;
typedef unsigned long                 ULONG;
typedef short                         SHORT;
typedef unsigned short                USHORT;
typedef unsigned int                  time_t;


#define OS_SUCCESS     0x00
#define OS_ERROR       0x01

#define ULONG_MAX      0xFFFFFFFF

#define os_msleep(ticks)               vTaskDelay(ticks)
#define os_msleep_until(start, ticks)  xTaskDelayUntil(start, ticks)
#define os_thread_sleep(ticks)         vTaskDelay(ticks * 20)
#define os_time_get()                  (TickType_t)xTaskGetTickCount()
#define os_time_get_ISR()              (TickType_t)xTaskGetTickCountFromISR()
#define os_free(ptr)                   if(ptr) vPortFree(ptr)

#define time_block(ticks) ((ticks == -1) ? portMAX_DELAY : ticks)
/*static mem*/
#define os_byte_pool           StackType_t  //StackType_t  Stackpool[8]

/*dynamic mem*/
////[1]queue
#define os_queue_t             QueueHandle_t

////[2]timer


////[3]task
#define os_task_t                     TaskHandle_t
#define os_task_create(fun, name, stack, parm, pri, tHandle) \
								      xTaskCreate(fun, name, stack, parm, pri, tHandle);
#define os_task_destory(tHandle)	  vTaskDelete(tHandle)
#define os_task_destory_by_self()     vTaskDelete(NULL)
#define os_task_suspend(tHandle)      vTaskSuspend(tHandle)
#define os_task_resume(tHandle)       vTaskResume(tHandle)
//0/1 sema improve by tasknofity(1:1)
//count sema improve by tasknofity(1:1)
//b = pdTRUE : auto clear 0 (0/1 sema)
//b = pdFALSE: auto sub 1   (count sema)
#define os_task_sema_wait(b, tick)            (uint32_t)ulTaskNotifyTake(b, time_block(tick))
#define os_task_sema_post(tHandle)            xTaskNotifyGive(tHandle)
#define os_task_sema_post_ISR(tHandle, out)   vTaskNotifyGiveFromISR(tHandle, out)
//mailbox improve by tasknofity
#define os_task_mailbox_send(tHandle, value)  xTaskNotify(tHandle, value, (eNotifyAction)eSetValueWithOverwrite)
#define os_task_mailbox_recv(out, tick)       xTaskNotifyWait(0x00, ULONG_MAX, out, time_block(tick))
//event group improve by tasknofity
#define os_task_event_send(tHandle, value)    xTaskNotify(tHandle, value, (eNotifyAction)eSetBits)
#define os_task_event_recv(out, tick)         xTaskNotifyWait(0x00, ULONG_MAX, out, time_block(tick))

////[4]mutex lock(un support ISR)
#define os_mutex_t                         SemaphoreHandle_t
#define os_mutex_init()                    xSemaphoreCreateMutex()
#define os_mutex_destory(mHandle)          vSemaphoreDelete(mHandle)
#define os_mutex_lock(mHandle, tick)       xSemaphoreTake(mHandle, time_block(tick))
#define os_mutex_unlock(mHandle)           xSemaphoreGive(mHandle)

////[5]nest mutex lock(un support ISR)
#define os_nest_mutex_t                    SemaphoreHandle_t
#define os_nest_mutex_init()               xSemaphoreCreateRecursiveMutex()
#define os_nest_mutex_destory(nsHandle)    vSemaphoreDelete(nsHandle)
#define os_nest_mutex_lock(nsHandle, tick) xSemaphoreTake(nsHandle, time_block(tick))
#define os_nest_mutex_unlock(nsHandle)     xSemaphoreGive(nsHandle)

////[6]0/1 sema
#define os_sema_t                          SemaphoreHandle_t
#define os_sema_init()                     xSemaphoreCreateBinary()
#define os_sema_destory(sHandle)           vSemaphoreDelete(sHandle)
#define os_sema_wait(sHandle, tick)        xSemaphoreTake(sHandle, time_block(tick))
#define os_sema_post(sHandle)              xSemaphoreGive(sHandle)
#define os_sema_wait_ISR(sHandle, out)     xSemaphoreTakeFromISR(sHandle, out)
#define os_sema_post_ISR(sHandle, out)     xSemaphoreGiveFromISR(sHandle, out)

////[7]count sema
#define os_count_sema_t                    SemaphoreHandle_t
#define os_count_sema_init(m, i)           xSemaphoreCreateCounting(m ,i)
#define os_count_sema_destory(csHandle)    vSemaphoreDelete(csHandle)
#define os_count_sema_wait(csHandle, tick) xSemaphoreTake(csHandle, time_block(tick))
#define os_count_sema_post(csHandle)       xSemaphoreGive(csHandle)
#define os_count_sema_wait_ISR(csHandle, out) xSemaphoreTakeFromISR(csHandle, out)
#define os_count_sema_post_ISR(csHandle, out) xSemaphoreGiveFromISR(csHandle, out)

////[8]event group
#define os_event_group_t                  EventGroupHandle_t
#define os_event_group_create()           xEventGroupCreate()
#define os_event_group_clear(eHandle, fg) xEventGroupClearBits(eHandle, (EventBits_t)fg)
#define os_event_group_set(eHandle, fg)   xEventGroupSetBits(eHandle, (EventBits_t)fg)
#define os_event_group_get(eHandle)       xEventGroupGetBits(eHandle)
#define os_event_group_wait(eHandle, fg, cFg, wFg, tick) \
	                                      xEventGroupWaitBits(eHandle, (EventBits_t)fg, (BaseType_t)cFg, (BaseType_t)wFg, time_block(tick))
#define os_event_group_clear_ISR(eHandle, fg)    xEventGroupClearBitsFromISR(eHandle, (EventBits_t)fg)
#define os_event_group_set_ISR(eHandle, fg, out) xEventGroupSetBitsFromISR((eHandle, (EventBits_t)fg), out)
#define os_event_group_get_ISR(eHandle)          xEventGroupGetBitsFromISR(eHandle)


#ifdef __cplusplus
extern   "C" {
#endif

UINT os_malloc(VOID **memory_ptr, ULONG memory_size);

#ifdef __cplusplus
        }
#endif

#endif

