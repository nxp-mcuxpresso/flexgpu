//
// Copyright 2025-2026 NXP
//
// SPDX-License-Identifier: MIT
//

#ifndef __TN_SHBUF_H__
#define __TN_SHBUF_H__

/// @file tn_mbuf.h
/// Shared Memory Buffer.

#include "tn_types.h"


struct tn_range;
typedef struct tn_mbuf
{
    tn_vptr    vaddr;
    tn_size    size;
    tn_paddr   paddr;
} tn_mbuf;

tnNODISCARD tn_result tn_mbuf_init(void);

void tn_mbuf_deinit(void);

/// Create a new MemoryBuffer.
/// \param[out] id_shbuf            Newly created MemoryBuffer.
/// \param[in]  size             Size to allocate.
tn_result tn_mbuf_create(
    tn_mbuf_id *id_shbuf,
    tn_size     size);

/// Create a new aligned MemoryBuffer.
/// \param[out] id_shbuf            Newly created MemoryBuffer.
/// \param[in]  align            Memory alignment
/// \param[in]  size             Size to allocate.
tn_result tn_mbuf_create_aligned(
    tn_mbuf_id *id_shbuf,
    tn_u8       alignment,
    tn_size     size);

/// Create a new MemoryBuffer using \p data as backing storage.
/// \param[out] id_shbuf           Newly created MemoryBuffer.
/// \param[in]  range            Data to use as backing storage.
tn_result tn_mbuf_wrap(
    tn_mbuf_id           *id_shbuf,
    const struct tn_mbuf *shbuf);

/// Release a MemoryBuffer that was created with tn_mbuf_wrap.
/// Only the internal bookkeeping slot is released; the backing memory is
/// owned by the caller and is NOT freed.
/// \param[in] id_shbuf           The wrapped MemoryBuffer to release.
tn_result tn_mbuf_unwrap(tn_mbuf_id id_shbuf);

tn_result tn_mbuf_vaddr(
    tn_mbuf_id id_shbuf,
    tn_vptr   *addr);

tn_result tn_mbuf_paddr(
    tn_mbuf_id id_shbuf,
    tn_paddr  *addr);

tn_result tn_mbuf_copy(
    tn_mbuf_id id_shbuf,
    tn_mbuf   *shbuf);

/// Destroy a MemoryBuffer.
/// \param[in] id_shbuf            The MemoryBuffer to free.
tn_result tn_mbuf_destroy(tn_mbuf_id id_shbuf);

tn_result tn_mbuf_ref(
    tn_mbuf_id id_shbuf,
    tn_mbuf**);

/// Allocate shared memory into a caller-owned MemoryBuffer descriptor.
/// Produces the same vaddr/paddr/size triple as tn_mbuf_create() followed by
/// tn_mbuf_vaddr() and tn_mbuf_paddr(), without consuming a slab slot.
/// \param[out] mbuf             Descriptor to fill in.
/// \param[in]  size             Size to allocate.
tn_result tn_mbuf_alloc(
    tn_mbuf *mbuf,
    tn_size  size);

/// Aligned counterpart of tn_mbuf_alloc(), matching tn_mbuf_create_aligned().
/// \param[out] mbuf             Descriptor to fill in.
/// \param[in]  alignment        Memory alignment.
/// \param[in]  size             Size to allocate.
tn_result tn_mbuf_alloc_aligned(
    tn_mbuf *mbuf,
    tn_u8    alignment,
    tn_size  size);

/// Free memory obtained from tn_mbuf_alloc() or tn_mbuf_alloc_aligned() and
/// reset the descriptor. A descriptor with a NULL vaddr is left alone, so this
/// is safe to call unconditionally and safe to call twice.
/// \param[in,out] mbuf          Descriptor to release.
tn_result tn_mbuf_release(tn_mbuf *mbuf);

#endif // __TN_SHBUF_H__
