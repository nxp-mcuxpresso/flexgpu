//
// Copyright 2025-2026 NXP
//
// SPDX-License-Identifier: MIT
//

#ifndef __TN_COLOR_H__
#define __TN_COLOR_H__

/// @file tn_color.h
/// Color formats manipulation functions.

#include "tn_types.h"

#define COLOR_SRGBA 1
#define COLOR_TRANSFORM_MAX 8

// RGB{A,X} channel ordering
#define  TN_sRGBX_8888                               0U
#define  TN_sRGBX_8888_PRE                           1U
#define  TN_sRGBA_8888                               2U
#define  TN_sRGBA_8888_PRE                           3U
#define  TN_sRGBA_5551                               4U
#define  TN_sRGBA_5551_PRE                           5U
#define  TN_lRGBA_5551                               6U
#define  TN_lRGBA_5551_PRE                           7U
#define  TN_sRGBA_4444                               8U
#define  TN_sRGBA_4444_PRE                           9U
#define  TN_lRGBA_4444                               10U
#define  TN_lRGBA_4444_PRE                           11U
#define  TN_lRGBX_8888                               12U
#define  TN_lRGBX_8888_PRE                           13U
#define  TN_lRGBA_8888                               14U
#define  TN_lRGBA_8888_PRE                           15U

// {A,X}RGB channel ordering
#define  TN_sXRGB_8888                               16U
#define  TN_sARGB_8888                               17U
#define  TN_sARGB_8888_PRE                           18U
#define  TN_sARGB_1555                               19U
#define  TN_lARGB_1555                               20U
#define  TN_sARGB_4444                               21U
#define  TN_lARGB_4444                               22U
#define  TN_lXRGB_8888                               23U
#define  TN_lARGB_8888                               24U
#define  TN_lARGB_8888_PRE                           25U

// BGR{A,X} channel ordering
#define  TN_sBGRX_8888                               26U
#define  TN_sBGRA_8888                               27U
#define  TN_sBGRA_8888_PRE                           28U
#define  TN_sBGRA_5551                               29U
#define  TN_lBGRA_5551                               30U
#define  TN_sBGRA_4444                               31U
#define  TN_lBGRA_4444                               32U
#define  TN_lBGRX_8888                               33U
#define  TN_lBGRA_8888                               34U
#define  TN_lBGRA_8888_PRE                           35U

// {A,X}BGR channel ordering
#define  TN_sXBGR_8888                               36U
#define  TN_sABGR_8888                               37U
#define  TN_sABGR_8888_PRE                           38U
#define  TN_sABGR_1555                               39U
#define  TN_lABGR_1555                               40U
#define  TN_sABGR_4444                               41U
#define  TN_lABGR_4444                               42U
#define  TN_lXBGR_8888                               43U
#define  TN_lXBGR_8888_PRE                           44U
#define  TN_lABGR_8888                               45U
#define  TN_lABGR_8888_PRE                           46U

#define  TN_sBGR_565                                 47U
#define  TN_sRGB_565                                 48U
#define  TN_sRGB_565_PRE                             49U
#define  TN_lBGR_565                                 50U
#define  TN_lRGB_565                                 51U
#define  TN_lRGB_565_PRE                             52U
#define  TN_sL_8                                     53U
#define  TN_lL_8                                     54U
#define  TN_A_8                                      55U
#define  TN_lL_1                                     56U
#define  TN_A_1                                      57U
#define  TN_A_4                                      58U

// indexed formats
#define  TN_INDEX_1                                  59U
#define  TN_INDEX_2                                  60U
#define  TN_INDEX_4                                  61U
#define  TN_INDEX_8                                  62U

