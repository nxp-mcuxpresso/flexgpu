//
// Copyright 2026 NXP
//
// SPDX-License-Identifier: MIT
//

#include "tn_mem.h"
#include "tn_aver.h"
#include "tn_trace.h"
#include "trace/hooks.h"

#include <stdlib.h>
#include <string.h>



void*
tn_memmove(
    void       *dest,
    const void *src,
    tn_size     n)
{
    tnIN_ARG("dst:x%x src:x%x n:%d", dest, src, n);
    tnOUT();
    return memmove(dest, src, n);
}



void*
tn_memcpy(
    void       *dest,
    const void *src,
    tn_size     n)
{
    tnIN_ARG("dst:x%x src:x%x n:%d", dest, src, n);
    tnOUT();
    return memcpy(dest, src, n);
}



int
tn_memcmp(
    const void *s1,
    const void *s2,
    tn_size     n)
{
    tnIN_ARG("s1:x%x s2:x%x n:%d", s1, s2, n);
    tnOUT();
    return memcmp(s1, s2, n);
}



void*
tn_memset(
    void   *s,
    int     c,
    tn_size n)
{
    tnIN_ARG("s:x%x c:%d n:%d", s, c, n);
    tnOUT();
    return memset(s, c, n);
}
