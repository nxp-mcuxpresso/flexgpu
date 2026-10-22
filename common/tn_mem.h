//
// Copyright 2025-2026 NXP
//
// SPDX-License-Identifier: MIT
//

#ifndef __TN_MEM_H__
#define __TN_MEM_H__

/// @file tn_mem.h
/// Moving, copying, comparing memory.


#include "tn_types.h"

#ifdef __cplusplus
extern "C" {
#endif

void *tn_memmove(
    void       *dest,
    const void *src,
    tn_size     n);

void *tn_memcpy(
    void       *dest,
    const void *src,
    tn_size     n);

int tn_memcmp(
    const void *s1,
    const void *s2,
    tn_size     n);

void * tn_memset(
    void   *s,
    int     c,
    tn_size n);

#ifdef __cplusplus
}
#endif

#endif // __TN_MEM_H__
