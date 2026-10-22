//
// Copyright 2025-2026 NXP
//
// SPDX-License-Identifier: MIT
//

#ifndef __TN_SORT_H__
#define __TN_SORT_H__

#include "tn_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*tn_sort_cmp)(
    const void*,
    const void*);
typedef int (*tn_sort_cmp_r)(
    const void*,
    const void*,
    void*);

void tn_sort(void *base, tn_size nmemb, tn_size size, tn_sort_cmp);

void tn_sort_r(
    void       *base,
    tn_size     nmemb,
    tn_size     size,
    int       (*cmp_r)(
        const void*,
        const void*,
        void*),
    const void *arg);

#ifdef __cplusplus
}
#endif

#endif // __TN_SORT_H__
