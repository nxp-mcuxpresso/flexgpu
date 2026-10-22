//
// Copyright 2026 NXP
//
// SPDX-License-Identifier: MIT
//

#include "tn_types.h"
#include "tn_result.h"
#include "tn_aver.h"
#include "tn_regs_image.h"
#include "tn_gpu_regs.h"
#include "tn_hw.h"
#include "tn_cmdq_ccdma.h"
#include "tn_sync.h"
#include "tn_cmd_buf_ccdma.h"
#include "tn_mbuf.h"
#include "tn_mem.h"
#include "tn_m3f32.h"


#define TIN_CMD_END         0x0
#define TIN_CMD_WAIT        0x1
#define TIN_CMD_LINK        0x2
#define TIN_CMD_EVENT       0x3
#define TIN_CMD_RUN         0x4
#define TIN_CMD_RESERVED_5  0x5
#define TIN_CMD_RESERVED_6  0x6
#define TIN_CMD_RESERVED_7  0x7
#define TIN_CMD_LOADNREG    0x8
#define TIN_CMD_SAVENREG    0x9
#define TIN_CMD_LOADNIREG   0xA
#define TIN_CMD_SAVENIREG   0xB
#define TIN_CMD_RESERVED_C  0xC
#define TIN_CMD_RESERVED_D  0xD
#define TIN_CMD_RESERVED_E  0xE
#define TIN_CMD_LOAD1REG    0xF

#define TIN_WAIT_DELAY     0x10 // TODO: is 16 cycles a proper delay value?

#define TIN_CMD_RUN_IMG_FLAG    (1 << 5)
#define TIN_CMD_RUN_PAUSE_FLAG  (1 << 6)

#define MINIMUM_BUFFER_SIZE  (256)  // 256 elements, each 8 bytes
#define DEFAULT_BUFFER_SIZE (4096) // default buffer size is 32k (4096 entries)

static tn_u32 _current_cmd_buf_size = 0;
static tn_size _prev_link_cmd_index = 1;
static tn_size _curr_wait_cmd_index = 0;
static tn_size _curr_link_cmd_index = 1;

static tn_bool _new_cmds_ready = tnFALSE;
static tn_bool _ccdma_initialized = tnFALSE;

// --- Cache command buffer ---
// Each cached slot owns a dynamically-sized mbuf that holds a flat sequence
// of CCDMA entries (no WAIT/LINK loop).  The slot grows by a factor of
// CACHE_SLOT_GROW_FACTOR whenever it is full during recording.

#define CACHE_CMD_BUF_MAX_SLOTS    8    // maximum number of cached slots
#define CACHE_SLOT_INITIAL_ENTRIES 256  // initial allocation per slot (2 KB)
#define CACHE_SLOT_GROW_FACTOR     2
#define CACHE_SLOT_BUF_ALIGN       8    // byte alignment required by CCDMA DMA

// Sentinel value meaning "index not recorded".
#define CACHE_IDX_NONE  (~0U)

typedef struct
{
    tn_mbuf                       mbuf; // buffer holding the flat CCDMA entry sequence
                                        // mbuf.vaddr: virtual address of the buffer
                                        // mbuf.paddr: physical address used by the CCDMA
    tn_u32                        write_offset; // next free entry index (used during recording)
    tn_u32                        num_entries; // sealed entry count (set by CACHE_END)
    tn_u32                        capacity; // currently allocated entry count
    // Entry index (inside the flat buffer) of the TINGPU_S2_TRANSFORM_CTRL
    // write recorded during START..END.  CACHE_IDX_NONE if none was recorded.
    tn_u32                        xform_ctrl_idx;
    // Entry index of the TINGPU_S2_PROJ_TF0 write.  TF1..TF8 follow at
    // xform_tf0_idx+1 .. xform_tf0_idx+8 (guaranteed sequential by the
    // single tn_cmd_buf_ccdma_loadNamedRegValues call in tn_regs_set_xform).
    // CACHE_IDX_NONE if no xform write was recorded.
    tn_u32                        xform_tf0_idx;
    // Entry indices of the DST / S1 size and base-address writes.  These
    // register payloads are derived from the matrix-dependent blend area at
    // blit time, so they must be re-patched whenever EXECUTE injects a new
    // transform.  CACHE_IDX_NONE if the corresponding write was not recorded.
    tn_u32                        dst_size_idx;
    tn_u32                        dst_addr0_idx;
    tn_u32                        s1_size_idx;
    tn_u32                        s1_addr0_idx;
    // Blit geometry captured by tn_cmd_buf_ccdma_cache_note_blit_geometry().
    // Used at EXECUTE time to re-derive the blend area (and hence the DST/S1
    // registers above) for the injected matrix.  geo.injectable is tnFALSE
    // when the recorded blit is not a simple affine RGB blit.
    tn_cmd_buf_cache_blit_geo_t   geo;
} tn_cache_cmd_slot_t;


