//
// Copyright 2026 NXP
//
// SPDX-License-Identifier: MIT
//

#ifndef   __TN_GPU_REGS_H__
#define   __TN_GPU_REGS_H__

// Register names for the driver. Both branches end at a tn_gpu_regs_wrap.h,
// which publishes the same TINGPU_<REG> byte offsets either way, so a driver
// only has to include this header to name a register:
//
//   tn_hw_reg_set(TINGPU_INTEN, TINGPU_INTEN_CCDMA_EVENT_EN(1));
//
// What differs is where the offsets come from. In an SDK build the CMSIS
// struct is the single source of truth and the wrapper takes the offsetof of
// each member. In a standalone build PERI_TINGPU.h is unusable - it needs a
// CPU_MIMXRT* define and the CMSIS core headers - so the offsets come from the
// tin_gpu_regs.h table instead, and the wrapper itself supplies the TINGPU_*
// field macros the SDK header would otherwise provide.

#if defined(TN_MCUXSDK_BUILD)
// MCUXpresso SDK build (RT2660, etc.)
    #include "fsl_device_registers.h"
    #define TIN_GPU_BASE    MEDIA__TINGPU_BASE
#else
    #ifndef TIN_GPU_BASE
        #define TIN_GPU_BASE    0x40010000
    #endif

    #ifndef TN_SC_REG_32
        #define TN_SC_REG_32(a) (a)
    #endif

    #include "tin_gpu_regs.h"
#endif

#include "tn_gpu_regs_wrap.h"

#endif // __TN_GPU_REGS_H__
