//
// Copyright 2025-2026 NXP
//
// SPDX-License-Identifier: MIT
//

#pragma once

// use this unit to:
// * sync caches
// * buffers unlock
// * read status registers
// * cleanup or perform any preparation for the next frame

#include "tn_result.h"
#include "tn_types.h"

tn_result tn_sync_on_finish(void);

tn_result tn_sync_on_flush(void);

tn_result tn_sync_on_img_operation(void);
