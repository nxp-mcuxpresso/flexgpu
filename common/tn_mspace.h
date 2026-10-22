//
// Copyright 2026 NXP
//
// SPDX-License-Identifier: MIT
//

#ifndef   __MSPACE_H__
#define   __MSPACE_H__

/// @file tn_mspace.h
/// MemorySpace, the allocator context.
/// Sysheap is the default global MemorySpace.

#include "tn_types.h"
#include "tn_result.h"
#include "tn_mspace_allocator.h"
#include "tn_mspace_mapper.h"
#include "tn_mspace_pool.h"
#include "tn_config.h"  // for TN_CONFIG_SHM0_POOL_SIZE

typedef enum
{
    TN_MSPACE_SYSHEAP = 0,
    TN_MSPACE_SHMEM_0 = 1,
    TN_MSPACE_ID_MAX
} TN_MSPACE_ID;


struct tn_mspace;


typedef void* (*mspace_realloc_fptr)(
    tn_mspace_id id_mspace,
    void        *ptr,
    tn_size      size);

//typedef void* (*mspace_create_fptr)(
//    tn_mspace_id id_mspace,
//    struct tn_mspace *mspace);
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////


typedef struct tn_mspace
{
    tn_mspace_allocator   allocator;
    tn_mspace_pool        pool;
    tn_mspace_mapper      mapper;
    void                 *ctx;
} tn_mspace;
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

void tn_mspace_init(void);

/// \param[in] id_mspace            The memory space to create.
/// \param[in] mspace_allocator     Allocator backend to bind.
/// \param[in] mspace_mapper        Virtual/physical address mapper to bind.
tnNODISCARD tn_result tn_mspace_create(
    tn_mspace_id                id_mspace,
    struct tn_mspace_allocator *mspace_allocator,
    struct tn_mspace_mapper    *mspace_mapper);

tn_result tn_mspace_destroy(tn_mspace_id id_mspace);

//
//

/// Allocate a chunk of memory.
/// \param[in]  id_mspace               The Allocator-Object.
/// \param[out] mbuf                Allocated buffer
/// \param[in]  mbuf                Buffer size
void * tn_mspace_alloc(
    tn_mspace_id id_mspace,
    tn_size      size);

/// Allocate a chunk of aligned memory.
/// \param[in]  id_mspace               The Allocator-Object.
/// \param[out] mbuf                Allocated buffer
/// \param[in]  mbuf                Buffer size
/// \param[in]  align                Alignment
void * tn_mspace_alloc_aligned(
    tn_mspace_id id_mspace,
    tn_u8        alignment,
    tn_size      size);

/// Reallocate a chunk of memory.
/// \param[in]  id_mspace               The Allocator-Object.
/// \param[out] vaddr                Allocated memory address.
/// \param[in]  size                 How much memory to allocate in bytes.
void * tn_mspace_realloc(
    tn_mspace_id id_mspace,
    void        *vaddr,
    tn_size      size);

/// Free a chunk of memory.
/// \param[in]  id_mspace               The Allocator-Object.
/// \param[in]  vaddr                Memory address to free.
tn_result tn_mspace_free(
    tn_mspace_id id_mspace,
    void        *vaddr);

/// \param[in] id_mspace            The memory space to attach the pool to.
/// \param[in] mspace_pool          Pool backend to bind.
/// \param[in] mbuf                 Descriptor of the memory region to manage.
tnNODISCARD tn_result tn_mspace_pool_create(
    tn_mspace_id           id_mspace,
    struct tn_mspace_pool *mspace_pool,
    struct tn_mbuf        *mbuf);

void tn_mspace_pool_info_get(
    tn_mspace_id    id_mspace,
    struct tn_mbuf *o_hbuf);

tn_vptr tn_mspace_vaddr(
    tn_mspace_id id_mspace,
    tn_paddr     paddr);

tn_paddr tn_mspace_paddr(
    tn_mspace_id id_mspace,
    tn_vptr      vaddr);

tn_size tn_mspace_get_free_space(tn_mspace_id id_mspace);

#endif // __MSPACE_H__
