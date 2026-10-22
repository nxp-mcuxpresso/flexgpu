//
// Copyright 2025-2026 NXP
//
// SPDX-License-Identifier: MIT
//

#ifndef __TN_INIT_H__
#define __TN_INIT_H__


#include "tn_result.h"

#ifdef __cplusplus
extern "C" {
#endif

/// See: https://github.com/picolibc/picolibc/blob/main/doc/init.md
/// See: https://sourceware.org/git/?p=newlib-cygwin.git;a=blob_plain;f=libgloss/riscv/crt0.S

/// Bring up the driver. Fallible: the memory-space and arena reservations it
/// performs can run out of memory or resources, and the code must reach the API
/// boundary so the caller can report it. Annotated tnNODISCARD so a caller that
/// drops the status is a compile-time diagnostic.
tnNODISCARD tn_result tn_init(void);

void tn_deinit(void);

#ifdef __cplusplus
}
#endif

#endif // __TN_INIT_H__
