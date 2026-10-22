//
// Copyright 2026 NXP
//
// SPDX-License-Identifier: MIT
//


#include "tn_gbuf.h"
#include "tn_mspace.h"
#include "tn_aver.h"
#include "tn_result.h"
#include "tn_trace.h"

#define GBUF_ALIGN     (8)


// Fill in everything except the addresses and the ownership flag, which the
// two entry points below set according to who owns the memory.
static void
tn_gbuf_set_layout(
    tn_gbuf              *g,
    const tn_gbuf_layout *layout)
{
    g->width        = layout->width;
    g->height       = layout->height;
    g->stride       = layout->stride;
    g->color_format = layout->color_format;
    // No image object supplies these any more; both are the defaults
    // tn_image_create() used to leave behind for a freshly created image.
    g->quality      = 0;
    g->mode         = 0;
    g->yuv2rgb      = layout->yuv2rgb;
    g->uv_offset    = layout->uv_offset;
    g->v_offset     = layout->v_offset;
    g->valid        = tnTRUE;
}



tn_result
tn_gbuf_create(
    tn_gbuf              *g,
    const tn_gbuf_layout *layout)
{
    tnIN();

    tnEXPECT(!tn_is_null(g));
    tnEXPECT(!tn_is_null(layout));
    tnEXPECT(layout->size != 0);

    tn_gbuf_clear(g);

    tn_vptr vaddr = tn_mspace_alloc_aligned(TN_MSPACE_SHMEM_0, GBUF_ALIGN, layout->size);
    tnRET_IF(tn_is_null(vaddr), TN_OUT_OF_MEMORY_ERROR);

    tn_gbuf_set_layout(g, layout);
    g->vaddr = vaddr;
    g->paddr = tn_mspace_paddr(TN_MSPACE_SHMEM_0, vaddr);
    g->size  = layout->size;
    // This is the one entry point that allocates, so it is the one that owns.
    g->owned = tnTRUE;

    tnOUT();
    return TN_NO_ERROR;
}



tn_result
tn_gbuf_wrap(
    tn_gbuf              *g,
    const tn_gbuf_layout *layout,
    tn_vptr               vaddr,
    tn_size               vaddr_len)
{
    tnIN();

    tnEXPECT(!tn_is_null(g));
    tnEXPECT(!tn_is_null(layout));
    tnEXPECT(vaddr != tnNULL);

    tnRET_IF(vaddr_len == 0, TN_ILLEGAL_ARGUMENT_ERROR);
    tnRET_IF(vaddr_len < layout->size, TN_ILLEGAL_ARGUMENT_ERROR);

    tn_gbuf_clear(g);
    tn_gbuf_set_layout(g, layout);
    g->vaddr = vaddr;
    g->paddr = tn_mspace_paddr(TN_MSPACE_SHMEM_0, vaddr);
    g->size  = vaddr_len;
    // The memory belongs to somebody else; tn_gbuf_destroy() must not free it.
    g->owned = tnFALSE;

    tnOUT();
    return TN_NO_ERROR;
}



tn_result
tn_gbuf_destroy(tn_gbuf *g)
{
    tnIN();

    if (g == tnNULL)
    {
        tnOUT();
        return TN_ILLEGAL_ARGUMENT_ERROR;
    }

    if (!g->valid || !g->owned || g->vaddr == tnNULL)
    {
        tn_gbuf_clear(g);
        tnOUT();
        return TN_NO_ERROR;
    }

    tn_result res = tn_mspace_free(TN_MSPACE_SHMEM_0, g->vaddr);
    tn_gbuf_clear(g);

    tnOUT();
    return res;
}
