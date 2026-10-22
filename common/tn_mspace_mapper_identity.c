//
// Copyright 2026 NXP
//
// SPDX-License-Identifier: MIT
//

#include "tn_mspace_mapper_identity.h"
#include "tn_mspace.h"
#include "tn_compiler.h"
#include "tn_aver.h"
#include "tn_trace.h"



TN_ALWAYS_INLINE static
tn_vptr
_identity_mapper_vaddr(
    tn_mspace *mspace,
    tn_paddr   paddr)
{
    tnIN_ARG("paddr:x%x", paddr);
    tn_vptr vaddr = (tn_vptr)(tn_uptr)paddr;
    tnOUT_ARG("vaddr:x%x", vaddr);
    return vaddr;
}



TN_ALWAYS_INLINE static
tn_paddr
_identity_mapper_paddr(
    tn_mspace *mspace,
    tn_vptr    vaddr)
{
    tnIN_ARG("vaddr:x%x", vaddr);
    tn_paddr paddr = (tn_paddr)(tn_uptr)vaddr;
    tnOUT_ARG("paddr:x%x", paddr);
    return paddr;
}



tn_mspace_mapper tn_mspace_mapper_identity =
{
    .paddr_to_vaddr = &_identity_mapper_vaddr,
    .vaddr_to_paddr = &_identity_mapper_paddr,
};
