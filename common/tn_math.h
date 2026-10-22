//
// Copyright 2025-2026 NXP
//
// SPDX-License-Identifier: MIT
//

#ifndef __TN_MATH_H__
#define __TN_MATH_H__

/// @file tn_math.h


#include "tn_aver.h"
#include "tn_types.h"

#include <limits.h>
#include <float.h> // FLT_MAX
#include <math.h>

#ifndef PI
    #define PI 3.141592654f
#endif



static float gPiConsts[5] = { 1.5707963267f, 3.1415926535f,
                              4.7123889803f, 6.2831853071f,
                              1.2732395447f };
static float gSinConsts[4] = {
    0.999996615908002773079325846913220383f,
    -0.16664828381895056829366054140948866f,
    0.00830632522715989396465411782615901079f,
    -0.00018363653976946785297280224158683484f };

static float gCosConsts[5] = {
    0.999999953466670136306412430924463351f,
    -0.49999905347076729097546897993796764f,
    0.0416635846931078386653947196040757567f,
    -0.00138537043082318983893723662479142648f,
    0.0000231539316590538762175742441588523467f };

static inline float tn_sinf32(float angleRad);

static inline float
tn_sinf32(float angleRad)
{
    float trigApprox;
    float x2, x4;
    float c1, c2, c3;

    // wrap to [0, 2PI]
    if (angleRad < 0)
        angleRad += gPiConsts[3];

    // map to [-PI/2, PI/2]
    if (angleRad > gPiConsts[2])
    {
        angleRad = angleRad - gPiConsts[3];
    }
    else if (angleRad > gPiConsts[0])
    {
        angleRad = gPiConsts[1] - angleRad;
    }

    x2 = angleRad * angleRad;
    x4 = x2 * x2;

    trigApprox = angleRad * gSinConsts[0];
    c1 = angleRad * gSinConsts[3];
    c2 = angleRad * gSinConsts[1];
    c3 = angleRad * gSinConsts[2];

    c1 *= x2;

    trigApprox += c2 * x2;
    trigApprox += c3 * x4;
    trigApprox += c1 * x4;

    return trigApprox;
}



static inline float tn_cosf32(float angleRad);

static inline float
tn_cosf32(float angleRad)
{
    uint8_t sign = 0;
    float trigApprox;
    float x2, x4;
    float c1, c2;

    // map to [-PI, PI]
    if (angleRad > gPiConsts[1])
    {
        angleRad -= gPiConsts[3];
    }
    else if (angleRad < -gPiConsts[1])
    {
        angleRad += gPiConsts[3];
    }

    // map to [-PI/2, PI/2]
    if (angleRad > gPiConsts[0])
    {
        angleRad = gPiConsts[1] - angleRad;
        sign = 1;
    }
    else if (angleRad < -gPiConsts[0])
    {
        angleRad = -gPiConsts[1] - angleRad;
        sign = 1;
    }

    x2 = angleRad * angleRad;
    x4 = x2 * x2;

    trigApprox = gCosConsts[0];
    trigApprox += gCosConsts[1] * x2;
    trigApprox += gCosConsts[2] * x4;

    c1 = gCosConsts[3] * x2;
    c2 = gCosConsts[4] * x4;

    trigApprox += c1 * x4;
    trigApprox += c2 * x4;

    if (sign)
        trigApprox = -trigApprox;

    return trigApprox;
}



static inline tn_f32
tn_sqrtf32(tn_f32 x)
{
#if defined(TN_MATH_ZV3400)
    // RISC-V path
    float result;
    __asm volatile ("fsqrt.s %0, %1" : "=r" (result) : "r" (x));
    return result;
#elif defined(__ARM_ARCH) && (__ARM_ARCH >= 7)
    // Using VFP inline assembly for ARMv7+
    float result;
    __asm volatile ("vsqrt.f32 %0, %1" : "=t" (result) : "t" (x));
    return result;
#else
    // Portable fallback
    return sqrtf(x);
#endif
}



static inline tn_f32
tn_copysignf32 (
    tn_f32 x,
    tn_f32 y)
{
#if defined(TN_MATH_ZV3400)
    tn_f32 result;
    __asm volatile ("fsgnj.s %0, %1, %2" : "=r" (result) : "r" (x), "r" (y));
    return result;
#else
    return copysignf(x, y);
#endif
}



