//
// Copyright 2025-2026 NXP
//
// SPDX-License-Identifier: MIT
//


#include "tn_ctx_api.h"
#include "vg_lite_ext.h"
#include "tn_mem.h"
#include "tn_trace.h"
#include "tn_array_size.h"

#ifndef TN_CTX_API_FEATURES_H
    #define TN_CTX_API_FEATURES_H "tn_ctx_api_features.h"
#endif
#include TN_CTX_API_FEATURES_H


// VGLite features table
tn_u8 s_ftable[gcFEATURE_EXT_LAST];


void
tn_ctx_api_init(void)
{
    tnIN();

    // enable all features
    tn_memset(s_ftable, 1, gcFEATURE_EXT_LAST);

    // Disable unsupported features
    tn_size length = tnARRAY_SIZE(unsupported_features);
    for (tn_size i = 0; i < length; i++)
    {
        s_ftable[unsupported_features[i]] = 0;
    }
    tnOUT();
}



void
tn_ctx_api_deinit(void)
{
    // The feature table doesn't update at runtime, no need to deinit
}



tn_u32
tn_ctx_api_has_feature(tn_u32 feature)
{
    tnIN();

    if (feature >= gcFEATURE_EXT_LAST)
    {
        return 0;
    }

    tnOUT();
    return s_ftable[feature];
}
