//
// Copyright 2025-2026 NXP
//
// SPDX-License-Identifier: MIT
//

#include "tn_obj.h"
#include "tn_ctx_img.h"
#include "tn_regs_image.h"
#include "tn_ctx_ver.h"

tn_ctx_img g_img_ctx;

void
tn_ctx_img_init(tn_ctx_img *img_ctx)
{
    tn_ctx_ver_init(img_ctx);

    // No scissor active yet
    img_ctx->scissor_num     = 0;

    img_ctx->mask_color_gbuf = tnNULL;
    img_ctx->mask_color_enabled = tnFALSE;

    // No color keys enabled
    img_ctx->color_key_on_s1 = tnFALSE;
    img_ctx->color_key_on_s2 = tnFALSE;
    img_ctx->color_keys[0] = (color_key_pair){0, 0};
    img_ctx->color_keys[1] = (color_key_pair){0, 0};
    img_ctx->color_keys[2] = (color_key_pair){0, 0};
    img_ctx->color_keys[3] = (color_key_pair){0, 0};

    // Default to local alpha
    img_ctx->src_alpha_mode  = TN_LOCAL_ALPHA;
    img_ctx->src_alpha_value = 0xFF;
    img_ctx->dst_alpha_mode  = TN_LOCAL_ALPHA;
    img_ctx->dst_alpha_value = 0xFF;

    img_ctx->gamma_en = tnFALSE;
    img_ctx->gamma_cs = TN_CS_LINEAR;

    // vg_lite API-call dump trace defaults to disabled; enabled at runtime via
    // vg_lite_set_dump_api().
    img_ctx->dump_api_flag = tnFALSE;
}



void
tn_ctx_img_deinit(tn_ctx_img *img_ctx)
{
    // No memory was allocated, there is nothing to deinit
}
