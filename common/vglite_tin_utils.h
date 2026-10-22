//
// Copyright 2025-2026 NXP
//
// SPDX-License-Identifier: MIT
//

#ifndef _VGLITE_TIN_UTILS_H
#define _VGLITE_TIN_UTILS_H

#include "tin.h"
#include "tn_color.h"
#include "tn_gbuf.h"
#include "vg_lite.h"
#include "vg_lite_ext.h"
#include "tn_m3f32.h"
#include "tn_v2f32.h"

void setup_yuv_struct(
    vg_lite_buffer_t *buffer,
    tn_color_format   fmt,
    tn_size          *y,
    tn_size          *u,
    tn_size          *v,
    tn_size          *a);

tn_color_format getYVFormat(tn_color_format fmt);

tn_color_format getYUFormat(tn_color_format fmt);

tn_color_format vglite_color_2_tn_color(vg_lite_buffer_format_t fmt);

void get_planes_info(
    vg_lite_buffer_t *buffer,
    tn_u8            *num_planes,
    tn_u32            height[4],
    tn_u32            stride[4]);

// Total bytes the per-plane geometry from get_planes_info() will write, i.e.
// sum over planes of height[i] * stride[i]. Returns tnFALSE when that sum
// does not fit a tn_size, in which case *total_out is not written.
tn_bool vglite_planes_total_bytes(
    tn_u8        num_planes,
    const tn_u32 height[4],
    const tn_u32 stride[4],
    tn_size     *total_out);

/// Map an internal tn_result onto the public vg_lite_error_t.
/// Shared by the vg_lite_init() entry points so the resource-exhaustion codes
/// raised during bring-up reach the application unchanged instead of being
/// flattened to VG_LITE_FAIL.
static inline vg_lite_error_t
tn_result_to_vg_lite_error(tn_result rv)
{
    switch (rv)
    {
    case TN_NO_ERROR:               return VG_LITE_SUCCESS;
    case TN_NOT_SUPPORTED:          return VG_LITE_NOT_SUPPORT;
    case TN_ILLEGAL_ARGUMENT_ERROR: return VG_LITE_INVALID_ARGUMENT;
    case TN_OUT_OF_MEMORY_ERROR:    return VG_LITE_OUT_OF_MEMORY;
    case TN_OUT_OF_RESOURCES_ERROR:
    case TN_BUFFER_FULL_ERROR:
        return VG_LITE_OUT_OF_RESOURCES;
    case TN_TIMEOUT_ERROR:          return VG_LITE_TIMEOUT;
    case TN_GENERIC_IO_ERROR:       return VG_LITE_GENERIC_IO;
    case TN_NO_CONTEXT_ERROR:       return VG_LITE_NO_CONTEXT;
    default:                        return VG_LITE_FAIL;
    }
}



tn_i32 vglite_blend_to_tn_blend(vg_lite_blend_t blend);

tn_u8 vglite_blend_to_attr_blend(vg_lite_blend_t blend);

tn_i32 vglite_mask_op_to_tn_blend(vg_lite_mask_operation_t op);

tn_i32 vglite_image_mode_to_tn_image_mode(vg_lite_image_mode_t mode);

tn_i32 vglite_yuv2rgb_to_tn(vg_lite_yuv2rgb_t yuv2rgb);

tn_u8 vglite_quality_to_vg_quality(vg_lite_quality_t quality);

tn_result vglite_buffer_gbuf_create(
    tn_gbuf          *gbuf,
    vg_lite_buffer_t *buffer);

// Describe caller-owned memory with a tn_gbuf built from buffer's geometry.
// user_len is the number of bytes backing user_vaddr. Fails when the geometry
// does not fit in user_len.
tn_result vglite_buffer_gbuf_wrap(
    tn_gbuf          *gbuf,
    vg_lite_buffer_t *buffer,
    tn_vptr           user_vaddr,
    tn_size           user_len);

// Per-format source stride alignment table (VG_16PIXELS_ALIGN constraint).
vg_lite_error_t vglite_source_stride_align_check(
    vg_lite_buffer_format_t format,
    tn_u32                  stride);

// Aggregate source-buffer address/stride/format alignment check.
vg_lite_error_t vglite_srcbuf_align_check(vg_lite_buffer_t *source);

// Aggregate destination-buffer address/stride/format alignment check.
vg_lite_error_t vglite_dstbuf_align_check(vg_lite_buffer_t *target);

// PE-clear destination alignment check for a region of the given height.
vg_lite_error_t vglite_peclear_align_check(
    vg_lite_buffer_t *target,
    tn_i32            y);

#endif
