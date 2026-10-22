//
// Copyright 2025-2026 NXP
//
// SPDX-License-Identifier: MIT
//

#ifndef   __TN_FXP_H__
#define   __TN_FXP_H__

#include "tn_types.h"
#include "tn_trace.h"
#include "tn_log.h"

//Signed fixed-point types are defined as follows:
//Qm_n
//where:
//- m is the number of data bits to the left of the radix point.
//- n is the number of data bits to the right of the radix point.
typedef //@abstract@
    tn_u8   tn_S1Q6;
typedef //@abstract@
    tn_u16   tn_S11Q4;
typedef //@abstract@
    tn_u16   tn_S4Q11;
typedef //@abstract@
    tn_u16   tn_S5Q10;
typedef //@abstract@
    tn_u16   tn_S7Q8;
typedef //@abstract@
    tn_u16   tn_S0Q8;
typedef //@abstract@
    tn_u32   tn_S15Q16;

//Unsigned fixed-point types are defined as follows:
//UQm_n
//where:
//- m is the number of data bits to the left of the radix point.
//- n is the number of data bits to the right of the radix point.
typedef //@abstract@
    tn_u16   tn_U1Q15;

// S : sign content, I : integer content, F : fractional content
typedef enum
{
    Q8_OUT_OF_RANGE = 0,
    Q6_1,// SIIIIIIF <-    -64 < float < 64
    Q5_2,// SIIIIIFF <-    -32 < float < 32
    Q4_3,// SIIIIFFF <-    -16 < float < 16
    Q3_4,// SIIIFFFF <-     -8 < float < 8
    Q2_5,// SIIFFFFF <-     -4 < float < 4
    Q1_6,// SIFFFFFF <-     -2 < float < 2
    Q0_7,// SFFFFFFF <-     -1 < float < 1
    Q8_OUT_OF_RESOLUTION
} tn_q8_fractional;

// S : sign content, I : integer content, F : fractional content
typedef enum
{
    Q16_OUT_OF_RANGE = 0,
    Q14_1,// SIIIIIIIIIIIIIIF <- -16384 < float < 16384
    Q13_2,// SIIIIIIIIIIIIIFF <-  -8192 < float < 8192
    Q12_3,// SIIIIIIIIIIIIFFF <-  -4096 < float < 4096
    Q11_4,// SIIIIIIIIIIIFFFF <-  -2048 < float < 2048
    Q10_5,// SIIIIIIIIIIFFFFF <-  -1024 < float < 1024
    Q9_6, // SIIIIIIIIIFFFFFF <-   -512 < float < 512
    Q8_7, // SIIIIIIIIFFFFFFF <-   -256 < float < 256
    Q7_8, // SIIIIIIIFFFFFFFF <-   -128 < float < 128
    Q6_9, // SIIIIIIFFFFFFFFF <-    -64 < float < 64
    Q5_10,// SIIIIIFFFFFFFFFF <-    -32 < float < 32
    Q4_11,// SIIIIFFFFFFFFFFF <-    -16 < float < 16
    Q3_12,// SIIIFFFFFFFFFFFF <-     -8 < float < 8
    Q2_13,// SIIFFFFFFFFFFFFF <-     -4 < float < 4
    Q1_14,// SIFFFFFFFFFFFFFF <-     -2 < float < 2
    Q0_15,// SFFFFFFFFFFFFFFF <-     -1 < float < 1
    Q16_OUT_OF_RESOLUTION
} tn_q16_fractional;

// I : integer content, F : fractional content
typedef enum
{
    UQ16_OUT_OF_RANGE = 0,
    UQ15_1,// IIIIIIIIIIIIIIIF <- 0.0 < float < 32768
    UQ14_2,// IIIIIIIIIIIIIIFF <- 0.0 < float < 16384
    UQ13_3,// IIIIIIIIIIIIIFFF <- 0.0 < float < 8192
    UQ12_4,// IIIIIIIIIIIIFFFF <- 0.0 < float < 4096
    UQ11_5,// IIIIIIIIIIIFFFFF <- 0.0 < float < 2048
    UQ10_6,// IIIIIIIIIIFFFFFF <- 0.0 < float < 1024
    UQ9_7, // IIIIIIIIIFFFFFFF <- 0.0 < float < 512
    UQ8_8, // IIIIIIIIFFFFFFFF <- 0.0 < float < 256
    UQ7_9, // IIIIIIIFFFFFFFFF <- 0.0 < float < 128
    UQ6_10,// IIIIIIFFFFFFFFFF <- 0.0 < float < 64
    UQ5_11,// IIIIIFFFFFFFFFFF <- 0.0 < float < 32
    UQ4_12,// IIIIFFFFFFFFFFFF <- 0.0 < float < 16
    UQ3_13,// IIIFFFFFFFFFFFFF <- 0.0 < float < 8
    UQ2_14,// IIFFFFFFFFFFFFFF <- 0.0 < float < 4
    UQ1_15,// IFFFFFFFFFFFFFFF <- 0.0 < float < 2
    UQ0_16,// FFFFFFFFFFFFFFFF <- 0.0 < float < 1
    UQ16_OUT_OF_RESOLUTION
} tn_uq16_fractional;