static inline tn_f32
tn_ceilf32(tn_f32 x)
{
#if defined(TN_MATH_ZV3400)
    tn_i32 tmp_i32;
    tn_f32 tmp_f32;
    __asm volatile ("fcvt.w.s %0, %1, rup" : "=r" (tmp_i32) : "r" (x));
    __asm volatile ("fcvt.s.w %0, %1, rup" : "=r" (tmp_f32) : "r" (tmp_i32));
    return tn_copysignf32(tmp_f32, x);
#else
    return ceilf(x);
#endif
}



static inline tn_f32
tn_floorf32(tn_f32 x)
{
#if defined(TN_MATH_ZV3400)
    tn_i32 tmp_i32;
    tn_f32 tmp_f32;
    __asm volatile ("fcvt.w.s %0, %1, rdn" : "=r" (tmp_i32) : "r" (x));
    __asm volatile ("fcvt.s.w %0, %1, rdn" : "=r" (tmp_f32) : "r" (tmp_i32));
    return tn_copysignf32(tmp_f32, x);
#else
    return floorf(x);
#endif
}



static inline tn_f32
tn_expf32(tn_f32 y)
{
    return expf(y);
}



static inline tn_f32
tn_rcpsqrtf32(tn_f32 y)
{
    return 1.0f / tn_sqrtf32(y);
}



static inline tn_f32
tn_powf32(
    tn_f32 x,
    tn_f32 y)
{
    return powf(x, y);
}



static inline tn_f32
tn_ldexpf32(
    tn_f32 x,
    tn_i32 y)
{
    return ldexpf(x, y);
}



static inline tn_f32
tn_frexpf32(
    tn_f32 x,
    int   *eptr)
{
    return frexpf(x, eptr);
}



static inline tn_f32
tn_hypotf32(
    tn_f32 x,
    tn_f32 y)
{
#if defined(TN_MATH_ZV3400)
    return tn_sqrtf32((x * x) + (y * y));
#elif defined(__ARM_ARCH) && (__ARM_ARCH >= 7)
    return tn_sqrtf32((x * x) + (y * y));
#else
    return hypotf(x, y);
#endif
}



static inline tn_f32
tn_f32_frac(tn_f32 x)
{
#if defined(TN_MATH_ZV3400)
    tn_i32 i;
    tn_f32 i_f;

    /* i = trunc(x)
       i_f = (float)i
       frac = x - i_f */
    __asm volatile ( "fcvt.w.s %0, %1, rtz" : "=r" (i) : "r" (x) );
    __asm volatile ( "fcvt.s.w %0, %1" : "=r" (i_f) : "r" (i) );
    return x - i_f;
#else
    tn_f32 d;
    return modff(x, &d);
#endif
}



static inline tn_f32
tn_minf32 (
    tn_f32 a,
    tn_f32 b)
{
#if defined(TN_MATH_ZV3400)
    tn_f32 result;
    __asm volatile ("fmin.s %0, %1, %2" : "=r" (result) : "r" (a), "r" (b));
    return result;
#elif defined(__ARM_ARCH) && (__ARM_ARCH >= 7)
    tn_f32 result;
    __asm volatile ("vminnm.f32 %0, %1, %2" : "=t" (result) : "t" (a), "t" (b));
    return result;
#else
    return a < b ? a : b;
#endif
}



static inline tn_f32
tn_maxf32 (
    tn_f32 a,
    tn_f32 b)
{
#if defined(TN_MATH_ZV3400)
    tn_f32 result;
    __asm volatile ("fmax.s %0, %1, %2" : "=r" (result) : "r" (a), "r" (b));
    return result;
#elif defined(__ARM_ARCH) && (__ARM_ARCH >= 7)
    tn_f32 result;
    __asm volatile ("vmaxnm.f32 %0, %1, %2" : "=t" (result) : "t" (a), "t" (b));
    return result;
#else
    return a > b ? a : b;
#endif
}



static inline tn_f32
tn_fabsf32(tn_f32 x)
{
    return x < 0.0f ? -x : x;
}



static inline tn_f32
tn_acosf32(tn_f32 x)
{
    return acosf(x);
}



static inline tn_f32
tn_fmodf32(
    tn_f32 x,
    tn_f32 y)
{
    return fmodf(x, y);
}



typedef union
{
    tn_f32   f;
    tn_u32   i;
} tn_f32_u32;



static inline tn_bool
tnISNAN(tn_f32 a)
{
    tn_f32_u32 p;
    p.f = a;
    // coverity[INCONSISTENT_UNION_ACCESS]
    tn_u32 exponent = (p.i >> 23) & 0xff;
    tn_u32 mantissa = p.i & 0x7fffff;
    if (exponent == 255 && mantissa)
        return 1;
    return 0;
}