static tn_cache_cmd_slot_t _cache_slots[CACHE_CMD_BUF_MAX_SLOTS];
static tn_u32 _cache_num_slots = 0;
static tn_bool _cache_recording = tnFALSE;
static tn_u32 _cache_recording_slot_idx = 0;

static tn_result cache_slot_shadow_enq(
    tn_u32 cmd,
    tn_u32 payload);

static void cache_clear(void);

// This function is optional.
// If used, call it before vg_lite_init if you want to change the command buffer size.
// Default value is 32K.
tn_result
tn_cmd_buf_ccdma_set_buffer_size(tn_u32 size)
{
    if (size < (MINIMUM_BUFFER_SIZE << 3) || (size & 0x7U) != 0)
    {
        return TN_ILLEGAL_ARGUMENT_ERROR;
    }

    _current_cmd_buf_size = size >> 3;

    return TN_NO_ERROR;
}



static void
add_loop(void)
{
    _curr_wait_cmd_index = tn_cmdq_ccdma_enq(TIN_CMD_WAIT, 0);
    tn_u32 curr_wait_cmd_addr = (tn_u32)tn_cmdq_ccdma_get_addr(_curr_wait_cmd_index);
    _curr_link_cmd_index = tn_cmdq_ccdma_enq(TIN_CMD_LINK, curr_wait_cmd_addr);
}



static void
break_prev_loop(void)
{
    tn_size next_index = tn_cmdq_ccdma_get_next_index(_prev_link_cmd_index);
    tn_u32 next_addr = (tn_u32)tn_cmdq_ccdma_get_addr(next_index);
    tn_cmdq_ccdma_write_cmd(_prev_link_cmd_index, TIN_CMD_LINK, next_addr);
    _prev_link_cmd_index = _curr_link_cmd_index;
}



static tn_result
wait_for_finish(void)
{
    return tn_hw_wait_for_event(TN_HW_OS_EVENT_FINISH);
}



static tn_result
wait_for_flush(void)
{
    return tn_hw_wait_for_event(TN_HW_OS_EVENT_FLUSH);
}



static tn_result
submit(tn_u32 event)
{
    tnIN();

    tn_result result;
    // check if there is enough space for
    // one EVENT and one LOOP (CMD_WAIT + CMD_LINK)
    tn_cmdq_ccdma_update_start(tn_hw_get_ccdma_flushed_index());
    if (tn_cmdq_ccdma_get_free_space() < 3)
    {
        tn_bool has_pending = tn_hw_ccdma_has_pending_cmdqs();
        if (!has_pending)
        {
            tnABORT(TN_FAULT_INTERNAL, "CCDMA buffer is full");
        }
        else
        {
            result = wait_for_flush();
            if (result != TN_NO_ERROR)
            {
                tnABORT(TN_FAULT_INTERNAL, "CCDMA timeout");
            }
        }
    }

    tn_size next_start = tn_cmdq_ccdma_get_next_index(tn_cmdq_ccdma_get_end() + 3 - 1);
    event |= (next_start & TN_EVENT_PAYLOAD_MASK);
    (void)tn_cmdq_ccdma_enq(TIN_CMD_EVENT, event);
    tn_hw_ccdma_set_submit_index(next_start);
    add_loop();
    break_prev_loop();

    tnOUT();
    return TN_NO_ERROR;
}



