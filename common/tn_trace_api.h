//
// Copyright 2025-2026 NXP
//
// SPDX-License-Identifier: MIT
//

#ifndef __TN_TRACE_API_H__
#define __TN_TRACE_API_H__

#include "tn_ctx_img.h"
#include "tn_trace.h"

// Both forms of the conditional comma are spelled out here, selected on
// TN_HAS_VA_OPT (tn_trace.h), because __VA_OPT__ is C23 / C++20 only and IAR
// rejects it in the toolchain's default C mode. The comma cannot be factored
// into a shared helper - see the comment next to TN_HAS_VA_OPT for why.
#if TN_HAS_VA_OPT
    #define DUMP_API_CALL(func, fmt, ...)                                  \
        do {                                                               \
            if (g_img_ctx.dump_api_flag) {                                 \
                tn_trace_dump_api(#func,                                   \
                                  fmt __VA_OPT__( , ) __VA_ARGS__);        \
            }                                                              \
        } while (0)
#else
    #define DUMP_API_CALL(func, fmt, ...)                                  \
        do {                                                               \
            if (g_img_ctx.dump_api_flag) {                                 \
                tn_trace_dump_api(#func, fmt, ## __VA_ARGS__);             \
            }                                                              \
        } while (0)
#endif

#endif // __TN_TRACE_API_H__
