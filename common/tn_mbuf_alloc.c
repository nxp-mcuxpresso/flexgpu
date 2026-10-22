//
// Copyright 2026 NXP
//
// SPDX-License-Identifier: MIT
//

#include "tin.h"
#include "tn_aver.h"
#include "tn_mspace.h"
#include "tn_mbuf.h"
#include "tn_types.h"


tn_result
tn_mbuf_alloc(
    tn_mbuf *mbuf,
    tn_size  size)
{
    tnIN_ARG("size:%d", size);

    tnEXPECT(!tn_is_null(mbuf));

    mbuf->vaddr = tnNULL;
    mbuf->paddr = 0;
    mbuf->size  = 0;

    tn_vptr vaddr = tn_mspace_alloc(TN_MSPACE_SHMEM_0, size);
    tnRET_IF(tn_is_null(vaddr), TN_OUT_OF_MEMORY_ERROR);

    mbuf->vaddr = vaddr;
    mbuf->size  = size;
    mbuf->paddr = tn_mspace_paddr(TN_MSPACE_SHMEM_0, vaddr);

    tnOUT_ARG("vaddr:%p size:%d", mbuf->vaddr, size);
    return TN_NO_ERROR;
}



tn_result
tn_mbuf_alloc_aligned(
    tn_mbuf *mbuf,
    tn_u8    alignment,
    tn_size  size)
{
    tnIN_ARG("size:%d", size);

    tnEXPECT(!tn_is_null(mbuf));

    mbuf->vaddr = tnNULL;
    mbuf->paddr = 0;
    mbuf->size  = 0;

    tn_vptr vaddr = tn_mspace_alloc_aligned(TN_MSPACE_SHMEM_0, alignment, size);
    tnRET_IF(tn_is_null(vaddr), TN_OUT_OF_MEMORY_ERROR);

    mbuf->vaddr = vaddr;
    mbuf->size  = size;
    mbuf->paddr = tn_mspace_paddr(TN_MSPACE_SHMEM_0, vaddr);

    tnOUT_ARG("vaddr:%p size:%d", mbuf->vaddr, size);
    return TN_NO_ERROR;
}



tn_result
tn_mbuf_release(tn_mbuf *mbuf)
{
    tnIN();

    tnEXPECT(!tn_is_null(mbuf));
    tnEXPECT(mbuf->vaddr != tnNULL);

    tn_mspace_free(TN_MSPACE_SHMEM_0, mbuf->vaddr);

    mbuf->vaddr = tnNULL;
    mbuf->paddr = 0;
    mbuf->size  = 0;

    tnOUT();
    return TN_NO_ERROR;
}
