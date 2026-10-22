//
// Copyright 2026 NXP
//
// SPDX-License-Identifier: MIT
//

#ifndef __TN_GBUF_H__
#define __TN_GBUF_H__

#include "tn_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct tn_gbuf
{
    tn_vptr        vaddr;           ///< CPU address of plane 0.
    tn_paddr       paddr;           ///< GPU address of plane 0.
    tn_map_coord   width;           ///< width in pixels.
    tn_map_coord   height;          ///< height in pixels.
    tn_map_coord   stride;          ///< bytes per line of plane 0.
    tn_u32         color_format;    ///< tn_color_format of the pixels.
    tn_i32         quality;         ///< non-antialiased, better, faster.
    tn_i32         mode;            ///< tn_image_mode: premultiplied, nonlinear, luminance.
    tn_i32         yuv2rgb;         ///< conversion standard: YUV601, YUV709, YUV470.
    tn_u32         uv_offset;       ///< byte offset of the uv plane, 0 when not yuv.
    tn_u32         v_offset;        ///< byte offset of the v plane, 0 when not yuv.
    tn_size        size;            ///< bytes backing vaddr, across all planes.
    tn_bool        valid;           ///< tnTRUE once the descriptor has been populated.
    /// tnTRUE when this descriptor owns the memory at vaddr and must release it.
    /// Only tn_gbuf_create() sets it. A descriptor that merely describes memory
    /// belonging to someone else - one filled in by tn_gbuf_wrap(), where the
    /// caller of the VGLite API owns the buffer - keeps it tnFALSE, and
    /// tn_gbuf_destroy() then frees nothing. Without the flag there is no
    /// other way to tell the two apart: a wrapped descriptor has a perfectly
    /// good vaddr.
    tn_bool        owned;
} tn_gbuf;


/// Reset a descriptor to the not-populated state.
static inline void
tn_gbuf_clear(tn_gbuf *g)
{
    g->vaddr        = tnNULL;
    g->paddr        = 0;
    g->width        = 0;
    g->height       = 0;
    g->stride       = 0;
    g->color_format = 0;
    g->quality      = 0;
    g->mode         = 0;
    g->yuv2rgb      = 0;
    g->uv_offset    = 0;
    g->v_offset     = 0;
    g->size         = 0;
    g->valid        = tnFALSE;
    g->owned        = tnFALSE;
}



/// tnTRUE when the descriptor describes a usable pixel buffer.
static inline tn_bool
tn_gbuf_is_valid(const tn_gbuf *g)
{
    return (g != tnNULL) && g->valid;
}



typedef struct tn_gbuf_layout
{
    tn_map_coord   width;           ///< width in pixels.
    tn_map_coord   height;          ///< height in pixels.
    tn_map_coord   stride;          ///< bytes per line of plane 0, already resolved.
    tn_u32         color_format;    ///< tn_color_format of the pixels.
    tn_i32         yuv2rgb;         ///< conversion standard: YUV601, YUV709, YUV470.
    tn_size        size;            ///< total bytes of all planes, used by create().
    tn_u32         uv_offset;       ///< byte offset of the uv plane, 0 when not yuv.
    tn_u32         v_offset;        ///< byte offset of the v plane, 0 when not yuv.
} tn_gbuf_layout;

/// Allocate layout->size bytes from TN_MSPACE_SHMEM_0 and describe them.
/// Sets owned, so tn_gbuf_destroy() releases the memory.
tn_result tn_gbuf_create(
    tn_gbuf              *g,
    const tn_gbuf_layout *layout);

/// Describe memory the caller already owns. Leaves owned clear, so
/// tn_gbuf_destroy() frees nothing. vaddr_len is the number of bytes
/// readable and writable at vaddr; rejected with TN_ILLEGAL_ARGUMENT_ERROR
/// when 0 or smaller than layout->size.
tn_result tn_gbuf_wrap(
    tn_gbuf              *g,
    const tn_gbuf_layout *layout,
    tn_vptr               vaddr,
    tn_size               vaddr_len);

/// Release the memory if this descriptor owns it, and clear the descriptor.
tn_result tn_gbuf_destroy(tn_gbuf *g);

#ifdef __cplusplus
}
#endif

#endif // __TN_GBUF_H__
