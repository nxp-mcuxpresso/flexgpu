//
// Copyright 2025-2026 NXP
//
// SPDX-License-Identifier: MIT
//

#include "tn_init.h"
#include "tn_aver.h"
#include "tn_ctx.h"
#include "tn_cmd_buf_ccdma.h"
#include "tn_hw.h"
#include "tn_trace.h"
#include "tn_mspace.h"
#include "tn_mspace_allocator_sysheap.h"
#include "tn_mspace_allocator_tlsf.h"
#include "tn_mspace_pool_ldscript.h"
#include "tn_mspace_mapper_identity.h"
#include "tn_cmd_buf_ccdma.h"


extern uint32_t _shmem_pool_base;
extern uint32_t _shmem_pool_size;

static tn_result
tn_init_lib(void)
{
    tnIN();

    tn_result rv;

    tn_mspace_init();

    rv = tn_mspace_create(TN_MSPACE_SYSHEAP, &tn_mspace_allocator_sysheap, &tn_mspace_mapper_identity);
    tnRET_IF(rv != TN_NO_ERROR, rv);

    rv = tn_mspace_create(TN_MSPACE_SHMEM_0, &tn_mspace_allocator_tlsf, &tn_mspace_mapper_identity);
    tnRET_IF(rv != TN_NO_ERROR, rv);

    rv = tn_mspace_pool_create(TN_MSPACE_SHMEM_0, &tn_mspace_pool_ldscript,
                               &(tn_mbuf){(tn_vptr) & _shmem_pool_base, (tn_size) & _shmem_pool_size, (tn_paddr) & _shmem_pool_base});
    tnRET_IF(rv != TN_NO_ERROR, rv);

    rv = tn_cmd_buf_ccdma_init(0, 0);
    tnRET_IF(rv != TN_NO_ERROR, rv);

    tn_ctx_init();

    tnOUT();
    return TN_NO_ERROR;
}



static void
tn_deinit_lib(void)
{
    tnIN();

    tn_ctx_deinit();

    tn_cmd_buf_ccdma_deinit();

    tn_mspace_destroy(TN_MSPACE_SHMEM_0);
    tn_mspace_destroy(TN_MSPACE_SYSHEAP);

    tnOUT();
}



// init kernel driver
static void
tn_init_kernel(void)
{
    tnIN();

    tn_hw_init();

    tnOUT();
}



static void
tn_deinit_kernel(void)
{
    tnIN();

    tn_hw_deinit();

    tnOUT();
}



static int configured = 0;
tn_result
tn_init(void)
{
    tnIN();

    if (configured)
    {
        tnOUT();
        return TN_NO_ERROR;
    }

    tn_init_kernel();

    tn_result rv = tn_init_lib();
    tnRET_IF(rv != TN_NO_ERROR, rv);

    configured = 1;

    tnOUT();
    return TN_NO_ERROR;
}



void
tn_deinit(void)
{
    tnIN();

    if (configured)
    {
        tn_deinit_lib();
        tn_deinit_kernel();

        configured = 0;
    }

    tnOUT();
}
