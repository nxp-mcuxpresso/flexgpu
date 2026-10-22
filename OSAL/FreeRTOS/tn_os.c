//
// Copyright 2025-2026 NXP
//
// SPDX-License-Identifier: MIT
//

#include "tn_types.h"
#include "tn_os.h"
#include "tn_trace.h"
#include "tn_aver.h"

static void
prvRunThread(void *pxArg)
{
    tn_thread_t *pxThread = (tn_thread_t*)pxArg;

    /* Run the thread routine. */
    pxThread->pvStartRoutine((void*)pxThread->xTaskArg);

    vTaskDelete(NULL);
}



void
tn_os_msleep(tn_u32 ulMsecToDelay)
{
    const TickType_t xTicksToDelay = ulMsecToDelay / portTICK_PERIOD_MS;
    vTaskDelay(xTicksToDelay);
}



void
tn_os_sched(void)
{
    vTaskStartScheduler();
}



tn_thread_t*
tn_os_thread_init(
    tn_thread_prio_t xSchedPriority,
    int           ( *pvStartRoutine )(void*),
    tn_size          usStackSize,
    void            *xAttr)
{
    tn_thread_t *pxThread;

    pxThread = (tn_thread_t*)pvPortMalloc(sizeof(tn_thread_t));
    if (!pxThread)
    {
        return NULL;
    }

    pxThread->xTaskArg = xAttr;
    pxThread->pvStartRoutine = pvStartRoutine;

    // tn_os_thread_init() takes the stack size in bytes, but xTaskCreate()
    // counts it in words, so convert here rather than leaking the unit into the
    // OS-agnostic headers. Round up so a request is never under-served.
    const tn_size xStackWords =
        (usStackSize + sizeof(StackType_t) - 1) / sizeof(StackType_t);

    BaseType_t xTaskCreateStatus = xTaskCreate(
        prvRunThread,
        "FreeRTOS-Task",
        (configSTACK_DEPTH_TYPE)xStackWords,
        (void*)pxThread,
        tskIDLE_PRIORITY + xSchedPriority,
        &pxThread->xTaskHandle);

    if (xTaskCreateStatus != pdPASS)
    {
        vPortFree(pxThread);
        return NULL;
    }

    return pxThread;
}



void
tn_os_thread_delete(tn_thread_t *pxThread)
{
    vTaskDelete(pxThread->xTaskHandle);
    vPortFree(pxThread);
}



tn_u32
tn_os_current_thread_id(void)
{
    /* Obtain the handle of the current task. */
    TaskHandle_t xHandle = xTaskGetCurrentTaskHandle();

    /* Return the handle instead of the id to avoid enabling trace. */
    return (tn_u32)xHandle;
}



tn_thread_sync_t*
tn_os_thread_sync_init(void)
{
    tn_thread_sync_t *pxTaskToNotify;

    pxTaskToNotify = (tn_thread_sync_t*)pvPortMalloc(sizeof(tn_thread_sync_t));
    if (!pxTaskToNotify)
    {
        return NULL;
    }

    /* Store the handle of the calling task. */
    *pxTaskToNotify = xTaskGetCurrentTaskHandle();

    return pxTaskToNotify;
}



tn_bool
tn_os_thread_sync_wait_timeout(
    tn_thread_sync_t *pxTaskToNotify,
    tn_u32            ulMsecToWait)
{
    tnUNUSED(pxTaskToNotify);

    TickType_t xTicksToWait;

    if (ulMsecToWait != 0)
        xTicksToWait = ulMsecToWait / portTICK_PERIOD_MS;
    else
        xTicksToWait = portMAX_DELAY;

    /* Wait for other task to notify this task. */
    tn_u32 notif_counter = ulTaskNotifyTake(pdTRUE, xTicksToWait);

    return notif_counter > 0 ? tnFALSE : tnTRUE;
}



tn_bool
tn_os_thread_sync_wait_event(
    tn_thread_sync_t *pxTaskToNotify,
    tn_u8             event,
    tn_u32            ulMsecToWait)
{
    tnUNUSED(pxTaskToNotify);
    TickType_t xTicksToWait =  pdMS_TO_TICKS(ulMsecToWait);

    if (ulTaskNotifyTakeIndexed(event, pdTRUE, xTicksToWait) > 0)
    {
        return tnTRUE;
    }
    return tnFALSE;
}



void
tn_os_thread_sync_signal(tn_thread_sync_t *pxTaskToNotify)
{
    /* At this point pxTaskToNotify should not be NULL. */
    configASSERT(pxTaskToNotify != NULL);

    /* Send a notification to the task waiting. */
    xTaskNotifyGive(*pxTaskToNotify);
}