// Submit an IMG_FLUSH event and return without waiting.  Skips the submit
// when no new commands have been queued since the last flush.
tn_result
tn_cmd_buf_ccdma_flush(void)
{
    tnIN();

    tn_result result;

    if (_new_cmds_ready == tnFALSE)
    {
        tnOUT();
        return TN_NO_ERROR;
    }

    result = submit(TN_CCDMA_EVENT_IMG_FLUSH);
    if (result != TN_NO_ERROR)
    {
        tnABORT(TN_FAULT_INTERNAL, "CCDMA submit error");
    }

    _new_cmds_ready = tnFALSE;

    tnOUT();
    return TN_NO_ERROR;
}



// Submit an IMG_FINISH event and block until the GPU is idle.  Skips the
// submit when no new commands have been queued and the HW has no pending work.
tn_result
tn_cmd_buf_ccdma_finish(void)
{
    tnIN();

    tn_result result;

    if (_new_cmds_ready == tnFALSE && !tn_hw_ccdma_has_pending_cmdqs())
    {
        tnOUT();
        return TN_NO_ERROR;
    }

    result = submit(TN_CCDMA_EVENT_IMG_FINISH);
    if (result != TN_NO_ERROR)
    {
        tnABORT(TN_FAULT_INTERNAL, "CCDMA submit error");
    }

    result = wait_for_finish();
    if (result != TN_NO_ERROR)
    {
        tnABORT(TN_FAULT_INTERNAL, "CCDMA finish error");
    }

    _new_cmds_ready = tnFALSE;

    tnOUT();
    return TN_NO_ERROR;
}



// Submit a FRAME_FLUSH event unconditionally, regardless of _new_cmds_ready.
// Unlike do_flush, the submit is never skipped: the FRAME_FLUSH event must
// always appear on the HW event stream so that consumers can detect every
// frame boundary, even when the frame contained no GPU work (e.g.
// vg_lite_frame_delimiter called right after a prior vg_lite_flush with no
// intervening draw calls).
tn_result
tn_cmd_buf_ccdma_frame_flush(void)
{
    tnIN();

    tn_result result = submit(TN_CCDMA_EVENT_FRAME_FLUSH);
    if (result != TN_NO_ERROR)
    {
        tnABORT(TN_FAULT_INTERNAL, "CCDMA submit error");
    }

    _new_cmds_ready = tnFALSE;

    tnOUT();
    return TN_NO_ERROR;
}



// Submit a FRAME_FLUSH event unconditionally and block until the GPU is idle.
// Implemented as frame_flush + wait so no separate FRAME_FINISH event token
// is needed.
tn_result
tn_cmd_buf_ccdma_frame_finish(void)
{
    tnIN();

    tn_result result = tn_cmd_buf_ccdma_frame_flush();
    if (result != TN_NO_ERROR)
    {
        tnOUT();
        return result;
    }

    result = wait_for_finish();
    if (result != TN_NO_ERROR)
    {
        tnABORT(TN_FAULT_INTERNAL, "CCDMA finish error");
    }

    tnOUT();
    return TN_NO_ERROR;
}



