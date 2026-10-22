//
// Copyright 2025-2026 NXP
//
// SPDX-License-Identifier: MIT
//

#include "tn_hw.h"
#include "tn_os.h"
#include "tn_barrier.h"
#include "tn_regs_image.h"
#include "tn_gpu_regs.h"
#include "tn_cmd_buf_ccdma.h"
#ifdef TN_PROF
    #include "tn_prof.h"
#endif


#define TIN_GPU_FRAME_TIMEOUT 100000000
#define TIN_GPU_RESET_RETRIES 100000
#define TN_WRITE_GPU_REG32(offset, value) (*((volatile unsigned long*)(TIN_GPU_BASE + (offset)))) = (value)
#define TN_READ_GPU_REG32(offset) (*((volatile unsigned long*)(TIN_GPU_BASE + (offset))))


tn_thread_sync_t *thread_sync;

static tn_os_lock_t hw_lock;
static volatile tn_size last_ccdma_flushed_index = 0;
static volatile tn_size last_ccdma_submitted_index = 0;

#if defined(TN_PROF) && TN_ARCH_CORTEX_M
// Free-running cycle-counter value captured at tn_hw_init(), used as the
// reference point for tn_hw_get_running_time_ms().
static tn_u64 hw_start_ticks = 0;
#endif


void
tn_hw_reg_set(
    tn_u32 reg,
    tn_u32 val)
{
    tnIN_ARG("reg:x%x val:x%x", reg, val);

    TN_WRITE_GPU_REG32(reg, val);

    tnOUT();
}



tn_u32
tn_hw_reg_get(tn_u32 reg)
{
    tnIN();

    tn_u32 val = TN_READ_GPU_REG32(reg);

    tnOUT_ARG("reg:x%x val:x%x", reg, val);
    return val;
}



tn_result
tn_hw_init(void)
{
    tnIN();

    last_ccdma_submitted_index = 0;
    last_ccdma_flushed_index = 0;

    // Reset the GPU. The SW_RST bit is self-clearing
    tn_hw_reg_set(TINGPU_GPU_CTRL, TINGPU_GPU_CTRL_SW_RST(1));

    // Make sure the reset request has reached the GPU before polling for its
    // completion, otherwise the first read could be served ahead of the write.
    tn_barrier_dsb();

    tn_u32 reset_retries = TIN_GPU_RESET_RETRIES;
    while ((tn_hw_reg_get(TINGPU_GPU_CTRL) & TINGPU_GPU_CTRL_SW_RST_MASK) != 0)
    {
        if (--reset_retries == 0)
        {
            tnOUT();
            return TN_TIMEOUT_ERROR;
        }
    }

    thread_sync = tn_os_thread_sync_init();
    if (thread_sync == tnNULL)
    {
        tnOUT();
        return TN_OUT_OF_MEMORY_ERROR;
    }

    //enable CCDMA EVENT IRQ
    tn_hw_reg_set(TINGPU_INTEN, TINGPU_INTEN_CCDMA_EVENT_EN(1));

    //enable all AXI cache bits for all modules
    tn_hw_reg_set(TINGPU_AXI_CACHE_CTRL, 0xffffffff);

#if defined(TN_PROF)
    tn_ticks_enable();
    #if TN_ARCH_CORTEX_M
    hw_start_ticks = tn_ticks();
    #endif
#endif

    tnOUT();
    return TN_NO_ERROR;
}



// Return the wall-clock time in milliseconds elapsed since tn_hw_init().
// Only Cortex-M targets expose a usable core-clock cycle counter (DWT->CYCCNT)
// together with a known core-clock frequency (TN_CONFIG_CPU_CLOCK_HZ); on those the
// value is written to *ms and tnTRUE is returned. On every other target running
// time is not supported, so the function returns tnFALSE and leaves *ms
// untouched.
tn_bool
tn_hw_get_running_time_ms(tn_u32 *ms)
{
#if defined(TN_PROF) && TN_ARCH_CORTEX_M
    tn_u64 elapsed = tn_ticks() - hw_start_ticks;
    tn_u32 hz = (tn_u32)TN_CONFIG_CPU_CLOCK_HZ;
    if (hz == 0)
    {
        return tnFALSE;
    }
    *ms = (tn_u32)((elapsed * 1000ULL) / hz);
    return tnTRUE;
#else
    tnUNUSED(ms);
    return tnFALSE;
#endif
}



tn_result
tn_hw_deinit(void)
{
    tnIN();

    //disable all interrupts
    tn_hw_reg_set(TINGPU_INTEN, 0);

    //disable CCDMA
    tn_hw_reg_set(TINGPU_CCDMA_CTRL0, 0);

    if (thread_sync != tnNULL)
    {
        tn_os_thread_sync_delete(thread_sync);
        thread_sync = tnNULL;
    }

    tnOUT();
    return TN_NO_ERROR;
}



