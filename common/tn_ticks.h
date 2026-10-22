//
// Copyright 2025-2026 NXP
//
// SPDX-License-Identifier: MIT
//

#ifndef __TN_TICKS__
#define __TN_TICKS__

#include "tn_types.h"

// ARM Cortex-M mainline cores (v7-M, v7E-M, v8-M mainline, v8.1-M mainline)
// that expose the DWT cycle counter used by src/ticks/tn_ticks_cortex_m.h.
// Always defined as 0 or 1 so call sites can use "#if TN_ARCH_CORTEX_M".
#if defined(__ARM_ARCH_8_1M_MAIN__) || defined(__ARM_ARCH_8M_MAIN__) || \
    defined(__ARM_ARCH_7EM__) || defined(__ARM_ARCH_7M__)
    #define TN_ARCH_CORTEX_M 1
#else
    #define TN_ARCH_CORTEX_M 0
#endif

#if __riscv_xlen == 32
    #include "ticks/tn_ticks_riscv32.h"
#elif __riscv_xlen == 64
    #include "ticks/tn_ticks_riscv64.h"
#elif defined(__x86_64__)
    #include "ticks/tn_ticks_amd64.h"
#elif defined(__aarch64__)
    #include "ticks/tn_ticks_arm64.h"
#elif TN_ARCH_CORTEX_M
    #include "ticks/tn_ticks_cortex_m.h"
#else
    #error "Add a src/ticks/ header for this platform"
#endif

#endif // __TN_TICKS__