// Y'CrCb formats
#define  TN_NV12                                     63U
#define  TN_NV21                                     64U
#define  TN_NV16                                     65U
#define  TN_NV61                                     66U
#define  TN_NV24                                     67U
#define  TN_NV42                                     68U
#define  TN_YV12                                     69U
#define  TN_YU12                                     70U
#define  TN_AYU12                                    71U
#define  TN_YV16                                     72U
#define  TN_YU16                                     73U
#define  TN_AYU16                                    74U
#define  TN_YV24                                     75U
#define  TN_YU24                                     76U
#define  TN_AYU24                                    77U
#define  TN_YUYV                                     78U
#define  TN_YUY2                                     79U // same as YUYV, to be cleaned up
#define  TN_ANV12                                    80U // exists in VGLite header, not supported by gpu_model
#define  TN_AYUY2                                    81U
#define  TN_YUV24                                    82U // (Y' Cb Cr) YUV444 packed, no alpha byte (24bit LSB)
#define  TN_AYUV32                                   83U // (A Y' Cb Cr) YUV444 packed, with alpha (32bit LSB)
#define  TN_VUYA32                                   84U // (Cr Cb Y' A) YUV444 packed, with alpha (32bit LSB)
#define  TN_YUVA32                                   85U // (Y' Cb Cr A) YUV444 packed, with alpha (32bit LSB)
#define  TN_AVUY32                                   86U // (Y' Cb Cr A) YUV444 packed, with alpha (32bit LSB)
#define  TN_XYUV32                                   87U // (X Y' Cb Cr) YUV444 packed, no alpha (32bit LSB)
#define  TN_VUYX32                                   88U // (Cr Cb Y' X) YUV444 packed, no alpha (32bit LSB)
#define  TN_YUVX32                                   89U // (Y' Cb Cr X) YUV444 packed, no alpha (32bit LSB)

#define  TN_sRGBA_2222                               90U
#define  TN_lRGBA_2222                               91U
#define  TN_sBGRA_2222                               92U
#define  TN_lBGRA_2222                               93U
#define  TN_sABGR_2222                               94U
#define  TN_lABGR_2222                               95U
#define  TN_sARGB_2222                               96U
#define  TN_lARGB_2222                               97U
#define  TN_sRGB_888                                 98U
#define  TN_lRGB_888                                 99U
#define  TN_sBGR_888                                 100U
#define  TN_lBGR_888                                 101U

// planar formats
#define  TN_sABGR_8565                               102U
#define  TN_lABGR_8565                               103U
#define  TN_sBGRA_5658                               104U
#define  TN_lBGRA_5658                               105U
#define  TN_sARGB_8565                               106U
#define  TN_lARGB_8565                               107U
#define  TN_sRGBA_5658                               108U
#define  TN_lRGBA_5658                               109U
#define  TN_sBGR_A8565                               110U
#define  TN_lBGR_A8565                               111U
#define  TN_sBGR_565A8                               112U
#define  TN_lBGR_565A8                               113U
#define  TN_sRGB_A8565                               114U
#define  TN_lRGB_A8565                               115U
#define  TN_sRGB_565A8                               116U
#define  TN_lRGB_565A8                               117U
#define  TN_sRGB_888A8                               118U
#define  TN_lRGB_888A8                               119U
#define  TN_sBGR_888A8                               120U
#define  TN_lBGR_888A8                               121U
#define  TN_sRGB_A8888                               122U
#define  TN_lRGB_A8888                               123U
#define  TN_sBGR_A8888                               124U
#define  TN_lBGR_A8888                               125U

// compressed formats
#define TN_COMPRESSED_ETC2_RGB                       126U
#define TN_COMPRESSED_ETC2_RGBA                      127U
#define TN_COMPRESSED_ETC2_RGBA1                     128U
#define TN_COMPRESSED_IBC_RGBA_8x8_16BPP             129U
#define TN_COMPRESSED_IBC_RGB_8x8_12BPP              130U
#define TN_COMPRESSED_IBC_RGBA_8x8_13BPP             131U
#define TN_COMPRESSED_IBC_RGB_8x8_10BPP              132U
#define TN_COMPRESSED_IBC_RGBA_8x8_11BPP             133U
#define TN_COMPRESSED_IBC_RGB_8x8_8BPP               134U
#define TN_COMPRESSED_IBC_RGBA_32x1_16BPP            135U
#define TN_COMPRESSED_IBC_RGB_32x1_12BPP             136U
#define TN_COMPRESSED_IBC_RGBA_32x1_13BPP            137U
#define TN_COMPRESSED_IBC_RGB_32x1_10BPP             138U
#define TN_COMPRESSED_IBC_RGBA_32x1_11BPP            139U
#define TN_COMPRESSED_IBC_RGB_32x1_8BPP              140U