// Write one CCDMA entry (8 bytes: cmd word + payload word) into the cache slot
// that is currently being recorded.  Grows the slot's backing buffer when full.
static tn_result
cache_slot_shadow_enq(
    tn_u32 cmd,
    tn_u32 payload)
{
    tn_cache_cmd_slot_t *slot = &_cache_slots[_cache_recording_slot_idx];

    tnRET_IF(tn_is_null(slot->mbuf.vaddr), TN_BAD_HANDLE_ERROR);

    if (slot->write_offset >= slot->capacity)
    {
        // Double the allocation to keep amortised cost O(1) per entry.
        tn_u32 new_capacity = slot->capacity * CACHE_SLOT_GROW_FACTOR;
        tn_mbuf new_mbuf = {0};
        tn_result rv = tn_mbuf_alloc_aligned(&new_mbuf, CACHE_SLOT_BUF_ALIGN,
                                             new_capacity * 8U);
        if (rv != TN_NO_ERROR)
        {
            return TN_OUT_OF_MEMORY_ERROR;
        }

        // copy existing entries to the larger buffer
        tn_memcpy(new_mbuf.vaddr, slot->mbuf.vaddr, slot->write_offset * 8U);

        tn_mbuf_release(&slot->mbuf);

        slot->mbuf     = new_mbuf;
        slot->capacity = new_capacity;
    }

    // Capture the flat-buffer index of the transform control and TF0 register
    // writes so that cache_inject_matrix() can patch them in O(1) at execute
    // time without scanning the whole buffer.
    if ((cmd & 0xFU) == TIN_CMD_LOAD1REG)
    {
        tn_u32 reg = (cmd >> 5) << 2;
        if (reg == TINGPU_S2_TRANSFORM_CTRL)
        {
            // last-write wins in case the caller issues it more than once
            slot->xform_ctrl_idx = slot->write_offset;
        }
        else if (reg == TINGPU_S2_PROJ_TF0)
        {
            slot->xform_tf0_idx = slot->write_offset;
        }
        else if (reg == TINGPU_DST_SIZE)
        {
            slot->dst_size_idx = slot->write_offset;
        }
        else if (reg == TINGPU_DST_ADDR0)
        {
            slot->dst_addr0_idx = slot->write_offset;
        }
        else if (reg == TINGPU_S1_SIZE)
        {
            slot->s1_size_idx = slot->write_offset;
        }
        else if (reg == TINGPU_S1_ADDR0)
        {
            slot->s1_addr0_idx = slot->write_offset;
        }
    }


    // Each entry is two consecutive 32-bit words: [cmd | payload].
    tn_u32 *buf = (tn_u32*)(tn_uptr)slot->mbuf.vaddr;
    buf[slot->write_offset * 2U]      = cmd;
    buf[slot->write_offset * 2U + 1U] = payload;
    slot->write_offset++;

    return TN_NO_ERROR;
}



static tn_result
cmd_buf_ccdma_safe_enq(
    tn_u32 cmd,
    tn_u32 payload)
{
    tnIN();

    if (tn_cmdq_ccdma_is_full())
    {
        tn_bool has_pending = tn_hw_ccdma_has_pending_cmdqs();
        tn_cmdq_ccdma_update_start(tn_hw_get_ccdma_flushed_index());
        if (tn_cmdq_ccdma_is_full() && has_pending)
        {
            tn_result result = wait_for_flush();
            if (result != TN_NO_ERROR)
            {
                tnLOG_ERR("Flush error");

                tnOUT();
                return result;
            }
            tn_cmdq_ccdma_update_start(tn_hw_get_ccdma_flushed_index());
        }
        else if (tn_cmdq_ccdma_is_full() && !has_pending)
        {
            tnLOG_ERR("Buffer is full");

            tnOUT();
            return TN_BUFFER_FULL_ERROR;
        }
    }

    _new_cmds_ready = tnTRUE;

    tn_cmdq_ccdma_enq(cmd, payload);

    // While recording, shadow every command into the cache slot so it can
    // be replayed later without re-issuing all draw calls.
    if (_cache_recording)
    {
        tn_result shadow_result = cache_slot_shadow_enq(cmd, payload);
        if (shadow_result != TN_NO_ERROR)
        {
            tnOUT();
            return shadow_result;
        }
    }

    tnOUT();
    return TN_NO_ERROR;
}



tn_result
tn_cmd_buf_ccdma_enq_event(tn_u32 event)
{
    return cmd_buf_ccdma_safe_enq(TIN_CMD_EVENT, event);
}



