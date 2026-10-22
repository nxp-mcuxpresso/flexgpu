//
// Copyright 2025 NXP
//
// SPDX-License-Identifier: MIT
//

#ifndef __TN_BARRIER_H__
#define __TN_BARRIER_H__

#define tn_barrier_dsb() __asm volatile("dsb")
#define tn_barrier_isb() __asm volatile("isb")
#define tn_barrier_dmb() __asm volatile("dmb")

#endif // __TN_BARRIER_H__
