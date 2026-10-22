//
// Copyright 2025-2026 NXP
//
// SPDX-License-Identifier: MIT
//

#include "tn_sort.h"
#include <stddef.h>
#if defined(__ICCARM__)
    #undef __STDC_WANT_LIB_EXT1__
    #define __STDC_WANT_LIB_EXT1__ 1
#endif
#include <stdlib.h>

#if !defined(_GNU_SOURCE) && !defined(__ICCARM__)
    #error "_GNU_SOURCE needs to be defined for qsort_r"
#endif

void
tn_sort(
    void   *base,
    tn_size nmemb,
    tn_size size,
    int   (*cmp)(
        const void*,
        const void*))
{
    qsort(base, nmemb, size, cmp);
}



void
tn_sort_r(
    void       *base,
    tn_size     nmemb,
    tn_size     size,
    int       (*cmp_r)(
        const void*,
        const void*,
        void*),
    const void *arg)
{
#if defined(__ICCARM__)
    (void)qsort_s(base, nmemb, size, cmp_r, (void*)arg);
#else
    qsort_r(base, nmemb, size, cmp_r, (void*)arg);
#endif
}
