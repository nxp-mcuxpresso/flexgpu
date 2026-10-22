//
// Copyright 2026 NXP
//
// SPDX-License-Identifier: MIT
//

#ifndef TN_CMDQ_CCDMA_H
#define TN_CMDQ_CCDMA_H

#include "tn_types.h"
#include "tn_mbuf.h"

typedef struct tn_cmdq_ccdma
{
    tn_mbuf   mbuf;
    tn_bool   owned;
    tn_u32    start;
    tn_u32    end;
    tn_size   size;
    tn_bool   is_full;
    tn_bool   external;
} tn_cmdq_ccdma;

void tn_cmdq_ccdma_init(void);

/// Create a circular CommandQueue-Object for CCDMA
/// \param[in]  phy_addr             CommandQueue physical addr. Set to 0 if the internal CCDMA buffer is used.
/// \param[in]  num_entries          CommandQueue size of 8 bytes entry.
/// \retval TN_BAD_HANDLE_ERROR:     Failed to create cmdq
/// \retval TN_NO_ERROR:             Everything went ok.
tn_result tn_cmdq_ccdma_create(
    tn_u32  phy_addr,
    tn_size num_entries);

void tn_cmdq_ccdma_destroy(void);

tn_paddr tn_cmdq_ccdma_paddr(void);

tn_i32 tn_cmdq_ccdma_enq(
    const tn_u32 cmd,
    const tn_u32 payload);

// Get physical address of the "index"th entry
tn_paddr tn_cmdq_ccdma_get_addr(const tn_u32 index);

// Get next (circular) index after "index"
tn_size tn_cmdq_ccdma_get_next_index(const tn_u32 index);

tn_size tn_cmdq_ccdma_get_start(void);

tn_size tn_cmdq_ccdma_get_end(void);

// Write command at specific index.
// ("end" is not updated)
void tn_cmdq_ccdma_write_cmd(
    const tn_size index,
    const tn_u32  cmd,
    const tn_u32  payload);

// new_start is the value received on ISR
// after CCDMA finshes processing a cmdq
void tn_cmdq_ccdma_update_start(const tn_size new_start);

tn_u32 tn_cmdq_ccdma_get_free_space(void);

tn_bool tn_cmdq_ccdma_is_empty(void);

tn_bool tn_cmdq_ccdma_is_full(void);

// Returns the usable ring buffer capacity in 8-byte entries.
// This is (num_entries - 1): the last entry is reserved for the CCDMA LINK command.
tn_size tn_cmdq_ccdma_get_size(void);

// Returns the cached base virtual address of the ring buffer.
// Lightweight read of g_cmd_buf->mbuf.vaddr with no trace overhead, for use in
// dump/debug paths that must not produce nested log entries.
tn_vptr tn_cmdq_ccdma_get_base(void);

#endif // TN_CMDQ_CCDMA_H
