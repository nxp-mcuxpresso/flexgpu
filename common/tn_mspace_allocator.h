//
// Copyright 2026 NXP
//
// SPDX-License-Identifier: MIT
//

#ifndef   __TN_MSPACE_ALLOCATOR_H__
#define   __TN_MSPACE_ALLOCATOR_H__

#include "tn_types.h"

struct tn_mspace;


typedef void  (*mspace_create_fptr)(struct tn_mspace *mspace);


typedef void* (*alloc_fptr)(
    struct tn_mspace *mspace,
    tn_size           size);


typedef void* (*realloc_fptr)(
    struct tn_mspace *mspace,
    void             *ptr,
    tn_size           size);


typedef void* (*alloc_aligned_fptr)(
    struct tn_mspace *mspace,
    tn_u8             alignment,
    tn_size           size);


typedef void      (*mspace_destroy_fptr)(struct tn_mspace *mspace);


typedef tn_result (*mspace_pool_add_fptr)(
    struct tn_mspace *mspace,
    void             *ptr,
    tn_size           size);


typedef tn_size (*mspace_get_free_space_fptr)(struct tn_mspace *mspace);


typedef struct tn_mspace_allocator
{
    mspace_create_fptr           create;
    mspace_destroy_fptr          destroy;
    alloc_fptr                   alloc;
    realloc_fptr                 realloc;
    alloc_aligned_fptr           alloc_aligned;
    mspace_pool_add_fptr         zone_add;
    mspace_get_free_space_fptr   get_free_space;
} tn_mspace_allocator;


#endif // __TN_MSPACE_ALLOCATOR_H__
