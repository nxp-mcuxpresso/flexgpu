//
// Copyright 2025-2026 NXP
//
// SPDX-License-Identifier: MIT
//

#ifndef __TN_TICKS_CORTEX_M__
#define __TN_TICKS_CORTEX_M__


#include "tn_types.h"
#include "tn_aver.h"
#include "cmsis.h"


static inline void
tn_ticks_enable(void)
{
    if (CoreDebug_DEMCR_TRCENA_Msk != (CoreDebug_DEMCR_TRCENA_Msk & CoreDebug->DEMCR))
    {
        // Enable the DWT trace
        CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    }

    // CYCCNT not supported on this device.
    tnEXPECT(DWT_CTRL_NOCYCCNT_Msk != (DWT->CTRL & DWT_CTRL_NOCYCCNT_Msk));

    // Read CYCCNT directly if CYCCENA has already been enabled, otherwise enable CYCCENT first.
    if (DWT_CTRL_CYCCNTENA_Msk != (DWT_CTRL_CYCCNTENA_Msk & DWT->CTRL))
    {
        DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
    }
}



static inline tn_u32
tn_ticks_check_overflow(void)
{
    static tn_u32 overflow_cnt = 0;
    static tn_u32 last_cyccnt = 0;
    tn_u32 current_cyccnt;

    // Disable interrupts to ensure atomic operation
    tn_u32 primask = __get_PRIMASK();
    __disable_irq();

    current_cyccnt = DWT->CYCCNT;

    if (current_cyccnt < last_cyccnt)
    {
        overflow_cnt++;
    }

    last_cyccnt = current_cyccnt;

    // Restore interrupt state
    __set_PRIMASK(primask);

    return overflow_cnt;
}



static inline tn_u64
tn_ticks(void)
{
    tn_u32 overflow_cnt = tn_ticks_check_overflow();
    return ((tn_u64)overflow_cnt << 32) | DWT->CYCCNT;
}



static inline tn_u32
tn_ticks_check_sleep_overflow(void)
{
    static tn_u32 sleep_overflow_cnt = 0;
    static tn_u32 last_sleepcnt = 0;
    tn_u32 current_sleepcnt;

    // Disable interrupts to ensure atomic operation
    tn_u32 primask = __get_PRIMASK();
    __disable_irq();

    current_sleepcnt = DWT->SLEEPCNT;

    if (current_sleepcnt < last_sleepcnt)
    {
        sleep_overflow_cnt++;
    }

    last_sleepcnt = current_sleepcnt;

    // Restore interrupt state
    __set_PRIMASK(primask);

    return sleep_overflow_cnt;
}



static inline tn_u64
tn_ticks_sleep(void)
{
    tn_u32 overflow_cnt = tn_ticks_check_sleep_overflow();
    return ((tn_u64)overflow_cnt << 32) | DWT->SLEEPCNT;
}



#endif // __TN_TICKS_CORTEX_M__
