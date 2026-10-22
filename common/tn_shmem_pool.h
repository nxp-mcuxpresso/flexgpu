//
// Copyright 2025-2026 NXP
//
// SPDX-License-Identifier: MIT
//

#ifndef   __TN_SHMEM_POOL_H__
#define   __TN_SHMEM_POOL_H__

#include "tn_mbuf.h"

#ifdef __cplusplus
extern "C" {
#endif

tn_result tn_shmem_pool_create(tn_mspace_id id_shmem);

tn_result tn_shmem_pool_destroy(tn_mspace_id id_shmem);

void tn_shmem_pool_info_get(
    tn_mspace_id    id_shmem,
    struct tn_mbuf *o_shbuf);

#ifdef __cplusplus
}
#endif


#endif // __TN_SHMEM_POOL_H__
