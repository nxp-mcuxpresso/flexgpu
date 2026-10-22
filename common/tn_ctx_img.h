//
// Copyright 2025-2026 NXP
//
// SPDX-License-Identifier: MIT
//
#ifndef   __TN_CTX_IMG_H__
#define   __TN_CTX_IMG_H__

#include "tin.h"
#include "tn_regs_image.h"
#include "tn_gbuf.h"

typedef struct tn_ctx_img
{
    // GPU version, sampled once from the VERID register at init
    tn_u8   gpu_major;
    tn_u8   gpu_minor;

    // FlexGPU supports up to SCISSOR_NUM scissor rectangles.
    // Use scissor_num to track the number of active scissors
    struct
    {
        tn_i32   sx;
        tn_i32   sy;
        tn_i32   w;
        tn_i32   h;
    } scissor_rects[SCISSOR_NUM];

    tn_u8            scissor_num;
    tn_bool          scissor_active;

    // Scissor adjustments are made occasionally by the blending area optimizations
    tn_map_coord     scissor_adj_x;
    tn_map_coord     scissor_adj_y;

    const tn_gbuf   *mask_color_gbuf;

    tn_bool          mask_color_enabled;

    // Source channel on which the color key is enabled
    tn_u8            color_key_on_s1;
    tn_u8            color_key_on_s2;

    tn_u8            src_alpha_mode;
    tn_u8            src_alpha_value;
    tn_u8            dst_alpha_mode;
    tn_u8            dst_alpha_value;

    // Cache for color key regs
    color_key_pair   color_keys[4];

    //TODO: check if blit ID can be removed
    tn_blt_id        blit;

    tn_bool          gamma_en;
    tn_color_space   gamma_cs;

    // Whether color transform is enabled or not
    tn_bool          color_transform;

    tn_bool          dump_api_flag;
} tn_ctx_img;


extern tn_ctx_img g_img_ctx;

void tn_ctx_img_init(tn_ctx_img *img_ctx);

void tn_ctx_img_deinit(tn_ctx_img *img_ctx);

#endif // __TN_CTX_IMG_H__