tn_result
tn_hw_wait_for_event(tn_hw_os_event event)
{
    tnIN_ARG("event:x%x", event);

    tn_bool event_received = tn_os_thread_sync_wait_event(thread_sync, (tn_u8)event, TIN_GPU_FRAME_TIMEOUT);

    if (event_received)
    {
        tnOUT();
        return TN_NO_ERROR;
    }
    else
    {
        tnOUT();
        return tn_hw_handle_irq_timeout();
    }
}



void
tn_hw_ccdma_set_wait_delay(tn_u16 delay)
{
    tnIN_ARG("delay:%d", delay);

    tn_hw_reg_set(TINGPU_CCDMA_WAIT, delay & TINGPU_CCDMA_WAIT_DELAY_MASK);

    tnOUT();
}



void
tn_hw_ccdma_start(tn_paddr phy_addr)
{
    tnIN();

    tn_hw_reg_set(TINGPU_CCDMA_CTRL0,
                  TINGPU_CCDMA_CTRL0_ENA(1) | (TINGPU_CCDMA_CTRL0_ADDR((phy_addr) >> 3)));

    tnOUT();
}



tn_bool
tn_hw_ccdma_has_pending_cmdqs(void)
{
    tnIN();

    tn_bool has_pending;
    tn_os_lock_acquire(&hw_lock);
    has_pending = (last_ccdma_submitted_index != last_ccdma_flushed_index) ? tnTRUE : tnFALSE;
    tn_os_lock_release(&hw_lock);

    tnOUT_ARG("has_pending:%d", has_pending);
    return has_pending;
}



void
tn_hw_ccdma_set_submit_index(tn_size submitted_index)
{
    tnIN_ARG("submitted_index:x%x", submitted_index);

    tn_os_lock_acquire(&hw_lock);
    last_ccdma_submitted_index = submitted_index & TN_EVENT_PAYLOAD_MASK;
    tn_os_lock_release(&hw_lock);

    tnOUT_ARG("last_ccdma_submitted_index:x%x", last_ccdma_submitted_index);
}



tn_size
tn_hw_get_ccdma_flushed_index(void)
{
    tnIN();

    tnOUT_ARG("last_ccdma_flushed_index:x%x", last_ccdma_flushed_index);
    return last_ccdma_flushed_index;
}



void
tn_hw_handle_irq_error(tn_u32 type)
{

}



tn_result
tn_hw_handle_irq_timeout(void)
{
    return TN_TIMEOUT_ERROR;
}



static inline void
update_last_flushed_index(const tn_size event)
{
    tnIN_ARG("event:x%x", event);

    last_ccdma_flushed_index =  event & TN_EVENT_PAYLOAD_MASK;

    tnOUT_ARG("last_ccdma_flushed_index:x%x", last_ccdma_flushed_index);
}



void
tn_hw_irq_handler(void)
{
    //check for interrupt type
    uint32_t irq_type = tn_hw_reg_get(TINGPU_INTR);

    if (irq_type & TINGPU_INTR_CCDMA_EVENT_MASK)
    {
        //W1C the event bit
        tn_hw_reg_set(TINGPU_INTR, irq_type | TINGPU_INTR_CCDMA_EVENT_MASK);

        uint32_t event = tn_hw_reg_get(TINGPU_CCDMA_EVT);
        if (tn_ccdma_event_is_finish(event))
        {
            tn_os_thread_sync_signal_event_from_isr(thread_sync, TN_HW_OS_EVENT_FINISH);
            update_last_flushed_index(event);
        }
        else if (tn_ccdma_event_is_flush(event))
        {
            tn_os_thread_sync_signal_event_from_isr(thread_sync, TN_HW_OS_EVENT_FLUSH);
            update_last_flushed_index(event);
        }
    }
    else if (irq_type & TINGPU_INTR_IMP_OP_DONE_MASK)
    {
        //W1C the fd_irq bit
        tn_hw_reg_set(TINGPU_INTR, irq_type | TINGPU_INTR_IMP_OP_DONE_MASK);

        // ignore FD IRQs, no useful case found till now

    }
    else   /*error handling*/
    {
        // TODO Check with safety team what should be the behavior for error handling
        // TINGPU_INTR_CCDMA_CMD_ERR_MASK
        // TINGPU_INTR_AXI_ERR_MASK
        // TINGPU_INTR_CTO_ERR_MASK
        // TINGPU_INTR_VG_EVENT_MASK
        // TINGPU_INTR_DST_ERR_MASK
        // TINGPU_INTR_S1_ERR_MASK
        // TINGPU_INTR_S2_ERR_MASK
        // TINGPU_INTR_MASK_ERR_MASK
        tn_hw_handle_irq_error(irq_type);
    }
}
