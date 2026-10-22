//
// Copyright 2025-2026 NXP
//
// SPDX-License-Identifier: MIT
//

#ifndef __TN_RANGE_H__
#define __TN_RANGE_H__

/// @file tn_range.h
/// Memory Range (address, size)


typedef struct tn_range
{
    void     *ptr;
    tn_size   size;
} tn_range;


#if defined(__cplusplus)
    #define tnRANGE(x)     tn_range{ &x, sizeof(x) }
    #define tnRANGE_REF(x) tn_range{ &x, sizeof(x) }
#else
    #define tnRANGE(x)         (tn_range){ &x, sizeof(x) }
    #define tnRANGE_REF(x)     & (tn_range){ &x, sizeof(x) }
    #define tnRANGE_AGGR(x, s) & (tn_range){ x, s }
#endif



#endif // __TN_RANGE_H__
