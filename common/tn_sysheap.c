//
// Copyright 2026 NXP
//
// SPDX-License-Identifier: MIT
//

#include "tn_aver.h"
#include "tn_trace.h"
#include "trace/hooks.h"
#include "tn_sysheap.h"

#include <stdlib.h>
#if !defined(__IAR_SYSTEMS_ICC__)
    #include <malloc.h>
#endif
#include <string.h>

void*
tn_malloc(tn_size size)
{
    tnIN();

    void *addr = malloc(size);
    tnEXPECT(!tn_is_null(addr));
    hook_mem_new(addr, size);

    tnOUT_ARG("addr=x%x size=%lu", addr, size);
    return addr;
}



void*
tn_malloc_aligned(
    tn_size alignment,
    tn_size size)
{
    tnIN();

    void *addr = aligned_alloc(alignment, size);
    tnEXPECT(!tn_is_null(addr));
    hook_mem_new(addr, size);

    tnOUT_ARG("addr=x%x size=%lu", addr, size);
    return addr;
}



void*
tn_calloc(
    tn_size nmemb,
    tn_size size)
{
    tnIN();

    void *addr = calloc(nmemb, size);
    tnEXPECT(!tn_is_null(addr));
    hook_mem_new(addr, size);

    tnOUT_ARG("addr=x%x size=%lu", addr, size);
    return addr;
}



void*
tn_realloc(
    void   *ptr,
    tn_size size)
{
    tnIN();

    void *addr = realloc(ptr, size);
    tnEXPECT(addr || !size);

    tnOUT_ARG("addr=x%x size=%lu", addr, size);
    return addr;
}



void
tn_free(void *addr)
{
    tnIN_ARG("addr=x%x", addr);

    hook_mem_del(addr);
    free(addr);

    tnOUT();
}