void
tn_os_thread_sync_signal_from_isr(tn_thread_sync_t *pxTaskToNotify)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    /* At this point pxTaskToNotify should not be NULL. */
    configASSERT(pxTaskToNotify != NULL);

    /* Notify the task waiting. */
    vTaskNotifyGiveFromISR(*pxTaskToNotify, &xHigherPriorityTaskWoken);

    /* If xHigherPriorityTaskWoken is now set to pdTRUE then a context switch
       should be performed to ensure the interrupt returns directly to the
       highest priority task. The macro used for this purpose is dependent on
       the port in use and may be called portEND_SWITCHING_ISR(). */
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}



void
tn_os_thread_sync_signal_event_from_isr(
    tn_thread_sync_t *pxTaskToNotify,
    tn_u8             event)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    /* At this point pxTaskToNotify should not be NULL. */
    configASSERT(pxTaskToNotify != NULL);

    /* Notify the task waiting. */
    vTaskNotifyGiveIndexedFromISR(*pxTaskToNotify, event, &xHigherPriorityTaskWoken);

    /* If xHigherPriorityTaskWoken is now set to pdTRUE then a context switch
       should be performed to ensure the interrupt returns directly to the
       highest priority task. The macro used for this purpose is dependent on
       the port in use and may be called portEND_SWITCHING_ISR(). */
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}



void
tn_os_thread_sync_delete(tn_thread_sync_t *pxTaskToNotify)
{
    vPortFree(pxTaskToNotify);
}



void
tn_os_mutex_init(tn_mutex_t *pxMutex)
{
    if (!pxMutex)
    {
        return;
    }

    /* Mutex initialization must be in a critical section to prevent two
       threads from initializing it at the same time. */
    taskENTER_CRITICAL();

    *pxMutex = xSemaphoreCreateMutex();

    /* Ensure that the FreeRTOS mutex was successfully created. */
    configASSERT(*pxMutex != NULL);

    /* Exit the critical section. */
    taskEXIT_CRITICAL();
}



void
tn_os_mutex_acquire(tn_mutex_t *pxMutex)
{
    BaseType_t xMutexTakeStatus = xSemaphoreTake(*pxMutex, portMAX_DELAY);

    configASSERT(xMutexTakeStatus == pdTRUE);
}



void
tn_os_mutex_release(tn_mutex_t *pxMutex)
{
    BaseType_t xMutexGiveStatus = xSemaphoreGive(*pxMutex);

    configASSERT(xMutexGiveStatus == pdTRUE);
}



void
tn_os_mutex_delete(tn_mutex_t *pxMutex)
{
    vSemaphoreDelete(*pxMutex);
    vPortFree(pxMutex);
}



void
tn_os_lock_acquire(tn_os_lock_t *pxLock)
{
    tnUNUSED(pxLock);

    taskENTER_CRITICAL();
}



void
tn_os_lock_release(tn_os_lock_t *pxLock)
{
    tnUNUSED(pxLock);

    taskEXIT_CRITICAL();
}



void vApplicationMallocFailedHook( void );

void vApplicationIdleHook( void );

void vApplicationTickHook( void );

void vApplicationDaemonTaskStartupHook( void );

void vAssertCalled(
    const char *pcFileName,
    uint32_t    ulLine );

void vApplicationGetTimerTaskMemory(
    StaticTask_t **ppxTimerTaskTCBBuffer,
    StackType_t  **ppxTimerTaskStackBuffer,
    uint32_t      *pulTimerTaskStackSize );

void vApplicationStackOverflowHook(
    TaskHandle_t pxTask,
    char        *pcTaskName );

void vApplicationGetIdleTaskMemory(
    StaticTask_t **ppxIdleTaskTCBBuffer,
    StackType_t  **ppxIdleTaskStackBuffer,
    uint32_t      *pulIdleTaskStackSize );

void
vApplicationMallocFailedHook( void )
{
    // vApplicationMallocFailedHook() will only be called if
    // configUSE_MALLOC_FAILED_HOOK is set to 1 in FreeRTOSConfig.h.  It is a hook
    // function that will get called if a call to pvPortMalloc() fails.
    // pvPortMalloc() is called internally by the kernel whenever a task, queue,
    // timer or semaphore is created using the dynamic allocation (as opposed to
    // static allocation) option.  It is also called by various parts of the
    // demo application.  If heap_1.c, heap_2.c or heap_4.c is being used, then the
    // size of the  heap available to pvPortMalloc() is defined by
    // configTOTAL_HEAP_SIZE in FreeRTOSConfig.h, and the xPortGetFreeHeapSize()
    // API function can be used to query the size of free heap space that remains
    // (although it does not provide information on how the remaining heap might be
    // fragmented).  See http://www.freertos.org/a00111.html for more
    // information.
    tnTRACE( "\r\n\r\nMalloc failed\r\n" );
    portDISABLE_INTERRUPTS();

    // coverity[infinite_loop]
    for ( ; ; )
    {
    }
}