static inline tn_f32
tnCLAMP(
    tn_f32 a,
    tn_f32 l,
    tn_f32 h)
{
    if (tnISNAN(a))
        return l;
    tnEXPECT(l <= h);
    return (a < l) ? l : (a > h) ? h : a;
}



#ifdef __cplusplus



template < class T >
T
tnMAX(
    T a,
    T b)
{
    return a > b ? a : b;
}



template < class T >
T
tnMIN(
    T a,
    T b)
{
    return a < b ? a : b;

}



#else



static inline tn_i32
modi(
    tn_i32 a,
    tn_i32 b)
{
    tnEXPECT(b >= 0);
    if (b == 0)
        return 0;
    tn_i32 i = a % b;
    if (i < 0)
        i += b;
    tnEXPECT(i >= 0 && i < b);
    return i;
}



static inline tn_u32
modu(
    tn_u32 a,
    tn_u32 b)
{
    tnEXPECT(b > 0);
    if (b == 0)
        return 0;
    tn_u32 i = a % b;
    tnEXPECT(i > 0 && i < b);
    return i;
}



    #define tnMOD(a, b) _Generic((a),          \
                                 tn_i32: modi, \
                                 tn_u32: modu, \
                                 default: modi \
)(a, b)



static inline tn_u32
maxu(
    tn_u32 a,
    tn_u32 b)
{
    return a > b ? a : b;
}



static inline tn_i32
maxi(
    tn_i32 a,
    tn_i32 b)
{
    return a > b ? a : b;
}



static inline tn_f32
maxf(
    tn_f32 a,
    tn_f32 b)
{
    return tn_maxf32(a, b);
}



static inline tn_u64
maxu64(
    tn_u64 a,
    tn_u64 b)
{
    return a > b ? a : b;
}



    #define tnMAX(a, b) _Generic((a),            \
                                 tn_u8: maxu,    \
                                 tn_u32: maxu,   \
                                 tn_u64: maxu64, \
                                 tn_i32: maxi,   \
                                 tn_f32: maxf,   \
                                 default: maxi   \
)(a, b)



static inline tn_u32
minu(
    tn_u32 a,
    tn_u32 b)
{
    return a < b ? a : b;
}



static inline tn_i32
mini(
    tn_i32 a,
    tn_i32 b)
{
    return a < b ? a : b;
}



static inline tn_f32
minf(
    tn_f32 a,
    tn_f32 b)
{
    return tn_minf32(a, b);
}



    #define tnMIN(a, b) _Generic((a),          \
                                 tn_u8: minu,  \
                                 tn_u32: minu, \
                                 tn_i32: mini, \
                                 tn_u64: minu, \
                                 tn_f32: minf, \
                                 default: mini \
)(a, b)
#endif


#define tnABS(a) ((a) < 0 ? -1 * (a) : (a))


#define TN_FLOAT_MAX  FLT_MAX
#define TN_INT32_MAX  INT_MAX
#define TN_INT32_MIN  INT_MIN
#define TN_INT16_MAX  INT16_MAX
#define TN_INT16_MIN  INT16_MIN
#define TN_UINT16_MAX UINT16_MAX
#define TN_INT17_MAX  65535
#define TN_INT17_MIN  -65536



static inline tn_i32
intaddsaturate(
    tn_i32 a,
    tn_i32 b)
{
    tnEXPECT(b > 0);
    tn_i32 r = a + b;
    return (r >= a) ? r : TN_INT32_MAX;
}



static inline tn_f32
tn_sqrf32(tn_f32 a)
{
    return a * a;
}



#ifdef __cplusplus



template < class T >
tn_i32
tnSIGN(T x)
{
    if (x > 0)
        return 1;
    if (x < 0)
        return -1;
    return 0;
}



#else



static inline tn_i32
tn_signi(tn_i32 x)
{
    if (x > 0)
        return 1;
    if (x < 0)
        return -1;
    return 0;
}



static inline tn_i32
tn_signf(tn_f32 x)
{
    if (x > 0.0f)
        return 1;
    if (x < 0.0f)
        return -1;
    return 0;
}



    #define tnSIGN(a) _Generic((a),               \
                               tn_i32: tn_signi,  \
                               tn_f32: tn_signff, \
                               default: tn_signi  \
)(a)
#endif

//getF32Max()

#endif // __tn_math_libm__