tn_result
tn_cmd_buf_ccdma_enable_gpu_ctrl()
{
    const tn_u32 cmd = TIN_CMD_RUN | TIN_CMD_RUN_IMG_FLAG | TIN_CMD_RUN_PAUSE_FLAG;
    const tn_u32 payload = 0;

    return cmd_buf_ccdma_safe_enq(cmd, payload);
}



tn_result
tn_cmd_buf_ccdma_load_reg(
    tn_u32 reg_offset,
    tn_u32 reg_value)
{
    const tn_u32 cmd = TIN_CMD_LOAD1REG | ((reg_offset >> 2) << 5);
    return cmd_buf_ccdma_safe_enq(cmd, reg_value);
}



tn_result
tn_cmd_buf_ccdma_loadNamedRegValues(
    tn_u32  count,
    tn_u32 *reg_offsets,
    tn_u32 *reg_values)
{
    tnIN();

    if (count == 0)
    {
        tnOUT();
        return TN_NO_ERROR;
    }
    if (reg_offsets == tnNULL || reg_values == tnNULL)
    {
        tnOUT();
        return TN_ILLEGAL_ARGUMENT_ERROR;
    }

    for (tn_u32 num_regs = 0; num_regs < count; num_regs++)
    {
        tn_result result = tn_cmd_buf_ccdma_load_reg(reg_offsets[num_regs], reg_values[num_regs]);
        if (result != TN_NO_ERROR)
        {
            tnOUT();
            return result;
        }
    }

    tnOUT();
    return TN_NO_ERROR;
}



//tn_init.h
tn_result
tn_cmd_buf_ccdma_init(
    tn_u32 physical,
    tn_u32 size)
{
    tnIN();

    tn_result result;

    tn_cmdq_ccdma_init();

    // the size set by the user takes priority
    if (_current_cmd_buf_size == 0)
    {
        if (size == 0)
        {
            _current_cmd_buf_size = DEFAULT_BUFFER_SIZE;
        }
        else
        {
            _current_cmd_buf_size = (size >> 3);
        }
    }

    if (physical == 0) //internal buffer
    {
        result = tn_cmdq_ccdma_create(0, _current_cmd_buf_size);
    }
    else
    {
        result = tn_cmdq_ccdma_create(physical, _current_cmd_buf_size);
    }
    if (result != TN_NO_ERROR)
    {
        tnLOG_ERR("cmdq create error");

        tnOUT();
        return result;
    }


    tn_paddr base_addr = (tn_u32)tn_cmdq_ccdma_get_addr(0);
    // make it circular
    tn_cmdq_ccdma_write_cmd(_current_cmd_buf_size - 1, TIN_CMD_LINK, base_addr);

    _new_cmds_ready = tnFALSE;

    // add initial LOOP sequence and start CCDMA
    _curr_wait_cmd_index = 0;
    _curr_link_cmd_index = _prev_link_cmd_index = 1;
    add_loop();

    // enable CCDMA (stay in the loop until flush or finish)
    tn_hw_ccdma_set_wait_delay(TIN_WAIT_DELAY);
    tn_hw_ccdma_start(tn_cmdq_ccdma_paddr());
    _ccdma_initialized = tnTRUE;

    tnOUT();
    return TN_NO_ERROR;
}



void
tn_cmd_buf_ccdma_deinit(void)
{
    tnIN();

    if (_ccdma_initialized)
    {
        tn_cmd_buf_ccdma_finish();
        cache_clear();
        tn_cmdq_ccdma_destroy();
        _ccdma_initialized = tnFALSE;
    }

    tnOUT();
}



// ---------------------------------------------------------------------------
// Cache command buffer API
// ---------------------------------------------------------------------------