// GRAY16 format
#define  TN_lL_16_LE                                 141U

// VG_Lite formats (no gamma)
#define  TN_RGBX_8888                                142U
#define  TN_RGBA_8888                                143U
#define  TN_RGBA_5551                                144U
#define  TN_RGBA_4444                                145U
#define  TN_XRGB_8888                                146U
#define  TN_ARGB_8888                                147U
#define  TN_ARGB_1555                                148U
#define  TN_ARGB_4444                                149U
#define  TN_BGRX_8888                                150U
#define  TN_BGRA_8888                                151U
#define  TN_BGRA_5551                                152U
#define  TN_BGRA_4444                                153U
#define  TN_XBGR_8888                                154U
#define  TN_ABGR_8888                                155U
#define  TN_ABGR_1555                                156U
#define  TN_ABGR_4444                                157U
#define  TN_BGR_565                                  158U
#define  TN_RGB_565                                  159U
#define  TN_L_8                                      160U
#define  TN_L_1                                      161U
#define  TN_RGBA_2222                                162U
#define  TN_BGRA_2222                                163U
#define  TN_ABGR_2222                                164U
#define  TN_ARGB_2222                                165U
#define  TN_RGB_888                                  166U
#define  TN_BGR_888                                  167U
#define  TN_ABGR_8565                                168U
#define  TN_BGRA_5658                                169U
#define  TN_ARGB_8565                                170U
#define  TN_RGBA_5658                                171U
#define  TN_BGR_A8565                                172U
#define  TN_BGR_565A8                                173U
#define  TN_RGB_A8565                                174U
#define  TN_RGB_565A8                                175U
#define  TN_RGB_888A8                                176U
#define  TN_BGR_888A8                                177U
#define  TN_RGB_A8888                                178U
#define  TN_BGR_A8888                                179U
#define  TN_L_16_LE                                  180U

#define  TN_MAX_COLOR_FORMAT                         181U

/// A color space determines:
///   - the color in each component by specifying the color set.
///   - the type of the colorspace (VGLITE (no gamma), linear, non-linear, premultiplied, luminance)
///
/// Color Spaces
#define TN_CS_LINEAR            0x0U
#define TN_CS_NONLINEAR         0x1U
#define TN_CS_PREMULTIPLIED     0x2U
#define TN_CS_LUMINANCE         0x4U
#define TN_CS_YUV               0x8U
#define TN_CS_COMPRESSED        0x10U

#define TN_CS_lRGBA             (TN_CS_LINEAR)
#define TN_CS_sRGBA             (TN_CS_NONLINEAR)
#define TN_CS_lRGBA_PRE         (TN_CS_PREMULTIPLIED | TN_CS_LINEAR)
#define TN_CS_sRGBA_PRE         (TN_CS_PREMULTIPLIED | TN_CS_NONLINEAR)
#define TN_CS_lLA               (TN_CS_LUMINANCE | TN_CS_LINEAR)
#define TN_CS_sLA               (TN_CS_LUMINANCE | TN_CS_NONLINEAR)
#define TN_CS_lLA_PRE           (TN_CS_LUMINANCE | TN_CS_PREMULTIPLIED | TN_CS_LINEAR)
#define TN_CS_sLA_PRE           (TN_CS_LUMINANCE | TN_CS_PREMULTIPLIED | TN_CS_NONLINEAR)
#define TN_CS_sYUV              (TN_CS_YUV | TN_CS_NONLINEAR)

