//
// Copyright 2026 NXP
//
// SPDX-License-Identifier: MIT
//

#include "tn_mspace.h"
#include "tn_mbuf.h"
#include "tn_mem.h"
#include "tn_aver.h"
#include "tn_trace.h"


static tn_mspace g_mspace[TN_MSPACE_ID_MAX];



static inline tn_mspace*
tn_mspace_at(tn_mspace_id id_mspace)
{
    tnEXPECT((tn_u32)id_mspace < (tn_u32)TN_MSPACE_ID_MAX);
    return &g_mspace[id_mspace];
}



void
tn_mspace_init(void)
{
    tnIN();

    tn_memset(g_mspace, 0, sizeof(g_mspace));

    tnOUT();
}



tn_result
tn_mspace_create(
    tn_mspace_id                id_mspace,
    struct tn_mspace_allocator *mspace_allocator,
    struct tn_mspace_mapper    *mspace_mapper)
{
    tnIN();

    tnEXPECT(!tn_is_null(mspace_allocator));
    tnEXPECT(!tn_is_null(mspace_mapper));

    tnRET_IF(id_mspace >= (tn_mspace_id)TN_MSPACE_ID_MAX, TN_ILLEGAL_ARGUMENT_ERROR);

    tn_mspace *mspace = &g_mspace[id_mspace];
    mspace->allocator = *mspace_allocator;
    mspace->mapper = *mspace_mapper;
    if (mspace->allocator.create)
        mspace->allocator.create(mspace);

    tnOUT();
    return TN_NO_ERROR;
}



tn_result
tn_mspace_destroy(tn_mspace_id id_mspace)
{
    tnIN_ARG("id_mspace:x%x", id_mspace);

    tnRET_IF(id_mspace >= (tn_mspace_id)TN_MSPACE_ID_MAX, TN_ILLEGAL_ARGUMENT_ERROR);
    tn_mspace *mspace = &g_mspace[id_mspace];
    if (mspace->pool.destroy)
        mspace->pool.destroy(mspace);
    if (mspace->allocator.destroy)
        mspace->allocator.destroy(mspace);
    tn_memset(mspace, 0, sizeof(*mspace));

    tnOUT();
    return TN_NO_ERROR;
}



void*
tn_mspace_alloc(
    tn_mspace_id id_mspace,
    tn_size      size)
{
    tnIN_ARG("id_mspace:x%x mbuf.size:%d", id_mspace, size);

    tn_mspace *mspace = tn_mspace_at(id_mspace);

    tn_vptr out_addr = tnNULL;
    tnEXPECT(mspace->allocator.alloc != tnNULL);
    out_addr = mspace->allocator.alloc(mspace, size);
    tnRET_IF(tn_is_null(out_addr), tnNULL);

    tnOUT_ARG("id_mspace:x%x out_addr:x%x", id_mspace, out_addr);
    return out_addr;
}



void*
tn_mspace_alloc_aligned(
    tn_mspace_id id_mspace,
    tn_u8        alignment,
    tn_size      size)
{
    tnIN_ARG("id_mspace:x%x alignment:%d mbuf.size:%d", id_mspace, alignment, size);

    tn_mspace *mspace = tn_mspace_at(id_mspace);

    tn_vptr out_addr = tnNULL;
    tnEXPECT(mspace->allocator.alloc_aligned != tnNULL);
    out_addr = mspace->allocator.alloc_aligned(mspace, alignment, size);
    tnRET_IF(tn_is_null(out_addr), tnNULL);

    tnOUT_ARG("id_mspace:x%x out_addr:x%x", id_mspace, out_addr);
    return out_addr;
}



void*
tn_mspace_realloc(
    tn_mspace_id id_mspace,
    void        *vaddr,
    tn_size      size)
{
    tnIN_ARG("id_mspace:x%x", id_mspace);

    tn_mspace *mspace = tn_mspace_at(id_mspace);

    tn_vptr out_addr = tnNULL;
    tnEXPECT(mspace->allocator.realloc != tnNULL);
    out_addr = mspace->allocator.realloc(mspace, vaddr, size);
    // a size of 0 legitimately yields a null pointer, that is not a failure
    tnRET_IF(size != 0 && tn_is_null(out_addr), tnNULL);

    tnOUT_ARG("id_mspace:x%x vaddr:x%x:x%x paddr:x%x", id_mspace, out_addr);
    return out_addr;
}



tn_result
tn_mspace_free(
    tn_mspace_id id_mspace,
    void        *vaddr)
{
    tnIN_ARG("id_mspace:%x", id_mspace);

    tn_mspace *mspace = tn_mspace_at(id_mspace);
    tnEXPECT(mspace->allocator.realloc != tnNULL);
    // coverity[RESOURCE_LEAK]
    (void)mspace->allocator.realloc(mspace, vaddr, 0);

    tnOUT();
    return TN_NO_ERROR;
}



tn_result
tn_mspace_pool_create(
    tn_mspace_id           id_mspace,
    struct tn_mspace_pool *mspace_pool,
    struct tn_mbuf        *mbuf)
{
    tnEXPECT(!tn_is_null(mbuf));
    tnEXPECT(!tn_is_null(mspace_pool));
    tnIN_ARG("id_mspace:x%x vaddr:x%x paddr:x%x size:x%x", id_mspace, mbuf->vaddr, mbuf->size);

    tn_mspace *mspace = tn_mspace_at(id_mspace);
    mspace->pool = *mspace_pool;
    tnEXPECT(mspace->pool.create != tnNULL);
    tn_result rv = mspace->pool.create(mspace, mbuf);

    tnOUT();

    return rv;
}



tn_vptr
tn_mspace_vaddr(
    tn_mspace_id id_mspace,
    tn_paddr     paddr)
{
    tnIN_ARG("paddr:x%x", paddr);

    tn_mspace *mspace = tn_mspace_at(id_mspace);
    tnEXPECT(mspace->mapper.paddr_to_vaddr != tnNULL);
    tn_vptr vaddr = mspace->mapper.paddr_to_vaddr(mspace, paddr);

    tnOUT_ARG("vaddr:x%x", vaddr);
    return vaddr;
}



tn_paddr
tn_mspace_paddr(
    tn_mspace_id id_mspace,
    tn_vptr      vaddr)
{
    tnIN_ARG("vaddr:x%x", vaddr);

    tn_mspace *mspace = tn_mspace_at(id_mspace);
    tnEXPECT(mspace->mapper.vaddr_to_paddr != tnNULL);
    tn_paddr paddr = mspace->mapper.vaddr_to_paddr(mspace, vaddr);

    tnOUT_ARG("paddr:x%x", paddr);
    return paddr;
}



void
tn_mspace_pool_info_get(
    tn_mspace_id    id_mspace,
    struct tn_mbuf *out_mbuf)
{
    tnEXPECT(!tn_is_null(out_mbuf));
    tn_mspace *mspace = tn_mspace_at(id_mspace);
    tnEXPECT(mspace->pool.info_get != tnNULL);
    mspace->pool.info_get(mspace, out_mbuf);
}



tn_size
tn_mspace_get_free_space(tn_mspace_id id_mspace)
{
    tnIN_ARG("id_mspace:x%x", id_mspace);

    tn_mspace *mspace = tn_mspace_at(id_mspace);

    if (mspace->allocator.get_free_space == tnNULL)
    {
        tnOUT_ARG("free_bytes:%u (unsupported)", 0U);
        return 0;
    }
    tn_size free_bytes = mspace->allocator.get_free_space(mspace);

    tnOUT_ARG("free_bytes:%u", free_bytes);
    return free_bytes;
}
