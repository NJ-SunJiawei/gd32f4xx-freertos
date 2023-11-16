 #ifndef OS_API_H
#define OS_API_H

//c system
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <time.h>

//driver
#include "gd32f450i_eval.h"
#include "drv_gd25q40.h"
#include "drv_gd25qxx.h"
#include "drv_sdcard.h"
#include "systick.h"

//rtos
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "event_groups.h"
#include "timers.h"
#include "semphr.h"

//fatfs
#include "ff.h" 

#define DRV_SPI_SWITCH 1   //1:drv_gd25q40/0:drv_gd25qxx
#define DMA_SPI_SWITCH 1   //drv_gd25q40 1:DMA+SPI 0:SPI
typedef void                          VOID;
typedef char                          CHAR;
typedef unsigned char                 UCHAR;

typedef short                         SHORT;
typedef unsigned short                USHORT;

typedef int                           INT;
typedef unsigned int                  UINT;

typedef long                          LONG;
typedef unsigned long                 ULONG;

typedef unsigned int                  time_t;

#define _in_ 
#define _out_
#define _res_
#define _lock_task_
#define _lock_isr_

#define ULONG_MAX      0xFFFFFFFF

#define os_msleep(ticks)               vTaskDelay(ticks)
#define os_msleep_until(start, ticks)  xTaskDelayUntil(start, ticks)
#define os_task_sleep(ticks)           vTaskDelay(ticks * 20)
#define os_time_get()                  _res_ (TickType_t)xTaskGetTickCount()
#define os_time_get_ISR()              _res_ (TickType_t)xTaskGetTickCountFromISR()
#define os_free(ptr)                   if(ptr) vPortFree(ptr)

#define time_block(ticks) ((ticks == (unsigned int)-1) ? portMAX_DELAY : ticks)
/*static mem*/
#define os_byte_pool           StackType_t  //StackType_t  Stackpool[8]

/*dynamic mem*/
////[0]scheduler
#define os_system_suspend()        _lock_isr_ taskENTER_CRITICAL()
#define os_system_resume()         _lock_isr_ taskEXIT_CRITICAL()
#define os_system_suspend_ISR()    _lock_isr_ taskENTER_CRITICAL_FROM_ISR()
#define os_system_resume_ISR()     _lock_isr_ taskEXIT_CRITICAL_FROM_ISR()

#define os_scheduler_suspend()     _lock_task_ vTaskSuspendAll()
#define os_scheduler_resume()      _lock_task_ xTaskResumeAll()

////[1]queue
#define os_queue_t                  QueueHandle_t
#define os_queue_create(dep, size)  _res_ (QueueHandle_t)xQueueCreate(dep, size)
#define os_queue_send(qHandle, value, tick)         xQueueSendToBack(qHandle, value, tick)
#define os_queue_send_front(qHandle, value, tick)   xQueueSendToFront(qHandle, value, tick)
#define os_queue_send_over(qHandle, value)          xQueueOverwrite(qHandle, value)
#define os_queue_recv(qHandle, buffer, tick)        xQueueReceive(qHandle, buffer, time_block(tick))
#define os_queue_peek(qHandle, buffer, tick)        xQueuePeek(qHandle, buffer, time_block(tick))
#define os_queue_avail(qHandle)                     _res_ (UBaseType_t)uxQueueSpacesAvailable(qHandle)
#define os_queue_used(qHandle)                      _res_ (UBaseType_t)uxQueueMessagesWaiting(qHandle)
#define os_queue_send_ISR(qHandle, value, switch)         xQueueSendToBackFromISR(qHandle, value, _out_ switch)
#define os_queue_send_front_ISR(qHandle, value, switch)   xQueueSendToFrontFromISR(qHandle, value, _out_ switch)
#define os_queue_send_over_ISR(qHandle, value, switch)    xQueueOverwriteFromISR(qHandle, value, _out_ switch)
#define os_queue_recv_ISR(qHandle, buffer, switch)        xQueueReceiveFromISR(qHandle, buffer, _out_ switch)
#define os_queue_peek_ISR(qHandle, buffer)                xQueuePeekFromISR(qHandle, buffer)


////[2]timer
#define os_timer_t             TimerHandle_t
//ms 500/portTICK_PERIOD_MS
#define os_timer_create(name, period, mode, id, func) \
                               _res_ (TimerHandle_t)xTimerCreate(name, period, mode, id, func)
#define os_timer_reset(tHandle)            xTimerReset(tHandle, 0)
#define os_timer_start(tHandle)            xTimerStart(tHandle, 0)
#define os_timer_stop(tHandle)             xTimerStop(tHandle, 0)
#define os_timer_reset_ISR(tHandle, switch)   xTimerResetFromISR(tHandle, _out_ switch)
#define os_timer_start_ISR(tHandle, switch)   xTimerStartFromISR(tHandle, _out_ switch)
#define os_timer_stop_ISR(tHandle, switch)    xTimerStopFromISR(tHandle, _out_ switch)

////[3]task
#define os_task_t                     TaskHandle_t
#define os_task_create(fun, name, stack, parm, pri, tHandle) \
								      xTaskCreate(fun, name, stack, parm, pri, _out_ tHandle);
