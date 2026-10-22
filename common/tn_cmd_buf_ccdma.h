//
// Copyright 2026 NXP
//
// SPDX-License-Identifier: MIT
//

#ifndef __TN_CMD_BUF_CCDMA_H__
#define __TN_CMD_BUF_CCDMA_H__

#include "tn_types.h"
#include "tn_m3f32.h"

#define TN_RESERVED_EVENTS      (5)
#define TN_NEXT_EVENT_INCREMENT (0x01000000)
#define TN_EVENT_TYPE_MASK      (0xFF000000)
#define TN_EVENT_PAYLOAD_MASK   (0x00FFFFFF)

enum tn_cmd_buf_img_ccdma_events
{
    TN_CCDMA_EVENT_IMG_FLUSH     = TN_RESERVED_EVENTS * TN_NEXT_EVENT_INCREMENT,
    TN_CCDMA_EVENT_IMG_FINISH    = TN_CCDMA_EVENT_IMG_FLUSH    + TN_NEXT_EVENT_INCREMENT,
    // Frame delimiter events (vg_lite_frame_delimiter()). Treated by the ISR
    // exactly like the plain IMG FLUSH/FINISH events; kept as separate event
    // tokens so the HW event stream distinguishes a frame boundary from a
    // regular flush/finish, in case a future user wants to forward that
    // distinction further up the stack.
    TN_CCDMA_EVENT_FRAME_FLUSH   = TN_CCDMA_EVENT_IMG_FINISH   + TN_NEXT_EVENT_INCREMENT,
    TN_CCDMA_EVENT_FRAME_FINISH  = TN_CCDMA_EVENT_FRAME_FLUSH  + TN_NEXT_EVENT_INCREMENT,
    TN_CCDMA_EVENT_IMG_LAST_ENUM = TN_CCDMA_EVENT_FRAME_FINISH + TN_NEXT_EVENT_INCREMENT
};

// Returns non-zero when event is a finish-class event (IMG_FINISH or FRAME_FINISH).
// Both tokens are handled identically by the ISR; the FRAME variant only exists
// so a frame boundary can be distinguished from a plain finish on the event stream.
static inline tn_bool
tn_ccdma_event_is_finish(tn_u32 e)
{
    tn_u32 t = e & TN_EVENT_TYPE_MASK;
    return (tn_bool)(t == (tn_u32)TN_CCDMA_EVENT_IMG_FINISH ||
                     t == (tn_u32)TN_CCDMA_EVENT_FRAME_FINISH);
}



// Returns non-zero when event is a flush-class event (IMG_FLUSH or FRAME_FLUSH).
static inline tn_bool
tn_ccdma_event_is_flush(tn_u32 e)
{
    tn_u32 t = e & TN_EVENT_TYPE_MASK;
    return (tn_bool)(t == (tn_u32)TN_CCDMA_EVENT_IMG_FLUSH ||
                     t == (tn_u32)TN_CCDMA_EVENT_FRAME_FLUSH);
}



// Operations accepted by tn_cmd_buf_ccdma_cache_command().
typedef enum tn_cmd_buf_cache_op
{
    TN_CMD_BUF_CACHE_START   = 0,  // begin recording into a new slot
    TN_CMD_BUF_CACHE_END     = 1,  // seal the current slot
    TN_CMD_BUF_CACHE_CLEAR   = 3,  // discard all slots and reset state
    TN_CMD_BUF_CACHE_EXECUTE = 4,  // replay a previously recorded slot
} tn_cmd_buf_cache_op_t;

// Blit geometry captured while a cache slot is being recorded.  The blit
// code fills this with the inputs it used to derive the S2 transform and the
// DST/S1 size+address registers, so that a later EXECUTE with a replacement
// matrix can re-derive those same register payloads for the new transform.
//
// Only the simple (affine, non-YUV, non-compressed, byte-aligned) case is
// injectable.  When injectable is tnFALSE, EXECUTE with a non-NULL matrix is
// rejected instead of rendering with stale geometry.
typedef struct tn_cmd_buf_cache_blit_geo
{
    tn_bool   injectable; // affine + simple (byte-aligned RGB) target format
    tn_u32    src_w;      // source image width  (pixels)
    tn_u32    src_h;      // source image height (pixels)
    tn_u32    tgt_w;      // target image width  (pixels)
    tn_u32    tgt_h;      // target image height (pixels)
    tn_u32    tgt_stride; // target stride (bytes)
    tn_u32    tgt_bpp;    // target bits per pixel
    tn_u32    tgt_addr;   // target base physical address
} tn_cmd_buf_cache_blit_geo_t;

tn_result tn_cmd_buf_ccdma_set_buffer_size(tn_u32 size);

tn_result tn_cmd_buf_ccdma_flush(void);

tn_result tn_cmd_buf_ccdma_finish(void);

// Frame delimiter variants of flush/finish, used by vg_lite_frame_delimiter().
// Behave exactly like tn_cmd_buf_ccdma_flush()/tn_cmd_buf_ccdma_finish(), but
// submit a distinct HW event token (TN_CCDMA_EVENT_FRAME_FLUSH/FINISH instead
// of TN_CCDMA_EVENT_IMG_FLUSH/FINISH) so the frame boundary can be told apart
// from a regular flush/finish on the HW event stream.
tn_result tn_cmd_buf_ccdma_frame_flush(void);

tn_result tn_cmd_buf_ccdma_frame_finish(void);

tn_result tn_cmd_buf_ccdma_enq_event(tn_u32 event);

tn_result tn_cmd_buf_ccdma_enable_gpu_ctrl(void);

tn_result tn_cmd_buf_ccdma_load_reg(
    tn_u32 reg_offset,
    tn_u32 reg_value);

tn_result tn_cmd_buf_ccdma_loadNamedRegValues(
    tn_u32  count,
    tn_u32 *reg_addrs,
    tn_u32 *reg_values);

tn_result tn_cmd_buf_ccdma_init(
    tn_u32 physical,
    tn_u32 size);

void tn_cmd_buf_ccdma_deinit(void);

// Cache a sequence of GPU commands so it can be replayed without re-issuing
// every individual register-load and GPU-kick.
//
// operation - one of TN_CMD_BUF_CACHE_START / END / CLEAR / EXECUTE
// buf_index - in/out slot index:
//             START writes the new slot index through this pointer
//             EXECUTE reads the slot to replay from this pointer
//             CLEAR ignores this pointer (may be NULL)
// matrix    - optional transform matrix for EXECUTE; when non-NULL the
//             S2 transform registers recorded in the slot are overwritten
//             with the values derived from this matrix before replay starts.
//             Ignored (may be NULL) for START, END, and CLEAR operations.
tn_result tn_cmd_buf_ccdma_cache_command(
    tn_cmd_buf_cache_op_t operation,
    tn_i32               *buf_index,
    const tn_m3f32       *matrix);

// Record the blit geometry into the slot that is currently being recorded.
// A no-op when no cache recording session is active.  Must be called from the
// blit path BEFORE it issues the transform / DST / S1 register writes.
void tn_cmd_buf_ccdma_cache_note_blit_geometry(const tn_cmd_buf_cache_blit_geo_t *geo);

#endif
