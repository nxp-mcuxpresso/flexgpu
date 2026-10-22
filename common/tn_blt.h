//
// Copyright 2025-2026 NXP
//
// SPDX-License-Identifier: MIT
//

#ifndef __TN_BLT_H__
#define __TN_BLT_H__

#include "tn_color.h"
#include "tn_gbuf.h"
#include "tn_m3f32.h"

/// Pixel Block Transfer (PIX-BLT) Commands


/// rectangular area to transfer.
typedef struct
{
    const tn_gbuf   *src_img;
    const tn_gbuf   *dst_img;
    tn_map_coord     sx;
    tn_map_coord     sy;
    tn_map_coord     dx;
    tn_map_coord     dy;
    tn_map_coord     width;
    tn_map_coord     height;
    const tn_m3f32  *mat;
} tn_blt_xform_t;

///  fill with specified color.
tn_result tn_blt_fill(
    tn_blt_id id_blt,
    tn_color  c,
    tn_blt_xform_t*);

///  blit and blend image.
tn_result tn_blt_image(
    const tn_gbuf *tgt_image,
    const tn_gbuf *src_image,
    tn_m3f32      *matrix,
    tn_i32         blend_mode,
    tn_color       color,
    tn_u32         image_mode,
    tn_u32         quality);

///  blit and blend two source images onto a target in a single hardware pass.
///  src0 (foreground) is transformed on S2 by matrix0; src1 (background) is
///  placed on S1 by matrix1 (translation only, S1 has no transform unit) and
///  src0 is blended over src1 using blend_mode.
tn_result tn_blt_image2(
    const tn_gbuf *tgt_image,
    const tn_gbuf *src0_image,
    const tn_gbuf *src1_image,
    tn_m3f32      *matrix0,
    tn_m3f32      *matrix1,
    tn_i32         blend_mode,
    tn_color       color,
    tn_u32         image_mode,
    tn_u32         quality);

///  blit and blend rectangular part of an image.
tn_result tn_blt_image_rect(
    const tn_gbuf *tgt_image,
    const tn_gbuf *src_image,
    tn_m3f32      *matrix,
    tn_i32         blend_mode,
    tn_color       color,
    tn_u32         image_mode,
    tn_u32         quality,
    tn_blt_xform_t*);

///  blit and blend based on the channel mask.
tn_result tn_blt_channel_select(
    const tn_gbuf *tgt_image,
    const tn_gbuf *src_image,
    tn_u16         channel_mask);

///  fill mask layer with specified value.
tn_result tn_mask_fill(
    const tn_gbuf  *mask_image,
    tn_blt_xform_t *rect,
    tn_u8           value);

///  blend two mask layers.
tn_result tn_mask_blend(
    const tn_gbuf  *dst,
    const tn_gbuf  *src,
    tn_i32          blend_mode,
    tn_blt_xform_t *rect);

///  set active mask layer.
tn_result tn_mask_set(const tn_gbuf *mask);

#endif // __TN_BLT_H__
