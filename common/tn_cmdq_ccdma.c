//
// Copyright 2026 NXP
//
// SPDX-License-Identifier: MIT
//

#include "tn_cmdq_ccdma.h"
#include "tn_aver.h"
#include "tn_mem.h"
#include "tn_mbuf.h"
#include "tn_misc.h"
#include "tn_init.h"
#include "tn_trace.h"

#define CMDQ_ALIGN     (8)

static tn_cmdq_ccdma _cmd_buf = {0};
static tn_bool _created = tnFALSE;

static tn_cmdq_ccdma *g_cmd_buf = &_cmd_buf;



void
tn_cmdq_ccdma_init(void)
{
    tnIN();
    tn_memset(g_cmd_buf, 0, sizeof(*g_cmd_buf));
    _created = tnFALSE;
    tnOUT();
}



tn_result
tn_cmdq_ccdma_create(
    tn_u32  phy_addr,
    tn_size num_entries)
{
    tnIN_ARG("buffer_size (8 bytes entry):%d", num_entries);

    tn_result rv;

    // Only one command queue exists, so a second create is a caller error.
    tnEXPECT(!_created);

    tn_mbuf mbuf = {0};
    tn_bool owned = tnFALSE;
    if (phy_addr == 0)
    {
        rv = tn_mbuf_alloc_aligned(&mbuf, CMDQ_ALIGN, num_entries * 8U);
        tnRET_IF(rv != TN_NO_ERROR, rv);
        owned = tnTRUE;
    }
    else
    {
        mbuf.vaddr = (tn_vptr)(tn_uptr)phy_addr;
        mbuf.size  = num_entries * 8U;
        mbuf.paddr = phy_addr;
    }

    g_cmd_buf->mbuf  = mbuf;
    g_cmd_buf->owned = owned;

    g_cmd_buf->start = 0;
    g_cmd_buf->end = 0;
    g_cmd_buf->is_full = tnFALSE;
    // last entry is a LINK command pointing to the base address
    g_cmd_buf->size = num_entries - 1;

    _created = tnTRUE;

    tnOUT();
    return TN_NO_ERROR;
}



void
tn_cmdq_ccdma_destroy(void)
{
    tnIN();
    if (_created)
    {
        if (g_cmd_buf->owned)
        {
            tn_mbuf_release(&g_cmd_buf->mbuf);
        }
        tn_memset(g_cmd_buf, 0, sizeof(*g_cmd_buf));
        _created = tnFALSE;
    }
    tnOUT();
}



tn_paddr
tn_cmdq_ccdma_paddr(void)
{
    tnIN_ARG("cmdq_ccdma:x%x", g_cmd_buf);
    tn_paddr paddr = g_cmd_buf->mbuf.paddr;
    tnOUT_ARG("paddr:x%x", paddr);
    return paddr;
}



tn_i32
tn_cmdq_ccdma_enq(
    const tn_u32 cmd,
    const tn_u32 payload)
{
    tnIN_ARG("start:%d end:%d", g_cmd_buf->start, g_cmd_buf->end);

    tnRET_IF(g_cmd_buf->is_full, -1);
    tnRET_IF(tn_is_null(g_cmd_buf->mbuf.vaddr), -1);

    ((tn_u32*)g_cmd_buf->mbuf.vaddr)[g_cmd_buf->end << 1] = cmd;
    ((tn_u32*)g_cmd_buf->mbuf.vaddr)[(g_cmd_buf->end << 1) + 1] = payload;

    tn_size curr_end = g_cmd_buf->end;
    g_cmd_buf->end = (g_cmd_buf->end + 1) % g_cmd_buf->size;

    // check if buffer is now full
    if (g_cmd_buf->start == g_cmd_buf->end)
    {
        g_cmd_buf->is_full = tnTRUE;
    }

    tnOUT_ARG("new end:%d", g_cmd_buf->end);
    return curr_end;
}



tn_paddr
tn_cmdq_ccdma_get_addr(const tn_u32 index)
{
    tnIN_ARG("index %d", index);
    tnOUT();
    return g_cmd_buf->mbuf.paddr + (index << 3);
}



tn_size
tn_cmdq_ccdma_get_next_index(const tn_u32 index)
{
    tnIN_ARG("index %d", index);
    tnOUT();
    return (index + 1) % g_cmd_buf->size;

}



tn_size
tn_cmdq_ccdma_get_start(void)
{
    return g_cmd_buf->start;
}



tn_size
tn_cmdq_ccdma_get_end(void)
{
    return g_cmd_buf->end;
}



void
tn_cmdq_ccdma_write_cmd(
    const tn_size index,
    const tn_u32  cmd,
    const tn_u32  payload)
{
    tnIN_ARG("index %d start:%d end:%d", index, g_cmd_buf->start, g_cmd_buf->end);

    tnRET_VOID_IF(tn_is_null(g_cmd_buf->mbuf.vaddr));

    ((tn_u32*)g_cmd_buf->mbuf.vaddr)[index << 1] = cmd;
    ((tn_u32*)g_cmd_buf->mbuf.vaddr)[(index << 1) + 1] = payload;

    tnOUT();
}



void
tn_cmdq_ccdma_update_start(const tn_size new_start)
{
    tnIN();

    if (new_start != g_cmd_buf->start)
    {
        g_cmd_buf->start = new_start;
        g_cmd_buf->is_full = tnFALSE;
    }

    tnOUT();
}



tn_u32
tn_cmdq_ccdma_get_free_space(void)
{
    if (g_cmd_buf->is_full)
    {
        return 0;
    }

    if (g_cmd_buf->start < g_cmd_buf->end)
    {
        return g_cmd_buf->size - (g_cmd_buf->end - g_cmd_buf->start);
    }
    else
    {
        return g_cmd_buf->start - g_cmd_buf->end;
    }
}



tn_bool
tn_cmdq_ccdma_is_empty(void)
{
    return (!g_cmd_buf->is_full) && (g_cmd_buf->start == g_cmd_buf->end);
}



tn_bool
tn_cmdq_ccdma_is_full(void)
{
    return g_cmd_buf->is_full;
}



tn_size
tn_cmdq_ccdma_get_size(void)
{
    return g_cmd_buf->size;
}



tn_vptr
tn_cmdq_ccdma_get_base(void)
{
    return g_cmd_buf->mbuf.vaddr;
}