// START: allocate a new slot and begin shadowing subsequent commands into it.
static tn_result
cache_start(tn_i32 *buf_index)
{
    if (buf_index == tnNULL)
    {
        return TN_ILLEGAL_ARGUMENT_ERROR;
    }
    if (_cache_recording)
    {
        // nested START is not supported
        return TN_ILLEGAL_ARGUMENT_ERROR;
    }
    if (_cache_num_slots >= CACHE_CMD_BUF_MAX_SLOTS)
    {
        return TN_OUT_OF_RESOURCES_ERROR;
    }

    tn_cache_cmd_slot_t *slot = &_cache_slots[_cache_num_slots];

    tn_result rv = tn_mbuf_alloc_aligned(&slot->mbuf,
                                         CACHE_SLOT_BUF_ALIGN,
                                         CACHE_SLOT_INITIAL_ENTRIES * 8U);
    if (rv != TN_NO_ERROR)
    {
        return TN_OUT_OF_MEMORY_ERROR;
    }

    slot->write_offset  = 0;
    slot->num_entries   = 0;
    slot->capacity      = CACHE_SLOT_INITIAL_ENTRIES;
    slot->xform_ctrl_idx = CACHE_IDX_NONE;
    slot->xform_tf0_idx  = CACHE_IDX_NONE;
    slot->dst_size_idx   = CACHE_IDX_NONE;
    slot->dst_addr0_idx  = CACHE_IDX_NONE;
    slot->s1_size_idx    = CACHE_IDX_NONE;
    slot->s1_addr0_idx   = CACHE_IDX_NONE;
    slot->geo.injectable = tnFALSE;

    _cache_recording_slot_idx = _cache_num_slots;

    _cache_recording          = tnTRUE;

    *buf_index = (tn_i32)_cache_num_slots;

    return TN_NO_ERROR;
}



// END: stop shadowing and seal the slot.
static tn_result
cache_end(void)
{
    if (!_cache_recording)
    {
        return TN_ILLEGAL_ARGUMENT_ERROR;
    }

    tn_cache_cmd_slot_t *slot = &_cache_slots[_cache_recording_slot_idx];
    slot->num_entries = slot->write_offset;

    _cache_recording = tnFALSE;
    _cache_num_slots++;

    return TN_NO_ERROR;
}



// CLEAR: destroy all slots and reset cache state.
static void
cache_clear(void)
{
    for (tn_u32 i = 0; i < _cache_num_slots; i++)
    {
        tn_mbuf_release(&_cache_slots[i].mbuf);
    }

    _cache_num_slots          = 0;
    _cache_recording          = tnFALSE;
    _cache_recording_slot_idx = 0;
}



// Write the 9 TF0..TF8 register payloads in the flat buffer from the (already
// inverted, destination->source) matrix work.  The 9 entries are sequential
// because tn_regs_set_xform issues them in one loadNamedRegValues call in
// row-major order.
static void
cache_patch_tf(
    tn_cache_cmd_slot_t *slot,
    const tn_m3f32      *work)
{
    if (slot->xform_tf0_idx == CACHE_IDX_NONE)
    {
        return;
    }
    tnRET_VOID_IF(tn_is_null(slot->mbuf.vaddr));

    tn_u32 *buf = (tn_u32*)(tn_uptr)slot->mbuf.vaddr;
    static const tn_u8 row_lut[9] = {0, 0, 0, 1, 1, 1, 2, 2, 2};
    static const tn_u8 col_lut[9] = {0, 1, 2, 0, 1, 2, 0, 1, 2};
    for (tn_u32 k = 0; k < 9U; k++)
    {
        tn_f32 f = work->m[row_lut[k]][col_lut[k]];
#ifdef TN_DQ_CT
        if (k == 8U)
        {
            f = 0.5f * (work->m[2][0] + work->m[2][1]) + work->m[2][2];
        }
#endif
        tn_u32 u = 0;
        tn_memcpy(&u, &f, sizeof(u));
        buf[(slot->xform_tf0_idx + k) * 2U + 1U] = u;
    }
}