#define os_task_destory(tHandle)	    vTaskDelete(tHandle)
#define os_task_destory_by_self()     vTaskDelete(NULL)
#define os_task_suspend(tHandle)      vTaskSuspend(tHandle)
#define os_task_resume(tHandle)       vTaskResume(tHandle)
//0/1 sema improve by tasknofity(1:1)
//count sema improve by tasknofity(1:1)
//b = pdTRUE : auto clear 0 (0/1 sema)
//b = pdFALSE: auto sub 1   (count sema)
#define os_task_sema_wait(b, tick)             _res_ (uint32_t)ulTaskNotifyTake(b, time_block(tick))
#define os_task_sema_post(tHandle)             xTaskNotifyGive(tHandle)
#define os_task_sema_post_ISR(tHandle, switch) vTaskNotifyGiveFromISR(tHandle, _out_ switch)
//mailbox improve by tasknofity
#define os_task_mailbox_send(tHandle, value)  xTaskNotify(tHandle, value, (eNotifyAction)eSetValueWithOverwrite)
#define os_task_mailbox_recv(value, tick)     xTaskNotifyWait(0x00, ULONG_MAX, _out_ value, time_block(tick))
//event group improve by tasknofity
#define os_task_event_send(tHandle, value)    xTaskNotify(tHandle, value, (eNotifyAction)eSetBits)
#define os_task_event_recv(value, tick)       xTaskNotifyWait(0x00, ULONG_MAX, _out_ value, time_block(tick))

////[4]mutex lock(un support ISR)
#define os_mutex_t                         SemaphoreHandle_t
#define os_mutex_init()                    _res_ (SemaphoreHandle_t)xSemaphoreCreateMutex()
#define os_mutex_destory(mHandle)          vSemaphoreDelete(mHandle)
#define os_mutex_lock(mHandle, tick)       xSemaphoreTake(mHandle, time_block(tick))
#define os_mutex_unlock(mHandle)           xSemaphoreGive(mHandle)

////[5]nest mutex lock(un support ISR)
#define os_nest_mutex_t                    SemaphoreHandle_t
#define os_nest_mutex_init()               _res_ (SemaphoreHandle_t)xSemaphoreCreateRecursiveMutex()
#define os_nest_mutex_destory(nsHandle)    vSemaphoreDelete(nsHandle)
#define os_nest_mutex_lock(nsHandle, tick) xSemaphoreTake(nsHandle, time_block(tick))
#define os_nest_mutex_unlock(nsHandle)     xSemaphoreGive(nsHandle)

////[6]0/1 sema
#define os_sema_t                          SemaphoreHandle_t
#define os_sema_init()                     _res_ (SemaphoreHandle_t)xSemaphoreCreateBinary()
#define os_sema_destory(sHandle)           vSemaphoreDelete(sHandle)
#define os_sema_wait(sHandle, tick)        xSemaphoreTake(sHandle, time_block(tick))
#define os_sema_post(sHandle)              xSemaphoreGive(sHandle)
#define os_sema_wait_ISR(sHandle, switch)  xSemaphoreTakeFromISR(sHandle, _out_ switch)
#define os_sema_post_ISR(sHandle, switch)  xSemaphoreGiveFromISR(sHandle, _out_ switch)

////[7]count sema
#define os_count_sema_t                    SemaphoreHandle_t
#define os_count_sema_init(m, i)           _res_ (SemaphoreHandle_t)xSemaphoreCreateCounting(m ,i)
#define os_count_sema_destory(csHandle)    vSemaphoreDelete(csHandle)
#define os_count_sema_wait(csHandle, tick) xSemaphoreTake(csHandle, time_block(tick))
#define os_count_sema_post(csHandle)       xSemaphoreGive(csHandle)
#define os_count_sema_wait_ISR(csHandle, switch) xSemaphoreTakeFromISR(csHandle, _out_ switch)
#define os_count_sema_post_ISR(csHandle, switch) xSemaphoreGiveFromISR(csHandle, _out_ switch)

////[8]event group
#define os_event_group_t                  EventGroupHandle_t
#define os_event_group_create()           _res_ (EventGroupHandle_t)xEventGroupCreate()
#define os_event_group_clear(eHandle, fg) xEventGroupClearBits(eHandle, (EventBits_t)fg)
#define os_event_group_set(eHandle, fg)   xEventGroupSetBits(eHandle, (EventBits_t)fg)
#define os_event_group_get(eHandle)       xEventGroupGetBits(eHandle)
#define os_event_group_wait(eHandle, fg, cFg, wFg, tick) \
	                                      xEventGroupWaitBits(eHandle, (EventBits_t)fg, (BaseType_t)cFg, (BaseType_t)wFg, time_block(tick))
#define os_event_group_clear_ISR(eHandle, fg)    xEventGroupClearBitsFromISR(eHandle, (EventBits_t)fg)
#define os_event_group_set_ISR(eHandle, fg, switch) xEventGroupSetBitsFromISR((eHandle, (EventBits_t)fg), _out_ switch)
#define os_event_group_get_ISR(eHandle)             xEventGroupGetBitsFromISR(eHandle)


#ifdef __cplusplus
extern   "C" {
#endif

typedef enum {
	OS_SUCCESS,
	OS_ERROR,
}OS_STATUS;

#ifdef __cplusplus
        }
#endif

#endif