#define Q6_1_MIN   ((tn_f32)INT8_MIN / (1UL << 1))
#define Q6_1_MAX   ((tn_f32)INT8_MAX / (1UL << 1))
#define Q6_1_PREC  (1.f / (1UL << 1))
#define Q5_2_MIN  ((tn_f32)INT8_MIN / (1UL << 2))
#define Q5_2_MAX  ((tn_f32)INT8_MAX / (1UL << 2))
#define Q5_2_PREC (1.f / (1UL << 2))
#define Q4_3_MIN  ((tn_f32)INT8_MIN / (1UL << 3))
#define Q4_3_MAX  ((tn_f32)INT8_MAX / (1UL << 3))
#define Q4_3_PREC (1.f / (1UL << 3))
#define Q3_4_MIN  ((tn_f32)INT8_MIN / (1UL << 4))
#define Q3_4_MAX  ((tn_f32)INT8_MAX / (1UL << 4))
#define Q3_4_PREC (1.f / (1UL << 4))
#define Q2_5_MIN  ((tn_f32)INT8_MIN / (1UL << 5))
#define Q2_5_MAX  ((tn_f32)INT8_MAX / (1UL << 5))
#define Q2_5_PREC (1.f / (1UL << 5))
#define Q1_6_MIN  ((tn_f32)INT8_MIN / (1UL << 6))
#define Q1_6_MAX  ((tn_f32)INT8_MAX / (1UL << 6))
#define Q1_6_PREC (1.f / (1UL << 6))
#define Q0_7_MIN  ((tn_f32)INT8_MIN / (1UL << 7))
#define Q0_7_MAX  ((tn_f32)INT8_MAX / (1UL << 7))
#define Q0_7_PREC (1.f / (1UL << 7))

#define Q14_1_MIN  ((tn_f32)INT16_MIN / (1UL << 1))
#define Q14_1_MAX  ((tn_f32)INT16_MAX / (1UL << 1))
#define Q14_1_PREC (1.f / (1UL << 1))
#define Q13_2_MIN  ((tn_f32)INT16_MIN / (1UL << 2))
#define Q13_2_MAX  ((tn_f32)INT16_MAX / (1UL << 2))
#define Q13_2_PREC (1.f / (1UL << 2))
#define Q12_3_MIN  ((tn_f32)INT16_MIN / (1UL << 3))
#define Q12_3_MAX  ((tn_f32)INT16_MAX / (1UL << 3))
#define Q12_3_PREC (1.f / (1UL << 3))
#define Q11_4_MIN  ((tn_f32)INT16_MIN / (1UL << 4))
#define Q11_4_MAX  ((tn_f32)INT16_MAX / (1UL << 4))
#define Q11_4_PREC (1.f / (1UL << 4))
#define Q10_5_MIN  ((tn_f32)INT16_MIN / (1UL << 5))
#define Q10_5_MAX  ((tn_f32)INT16_MAX / (1UL << 5))
#define Q10_5_PREC (1.f / (1UL << 5))
#define Q9_6_MIN   ((tn_f32)INT16_MIN / (1UL << 6))
#define Q9_6_MAX   ((tn_f32)INT16_MAX / (1UL << 6))
#define Q9_6_PREC  (1.f / (1UL << 6))
#define O8_7_MIN   ((tn_f32)INT16_MIN / (1UL << 7))
#define Q8_7_MAX   ((tn_f32)INT16_MAX / (1UL << 7))
#define Q8_7_PREC  (1.f / (1UL << 7))
#define Q7_8_MIN   ((tn_f32)INT16_MIN / (1UL << 8))
#define Q7_8_MAX   ((tn_f32)INT16_MAX / (1UL << 8))
#define Q7_8_PREC  (1.f / (1UL << 8))
#define Q6_9_MIN   ((tn_f32)INT16_MIN / (1UL << 9))
#define Q6_9_MAX   ((tn_f32)INT16_MAX / (1UL << 9))
#define Q6_9_PREC  (1.f / (1UL << 9))
#define Q5_10_MIN  ((tn_f32)INT16_MIN / (1UL << 10))
#define Q5_10_MAX  ((tn_f32)INT16_MAX / (1UL << 10))
#define Q5_10_PREC (1.f / (1UL << 10))
#define Q4_11_MIN  ((tn_f32)INT16_MIN / (1UL << 11))
#define Q4_11_MAX  ((tn_f32)INT16_MAX / (1UL << 11))
#define Q4_11_PREC (1.f / (1UL << 11))
#define Q3_12_MIN  ((tn_f32)INT16_MIN / (1UL << 12))
#define Q3_12_MAX  ((tn_f32)INT16_MAX / (1UL << 12))
#define Q3_12_PREC (1.f / (1UL << 12))
#define Q2_13_MIN  ((tn_f32)INT16_MIN / (1UL << 13))
#define Q2_13_MAX  ((tn_f32)INT16_MAX / (1UL << 13))
#define Q2_13_PREC (1.f / (1UL << 13))
#define Q1_14_MIN  ((tn_f32)INT16_MIN / (1UL << 14))
#define Q1_14_MAX  ((tn_f32)INT16_MAX / (1UL << 14))
#define Q1_14_PREC (1.f / (1UL << 14))
#define Q0_15_MIN  ((tn_f32)INT16_MIN / (1UL << 15))
#define Q0_15_MAX  ((tn_f32)INT16_MAX / (1UL << 15))
#define Q0_15_PREC (1.f / (1UL << 15))



