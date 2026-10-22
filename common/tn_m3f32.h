//
// Copyright 2025-2026 NXP
//
// SPDX-License-Identifier: MIT
//

#ifndef __TN_M3F32_H__
#define __TN_M3F32_H__

/// @file tn_m3f32.h
/// M3F32: A 3x3 matrix of 32-bit float values.

#include "tn_v3f32.h"
#include "tn_log.h"

#ifdef __cplusplus
extern "C" {
#endif


/// A 3x3 matrix consisting of floating pointer numbers in [row][column] order.
typedef union
{
    tn_f32     m[3][3];
    tn_v3f32   v[3];
} tn_m3f32;

void tn_m3f32_identity(tn_m3f32 *m1);

tn_m3f32 tn_m3f32_mul_m3f32(
    const tn_m3f32*,
    const tn_m3f32*);

void tn_m3f32_mul_m3f32_imm(
    tn_m3f32*,
    const tn_m3f32*);

tn_bool tn_m3f32_invert_imm(tn_m3f32*);

void tn_m3f32_translate_imm(
    tn_m3f32*,
    tn_f32 tx,
    tn_f32 ty);

void tn_m3f32_scale_imm(
    tn_m3f32*,
    tn_f32 sx,
    tn_f32 sy);

void tn_m3f32_shear_imm(
    tn_m3f32*,
    tn_f32 shx,
    tn_f32 shy);

void tn_m3f32_rotate_deg_imm(
    tn_m3f32*,
    tn_f32 degrees);

void tn_m3f32_perspective_imm(
    tn_m3f32 *m1,
    tn_f32    px,
    tn_f32    py);

tn_bool tn_m3f32_is_affine(const tn_m3f32 *m1);

tn_bool tn_m3f32_is_translation_only(const tn_m3f32 *m1);

void tn_m3f32_print(const tn_m3f32 *m1);

#if TN_LOG_LEVEL >= TN_LOG_INFO
    #define tnLOG_M3F32(label, m)                                             \
        do {                                                                  \
            tnLOG_INFO("%s", label);                                          \
            tn_m3f32_print(m);                                                \
        } while (0)
#else
    #define tnLOG_M3F32(label, m)                                             \
        do {                                                                  \
            if (0)                                                            \
            {                                                                 \
                tnLOG_INFO("%s", label);                                      \
                tn_m3f32_print(m);                                            \
            }                                                                 \
        } while (0)
#endif

void tn_m3f32_get_blend_area(
    tn_rect_t *rect,
    tn_m3f32  *mat,
    tn_u32     w,
    tn_u32     h,
    tn_bool    translation_only);



/// Matrix * Column-Vector
static inline tn_v3f32
tn_m3f32_mul_v3f32(
    const tn_m3f32 *m,
    const tn_v3f32 *v)
{
    tn_v3f32 rv;
    rv.x = v->x * m->m[0][0] + v->y * m->m[0][1] + v->z * m->m[0][2];
    rv.y = v->x * m->m[1][0] + v->y * m->m[1][1] + v->z * m->m[1][2];
    rv.z = v->x * m->m[2][0] + v->y * m->m[2][1] + v->z * m->m[2][2];
    return rv;
}



/// Matrix * Column-Vector
static inline void
tn_m3f32_mul_v3f32_imm(
    const tn_m3f32 *m,
    tn_v3f32       *v)
{
    tn_v3f32 rv;
    rv.x = v->x * m->m[0][0] + v->y * m->m[0][1] + v->z * m->m[0][2];
    rv.y = v->x * m->m[1][0] + v->y * m->m[1][1] + v->z * m->m[1][2];
    rv.z = v->x * m->m[2][0] + v->y * m->m[2][1] + v->z * m->m[2][2];
    *v = rv;
}



#ifdef __cplusplus
}
#endif

#endif // __TN_M3F32_H__