// Overwrite the S2 transform and the matrix-dependent DST/S1 register payloads
// in the cache buffer with values derived from mat.
//
// mat is the forward (source->destination) user matrix, exactly as it would be
// passed to the blit at record time.  The recorded TF registers hold the
// INVERSE (destination->source) matrix and the DST/S1 SIZE/ADDR registers are
// derived from the matrix-dependent blend area, so all of them must be
// re-derived here to replay the slot with a different transform.
//
// The full re-derivation (blend area + inverse + DST/S1 patch) is only done
// for the simple affine RGB case captured by tn_cmd_buf_ccdma_cache_note_blit_
// geometry().  Otherwise only the raw TF registers are patched as a
// best-effort fallback (legacy behaviour).
static void
cache_inject_matrix(
    tn_cache_cmd_slot_t *slot,
    const tn_m3f32      *mat)
{
    tn_u32 *buf = (tn_u32*)(tn_uptr)slot->mbuf.vaddr;
    tnRET_VOID_IF(tn_is_null(buf));

    // Patch TINGPU_S2_TRANSFORM_CTRL: choose affine or projective mode.
    if (slot->xform_ctrl_idx != CACHE_IDX_NONE)
    {
        tn_u32 mode = tn_m3f32_is_affine(mat) ?
                      TN_REGS_S2_XFORM_MODE_AFFINE :
                      TN_REGS_S2_XFORM_MODE_PROJECTIVE;
        buf[slot->xform_ctrl_idx * 2U + 1U] =
            TINGPU_S2_TRANSFORM_CTRL_MODE(mode);
    }

    if (!slot->geo.injectable || !tn_m3f32_is_affine(mat))
    {
        // Fallback: not a simple affine RGB blit (or the geometry was never
        // captured).  Patch only the TF registers.  The TF registers hold the
        // inverse (dst->src) transform, so mat must be inverted before being
        // written.  DST/S1 registers cannot be re-derived because the required
        // geometry is unknown.
        tn_m3f32 inv = *mat;
        if (!tn_m3f32_invert_imm(&inv))
        {
            // Singular matrix: leave registers untouched.
            return;
        }
        cache_patch_tf(slot, &inv);
        return;
    }

    // --- Simple affine RGB path: re-derive everything the blit derived ---

    // Work on a private copy of the forward matrix.  This mirrors
    // tn_blt_image(): compute the visible blend area from the forward matrix,
    // subtract the blend-area origin from the translation column, then invert
    // so the HW can map each destination pixel back to a source texel.
    tn_m3f32 work = *mat;

    tn_bool translation_only = tn_m3f32_is_translation_only(&work);

    tn_rect_t blend_area = {0, 0, (tn_map_coord)slot->geo.tgt_w,
                            (tn_map_coord)slot->geo.tgt_h};
    tn_m3f32_get_blend_area(&blend_area, &work, slot->geo.src_w,
                            slot->geo.src_h, translation_only);

    if (blend_area.width <= 0 || blend_area.height <= 0)
    {
        // Nothing visible for this transform.  Leaving the recorded registers
        // in place is acceptable; the replayed blit simply produces no output.
        return;
    }

    // Shift the translation components and invert (affine).
    work.m[0][2] -= (tn_f32)blend_area.x;
    work.m[1][2] -= (tn_f32)blend_area.y;
    if (!tn_m3f32_invert_imm(&work))
    {
        // Singular matrix: leave registers untouched.
        return;
    }

    cache_patch_tf(slot, &work);

    // Re-derive the DST/S1 SIZE and ADDR0 payloads from the new blend area.
    // These use the same encoding as tn_regs_set_size / tn_regs_set_stride_
    // and_addr (non-planar RGB) / tn_regs_set_base_addr in tn_regs_image.c.
    tn_u32 size_val = ((tn_u32)TINGPU_DST_SIZE_WIDTH(blend_area.width)) |
                      ((tn_u32)TINGPU_DST_SIZE_HEIGHT(blend_area.height));

    tn_u32 off_bytes = (tn_u32)blend_area.y * slot->geo.tgt_stride +
                       (((tn_u32)blend_area.x * slot->geo.tgt_bpp) >> 3);
    tn_u32 addr_val = TINGPU_DST_ADDR_ADDR(slot->geo.tgt_addr + off_bytes);

    if (slot->dst_size_idx != CACHE_IDX_NONE)
    {
        buf[slot->dst_size_idx * 2U + 1U] = size_val;
    }
    if (slot->dst_addr0_idx != CACHE_IDX_NONE)
    {
        buf[slot->dst_addr0_idx * 2U + 1U] = addr_val;
    }
    // S1 carries the target surface for the SRC_OVER blend path, so it uses
    // the identical size/address as DST.  The SIZE/ADDR shifts are the same
    // between DST and S1 (both 0 / 16), so size_val/addr_val are reused.
    if (slot->s1_size_idx != CACHE_IDX_NONE)
    {
        buf[slot->s1_size_idx * 2U + 1U] = size_val;
    }
    if (slot->s1_addr0_idx != CACHE_IDX_NONE)
    {
        buf[slot->s1_addr0_idx * 2U + 1U] = addr_val;
    }
}



