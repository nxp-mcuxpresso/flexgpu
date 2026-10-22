//
// Copyright 2026 NXP
//
// SPDX-License-Identifier: MIT
//

#include "tn_mspace_allocator_sysheap.h"
#include "tn_mspace.h"
#include "tn_compiler.h"
#include "tn_aver.h"
#ifndef __IAR_SYSTEMS_ICC__
    #include <malloc.h>
#endif
#include <stdlib.h>

TN_ALWAYS_INLINE static void * _sysheap_allocator_alloc(
    tn_mspace *mspace,
    tn_size    size);

TN_ALWAYS_INLINE
static void*
_sysheap_allocator_alloc(
    tn_mspace *mspace,
    tn_size    size)
{
    void *addr = malloc(size);
    tnEXPECT(!tn_is_null(addr));
    return addr;
}



TN_ALWAYS_INLINE static void * _sysheap_allocator_realloc(
    tn_mspace *mspace,
    void      *ptr,
    tn_size    size);

TN_ALWAYS_INLINE
static void*
_sysheap_allocator_realloc(
    tn_mspace *mspace,
    void      *ptr,
    tn_size    size)
{
    // realloc(ptr, 0) is implementation-defined in C, so handle it here: free
    // the block and return NULL, as the TLSF backend does. A NULL ptr is fine,
    // free(NULL) does nothing.
    if (size == 0)
    {
        free(ptr);
        return tnNULL;
    }
    return realloc(ptr, size);
}



TN_ALWAYS_INLINE static void * _sysheap_allocator_alloc_aligned(
    tn_mspace *mspace,
    tn_u8      alignment,
    tn_size    size);

TN_ALWAYS_INLINE
static void*
_sysheap_allocator_alloc_aligned(
    tn_mspace *mspace,
    tn_u8      alignment,
    tn_size    size)
{
    void *addr = aligned_alloc(alignment, size);
    tnEXPECT(!tn_is_null(addr));
    return addr;
}



tn_mspace_allocator tn_mspace_allocator_sysheap =
{
    .create         = tnNULL,
    .destroy        = tnNULL,
    .alloc          = &_sysheap_allocator_alloc,
    .realloc        = &_sysheap_allocator_realloc,
#ifndef __riscv
    .alloc_aligned  = &_sysheap_allocator_alloc_aligned,
#else
    .alloc_aligned  = tnNULL,
#endif
    .zone_add       = tnNULL,
    .get_free_space = tnNULL,
};