#ifdef __cplusplus
extern "C" {
#endif

typedef tn_u32   tn_color_format;
typedef tn_u32   tn_color_space;

/// HWMODULE: colorconv
typedef struct
{
    // cfgregHI:1 max_val:8  4b'
    union
    {
        tn_i32   r_bits;
        tn_i32   y_bits;
        tn_i32   i_bits;
    };
    // cfgregLO:1 max_val:24 5b'
    union
    {
        tn_i32   r_shift;
        tn_i32   y_shift;
        tn_i32   i_shift;
    };


    // cfgregHI:2 max_val:8  4b'
    union
    {
        tn_i32   g_bits;
        tn_i32   u_bits;
    };
    // cfgregLO:2 max_val:24 5b'
    union
    {
        tn_i32   g_shift;
        tn_i32   u_shift;
    };


    // cfgregHI:3 max_val:8  4b'
    union
    {
        tn_i32   b_bits;
        tn_i32   v_bits;
    };
    // cfgregLO:3 max_val:24 5b'
    union
    {
        tn_i32   b_shift;
        tn_i32   v_shift;
    };


    // cfgregHI:4 max_val:8  4b'
    tn_i32   a_bits;
    // cfgregLO:4 max_val:24 5b'
    tn_i32   a_shift;
} tn_color_swizzle;


typedef struct tn_color
{
    tn_f32             r, g, b, a;
    tn_color_format    format;
    tn_color_space     colorspace;

    tn_color_swizzle   swz;
    tn_i32             bpp;
} tn_color;


#define TN_YUV601 0
#define TN_YUV709 1
#define TN_YUV470 2

#define TN_16BIT_SIGNED_FMT 0
#define TN_8BIT_SIGNED_FMT  1

// Pixel color transform
typedef struct tn_color_transform
{
    tn_f32   a_scale;
    tn_f32   a_bias;
    tn_f32   r_scale;
    tn_f32   r_bias;
    tn_f32   g_scale;
    tn_f32   g_bias;
    tn_f32   b_scale;
    tn_f32   b_bias;
} tn_color_transform;

static inline tn_color_space
tn_color_get_csc(
    tn_bool gamma,
    tn_bool premul,
    tn_bool luminance)
{
    return ((gamma ? TN_CS_LINEAR : TN_CS_NONLINEAR) | \
            (premul ? TN_CS_PREMULTIPLIED : 0) |       \
            (luminance ? TN_CS_LUMINANCE : 0));
}



void tn_color_csc_imm(
    tn_color      *color,
    tn_color_space dst_colorspace);

tn_u32 tn_color_pack(tn_color *color);

tn_u32 tn_color_pack_to(
    tn_color       *color,
    const tn_color *colorI);

tn_u32 tn_color_rgba8888_to(
    tn_u32          rgba_color,
    const tn_color *colorI);

void tn_color_unpack(
    tn_u32    packed_pixel,
    tn_color *color);

void tn_color_clamp_imm(tn_color*);

void tn_color_premultiply_imm(tn_color*);

tn_color tn_color_scale(tn_color*, tn_f32);

void tn_color_copy(
    tn_color_format format,
    tn_color       *color);

/* Reverse the byte order of a packed 32-bit color word.
 * Converts between ABGR8888 and RGBA8888 (and back).
 *
 * Platform-specific fast paths use a single instruction where available:
 *   GCC / armclang : __builtin_bswap32 -> REV (ARM), BSWAP (x86), grev (RISC-V Zbb)
 *   IAR            : __REV             -> REV (ARM Cortex-M)
 *   Keil (armcc)   : __rev             -> REV (ARM Cortex-M) */
#if defined(__GNUC__)
    #define TN_COLOR_U32_BSWAP(v)   __builtin_bswap32(v)
#elif defined(__IAR_SYSTEMS_ICC__)
    #include <intrinsics.h>
    #define TN_COLOR_U32_BSWAP(v)   __REV((tn_u32)(v))
#elif defined(__CC_ARM)
    #define TN_COLOR_U32_BSWAP(v)   __rev((tn_u32)(v))
#else
/* Default portable fallback - shift-and-mask byte reversal. */
    #define TN_COLOR_U32_BSWAP(v)                    \
        ( (((tn_u32)(v) & 0xFF000000u) >> 24) |      \
          (((tn_u32)(v) & 0x00FF0000u) >> 8) |       \
          (((tn_u32)(v) & 0x0000FF00u) << 8) |       \
          (((tn_u32)(v) & 0x000000FFu) << 24) )
#endif


static inline tn_u32
tn_color_u32_bswap(tn_u32 v)
{
    return TN_COLOR_U32_BSWAP(v);
}



#ifdef __cplusplus
}
#endif

#endif // __TN_COLOR_H__
