//
// Copyright 2025-2026 NXP
//
// SPDX-License-Identifier: MIT
//

#include "tn_ctx.h"
#include "tn_ctx_img.h"
#include "tn_ctx_api.h"


void
tn_ctx_init(void)
{
    tn_ctx_img_init(&g_img_ctx);
    tn_ctx_api_init();
}



void
tn_ctx_deinit(void)
{
    tn_ctx_api_deinit();
    tn_ctx_img_deinit(&g_img_ctx);
}
