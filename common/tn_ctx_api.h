//
// Copyright 2025-2026 NXP
//
// SPDX-License-Identifier: MIT
//


#ifndef   __TN_CTX_API_H__
#define   __TN_CTX_API_H__

#include "tn_types.h"

void tn_ctx_api_init(void);

void tn_ctx_api_deinit(void);

tn_u32 tn_ctx_api_has_feature(tn_u32 feature);

tnNODISCARD tn_result tn_vg_init(void);

#endif // __TN_CTX_API_H__
