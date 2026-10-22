//
// Copyright 2026 NXP
//
// SPDX-License-Identifier: MIT
//

#ifndef   __TN_SYSHEAP_H__
#define   __TN_SYSHEAP_H__

/// @file tn_sysheap.h
/// Interface for the system heap.


#include "tn_types.h"

#ifdef __cplusplus
extern "C" {
#endif

void *tn_malloc(tn_size size);

void *tn_malloc_aligned(
    tn_size alignment,
    tn_size size);

void *tn_calloc(
    tn_size nmemb,
    tn_size size);

void *tn_realloc(
    void   *ptr,
    tn_size size);

void tn_free(void *ptr);

#ifdef __cplusplus
}
#endif

#endif // __TN_SYSHEAP_H__
