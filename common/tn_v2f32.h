//
// Copyright 2025-2026 NXP
//
// SPDX-License-Identifier: MIT
//

#ifndef __TN_V2F32_H__
#define __TN_V2F32_H__

/// @file tn_v2f32.h
/// V2I32: a 2x1 (row) of 32-bit float.

#include "tn_m3f32.h"
#include "tn_log.h"
#include "tn_dynarr.h"

typedef union
{
    tn_f32   v[2];
    struct
    {
        tn_f32   x;
        tn_f32   y;
    };
} tn_v2f32;



static inline int
tn_v2f32_is_zero (const tn_v2f32 *v)
{
    return (v->x == 0.0f) && (v->y == 0.0f);
}



static inline tn_bool
tn_v2f32_eq(
    const tn_v2f32 *p0,
    const tn_v2f32 *p1)
{
    return (p0->x == p1->x) && (p0->y == p1->y);
}



static inline tn_v2f32
tn_v2f32_sub(
    const tn_v2f32 *p0,
    const tn_v2f32 *p1)
{
    return (tn_v2f32){{ p0->x - p1->x, p0->y - p1->y }};
}



static inline void
tn_v2f32_sub_imm(
    tn_v2f32       *p0,
    const tn_v2f32 *p1)
{
    p0->x -= p1->x;
    p0->y -= p1->y;
}



static inline tn_v2f32
tn_v2f32_add(
    const tn_v2f32 *p0,
    const tn_v2f32 *p1)
{
    return (tn_v2f32){{ p0->x + p1->x, p0->y + p1->y }};
}



static inline void
tn_v2f32_add_imm(
    tn_v2f32       *p0,
    const tn_v2f32 *p1)
{
    p0->x += p1->x;
    p0->y += p1->y;
}



static inline tn_v2f32
tn_v2f32_scale(
    const tn_v2f32 *p0,
    const tn_f32    s)
{
    return (tn_v2f32){{ p0->x *s, p0->y *s }};
}



static inline tn_v2f32
tn_v2f32_frac(const tn_v2f32 *p0)
{
    return (tn_v2f32){{ tn_f32_frac(p0->x), tn_f32_frac(p0->y) }};
}



static inline void
tn_v2f32_scale_imm(
    tn_v2f32    *p0,
    const tn_f32 s)
{
    p0->x *= s;
    p0->y *= s;
}



// AN IMPROVED ALGORITHM FOR HYPOT(A,B)
static inline tn_f32
tn_v2f32_hypot(tn_v2f32 v)
{
    //tn_f64 l = (tn_f64)v.x * (tn_f64)v.x + (tn_f64)v.y * (tn_f64)v.y;
    //return tn_sqrtf32( (tn_f32)l );
    return tn_hypotf32(v.x, v.y);
}



static TN_ALWAYS_INLINE tn_v2f32
tn_v2f32_normalize(const tn_v2f32 v)
{
    // tn_f64 l = (tn_f64)v.x * (tn_f64)v.x + (tn_f64)v.y * (tn_f64)v.y;
    // //if( l != 0.0 )
    //  l = tn_rcpsqrtf32((tn_f32)l); //1.0 / tn_sqrtf32(l);
    // return (tn_v2f32){{(tn_f32)((tn_f64)v.x * l), (tn_f32)((tn_f64)v.y * l)}};
    tn_f32 l = 1.0f / tn_v2f32_hypot(v);
    return (tn_v2f32){{(v.x * l), (v.y * l)}};
}



static TN_ALWAYS_INLINE void
tn_v2f32_normalize_imm(tn_v2f32 *v)
{
    // tn_f64 l = (tn_f64)v->x * (tn_f64)v->x + (tn_f64)v->y * (tn_f64)v->y;
    // //if( l != 0.0 )
    //  l = tn_rcpsqrtf32((tn_f32)l); //1.0 / tn_sqrtf32(l);
    // v->x = (tn_f32)((tn_f64)v->x * l);
    // v->y = (tn_f32)((tn_f64)v->y * l);
    tn_f32 l = 1.0f / tn_v2f32_hypot(*v);
    v->x = v->x * l;
    v->y = v->y * l;
}



static inline void
tn_v2f32_print(tn_v2f32 *v)
{
    tnLOG_INFO("x:%f y:%f", v->x, v->y);
}



static inline tn_v2f32
tn_v2f32_affine(
    const tn_m3f32 *m,
    const tn_v2f32 *v)
{
    tnEXPECT(!tn_is_null(m));
    tnEXPECT(!tn_is_null(v));
    return (tn_v2f32) {{
                           v->x*m->m[0][0] + v->y * m->m[0][1] + m->m[0][2],
                                   v->x *m->m[1][0] + v->y * m->m[1][1] + m->m[1][2]
                       }};
}



static inline tn_v2f32
tn_v2f32_affine_tangent(
    tn_m3f32       *m,
    const tn_v2f32 *v)
{
    tnEXPECT(!tn_is_null(m));
    tnEXPECT(!tn_is_null(v));
    return (tn_v2f32) {{
                           v->x*m->m[0][0] + v->y * m->m[0][1],
                                   v->x *m->m[1][0] + v->y * m->m[1][1]
                       }};
}



static inline tn_v2f32
tn_v2f32_perpendicular(
    const tn_v2f32 *v,
    tn_bool         cw)
{
    if (cw)
    {
        return (tn_v2f32) {{v->y, -1.0f * v->x}};
    }
    else
    {
        return (tn_v2f32) {{-1.0f * v->y, v->x}};
    }
}



typedef_dynarr (tn_v2f32, tn_v2f32_list, static inline)

#endif // __TN_V2F32_H__