void
vApplicationIdleHook( void )
{
    // vApplicationIdleHook() will only be called if configUSE_IDLE_HOOK is set
    // to 1 in FreeRTOSConfig.h.  It will be called on each iteration of the idle
    // task.  It is essential that code added to this hook function never attempts
    // to block in any way (for example, call xQueueReceive() with a block time
    // specified, or call vTaskDelay()).  If application tasks make use of the
    // vTaskDelete() API function to delete themselves then it is also important
    // that vApplicationIdleHook() is permitted to return to its calling function,
    // because it is the responsibility of the idle task to clean up memory
    // allocated by the kernel to any task that has since deleted itself.
}



void
vApplicationStackOverflowHook(
    TaskHandle_t pxTask,
    char        *pcTaskName )
{
    ( void )pcTaskName;
    ( void )pxTask;

    // Run time stack overflow checking is performed if
    // configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2.  This hook
    // function is called if a stack overflow is detected.
    tnTRACE( "\r\n\r\nStack overflow in %s\r\n", pcTaskName );
    portDISABLE_INTERRUPTS();

    // coverity[infinite_loop]
    for ( ; ; )
    {
    }
}



void
vApplicationTickHook( void )
{
    // This function will be called by each tick interrupt if
    // configUSE_TICK_HOOK is set to 1 in FreeRTOSConfig.h.  User code can be
    // added here, but the tick hook is called from an interrupt context, so
    // code must not attempt to block, and only the interrupt safe FreeRTOS API
    // functions can be used (those that end in FromISR()).
}



void
vApplicationDaemonTaskStartupHook( void )
{
    // This function will be called once only, when the daemon task starts to
    // execute (sometimes called the timer task).  This is useful if the
    // application includes initialisation code that would benefit from executing
    // after the scheduler has been started.
}



void
vAssertCalled(
    const char *pcFileName,
    uint32_t    ulLine )
{
    volatile uint32_t ulSetToNonZeroInDebuggerToContinue = 0;

    // Called if an assertion passed to configASSERT() fails.  See
    // http://www.freertos.org/a00110.html#configASSERT for more information.

    tnTRACE( "ASSERT! Line %d, file %s\r\n", ( int )ulLine, pcFileName );

    taskENTER_CRITICAL();
    {
        // You can step out of this function to debug the assertion by using
        // the debugger to set ulSetToNonZeroInDebuggerToContinue to a non-zero
        // value.
        while (ulSetToNonZeroInDebuggerToContinue == 0)
        {
            __asm volatile ( "NOP" );
            __asm volatile ( "NOP" );
        }
    }
    taskEXIT_CRITICAL();
}



// configUSE_STATIC_ALLOCATION is set to 1, so the application must provide an
// implementation of vApplicationGetIdleTaskMemory() to provide the memory that is
// used by the Idle task.
void
vApplicationGetIdleTaskMemory(
    StaticTask_t **ppxIdleTaskTCBBuffer,
    StackType_t  **ppxIdleTaskStackBuffer,
    uint32_t      *pulIdleTaskStackSize )
{
    // If the buffers to be provided to the Idle task are declared inside this
    // function then they must be declared static - otherwise they will be allocated on
    // the stack and so not exists after this function exits.
    static StaticTask_t xIdleTaskTCB;
    static StackType_t uxIdleTaskStack[ configMINIMAL_STACK_SIZE ];

    // Pass out a pointer to the StaticTask_t structure in which the Idle task's
    // state will be stored.
    *ppxIdleTaskTCBBuffer = &xIdleTaskTCB;

    // Pass out the array that will be used as the Idle task's stack.
    *ppxIdleTaskStackBuffer = uxIdleTaskStack;

    // Pass out the size of the array pointed to by *ppxIdleTaskStackBuffer.
    // Note that, as the array is necessarily of type StackType_t,
    // configMINIMAL_STACK_SIZE is specified in words, not bytes.
    *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}



// configUSE_STATIC_ALLOCATION and configUSE_TIMERS are both set to 1, so the
// application must provide an implementation of vApplicationGetTimerTaskMemory()
// to provide the memory that is used by the Timer service task.
void
vApplicationGetTimerTaskMemory(
    StaticTask_t **ppxTimerTaskTCBBuffer,
    StackType_t  **ppxTimerTaskStackBuffer,
    uint32_t      *pulTimerTaskStackSize )
{
// If the buffers to be provided to the Timer task are declared inside this
// function then they must be declared static - otherwise they will be allocated on
// the stack and so not exists after this function exits.
    static StaticTask_t xTimerTaskTCB;
    static StackType_t uxTimerTaskStack[ configTIMER_TASK_STACK_DEPTH ];

    // Pass out a pointer to the StaticTask_t structure in which the Timer
    // task's state will be stored.
    *ppxTimerTaskTCBBuffer = &xTimerTaskTCB;

    // Pass out the array that will be used as the Timer task's stack.
    *ppxTimerTaskStackBuffer = uxTimerTaskStack;

    // Pass out the size of the array pointed to by *ppxTimerTaskStackBuffer.
    // Note that, as the array is necessarily of type StackType_t,
    // configMINIMAL_STACK_SIZE is specified in words, not bytes.
    *pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
}