static inline tn_S1Q6
S1Q6(tn_f32 f32)
{
    return (tn_S1Q6)(int)(f32 * (1UL << Q1_6));
}



static inline tn_f32
S1Q6_to_f32(tn_S1Q6 fxp8)
{
    return (tn_f32)(tn_i8)fxp8 / (1UL << Q1_6);
}



static inline tn_S7Q8
S7Q8(tn_f32 f32)
{
    return (tn_S7Q8)(int)(f32 * (1UL << Q7_8));
}



static inline tn_f32
S7Q8_to_f32(tn_S7Q8 fxp16)
{
    return (tn_f32)(tn_i16)fxp16 / (1UL << Q7_8);
}



static inline tn_S0Q8
S0Q8(tn_f32 f32)
{
    return (tn_S0Q8)(int)(f32 * (1UL << Q7_8));
}



static inline tn_S0Q8
S0Q8_sat(tn_f32 f32)
{
    // Scale
    int32_t raw = (int32_t)(f32 * 256.0f);

    // Saturate to Q1.8 range: [-256, +255]
    if (raw > 255)
        raw = 255;
    if (raw < -256)
        raw = -256;

    // Return 9-bit two's complement pattern
    return (tn_S0Q8)raw;
}



static inline tn_f32
S0Q8_to_f32(tn_S0Q8 fxp16)
{
    return (tn_f32)(tn_i16)fxp16 / (1UL << Q7_8);
}



static inline tn_S11Q4
S11Q4(tn_f32 f32)
{
    return (tn_S11Q4)(int)(f32 * (1UL << Q11_4));
}



static inline tn_f32
S11Q4_to_f32(tn_S11Q4 fxp16)
{
    return (tn_f32)(tn_i16)fxp16 / (1UL << Q11_4);
}



static inline tn_S4Q11
S4Q11(tn_f32 f32)
{
    return (tn_S4Q11)(int)(f32 * (1UL << Q4_11));
}



static inline tn_f32
S4Q11_to_f32(tn_S4Q11 fxp16)
{
    return (tn_f32)(tn_i16)fxp16 / (1UL << Q4_11);
}



static inline tn_S5Q10
S5Q10(tn_f32 f32)
{
    return (tn_S5Q10)(int)(f32 * (1UL << Q5_10));
}



static inline tn_f32
S5Q10_to_f32(tn_S5Q10 fxp16)
{
    return (tn_f32)(tn_i16)fxp16 / (1UL << Q5_10);
}



static inline tn_S11Q4
tn_S11Q4_add(
    tn_S11Q4 a,
    tn_S11Q4 b)
{
    tn_S11Q4 c; c = a + b; return c;
}



static inline tn_S11Q4
tn_S11Q4_mul_sat(
    tn_S11Q4 a,
    tn_S11Q4 b)
{
    tn_S11Q4 c;
    tn_i32 tmp;

    // tn_S11Q4 stores a signed 11.4 value in a tn_u16 (see S11Q4_to_f32,
    // which reads it through tn_i16). Reinterpret each operand as tn_i16
    // before widening so the sign is preserved and the product of two
    // 16-bit values stays within tn_i32 range instead of overflowing when
    // the raw tn_u16 bit patterns are zero-extended.
    tmp = (tn_i32)(tn_i16)a * (tn_i32)(tn_i16)b;
    // Rounding
    tmp += (1 << (Q11_4 - 1));
    tmp >>= Q11_4;
    if (tmp > INT16_MAX)
    {
        tnLOG_INFO("max saturation");
        tmp = INT16_MAX;
    }
    else if (tmp < INT16_MIN)
    {
        tnLOG_INFO("min saturation");
        tmp = INT16_MIN;
    }

    // coverity[INTEGER_OVERFLOW]
    c = (tn_S11Q4)tmp;
    return c;
}



static inline tn_U1Q15
U1Q15(tn_f32 f32)
{
    return (tn_U1Q15)(int)(f32 * (1UL << UQ1_15));
}



static inline tn_f32
U1Q15_to_f32(tn_U1Q15 fxp16)
{
    return (tn_f32)(tn_u16)fxp16 / (1UL << UQ1_15);
}



#endif // __TN_FXP_H__