void
tn_cmd_buf_ccdma_cache_note_blit_geometry(const tn_cmd_buf_cache_blit_geo_t *geo)
{
    if (!_cache_recording || geo == tnNULL)
    {
        // Only meaningful while a slot is being recorded.
        return;
    }

    _cache_slots[_cache_recording_slot_idx].geo = *geo;
}



// EXECUTE: patch the matrix in the slot's flat buffer, then re-inject every
// recorded entry into the ring buffer through the normal safe-enqueue path.
// The CCDMA processes them as it would any other command stream; completion
// is signalled by the subsequent vg_lite_finish() call via IMG_FINISH, so no
// HALT/RESUME or CACHE_FINISH event is needed.
static tn_result
cache_execute(
    tn_i32          slot_index,
    const tn_m3f32 *matrix)
{
    if (slot_index < 0 || (tn_u32)slot_index >= _cache_num_slots)
    {
        return TN_ILLEGAL_ARGUMENT_ERROR;
    }

    tn_cache_cmd_slot_t *slot = &_cache_slots[(tn_u32)slot_index];

    if (slot->num_entries == 0)
    {
        // empty slot - nothing to execute
        return TN_NO_ERROR;
    }

    tnRET_IF(tn_is_null(slot->mbuf.vaddr), TN_BAD_HANDLE_ERROR);

    // Patch S2 transform registers in the flat buffer before replay so the
    // new matrix takes effect without re-recording the slot.
    if (matrix != tnNULL)
    {
        cache_inject_matrix(slot, matrix);
    }

    // Re-inject every recorded entry into the ring buffer.  safe_enq is used
    // so back-pressure (ring buffer full) is handled correctly.  We are not
    // inside a recording session here so safe_enq will NOT shadow these
    // entries into another slot.
    tn_u32 *buf = (tn_u32*)(tn_uptr)slot->mbuf.vaddr;

    for (tn_u32 i = 0; i < slot->num_entries; i++)
    {
        tn_result result = cmd_buf_ccdma_safe_enq(buf[i * 2U], buf[i * 2U + 1U]);
        if (result != TN_NO_ERROR)
        {
            return result;
        }
    }

    return TN_NO_ERROR;
}



tn_result
tn_cmd_buf_ccdma_cache_command(
    tn_cmd_buf_cache_op_t operation,
    tn_i32               *buf_index,
    const tn_m3f32       *matrix)
{
    tnIN();

    tn_result result;

    switch (operation)
    {
    case TN_CMD_BUF_CACHE_START:
        result = cache_start(buf_index);
        break;

    case TN_CMD_BUF_CACHE_END:
        result = cache_end();
        break;

    case TN_CMD_BUF_CACHE_CLEAR:
        cache_clear();
        result = TN_NO_ERROR;
        break;

    case TN_CMD_BUF_CACHE_EXECUTE:
        if (buf_index == tnNULL)
        {
            result = TN_ILLEGAL_ARGUMENT_ERROR;
        }
        else
        {
            result = cache_execute(*buf_index, matrix);
        }
        break;

    default:
        result = TN_ILLEGAL_ARGUMENT_ERROR;
        break;
    }

    tnOUT();
    return result;
}
