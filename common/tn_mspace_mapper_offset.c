//
// Copyright 2026 NXP
//
// SPDX-License-Identifier: MIT
//

#include "tn_mspace_mapper_offset.h"
#include "tn_mspace.h"
#include "tn_compiler.h"
#include "tn_aver.h"
#include "tn_trace.h"



TN_ALWAYS_INLINE static
tn_vptr
_offset_mapper_vaddr(
    tn_mspace *mspace,
    tn_paddr   paddr)
{
    tnIN_ARG("paddr:x%x", paddr);
    tnEXPECT(mspace->pool.mbuf.paddr <= paddr);
    tn_ptrdiff addr_offset = (tn_uptr)paddr - (tn_uptr)mspace->pool.mbuf.paddr;
    tnEXPECT(0 <= addr_offset && addr_offset < mspace->pool.mbuf.size);
    tn_vptr vaddr = (tn_vptr)( (tn_uptr)mspace->pool.mbuf.vaddr + addr_offset );
    tnOUT_ARG("vaddr:x%x", vaddr);
    return vaddr;
}



TN_ALWAYS_INLINE static
tn_paddr
_offset_mapper_paddr(
    tn_mspace *mspace,
    tn_vptr    vaddr)
{
    tnIN_ARG("vaddr:x%x", vaddr);
    tnEXPECT(mspace->pool.mbuf.vaddr <= vaddr);
    tn_ptrdiff addr_offset = (tn_uptr)vaddr - (tn_uptr)mspace->pool.mbuf.vaddr;
    tnEXPECT(0 <= addr_offset);
    tnEXPECT(addr_offset < mspace->pool.mbuf.size);
    tnOUT_ARG("paddr:x%x", addr_offset);
    return addr_offset;
}



tn_mspace_mapper tn_mspace_mapper_offset =
{
    .paddr_to_vaddr = &_offset_mapper_vaddr,
    .vaddr_to_paddr = &_offset_mapper_paddr,
};
