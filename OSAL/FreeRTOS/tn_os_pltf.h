//
// Copyright 2025-2026 NXP
//
// SPDX-License-Identifier: MIT
//

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#define TN_CONFIG_STACK_SIZE   (configMINIMAL_STACK_SIZE * sizeof(StackType_t))

/* Core-clock frequency in Hz, exposed to the driver through the OS layer so it
   never has to reach into FreeRTOSConfig.h directly. */
#define TN_CONFIG_CPU_CLOCK_HZ   configCPU_CLOCK_HZ

typedef struct tn_thread_t
{
    int (*pvStartRoutine)(void*);        /**< Application thread function. */
    void          *xTaskArg;              /**< Arguments for application thread function. */
    TaskHandle_t   xTaskHandle;           /**< FreeRTOS task handle. */
} tn_thread_t;

/* Handle of the task to notify. */
typedef TaskHandle_t        tn_thread_sync_t;

/* FreeRTOS mutex. */
typedef SemaphoreHandle_t   tn_mutex_t;

/* taskENTER_CRITICAL() masks interrupts globally, so the lock itself carries no
   state. Kept as a struct so the type is not void. */
typedef struct tn_os_lock_t
{
    char   unused;
} tn_os_lock_t;
