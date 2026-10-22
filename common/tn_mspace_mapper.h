//
// Copyright 2026 NXP
//
// SPDX-License-Identifier: MIT
//

#ifndef   __TN_MSPACE_MAPPER_H__
#define   __TN_MSPACE_MAPPER_H__

#include "tn_types.h"

struct tn_mspace;


typedef tn_paddr (*mspace_paddr_fptr)(
    struct tn_mspace *mspace,
    tn_vptr           vaddr);

typedef tn_vptr (*mspace_vaddr_fptr)(
    struct tn_mspace *mspace,
    tn_paddr          paddr);


typedef struct tn_mspace_mapper
{
    mspace_paddr_fptr   vaddr_to_paddr;
    mspace_vaddr_fptr   paddr_to_vaddr;
} tn_mspace_mapper;


#endif // __TN_MSPACE_MAPPER_H__
