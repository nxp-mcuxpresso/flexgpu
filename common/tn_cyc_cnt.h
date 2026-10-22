//
// Copyright 2025 NXP
//
// SPDX-License-Identifier: MIT
//

#ifndef __TN_CYC_CNT_H__
#define __TN_CYC_CNT_H__

#include "tn_types.h"

typedef struct tn_prof_counters
{
    tn_u64   driver_cycles;
    tn_u64   gpu_cycles;
} tn_prof_counters;

void tn_prof_get_counters(tn_prof_counters *counters);

#endif // __TN_CYC_CNT_H__
