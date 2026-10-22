//
// Copyright 2025-2026 NXP
//
// SPDX-License-Identifier: MIT
//

#include "tn_cyc_cnt.h"
#include "tn_prof.h"

void
tn_prof_get_counters(tn_prof_counters *counters)
{
#if defined(TN_PROF)
    counters->driver_cycles = tn_ticks() - tn_ticks_sleep();
    counters->gpu_cycles = tn_gpu_ticks();
#endif
}
