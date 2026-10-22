//
// Copyright 2026 NXP
//
// SPDX-License-Identifier: MIT
//

#include "tn_mspace.h"
#include "tn_compiler.h"
#include "tn_aver.h"
#include "tn_mbuf.h"
#include "tlsf/tlsf.h"
#include "tn_trace.h"


typedef struct tlsf_ctx
{
    tlsf_t   tlsf;
} tlsf_ctx;


static tlsf_ctx _tlsf_allocator_ctx;

TN_ALWAYS_INLINE static void _tlsf_allocator_create(struct tn_mspace *mspace);

TN_ALWAYS_INLINE
static void
_tlsf_allocator_create(struct tn_mspace *mspace)
{
    tnIN();
    tnEXPECT(!tn_is_null(mspace));
    mspace->ctx = &_tlsf_allocator_ctx;
    tnOUT();
}



TN_ALWAYS_INLINE static void _tlsf_allocator_destroy(tn_mspace *mspace);

TN_ALWAYS_INLINE
static void
_tlsf_allocator_destroy(tn_mspace *mspace)
{
    tnIN();
    tnEXPECT(!tn_is_null(mspace));
    tnEXPECT(!tn_is_null(mspace->ctx));
    tlsf_ctx *ctx = (tlsf_ctx*)mspace->ctx;
    tlsf_destroy(ctx->tlsf);
    // Clear the handle so that future calls to _tlsf_allocator_pool_add()
    // take the tlsf_create_with_pool() path instead of tlsf_add_pool().
    ctx->tlsf = tnNULL;
    mspace->ctx = tnNULL;
    tnOUT();
}



TN_ALWAYS_INLINE static void * _tlsf_allocator_alloc(
    tn_mspace *mspace,
    tn_size    size);

TN_ALWAYS_INLINE
static void*
_tlsf_allocator_alloc(
    tn_mspace *mspace,
    tn_size    size)
{
    tnIN();
    tnEXPECT(!tn_is_null(mspace));
    tnEXPECT(!tn_is_null(mspace->ctx));
    tlsf_ctx *ctx = (tlsf_ctx*)mspace->ctx;
    tnEXPECT(!tn_is_null(ctx->tlsf), "The TLSF allocator requires tn_mspace_pool_create");

    void *rv = tlsf_malloc(ctx->tlsf, size);
    tnRET_IF(tn_is_null(rv), tnNULL);

    tnOUT_ARG("rv:x%x", rv);
    return rv;
}



TN_ALWAYS_INLINE static void * _tlsf_allocator_realloc(
    tn_mspace *ctx,
    void      *ptr,
    tn_size    size);

TN_ALWAYS_INLINE
static void*
_tlsf_allocator_realloc(
    tn_mspace *mspace,
    void      *ptr,
    tn_size    size)
{
    tnIN();
    tnEXPECT(!tn_is_null(mspace));
    tnEXPECT(!tn_is_null(mspace->ctx));
    tlsf_ctx *ctx = (tlsf_ctx*)mspace->ctx;
    tnEXPECT(!tn_is_null(ctx->tlsf), "The TLSF allocator requires tn_mspace_pool_create");

    void *rv = tlsf_realloc(ctx->tlsf, ptr, size);
    // a size of 0 legitimately yields a null pointer, that is not a failure
    tnRET_IF(size != 0 && tn_is_null(rv), tnNULL);
    tnOUT_ARG("rv:x%x", rv);
    return rv;
}



TN_ALWAYS_INLINE static void * _tlsf_allocator_alloc_aligned(
    tn_mspace *mspace,
    tn_u8      alignment,
    tn_size    size);

TN_ALWAYS_INLINE
static void*
_tlsf_allocator_alloc_aligned(
    tn_mspace *mspace,
    tn_u8      alignment,
    tn_size    size)
{
    tnIN();
    tnEXPECT(!tn_is_null(mspace));
    tnEXPECT(!tn_is_null(mspace->ctx));
    tlsf_ctx *ctx = (tlsf_ctx*)mspace->ctx;
    tnEXPECT(!tn_is_null(ctx->tlsf), "The TLSF allocator requires tn_mspace_pool_create");

    void *rv = tlsf_memalign(ctx->tlsf, alignment, size);
    tnRET_IF(tn_is_null(rv), tnNULL);

    tnOUT_ARG("rv:x%x", rv);
    return rv;
}



/* Create tlsf pool. At the moment, only one tlsf pool is supported. */
TN_ALWAYS_INLINE static tn_result _tlsf_allocator_pool_add(
    tn_mspace *mspace,
    void      *addr,
    tn_size    size);

TN_ALWAYS_INLINE
static tn_result
_tlsf_allocator_pool_add(
    struct tn_mspace *mspace,
    void             *addr,
    tn_size           size)
{
    tnIN();
    tnEXPECT(!tn_is_null(mspace));
    tnEXPECT(!tn_is_null(mspace->ctx));
    tlsf_ctx *ctx = (tlsf_ctx*)mspace->ctx;
    if (ctx->tlsf == tnNULL)
    {
        ctx->tlsf = tlsf_create_with_pool(addr, size);
        tnRET_IF(tn_is_null(ctx->tlsf), TN_OUT_OF_MEMORY_ERROR);
    }
    else
    {
        tnEXPECT(!tn_is_null(ctx->tlsf));
        pool_t p = tlsf_add_pool(ctx->tlsf, addr, size);
        tnRET_IF(p == 0, TN_OUT_OF_MEMORY_ERROR);
    }
    tnOUT();
    return TN_NO_ERROR;
}



/* Accumulator passed through the pool walker. */
typedef struct
{
    tn_size   free_bytes;
} _tlsf_free_space_ctx;

/* tlsf_walker callback: add the size of every free block. */
static void
_tlsf_free_space_walker(
    void  *ptr,
    size_t size,
    int    used,
    void  *user)
{
    tnUNUSED(ptr);
    if (!used)
    {
        _tlsf_free_space_ctx *ctx = (_tlsf_free_space_ctx*)user;
        ctx->free_bytes += (tn_size)size;
    }
}



TN_ALWAYS_INLINE static tn_size _tlsf_allocator_get_free_space(tn_mspace *mspace);

TN_ALWAYS_INLINE
static tn_size
_tlsf_allocator_get_free_space(tn_mspace *mspace)
{
    tnIN();
    tnEXPECT(!tn_is_null(mspace));
    tnEXPECT(!tn_is_null(mspace->ctx));
    tlsf_ctx *ctx = (tlsf_ctx*)mspace->ctx;
    tnEXPECT(!tn_is_null(ctx->tlsf), "The TLSF allocator requires tn_mspace_pool_create");

    _tlsf_free_space_ctx acc;
    acc.free_bytes = 0;

    pool_t pool = tlsf_get_pool(ctx->tlsf);
    tlsf_walk_pool(pool, _tlsf_free_space_walker, &acc);

    tnOUT_ARG("free_bytes:%u", acc.free_bytes);
    return acc.free_bytes;
}



tn_mspace_allocator tn_mspace_allocator_tlsf =
{
    .create         = &_tlsf_allocator_create,
    .destroy        = &_tlsf_allocator_destroy,
    .alloc          = &_tlsf_allocator_alloc,
    .realloc        = &_tlsf_allocator_realloc,
    .alloc_aligned  = &_tlsf_allocator_alloc_aligned,
    .zone_add       = &_tlsf_allocator_pool_add,
    .get_free_space = &_tlsf_allocator_get_free_space,
};
