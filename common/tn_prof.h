//
// Copyright 2025-2026 NXP
//
// SPDX-License-Identifier: MIT
//

#ifndef __TN_PROF_H__
#define __TN_PROF_H__

#include "tn_config.h"

#ifdef TN_PROF

    #include "tn_types.h"
    #include "tn_ticks.h"
    #include "tn_trace.h"

/* For tn_gpu_ticks() */
    #include "tn_hw.h"
    #define TN_GPU_STS2 0x0058

/* Suppress unused-variable warnings for all supported compilers. */
static tn_u64 prof_ticks_begin __attribute__((unused)), prof_ticks_end __attribute__((unused));
static tn_u64 prof_gpu_ticks_begin __attribute__((unused)), prof_gpu_ticks_end __attribute__((unused));

    #define tnPROF_START() do { prof_ticks_begin = tn_ticks(); } while (0)
    #define tnPROF_STOP()  do { prof_ticks_end = tn_ticks(); } while (0)
    #define tnPROF_PRINT() do {                                                  \
            tn_trace_fmt("cycles: %16llu\n", prof_ticks_end - prof_ticks_begin); \
} while (0)


    #define tn_gpu_ticks() tn_hw_reg_get(TN_GPU_STS2)

    #define tnPROF_GPU_START() do { prof_gpu_ticks_begin = tn_gpu_ticks(); } while (0)
    #define tnPROF_GPU_STOP()  do { prof_gpu_ticks_end = tn_gpu_ticks(); } while (0)
    #define tnPROF_GPU_PRINT() do {                                                          \
            tn_trace_fmt("GPU cycles: %16llu\n", prof_gpu_ticks_end - prof_gpu_ticks_begin); \
} while (0)

#else

    #define tnPROF_START()
    #define tnPROF_STOP()
    #define tnPROF_PRINT()

    #define tnPROF_GPU_START()
    #define tnPROF_GPU_STOP()
    #define tnPROF_GPU_PRINT()

#endif // TN_PROF

#endif // __TN_PROF_H__
