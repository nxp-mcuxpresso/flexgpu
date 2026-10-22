//
// Copyright 2026 NXP
//
// SPDX-License-Identifier: MIT
//

#ifndef   __TN_CTX_VER_H__
#define   __TN_CTX_VER_H__

// Default (hardware / modeled) version init.

#include "tn_ctx_img.h"
#include "tn_hw.h"

static inline void
tn_ctx_ver_init(tn_ctx_img *img_ctx)
{
    tn_u32 verid = tn_hw_reg_get(TINGPU_VERID);

    img_ctx->gpu_major = (verid & TINGPU_VERID_MAJOR_MASK) >> TINGPU_VERID_MAJOR_SHIFT;
    img_ctx->gpu_minor = (verid & TINGPU_VERID_MINOR_MASK) >> TINGPU_VERID_MINOR_SHIFT;
}



#endif // __TN_CTX_VER_H__
