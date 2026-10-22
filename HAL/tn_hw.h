//
// Copyright 2025-2026 NXP
//
// SPDX-License-Identifier: MIT
//

#ifndef   __TN_HW_H__
#define   __TN_HW_H__

#include "tn_types.h"
#include "tn_gpu_regs.h"

typedef enum
{
    TN_HW_OS_EVENT_FLUSH = 0,
    TN_HW_OS_EVENT_FINISH = 1
} tn_hw_os_event;

/// Write a register directly in HW
/// \param reg register offset
/// \param val register value
void tn_hw_reg_set(
    tn_u32 reg,
    tn_u32 val);

/// Read register value directly from HW
/// \param reg register offset
tn_u32 tn_hw_reg_get(tn_u32 reg);

/// Return the wall-clock time in milliseconds elapsed since tn_hw_init().
/// Writes *ms and returns tnTRUE on Cortex-M targets; returns tnFALSE
/// (running time not supported) on all other targets.
/// \param ms out: elapsed time in milliseconds
tn_bool tn_hw_get_running_time_ms(tn_u32 *ms);

tn_result tn_hw_init(void);

tn_result tn_hw_deinit(void);

void tn_hw_ccdma_set_wait_delay(tn_u16 delay);

void tn_hw_ccdma_start(tn_paddr phy_addr);

tn_bool tn_hw_ccdma_has_pending_cmdqs(void);

void tn_hw_ccdma_set_submit_index(tn_size submitted_index);

tn_size tn_hw_get_ccdma_flushed_index(void);

tn_result tn_hw_wait_for_event(tn_hw_os_event event);

void tn_hw_handle_irq_error(tn_u32 type);

tn_result tn_hw_handle_irq_timeout(void);

void tn_hw_irq_handler(void);

#endif // __TN_HW_H__
