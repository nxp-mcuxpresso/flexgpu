//
// Copyright 2025-2026 NXP
//
// SPDX-License-Identifier: MIT
//

#ifndef __TN_V3F32_H__
#define __TN_V3F32_H__

/// @file tn_v3f32.h
/// V3F32: a 3x1 (row) of 32-bit floats.

#include "tn_math.h"
#include "tn_trace.h"
#include "tn_log.h"

typedef union
{
    tn_f32   v[3];
    struct
    {
        tn_f32   x;
        tn_f32   y;
        tn_f32   z;
    };
} tn_v3f32;



static inline int
tn_v3f32_is_zero (const tn_v3f32 *v)
{
    return (v->x == 0.0f) && (v->y == 0.0f) && (v->z == 0.0f);
}



static inline tn_bool
tn_v3f32_eq(
    const tn_v3f32 *p0,
    const tn_v3f32 *p1)
{
    return (p0->x == p1->x) && (p0->y == p1->y) && (p0->z == p1->z);
}



static inline tn_v3f32
tn_v3f32_sub(
    const tn_v3f32 *p0,
    const tn_v3f32 *p1)
{
    return (tn_v3f32){{ p0->x - p1->x, p0->y - p1->y, p0->z - p1->z }};
}



static inline void
tn_v3f32_sub_imm(
    tn_v3f32       *p0,
    const tn_v3f32 *p1)
{
    p0->x -= p1->x;
    p0->y -= p1->y;
    p0->z -= p1->z;
}



static inline tn_v3f32
tn_v3f32_add(
    const tn_v3f32 *p0,
    const tn_v3f32 *p1)
{
    return (tn_v3f32){{ p0->x + p1->x, p0->y + p1->y, p0->z + p1->z }};
}



static inline void
tn_v3f32_add_imm(
    tn_v3f32       *p0,
    const tn_v3f32 *p1)
{
    p0->x += p1->x;
    p0->y += p1->y;
    p0->z += p1->z;
}



static inline tn_v3f32
tn_v3f32_scale(
    const tn_v3f32 *p0,
    const tn_f32    s)
{
    return (tn_v3f32){{ p0->x *s, p0->y *s, p0->z *s }};
}



static inline void
tn_v3f32_scale_imm(
    tn_v3f32    *p0,
    const tn_f32 s)
{
    p0->x *= s;
    p0->y *= s;
    p0->z *= s;
}



static inline tn_f32
tn_v3f32_length(tn_v3f32 p)
{
    tn_f64 l2 =  (tn_f64)p.x * (tn_f64)p.x + (tn_f64)p.y * (tn_f64)p.y +  (tn_f64)p.z * (tn_f64)p.z;
    return tn_sqrtf32( (tn_f32)l2 );
}



static inline tn_v3f32
tn_v3f32_normalize(const tn_v3f32 v)
{
    tn_f64 l2 = (tn_f64)v.x * (tn_f64)v.x + (tn_f64)v.y * (tn_f64)v.y + (tn_f64)v.z * (tn_f64)v.z;
    //if( l2 != 0.0 )
    l2 = (tn_f64)tn_rcpsqrtf32( (tn_f32)l2 );     //1.0 / tn_sqrtf32(l);
    return (tn_v3f32){{
                          (tn_f32)((tn_f64)v.x * l2),
                          (tn_f32)((tn_f64)v.y * l2),
                          (tn_f32)((tn_f64)v.z * l2)
                      }};
}



static inline void
tn_v3f32_normalize_imm(tn_v3f32 *v)
{
    tn_f64 l2 = (tn_f64)v->x * (tn_f64)v->x + (tn_f64)v->y * (tn_f64)v->y + (tn_f64)v->z * (tn_f64)v->z;
    //if( l2 != 0.0 )
    l2 = (tn_f64)tn_rcpsqrtf32( (tn_f32)l2 );     //1.0 / tn_sqrtf32(l);
    v->x = (tn_f32)((tn_f64)v->x * l2);
    v->y = (tn_f32)((tn_f64)v->y * l2);
    v->z = (tn_f32)((tn_f64)v->z * l2);
}



static inline void
tn_v3f32_print(tn_v3f32 *v)
{
    tnLOG_INFO("x:%f y:%f z:%f", v->x, v->y, v->z);
}



#endif // __TN_V3F32_H__
