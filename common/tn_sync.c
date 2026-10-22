//
// Copyright 2025-2026 NXP
//
// SPDX-License-Identifier: MIT
//

#include "tn_sync.h"
#include "tn_cmd_buf_ccdma.h"


tn_result
tn_sync_on_finish(void)
{
    return tn_cmd_buf_ccdma_finish();
}



tn_result
tn_sync_on_flush(void)
{
    return tn_cmd_buf_ccdma_flush();
}



tn_result
tn_sync_on_img_operation(void)
{
    return TN_NO_ERROR;
}
