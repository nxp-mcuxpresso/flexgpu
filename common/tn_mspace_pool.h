//
// Copyright 2026 NXP
//
// SPDX-License-Identifier: MIT
//

#ifndef   __TN_MSPACE_ZONE_H__
#define   __TN_MSPACE_ZONE_H__

#include "tn_mbuf.h"

struct tn_mspace;

typedef tn_result (*mspace_pool_create_fptr)(
    struct tn_mspace *mspace,
    struct tn_mbuf   *mbuf);

typedef tn_result (*mspace_pool_destroy_fptr)(struct tn_mspace *mspace);

typedef tn_vptr   (*mspace_pool_vaddr_fptr)(
    struct tn_mspace *mspace,
    tn_paddr          paddr);

typedef tn_paddr (*mspace_pool_paddr_fptr)(
    struct tn_mspace *mspace,
    tn_vptr           vaddr);

typedef void (*mspace_pool_info_get_fptr)(
    struct tn_mspace *mspace,
    struct tn_mbuf   *mbuf);


typedef struct tn_mspace_pool
{
    mspace_pool_create_fptr     create;
    mspace_pool_destroy_fptr    destroy;
    mspace_pool_vaddr_fptr      vaddr;
    mspace_pool_paddr_fptr      paddr;
    mspace_pool_info_get_fptr   info_get;
    struct tn_mbuf              mbuf;
} tn_mspace_pool;

#endif // __TN_MSPACE_ZONE_H__
