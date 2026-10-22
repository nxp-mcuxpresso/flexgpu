//
// Copyright 2026 NXP
//
// SPDX-License-Identifier: MIT
//

#include "tn_mspace.h"
#include "tn_compiler.h"
#include "tn_aver.h"
#include "tn_mspace_pool.h"
#include "tn_trace.h"


// memory reserved in .ld configuration file
// these variables are provided to configure memory start and size
// extern tn_u32 _shmem_pool_base;
// extern tn_u32 _shmem_pool_size;

TN_ALWAYS_INLINE static tn_result _ldscript_pool_create(
    struct tn_mspace *mspace,
    struct tn_mbuf   *mbuf);

TN_ALWAYS_INLINE
static tn_result
_ldscript_pool_create(
    struct tn_mspace *mspace,
    struct tn_mbuf   *mbuf)
{
    tnIN();
    tnEXPECT(!tn_is_null(mspace));
    tnEXPECT(!tn_is_null(mbuf));
    tnEXPECT(mbuf->size);

    tnRET_IF(tn_is_null(mbuf->vaddr), TN_ILLEGAL_ARGUMENT_ERROR);

    mspace->pool.mbuf.size = mbuf->size;
    mspace->pool.mbuf.vaddr = mbuf->vaddr;
    mspace->pool.mbuf.paddr = (tn_paddr)(tn_uptr)mbuf->vaddr;

    mspace->allocator.zone_add(mspace, mspace->pool.mbuf.vaddr, mspace->pool.mbuf.size);
    tnOUT_ARG("size:%d vaddr:%p paddr:0x%016llx", mspace->pool.mbuf.size, mspace->pool.mbuf.vaddr, mspace->pool.mbuf.paddr);

    return TN_NO_ERROR;
}



TN_ALWAYS_INLINE static tn_result _ldscript_pool_destroy(struct tn_mspace *mspace);

TN_ALWAYS_INLINE
static tn_result
_ldscript_pool_destroy(struct tn_mspace *mspace)
{
    tnIN();
    mspace->pool.mbuf.vaddr = 0;
    mspace->pool.mbuf.paddr = 0;
    mspace->pool.mbuf.size = 0;
    tnOUT();
    return TN_NO_ERROR;
}



TN_ALWAYS_INLINE static void _ldscript_pool_info_get(
    tn_mspace      *mspace,
    struct tn_mbuf *out_mbuf);

TN_ALWAYS_INLINE
static void
_ldscript_pool_info_get(
    tn_mspace      *mspace,
    struct tn_mbuf *out_mbuf)
{
    tnIN();
    tnEXPECT(!tn_is_null(mspace));
    *out_mbuf = mspace->pool.mbuf;
    tnOUT();
}



TN_ALWAYS_INLINE static tn_vptr _ldscript_pool_vaddr(
    tn_mspace *mspace,
    tn_paddr   paddr);

TN_ALWAYS_INLINE
static tn_vptr
_ldscript_pool_vaddr(
    tn_mspace *mspace,
    tn_paddr   paddr)
{
    tnIN_ARG("paddr:0x%llx", paddr);
    tn_vptr vaddr = (tn_vptr)(tn_uptr)paddr;
    tnOUT_ARG("vaddr:%p", vaddr);
    return vaddr;
}



TN_ALWAYS_INLINE static tn_paddr _ldscript_pool_paddr(
    tn_mspace *shmem,
    tn_vptr    vaddr);

TN_ALWAYS_INLINE
static tn_paddr
_ldscript_pool_paddr(
    tn_mspace *mspace,
    tn_vptr    vaddr)
{
    tnIN_ARG("vaddr:%p", vaddr);
    tn_paddr paddr = (tn_paddr)(tn_uptr)vaddr;
    tnOUT_ARG("paddr:0x%llx", paddr);
    return paddr;
}



tn_mspace_pool tn_mspace_pool_ldscript =
{
    .create   = &_ldscript_pool_create,
    .destroy  = &_ldscript_pool_destroy,
    .info_get = &_ldscript_pool_info_get,
    .paddr    = &_ldscript_pool_paddr,
    .vaddr    = &_ldscript_pool_vaddr,
};
