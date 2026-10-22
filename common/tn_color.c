//
// Copyright 2025-2026 NXP
//
// SPDX-License-Identifier: MIT
//

#include "tn_color.h"
#include "tn_math.h"
#include "tn_aver.h"

/// Packings and Color Spaces
///
/// A packing determines:
///   - which of the four components are sampled, either RGBA or CrYCb (sometimes called VYUA)
///   - the sampling pattern (for example, 4:4:4:4 or 4:2:2:4), which specifies
///     where and how often each component of the image is sampled.
///


/// These are the OpenVG required formats.
/// This can be trimmed when not all of these are used.
static const tn_color color_formats[] =
{
//   RGBA
    [TN_sRGBX_8888]    =                  { .swz = { .r_bits = 8, .g_bits = 8, .b_bits = 8, .a_bits = 0, .r_shift = 24, .g_shift = 16, .b_shift = 8,  .a_shift = 0 }, .bpp = 32, .colorspace = TN_CS_sRGBA     },
    [TN_sRGBX_8888_PRE] =                 { .swz = { .r_bits = 8, .g_bits = 8, .b_bits = 8, .a_bits = 0, .r_shift = 24, .g_shift = 16, .b_shift = 8,  .a_shift = 0 }, .bpp = 32, .colorspace = TN_CS_sRGBA_PRE },
    [TN_sRGBA_8888]    =                  { .swz = { .r_bits = 8, .g_bits = 8, .b_bits = 8, .a_bits = 8, .r_shift = 24, .g_shift = 16, .b_shift = 8,  .a_shift = 0 }, .bpp = 32, .colorspace = TN_CS_sRGBA     },
    [TN_sRGBA_8888_PRE] =                 { .swz = { .r_bits = 8, .g_bits = 8, .b_bits = 8, .a_bits = 8, .r_shift = 24, .g_shift = 16, .b_shift = 8,  .a_shift = 0 }, .bpp = 32, .colorspace = TN_CS_sRGBA_PRE },
    [TN_sRGBA_5551]    =                  { .swz = { .r_bits = 5, .g_bits = 5, .b_bits = 5, .a_bits = 1, .r_shift = 11, .g_shift = 6,  .b_shift = 1,  .a_shift = 0 }, .bpp = 16, .colorspace = TN_CS_sRGBA     },
    [TN_sRGBA_5551_PRE] =                 { .swz = { .r_bits = 5, .g_bits = 5, .b_bits = 5, .a_bits = 1, .r_shift = 11, .g_shift = 6,  .b_shift = 1,  .a_shift = 0 }, .bpp = 16, .colorspace = TN_CS_sRGBA_PRE },
    [TN_sRGBA_4444]    =                  { .swz = { .r_bits = 4, .g_bits = 4, .b_bits = 4, .a_bits = 4, .r_shift = 12, .g_shift = 8,  .b_shift = 4,  .a_shift = 0 }, .bpp = 16, .colorspace = TN_CS_sRGBA     },
    [TN_sRGBA_4444_PRE] =                 { .swz = { .r_bits = 4, .g_bits = 4, .b_bits = 4, .a_bits = 4, .r_shift = 12, .g_shift = 8,  .b_shift = 4,  .a_shift = 0 }, .bpp = 16, .colorspace = TN_CS_sRGBA_PRE },
    [TN_lRGBX_8888]    =                  { .swz = { .r_bits = 8, .g_bits = 8, .b_bits = 8, .a_bits = 0, .r_shift = 24, .g_shift = 16, .b_shift = 8,  .a_shift = 0 }, .bpp = 32, .colorspace = TN_CS_lRGBA     },
    [TN_lRGBX_8888_PRE] =                 { .swz = { .r_bits = 8, .g_bits = 8, .b_bits = 8, .a_bits = 0, .r_shift = 24, .g_shift = 16, .b_shift = 8,  .a_shift = 0 }, .bpp = 32, .colorspace = TN_CS_lRGBA_PRE },
    [TN_lRGBA_8888]    =                  { .swz = { .r_bits = 8, .g_bits = 8, .b_bits = 8, .a_bits = 8, .r_shift = 24, .g_shift = 16, .b_shift = 8,  .a_shift = 0 }, .bpp = 32, .colorspace = TN_CS_lRGBA     },
    [TN_lRGBA_8888_PRE] =                 { .swz = { .r_bits = 8, .g_bits = 8, .b_bits = 8, .a_bits = 8, .r_shift = 24, .g_shift = 16, .b_shift = 8,  .a_shift = 0 }, .bpp = 32, .colorspace = TN_CS_lRGBA_PRE },
    [TN_lRGBA_5551]    =                  { .swz = { .r_bits = 5, .g_bits = 5, .b_bits = 5, .a_bits = 1, .r_shift = 11, .g_shift = 6,  .b_shift = 1,  .a_shift = 0 }, .bpp = 16, .colorspace = TN_CS_lRGBA     },
    [TN_lRGBA_5551_PRE] =                 { .swz = { .r_bits = 5, .g_bits = 5, .b_bits = 5, .a_bits = 1, .r_shift = 11, .g_shift = 6,  .b_shift = 1,  .a_shift = 0 }, .bpp = 16, .colorspace = TN_CS_lRGBA_PRE },
    [TN_lRGBA_4444]    =                  { .swz = { .r_bits = 4, .g_bits = 4, .b_bits = 4, .a_bits = 4, .r_shift = 12, .g_shift = 8,  .b_shift = 4,  .a_shift = 0 }, .bpp = 16, .colorspace = TN_CS_lRGBA     },
    [TN_lRGBA_4444_PRE] =                 { .swz = { .r_bits = 4, .g_bits = 4, .b_bits = 4, .a_bits = 4, .r_shift = 12, .g_shift = 8,  .b_shift = 4,  .a_shift = 0 }, .bpp = 16, .colorspace = TN_CS_lRGBA_PRE },
//   ARGB
    [TN_sXRGB_8888]    =                  { .swz = { .r_bits = 8, .g_bits = 8, .b_bits = 8, .a_bits = 0, .r_shift = 16, .g_shift = 8,  .b_shift = 0,  .a_shift = 24}, .bpp = 32, .colorspace = TN_CS_sRGBA     },
    [TN_sARGB_8888]    =                  { .swz = { .r_bits = 8, .g_bits = 8, .b_bits = 8, .a_bits = 8, .r_shift = 16, .g_shift = 8,  .b_shift = 0,  .a_shift = 24}, .bpp = 32, .colorspace = TN_CS_sRGBA     },
    [TN_sARGB_8888_PRE] =                 { .swz = { .r_bits = 8, .g_bits = 8, .b_bits = 8, .a_bits = 8, .r_shift = 16, .g_shift = 8,  .b_shift = 0,  .a_shift = 24}, .bpp = 32, .colorspace = TN_CS_sRGBA_PRE },
    [TN_sARGB_1555]    =                  { .swz = { .r_bits = 5, .g_bits = 5, .b_bits = 5, .a_bits = 1, .r_shift = 10, .g_shift = 5,  .b_shift = 0,  .a_shift = 15}, .bpp = 16, .colorspace = TN_CS_sRGBA     },
    [TN_sARGB_4444]    =                  { .swz = { .r_bits = 4, .g_bits = 4, .b_bits = 4, .a_bits = 4, .r_shift = 8,  .g_shift = 4,  .b_shift = 0,  .a_shift = 12}, .bpp = 16, .colorspace = TN_CS_sRGBA     },
    [TN_lXRGB_8888]    =                  { .swz = { .r_bits = 8, .g_bits = 8, .b_bits = 8, .a_bits = 0, .r_shift = 16, .g_shift = 8,  .b_shift = 0,  .a_shift = 24}, .bpp = 32, .colorspace = TN_CS_lRGBA     },
    [TN_lARGB_8888]    =                  { .swz = { .r_bits = 8, .g_bits = 8, .b_bits = 8, .a_bits = 8, .r_shift = 16, .g_shift = 8,  .b_shift = 0,  .a_shift = 24}, .bpp = 32, .colorspace = TN_CS_lRGBA     },
    [TN_lARGB_8888_PRE] =                 { .swz = { .r_bits = 8, .g_bits = 8, .b_bits = 8, .a_bits = 8, .r_shift = 16, .g_shift = 8,  .b_shift = 0,  .a_shift = 24}, .bpp = 32, .colorspace = TN_CS_lRGBA_PRE },
    [TN_lARGB_1555]    =                  { .swz = { .r_bits = 5, .g_bits = 5, .b_bits = 5, .a_bits = 1, .r_shift = 10, .g_shift = 5,  .b_shift = 0,  .a_shift = 15}, .bpp = 16, .colorspace = TN_CS_lRGBA     },
    [TN_lARGB_4444]    =                  { .swz = { .r_bits = 4, .g_bits = 4, .b_bits = 4, .a_bits = 4, .r_shift = 8,  .g_shift = 4,  .b_shift = 0,  .a_shift = 12}, .bpp = 16, .colorspace = TN_CS_lRGBA     },
//   BGRA
    [TN_sBGRX_8888]    =                  { .swz = { .r_bits = 8, .g_bits = 8, .b_bits = 8, .a_bits = 0, .r_shift = 8,  .g_shift = 16, .b_shift = 24, .a_shift = 0 }, .bpp = 32, .colorspace = TN_CS_sRGBA     },
    [TN_sBGRA_8888]    =                  { .swz = { .r_bits = 8, .g_bits = 8, .b_bits = 8, .a_bits = 8, .r_shift = 8,  .g_shift = 16, .b_shift = 24, .a_shift = 0 }, .bpp = 32, .colorspace = TN_CS_sRGBA     },
    [TN_sBGRA_8888_PRE] =                 { .swz = { .r_bits = 8, .g_bits = 8, .b_bits = 8, .a_bits = 8, .r_shift = 8,  .g_shift = 16, .b_shift = 24, .a_shift = 0 }, .bpp = 32, .colorspace = TN_CS_sRGBA_PRE },
    [TN_sBGRA_5551]    =                  { .swz = { .r_bits = 5, .g_bits = 5, .b_bits = 5, .a_bits = 1, .r_shift = 1,  .g_shift = 6,  .b_shift = 11, .a_shift = 0 }, .bpp = 16, .colorspace = TN_CS_sRGBA     },
    [TN_sBGRA_4444]    =                  { .swz = { .r_bits = 4, .g_bits = 4, .b_bits = 4, .a_bits = 4, .r_shift = 4,  .g_shift = 8,  .b_shift = 12, .a_shift = 0 }, .bpp = 16, .colorspace = TN_CS_sRGBA     },
    [TN_lBGRX_8888]    =                  { .swz = { .r_bits = 8, .g_bits = 8, .b_bits = 8, .a_bits = 0, .r_shift = 8,  .g_shift = 16, .b_shift = 24, .a_shift = 0 }, .bpp = 32, .colorspace = TN_CS_lRGBA     },
    [TN_lBGRA_8888]    =                  { .swz = { .r_bits = 8, .g_bits = 8, .b_bits = 8, .a_bits = 8, .r_shift = 8,  .g_shift = 16, .b_shift = 24, .a_shift = 0 }, .bpp = 32, .colorspace = TN_CS_lRGBA     },
    [TN_lBGRA_8888_PRE] =                 { .swz = { .r_bits = 8, .g_bits = 8, .b_bits = 8, .a_bits = 8, .r_shift = 8,  .g_shift = 16, .b_shift = 24, .a_shift = 0 }, .bpp = 32, .colorspace = TN_CS_lRGBA_PRE },
    [TN_lBGRA_5551]    =                  { .swz = { .r_bits = 5, .g_bits = 5, .b_bits = 5, .a_bits = 1, .r_shift = 1,  .g_shift = 6,  .b_shift = 11, .a_shift = 0 }, .bpp = 16, .colorspace = TN_CS_lRGBA     },
    [TN_lBGRA_4444]    =                  { .swz = { .r_bits = 4, .g_bits = 4, .b_bits = 4, .a_bits = 4, .r_shift = 4,  .g_shift = 8,  .b_shift = 12, .a_shift = 0 }, .bpp = 16, .colorspace = TN_CS_lRGBA     },
//   ABGR
    [TN_sXBGR_8888]    =                  { .swz = { .r_bits = 8, .g_bits = 8, .b_bits = 8, .a_bits = 0, .r_shift = 0,  .g_shift = 8,  .b_shift = 16, .a_shift = 24}, .bpp = 32, .colorspace = TN_CS_sRGBA     },
    [TN_sABGR_8888]    =                  { .swz = { .r_bits = 8, .g_bits = 8, .b_bits = 8, .a_bits = 8, .r_shift = 0,  .g_shift = 8,  .b_shift = 16, .a_shift = 24}, .bpp = 32, .colorspace = TN_CS_sRGBA     },
    [TN_sABGR_8888_PRE] =                 { .swz = { .r_bits = 8, .g_bits = 8, .b_bits = 8, .a_bits = 8, .r_shift = 0,  .g_shift = 8,  .b_shift = 16, .a_shift = 24}, .bpp = 32, .colorspace = TN_CS_sRGBA_PRE },
    [TN_sABGR_1555]    =                  { .swz = { .r_bits = 5, .g_bits = 5, .b_bits = 5, .a_bits = 1, .r_shift = 0,  .g_shift = 5,  .b_shift = 10, .a_shift = 15}, .bpp = 16, .colorspace = TN_CS_sRGBA     },
    [TN_sABGR_4444]    =                  { .swz = { .r_bits = 4, .g_bits = 4, .b_bits = 4, .a_bits = 4, .r_shift = 0,  .g_shift = 4,  .b_shift = 8,  .a_shift = 12}, .bpp = 16, .colorspace = TN_CS_sRGBA     },
    [TN_lXBGR_8888]    =                  { .swz = { .r_bits = 8, .g_bits = 8, .b_bits = 8, .a_bits = 0, .r_shift = 0,  .g_shift = 8,  .b_shift = 16, .a_shift = 24}, .bpp = 32, .colorspace = TN_CS_lRGBA     },
    [TN_lXBGR_8888_PRE] =                 { .swz = { .r_bits = 8, .g_bits = 8, .b_bits = 8, .a_bits = 0, .r_shift = 0,  .g_shift = 8,  .b_shift = 16, .a_shift = 24}, .bpp = 32, .colorspace = TN_CS_lRGBA_PRE },
    [TN_lABGR_8888]    =                  { .swz = { .r_bits = 8, .g_bits = 8, .b_bits = 8, .a_bits = 8, .r_shift = 0,  .g_shift = 8,  .b_shift = 16, .a_shift = 24}, .bpp = 32, .colorspace = TN_CS_lRGBA     },
    [TN_lABGR_8888_PRE] =                 { .swz = { .r_bits = 8, .g_bits = 8, .b_bits = 8, .a_bits = 8, .r_shift = 0,  .g_shift = 8,  .b_shift = 16, .a_shift = 24}, .bpp = 32, .colorspace = TN_CS_lRGBA_PRE },
    [TN_lABGR_1555]    =                  { .swz = { .r_bits = 5, .g_bits = 5, .b_bits = 5, .a_bits = 1, .r_shift = 0,  .g_shift = 5,  .b_shift = 10, .a_shift = 15}, .bpp = 16, .colorspace = TN_CS_lRGBA     },
    [TN_lABGR_4444]    =                  { .swz = { .r_bits = 4, .g_bits = 4, .b_bits = 4, .a_bits = 4, .r_shift = 0,  .g_shift = 4,  .b_shift = 8,  .a_shift = 12}, .bpp = 16, .colorspace = TN_CS_lRGBA     },

    [TN_sBGR_565]      =                  { .swz = { .r_bits = 5, .g_bits = 6, .b_bits = 5, .a_bits = 0, .r_shift = 0,  .g_shift = 5,  .b_shift = 11, .a_shift = 0 }, .bpp = 16, .colorspace = TN_CS_sRGBA     },
    [TN_sRGB_565]      =                  { .swz = { .r_bits = 5, .g_bits = 6, .b_bits = 5, .a_bits = 0, .r_shift = 11, .g_shift = 5,  .b_shift = 0,  .a_shift = 0 }, .bpp = 16, .colorspace = TN_CS_sRGBA     },
    [TN_sRGB_565_PRE]  =                  { .swz = { .r_bits = 5, .g_bits = 6, .b_bits = 5, .a_bits = 0, .r_shift = 11, .g_shift = 5,  .b_shift = 0,  .a_shift = 0 }, .bpp = 16, .colorspace = TN_CS_sRGBA_PRE },
    [TN_lBGR_565]      =                  { .swz = { .r_bits = 5, .g_bits = 6, .b_bits = 5, .a_bits = 0, .r_shift = 0,  .g_shift = 5,  .b_shift = 11, .a_shift = 0 }, .bpp = 16, .colorspace = TN_CS_lRGBA     },
    [TN_lRGB_565]      =                  { .swz = { .r_bits = 5, .g_bits = 6, .b_bits = 5, .a_bits = 0, .r_shift = 11, .g_shift = 5,  .b_shift = 0,  .a_shift = 0 }, .bpp = 16, .colorspace = TN_CS_lRGBA     },
    [TN_lRGB_565_PRE]  =                  { .swz = { .r_bits = 5, .g_bits = 6, .b_bits = 5, .a_bits = 0, .r_shift = 11, .g_shift = 5,  .b_shift = 0,  .a_shift = 0 }, .bpp = 16, .colorspace = TN_CS_lRGBA_PRE },
    [TN_sBGR_888]      =                  { .swz = { .r_bits = 8, .g_bits = 8, .b_bits = 8, .a_bits = 0, .r_shift = 0,  .g_shift = 8,  .b_shift = 16, .a_shift = 0 }, .bpp = 24, .colorspace = TN_CS_sRGBA     },
    [TN_lBGR_888]      =                  { .swz = { .r_bits = 8, .g_bits = 8, .b_bits = 8, .a_bits = 0, .r_shift = 0,  .g_shift = 8,  .b_shift = 16, .a_shift = 0 }, .bpp = 24, .colorspace = TN_CS_lRGBA     },
    [TN_sRGB_888]      =                  { .swz = { .r_bits = 8, .g_bits = 8, .b_bits = 8, .a_bits = 0, .r_shift = 16, .g_shift = 8,  .b_shift = 0,  .a_shift = 0 }, .bpp = 24, .colorspace = TN_CS_sRGBA     },
    [TN_lRGB_888]      =                  { .swz = { .r_bits = 8, .g_bits = 8, .b_bits = 8, .a_bits = 0, .r_shift = 16, .g_shift = 8,  .b_shift = 0,  .a_shift = 0 }, .bpp = 24, .colorspace = TN_CS_lRGBA     },
    [TN_lL_16_LE]      =                  { .swz = { .r_bits = 8, .g_bits = 8, .b_bits = 0, .a_bits = 0, .r_shift = 8,  .g_shift = 0,  .b_shift = 0,  .a_shift = 0 }, .bpp = 16, .colorspace = TN_CS_lLA       },
    [TN_sL_8]          =                  { .swz = { .y_bits = 8, .u_bits = 0, .v_bits = 0, .a_bits = 0, .y_shift = 0,  .u_shift = 0,  .v_shift = 0,  .a_shift = 0 }, .bpp = 8,  .colorspace = TN_CS_sLA       },
    [TN_lL_8]          =                  { .swz = { .y_bits = 8, .u_bits = 0, .v_bits = 0, .a_bits = 0, .y_shift = 0,  .u_shift = 0,  .v_shift = 0,  .a_shift = 0 }, .bpp = 8,  .colorspace = TN_CS_lLA       },
    [TN_A_8]           =                  { .swz = { .r_bits = 0, .g_bits = 0, .b_bits = 0, .a_bits = 8, .r_shift = 0,  .g_shift = 0,  .b_shift = 0,  .a_shift = 0 }, .bpp = 8,  .colorspace = TN_CS_lRGBA     },
    [TN_lL_1]          =                  { .swz = { .y_bits = 1, .u_bits = 0, .v_bits = 0, .a_bits = 0, .y_shift = 0,  .u_shift = 0,  .v_shift = 0,  .a_shift = 0 }, .bpp = 1,  .colorspace = TN_CS_lLA       },
    [TN_A_1]           =                  { .swz = { .r_bits = 0, .g_bits = 0, .b_bits = 0, .a_bits = 1, .r_shift = 0,  .g_shift = 0,  .b_shift = 0,  .a_shift = 0 }, .bpp = 1,  .colorspace = TN_CS_lRGBA     },
    [TN_A_4]           =                  { .swz = { .r_bits = 0, .g_bits = 0, .b_bits = 0, .a_bits = 4, .r_shift = 0,  .g_shift = 0,  .b_shift = 0,  .a_shift = 0 }, .bpp = 4,  .colorspace = TN_CS_lRGBA     },
    [TN_INDEX_1]       =                  { .swz = { .r_bits = 8, .g_bits = 8, .b_bits = 8, .a_bits = 8, .r_shift = 16, .g_shift = 8,  .b_shift = 0,  .a_shift = 24}, .bpp = 1,  .colorspace = TN_CS_sRGBA     },
    [TN_INDEX_2]       =                  { .swz = { .r_bits = 8, .g_bits = 8, .b_bits = 8, .a_bits = 8, .r_shift = 16, .g_shift = 8,  .b_shift = 0,  .a_shift = 24}, .bpp = 2,  .colorspace = TN_CS_sRGBA     },
    [TN_INDEX_4]       =                  { .swz = { .r_bits = 8, .g_bits = 8, .b_bits = 8, .a_bits = 8, .r_shift = 16, .g_shift = 8,  .b_shift = 0,  .a_shift = 24}, .bpp = 4,  .colorspace = TN_CS_sRGBA     },
    [TN_INDEX_8]       =                  { .swz = { .r_bits = 8, .g_bits = 8, .b_bits = 8, .a_bits = 8, .r_shift = 16, .g_shift = 8,  .b_shift = 0,  .a_shift = 24}, .bpp = 8,  .colorspace = TN_CS_sRGBA     },
    [TN_NV12]          =                  { .swz = { .y_bits = 8, .u_bits = 8, .v_bits = 8, .a_bits = 0, .y_shift = 0,  .u_shift = 0,  .v_shift = 8,  .a_shift = 0 }, .bpp = 12, .colorspace = TN_CS_sYUV      },
    [TN_NV21]          =                  { .swz = { .y_bits = 8, .u_bits = 8, .v_bits = 8, .a_bits = 0, .y_shift = 0,  .u_shift = 8,  .v_shift = 0,  .a_shift = 0 }, .bpp = 12, .colorspace = TN_CS_sYUV      },
    [TN_NV16]          =                  { .swz = { .y_bits = 8, .u_bits = 8, .v_bits = 8, .a_bits = 0, .y_shift = 0,  .u_shift = 0,  .v_shift = 8,  .a_shift = 0 }, .bpp = 16, .colorspace = TN_CS_sYUV      },
    [TN_NV61]          =                  { .swz = { .y_bits = 8, .u_bits = 8, .v_bits = 8, .a_bits = 0, .y_shift = 0,  .u_shift = 8,  .v_shift = 0,  .a_shift = 0 }, .bpp = 16, .colorspace = TN_CS_sYUV      },
    [TN_NV24]          =                  { .swz = { .y_bits = 8, .u_bits = 8, .v_bits = 8, .a_bits = 0, .y_shift = 0,  .u_shift = 0,  .v_shift = 8,  .a_shift = 0 }, .bpp = 24, .colorspace = TN_CS_sYUV      },
    [TN_NV42]          =                  { .swz = { .y_bits = 8, .u_bits = 8, .v_bits = 8, .a_bits = 0, .y_shift = 0,  .u_shift = 8,  .v_shift = 0,  .a_shift = 0 }, .bpp = 24, .colorspace = TN_CS_sYUV      },
    [TN_YV12]          =                  { .swz = { .y_bits = 8, .u_bits = 8, .v_bits = 8, .a_bits = 0, .y_shift = 0,  .u_shift = 8,  .v_shift = 0,  .a_shift = 0 }, .bpp = 12, .colorspace = TN_CS_sYUV      },
    [TN_YU12]          =                  { .swz = { .y_bits = 8, .u_bits = 8, .v_bits = 8, .a_bits = 0, .y_shift = 0,  .u_shift = 0,  .v_shift = 8,  .a_shift = 0 }, .bpp = 12, .colorspace = TN_CS_sYUV      },
    [TN_AYU12]         =                  { .swz = { .y_bits = 8, .u_bits = 8, .v_bits = 8, .a_bits = 8, .y_shift = 0,  .u_shift = 0,  .v_shift = 8,  .a_shift = 0 }, .bpp = 20, .colorspace = TN_CS_sYUV      },
    [TN_YV16]          =                  { .swz = { .y_bits = 8, .u_bits = 8, .v_bits = 8, .a_bits = 0, .y_shift = 0,  .u_shift = 8,  .v_shift = 0,  .a_shift = 0 }, .bpp = 16, .colorspace = TN_CS_sYUV      },
    [TN_YU16]          =                  { .swz = { .y_bits = 8, .u_bits = 8, .v_bits = 8, .a_bits = 0, .y_shift = 0,  .u_shift = 0,  .v_shift = 8,  .a_shift = 0 }, .bpp = 16, .colorspace = TN_CS_sYUV      },
    [TN_AYU16]         =                  { .swz = { .y_bits = 8, .u_bits = 8, .v_bits = 8, .a_bits = 8, .y_shift = 0,  .u_shift = 0,  .v_shift = 8,  .a_shift = 0 }, .bpp = 24, .colorspace = TN_CS_sYUV      },
    [TN_YV24]          =                  { .swz = { .y_bits = 8, .u_bits = 8, .v_bits = 8, .a_bits = 0, .y_shift = 0,  .u_shift = 8,  .v_shift = 0,  .a_shift = 0 }, .bpp = 24, .colorspace = TN_CS_sYUV      },
    [TN_YU24]          =                  { .swz = { .y_bits = 8, .u_bits = 8, .v_bits = 8, .a_bits = 0, .y_shift = 0,  .u_shift = 0,  .v_shift = 8,  .a_shift = 0 }, .bpp = 24, .colorspace = TN_CS_sYUV      },
    [TN_AYU24]         =                  { .swz = { .y_bits = 8, .u_bits = 8, .v_bits = 8, .a_bits = 8, .y_shift = 0,  .u_shift = 0,  .v_shift = 8,  .a_shift = 0 }, .bpp = 32, .colorspace = TN_CS_sYUV      },
    [TN_YUYV]          =                  { .swz = { .y_bits = 8, .u_bits = 8, .v_bits = 8, .a_bits = 8, .y_shift = 0,  .u_shift = 8,  .v_shift = 24, .a_shift = 16}, .bpp = 16, .colorspace = TN_CS_sYUV      },
    [TN_YUV24]         =                  { .swz = { .y_bits = 8, .u_bits = 8, .v_bits = 8, .a_bits = 0, .y_shift = 0,  .u_shift = 8,  .v_shift = 16, .a_shift = 0 }, .bpp = 24, .colorspace = TN_CS_sYUV      },
    [TN_AYUV32]        =                  { .swz = { .y_bits = 8, .u_bits = 8, .v_bits = 8, .a_bits = 8, .y_shift = 8,  .u_shift = 16, .v_shift = 24, .a_shift = 0 }, .bpp = 32, .colorspace = TN_CS_sYUV      },
    [TN_VUYA32]        =                  { .swz = { .y_bits = 8, .u_bits = 8, .v_bits = 8, .a_bits = 8, .y_shift = 16, .u_shift = 8,  .v_shift = 0,  .a_shift = 24}, .bpp = 32, .colorspace = TN_CS_sYUV      },
    [TN_YUVA32]        =                  { .swz = { .y_bits = 8, .u_bits = 8, .v_bits = 8, .a_bits = 8, .y_shift = 0,  .u_shift = 8,  .v_shift = 16, .a_shift = 24}, .bpp = 32, .colorspace = TN_CS_sYUV      },
    [TN_AVUY32]        =                  { .swz = { .y_bits = 8, .u_bits = 8, .v_bits = 8, .a_bits = 8, .y_shift = 24, .u_shift = 16, .v_shift = 8,  .a_shift = 0 }, .bpp = 32, .colorspace = TN_CS_sYUV      },
    [TN_XYUV32]        =                  { .swz = { .y_bits = 8, .u_bits = 8, .v_bits = 8, .a_bits = 0, .y_shift = 8,  .u_shift = 16, .v_shift = 24, .a_shift = 0 }, .bpp = 32, .colorspace = TN_CS_sYUV      },
    [TN_VUYX32]        =                  { .swz = { .y_bits = 8, .u_bits = 8, .v_bits = 8, .a_bits = 0, .y_shift = 16, .u_shift = 8,  .v_shift = 0,  .a_shift = 24}, .bpp = 32, .colorspace = TN_CS_sYUV      },
    [TN_YUVX32]        =                  { .swz = { .y_bits = 8, .u_bits = 8, .v_bits = 8, .a_bits = 0, .y_shift = 0,  .u_shift = 8,  .v_shift = 16, .a_shift = 24}, .bpp = 32, .colorspace = TN_CS_sYUV      },
    [TN_sRGBA_2222]    =                  { .swz = { .r_bits = 2, .g_bits = 2, .b_bits = 2, .a_bits = 2, .r_shift = 6,  .g_shift = 4,  .b_shift = 2,  .a_shift = 0 }, .bpp = 8,  .colorspace = TN_CS_sRGBA     },
    [TN_sBGRA_2222]    =                  { .swz = { .r_bits = 2, .g_bits = 2, .b_bits = 2, .a_bits = 2, .r_shift = 2,  .g_shift = 4,  .b_shift = 6,  .a_shift = 0 }, .bpp = 8,  .colorspace = TN_CS_sRGBA     },
    [TN_sABGR_2222]    =                  { .swz = { .r_bits = 2, .g_bits = 2, .b_bits = 2, .a_bits = 2, .r_shift = 0,  .g_shift = 2,  .b_shift = 4,  .a_shift = 6 }, .bpp = 8,  .colorspace = TN_CS_sRGBA     },
    [TN_sARGB_2222]    =                  { .swz = { .r_bits = 2, .g_bits = 2, .b_bits = 2, .a_bits = 2, .r_shift = 4,  .g_shift = 2,  .b_shift = 0,  .a_shift = 6 }, .bpp = 8,  .colorspace = TN_CS_sRGBA     },
    [TN_lRGBA_2222]    =                  { .swz = { .r_bits = 2, .g_bits = 2, .b_bits = 2, .a_bits = 2, .r_shift = 6,  .g_shift = 4,  .b_shift = 2,  .a_shift = 0 }, .bpp = 8,  .colorspace = TN_CS_lRGBA     },
    [TN_lBGRA_2222]    =                  { .swz = { .r_bits = 2, .g_bits = 2, .b_bits = 2, .a_bits = 2, .r_shift = 2,  .g_shift = 4,  .b_shift = 6,  .a_shift = 0 }, .bpp = 8,  .colorspace = TN_CS_lRGBA     },
    [TN_lABGR_2222]    =                  { .swz = { .r_bits = 2, .g_bits = 2, .b_bits = 2, .a_bits = 2, .r_shift = 0,  .g_shift = 2,  .b_shift = 4,  .a_shift = 6 }, .bpp = 8,  .colorspace = TN_CS_lRGBA     },
    [TN_lARGB_2222]    =                  { .swz = { .r_bits = 2, .g_bits = 2, .b_bits = 2, .a_bits = 2, .r_shift = 4,  .g_shift = 2,  .b_shift = 0,  .a_shift = 6 }, .bpp = 8,  .colorspace = TN_CS_lRGBA     },
    [TN_sABGR_8565]    =                  { .swz = { .r_bits = 5, .g_bits = 6, .b_bits = 5, .a_bits = 8, .r_shift = 0,  .g_shift = 5,  .b_shift = 11, .a_shift = 16}, .bpp = 24, .colorspace = TN_CS_sRGBA     },
    [TN_sBGRA_5658]    =                  { .swz = { .r_bits = 5, .g_bits = 6, .b_bits = 5, .a_bits = 8, .r_shift = 8,  .g_shift = 13, .b_shift = 19, .a_shift = 0 }, .bpp = 24, .colorspace = TN_CS_sRGBA     },
    [TN_sARGB_8565]    =                  { .swz = { .r_bits = 5, .g_bits = 6, .b_bits = 5, .a_bits = 8, .r_shift = 11, .g_shift = 5,  .b_shift = 0,  .a_shift = 16}, .bpp = 24, .colorspace = TN_CS_sRGBA     },
    [TN_sRGBA_5658]    =                  { .swz = { .r_bits = 5, .g_bits = 6, .b_bits = 5, .a_bits = 8, .r_shift = 19, .g_shift = 13, .b_shift = 8,  .a_shift = 0 }, .bpp = 24, .colorspace = TN_CS_sRGBA     },
    [TN_sBGR_A8565]    =                  { .swz = { .r_bits = 5, .g_bits = 6, .b_bits = 5, .a_bits = 8, .r_shift = 0,  .g_shift = 5,  .b_shift = 11, .a_shift = 16}, .bpp = 24, .colorspace = TN_CS_sRGBA     },
    [TN_sBGR_565A8]    =                  { .swz = { .r_bits = 5, .g_bits = 6, .b_bits = 5, .a_bits = 8, .r_shift = 0,  .g_shift = 5,  .b_shift = 11, .a_shift = 16}, .bpp = 24, .colorspace = TN_CS_sRGBA     },
    [TN_sRGB_A8565]    =                  { .swz = { .r_bits = 5, .g_bits = 6, .b_bits = 5, .a_bits = 8, .r_shift = 11, .g_shift = 5,  .b_shift = 0,  .a_shift = 16}, .bpp = 24, .colorspace = TN_CS_sRGBA     },
    [TN_sRGB_565A8]    =                  { .swz = { .r_bits = 5, .g_bits = 6, .b_bits = 5, .a_bits = 8, .r_shift = 11, .g_shift = 5,  .b_shift = 0,  .a_shift = 16}, .bpp = 24, .colorspace = TN_CS_sRGBA     },
    [TN_sRGB_888A8]    =                  { .swz = { .r_bits = 8, .g_bits = 8, .b_bits = 8, .a_bits = 8, .r_shift = 16, .g_shift = 8,  .b_shift = 0,  .a_shift = 24}, .bpp = 32, .colorspace = TN_CS_sRGBA     },
    [TN_sBGR_888A8]    =                  { .swz = { .r_bits = 8, .g_bits = 8, .b_bits = 8, .a_bits = 8, .r_shift = 0,  .g_shift = 8,  .b_shift = 16, .a_shift = 24}, .bpp = 32, .colorspace = TN_CS_sRGBA     },
    [TN_sRGB_A8888]    =                  { .swz = { .r_bits = 8, .g_bits = 8, .b_bits = 8, .a_bits = 8, .r_shift = 16, .g_shift = 8,  .b_shift = 0,  .a_shift = 24}, .bpp = 32, .colorspace = TN_CS_sRGBA     },
    [TN_sBGR_A8888]    =                  { .swz = { .r_bits = 8, .g_bits = 8, .b_bits = 8, .a_bits = 8, .r_shift = 0,  .g_shift = 8,  .b_shift = 16, .a_shift = 24}, .bpp = 32, .colorspace = TN_CS_sRGBA     },
    [TN_lABGR_8565]    =                  { .swz = { .r_bits = 5, .g_bits = 6, .b_bits = 5, .a_bits = 8, .r_shift = 0,  .g_shift = 5,  .b_shift = 11, .a_shift = 16}, .bpp = 24, .colorspace = TN_CS_lRGBA     },
    [TN_lBGRA_5658]    =                  { .swz = { .r_bits = 5, .g_bits = 6, .b_bits = 5, .a_bits = 8, .r_shift = 8,  .g_shift = 13, .b_shift = 19, .a_shift = 0 }, .bpp = 24, .colorspace = TN_CS_lRGBA     },
    [TN_lARGB_8565]    =                  { .swz = { .r_bits = 5, .g_bits = 6, .b_bits = 5, .a_bits = 8, .r_shift = 11, .g_shift = 5,  .b_shift = 0,  .a_shift = 16}, .bpp = 24, .colorspace = TN_CS_lRGBA     },
    [TN_lRGBA_5658]    =                  { .swz = { .r_bits = 5, .g_bits = 6, .b_bits = 5, .a_bits = 8, .r_shift = 19, .g_shift = 13, .b_shift = 8,  .a_shift = 0 }, .bpp = 24, .colorspace = TN_CS_lRGBA     },
    [TN_lBGR_A8565]    =                  { .swz = { .r_bits = 5, .g_bits = 6, .b_bits = 5, .a_bits = 8, .r_shift = 0,  .g_shift = 5,  .b_shift = 11, .a_shift = 16}, .bpp = 24, .colorspace = TN_CS_lRGBA     },
    [TN_lBGR_565A8]    =                  { .swz = { .r_bits = 5, .g_bits = 6, .b_bits = 5, .a_bits = 8, .r_shift = 0,  .g_shift = 5,  .b_shift = 11, .a_shift = 16}, .bpp = 24, .colorspace = TN_CS_lRGBA     },
    [TN_lRGB_A8565]    =                  { .swz = { .r_bits = 5, .g_bits = 6, .b_bits = 5, .a_bits = 8, .r_shift = 11, .g_shift = 5,  .b_shift = 0,  .a_shift = 16}, .bpp = 24, .colorspace = TN_CS_lRGBA     },
    [TN_lRGB_565A8]    =                  { .swz = { .r_bits = 5, .g_bits = 6, .b_bits = 5, .a_bits = 8, .r_shift = 11, .g_shift = 5,  .b_shift = 0,  .a_shift = 16}, .bpp = 24, .colorspace = TN_CS_lRGBA     },
    [TN_lRGB_888A8]    =                  { .swz = { .r_bits = 8, .g_bits = 8, .b_bits = 8, .a_bits = 8, .r_shift = 16, .g_shift = 8,  .b_shift = 0,  .a_shift = 24}, .bpp = 32, .colorspace = TN_CS_lRGBA     },
    [TN_lBGR_888A8]    =                  { .swz = { .r_bits = 8, .g_bits = 8, .b_bits = 8, .a_bits = 8, .r_shift = 0,  .g_shift = 8,  .b_shift = 16, .a_shift = 24}, .bpp = 32, .colorspace = TN_CS_lRGBA     },
    [TN_lRGB_A8888]    =                  { .swz = { .r_bits = 8, .g_bits = 8, .b_bits = 8, .a_bits = 8, .r_shift = 16, .g_shift = 8,  .b_shift = 0,  .a_shift = 24}, .bpp = 32, .colorspace = TN_CS_lRGBA     },
    [TN_lBGR_A8888]    =                  { .swz = { .r_bits = 8, .g_bits = 8, .b_bits = 8, .a_bits = 8, .r_shift = 0,  .g_shift = 8,  .b_shift = 16, .a_shift = 24}, .bpp = 32, .colorspace = TN_CS_lRGBA     },
    [TN_COMPRESSED_ETC2_RGB]            = { .swz = { .r_bits = 0, .g_bits = 0, .b_bits = 0, .a_bits = 0, .r_shift = 0,  .g_shift = 0,  .b_shift = 0,  .a_shift = 0 }, .bpp = 4,  .colorspace = TN_CS_COMPRESSED},
    [TN_COMPRESSED_ETC2_RGBA]           = { .swz = { .r_bits = 0, .g_bits = 0, .b_bits = 0, .a_bits = 0, .r_shift = 0,  .g_shift = 0,  .b_shift = 0,  .a_shift = 0 }, .bpp = 8,  .colorspace = TN_CS_COMPRESSED},
    [TN_COMPRESSED_ETC2_RGBA1]          = { .swz = { .r_bits = 0, .g_bits = 0, .b_bits = 0, .a_bits = 0, .r_shift = 0,  .g_shift = 0,  .b_shift = 0,  .a_shift = 0 }, .bpp = 4,  .colorspace = TN_CS_COMPRESSED},
    [TN_COMPRESSED_IBC_RGBA_8x8_16BPP]  = { .swz = { .r_bits = 0, .g_bits = 0, .b_bits = 0, .a_bits = 0, .r_shift = 0,  .g_shift = 0,  .b_shift = 0,  .a_shift = 0 }, .bpp = 16, .colorspace = TN_CS_COMPRESSED},
    [TN_COMPRESSED_IBC_RGB_8x8_12BPP]   = { .swz = { .r_bits = 0, .g_bits = 0, .b_bits = 0, .a_bits = 0, .r_shift = 0,  .g_shift = 0,  .b_shift = 0,  .a_shift = 0 }, .bpp = 12, .colorspace = TN_CS_COMPRESSED},
    [TN_COMPRESSED_IBC_RGBA_8x8_13BPP]  = { .swz = { .r_bits = 0, .g_bits = 0, .b_bits = 0, .a_bits = 0, .r_shift = 0,  .g_shift = 0,  .b_shift = 0,  .a_shift = 0 }, .bpp = 13, .colorspace = TN_CS_COMPRESSED},
    [TN_COMPRESSED_IBC_RGB_8x8_10BPP]   = { .swz = { .r_bits = 0, .g_bits = 0, .b_bits = 0, .a_bits = 0, .r_shift = 0,  .g_shift = 0,  .b_shift = 0,  .a_shift = 0 }, .bpp = 10, .colorspace = TN_CS_COMPRESSED},
    [TN_COMPRESSED_IBC_RGBA_8x8_11BPP]  = { .swz = { .r_bits = 0, .g_bits = 0, .b_bits = 0, .a_bits = 0, .r_shift = 0,  .g_shift = 0,  .b_shift = 0,  .a_shift = 0 }, .bpp = 11, .colorspace = TN_CS_COMPRESSED},
    [TN_COMPRESSED_IBC_RGB_8x8_8BPP]    = { .swz = { .r_bits = 0, .g_bits = 0, .b_bits = 0, .a_bits = 0, .r_shift = 0,  .g_shift = 0,  .b_shift = 0,  .a_shift = 0 }, .bpp = 8,  .colorspace = TN_CS_COMPRESSED},
    [TN_COMPRESSED_IBC_RGBA_32x1_16BPP] = { .swz = { .r_bits = 0, .g_bits = 0, .b_bits = 0, .a_bits = 0, .r_shift = 0,  .g_shift = 0,  .b_shift = 0,  .a_shift = 0 }, .bpp = 16, .colorspace = TN_CS_COMPRESSED},
    [TN_COMPRESSED_IBC_RGB_32x1_12BPP]  = { .swz = { .r_bits = 0, .g_bits = 0, .b_bits = 0, .a_bits = 0, .r_shift = 0,  .g_shift = 0,  .b_shift = 0,  .a_shift = 0 }, .bpp = 12, .colorspace = TN_CS_COMPRESSED},
    [TN_COMPRESSED_IBC_RGBA_32x1_13BPP] = { .swz = { .r_bits = 0, .g_bits = 0, .b_bits = 0, .a_bits = 0, .r_shift = 0,  .g_shift = 0,  .b_shift = 0,  .a_shift = 0 }, .bpp = 13, .colorspace = TN_CS_COMPRESSED},
    [TN_COMPRESSED_IBC_RGB_32x1_10BPP]  = { .swz = { .r_bits = 0, .g_bits = 0, .b_bits = 0, .a_bits = 0, .r_shift = 0,  .g_shift = 0,  .b_shift = 0,  .a_shift = 0 }, .bpp = 10, .colorspace = TN_CS_COMPRESSED},
    [TN_COMPRESSED_IBC_RGBA_32x1_11BPP] = { .swz = { .r_bits = 0, .g_bits = 0, .b_bits = 0, .a_bits = 0, .r_shift = 0,  .g_shift = 0,  .b_shift = 0,  .a_shift = 0 }, .bpp = 11, .colorspace = TN_CS_COMPRESSED},
    [TN_COMPRESSED_IBC_RGB_32x1_8BPP]   = { .swz = { .r_bits = 0, .g_bits = 0, .b_bits = 0, .a_bits = 0, .r_shift = 0,  .g_shift = 0,  .b_shift = 0,  .a_shift = 0 }, .bpp = 8,  .colorspace = TN_CS_COMPRESSED},

    [TN_RGBX_8888]     =                  { .swz = { .r_bits = 8, .g_bits = 8, .b_bits = 8, .a_bits = 0, .r_shift = 24, .g_shift = 16, .b_shift = 8,  .a_shift = 0 }, .bpp = 32, .colorspace = TN_CS_sRGBA     },
    [TN_RGBA_8888]     =                  { .swz = { .r_bits = 8, .g_bits = 8, .b_bits = 8, .a_bits = 8, .r_shift = 24, .g_shift = 16, .b_shift = 8,  .a_shift = 0 }, .bpp = 32, .colorspace = TN_CS_sRGBA     },
    [TN_RGBA_5551]     =                  { .swz = { .r_bits = 5, .g_bits = 5, .b_bits = 5, .a_bits = 1, .r_shift = 11, .g_shift = 6,  .b_shift = 1,  .a_shift = 0 }, .bpp = 16, .colorspace = TN_CS_sRGBA     },
    [TN_RGBA_4444]     =                  { .swz = { .r_bits = 4, .g_bits = 4, .b_bits = 4, .a_bits = 4, .r_shift = 12, .g_shift = 8,  .b_shift = 4,  .a_shift = 0 }, .bpp = 16, .colorspace = TN_CS_sRGBA     },
    [TN_XRGB_8888]     =                  { .swz = { .r_bits = 8, .g_bits = 8, .b_bits = 8, .a_bits = 0, .r_shift = 16, .g_shift = 8,  .b_shift = 0,  .a_shift = 24}, .bpp = 32, .colorspace = TN_CS_sRGBA     },
    [TN_ARGB_8888]     =                  { .swz = { .r_bits = 8, .g_bits = 8, .b_bits = 8, .a_bits = 8, .r_shift = 16, .g_shift = 8,  .b_shift = 0,  .a_shift = 24}, .bpp = 32, .colorspace = TN_CS_sRGBA     },
    [TN_ARGB_1555]     =                  { .swz = { .r_bits = 5, .g_bits = 5, .b_bits = 5, .a_bits = 1, .r_shift = 10, .g_shift = 5,  .b_shift = 0,  .a_shift = 15}, .bpp = 16, .colorspace = TN_CS_sRGBA     },
    [TN_ARGB_4444]     =                  { .swz = { .r_bits = 4, .g_bits = 4, .b_bits = 4, .a_bits = 4, .r_shift = 8,  .g_shift = 4,  .b_shift = 0,  .a_shift = 12}, .bpp = 16, .colorspace = TN_CS_sRGBA     },
    [TN_BGRX_8888]     =                  { .swz = { .r_bits = 8, .g_bits = 8, .b_bits = 8, .a_bits = 0, .r_shift = 8,  .g_shift = 16, .b_shift = 24, .a_shift = 0 }, .bpp = 32, .colorspace = TN_CS_sRGBA     },
    [TN_BGRA_8888]     =                  { .swz = { .r_bits = 8, .g_bits = 8, .b_bits = 8, .a_bits = 8, .r_shift = 8,  .g_shift = 16, .b_shift = 24, .a_shift = 0 }, .bpp = 32, .colorspace = TN_CS_sRGBA     },
    [TN_BGRA_5551]     =                  { .swz = { .r_bits = 5, .g_bits = 5, .b_bits = 5, .a_bits = 1, .r_shift = 1,  .g_shift = 6,  .b_shift = 11, .a_shift = 0 }, .bpp = 16, .colorspace = TN_CS_sRGBA     },
    [TN_BGRA_4444]     =                  { .swz = { .r_bits = 4, .g_bits = 4, .b_bits = 4, .a_bits = 4, .r_shift = 4,  .g_shift = 8,  .b_shift = 12, .a_shift = 0 }, .bpp = 16, .colorspace = TN_CS_sRGBA     },
    [TN_XBGR_8888]     =                  { .swz = { .r_bits = 8, .g_bits = 8, .b_bits = 8, .a_bits = 0, .r_shift = 0,  .g_shift = 8,  .b_shift = 16, .a_shift = 24}, .bpp = 32, .colorspace = TN_CS_sRGBA     },
    [TN_ABGR_8888]     =                  { .swz = { .r_bits = 8, .g_bits = 8, .b_bits = 8, .a_bits = 8, .r_shift = 0,  .g_shift = 8,  .b_shift = 16, .a_shift = 24}, .bpp = 32, .colorspace = TN_CS_sRGBA     },
    [TN_ABGR_1555]     =                  { .swz = { .r_bits = 5, .g_bits = 5, .b_bits = 5, .a_bits = 1, .r_shift = 0,  .g_shift = 5,  .b_shift = 10, .a_shift = 15}, .bpp = 16, .colorspace = TN_CS_sRGBA     },
    [TN_ABGR_4444]     =                  { .swz = { .r_bits = 4, .g_bits = 4, .b_bits = 4, .a_bits = 4, .r_shift = 0,  .g_shift = 4,  .b_shift = 8,  .a_shift = 12}, .bpp = 16, .colorspace = TN_CS_sRGBA     },
    [TN_BGR_565]       =                  { .swz = { .r_bits = 5, .g_bits = 6, .b_bits = 5, .a_bits = 0, .r_shift = 0,  .g_shift = 5,  .b_shift = 11, .a_shift = 0 }, .bpp = 16, .colorspace = TN_CS_sRGBA     },
    [TN_RGB_565]       =                  { .swz = { .r_bits = 5, .g_bits = 6, .b_bits = 5, .a_bits = 0, .r_shift = 11, .g_shift = 5,  .b_shift = 0,  .a_shift = 0 }, .bpp = 16, .colorspace = TN_CS_sRGBA     },
    [TN_L_8]           =                  { .swz = { .y_bits = 8, .u_bits = 0, .v_bits = 0, .a_bits = 0, .y_shift = 0,  .u_shift = 0,  .v_shift = 0,  .a_shift = 0 }, .bpp = 8,  .colorspace = TN_CS_sLA       },
    [TN_L_1]           =                  { .swz = { .y_bits = 1, .u_bits = 0, .v_bits = 0, .a_bits = 0, .y_shift = 0,  .u_shift = 0,  .v_shift = 0,  .a_shift = 0 }, .bpp = 1,  .colorspace = TN_CS_sLA       },
    [TN_RGBA_2222]     =                  { .swz = { .r_bits = 2, .g_bits = 2, .b_bits = 2, .a_bits = 2, .r_shift = 6,  .g_shift = 4,  .b_shift = 2,  .a_shift = 0 }, .bpp = 8,  .colorspace = TN_CS_sRGBA     },
    [TN_BGRA_2222]     =                  { .swz = { .r_bits = 2, .g_bits = 2, .b_bits = 2, .a_bits = 2, .r_shift = 2,  .g_shift = 4,  .b_shift = 6,  .a_shift = 0 }, .bpp = 8,  .colorspace = TN_CS_sRGBA     },
    [TN_ABGR_2222]     =                  { .swz = { .r_bits = 2, .g_bits = 2, .b_bits = 2, .a_bits = 2, .r_shift = 0,  .g_shift = 2,  .b_shift = 4,  .a_shift = 6 }, .bpp = 8,  .colorspace = TN_CS_sRGBA     },
    [TN_ARGB_2222]     =                  { .swz = { .r_bits = 2, .g_bits = 2, .b_bits = 2, .a_bits = 2, .r_shift = 4,  .g_shift = 2,  .b_shift = 0,  .a_shift = 6 }, .bpp = 8,  .colorspace = TN_CS_sRGBA     },
    [TN_RGB_888]       =                  { .swz = { .r_bits = 8, .g_bits = 8, .b_bits = 8, .a_bits = 0, .r_shift = 16, .g_shift = 8,  .b_shift = 0,  .a_shift = 0 }, .bpp = 24, .colorspace = TN_CS_sRGBA     },
    [TN_BGR_888]       =                  { .swz = { .r_bits = 8, .g_bits = 8, .b_bits = 8, .a_bits = 0, .r_shift = 0,  .g_shift = 8,  .b_shift = 16, .a_shift = 0 }, .bpp = 24, .colorspace = TN_CS_sRGBA     },
    [TN_ABGR_8565]     =                  { .swz = { .r_bits = 5, .g_bits = 6, .b_bits = 5, .a_bits = 8, .r_shift = 0,  .g_shift = 5,  .b_shift = 11, .a_shift = 16}, .bpp = 24, .colorspace = TN_CS_sRGBA     },
    [TN_BGRA_5658]     =                  { .swz = { .r_bits = 5, .g_bits = 6, .b_bits = 5, .a_bits = 8, .r_shift = 8,  .g_shift = 13, .b_shift = 19, .a_shift = 0 }, .bpp = 24, .colorspace = TN_CS_sRGBA     },
    [TN_ARGB_8565]     =                  { .swz = { .r_bits = 5, .g_bits = 6, .b_bits = 5, .a_bits = 8, .r_shift = 11, .g_shift = 5,  .b_shift = 0,  .a_shift = 16}, .bpp = 24, .colorspace = TN_CS_sRGBA     },
    [TN_RGBA_5658]     =                  { .swz = { .r_bits = 5, .g_bits = 6, .b_bits = 5, .a_bits = 8, .r_shift = 19, .g_shift = 13, .b_shift = 8,  .a_shift = 0 }, .bpp = 24, .colorspace = TN_CS_sRGBA     },
    [TN_BGR_A8565]     =                  { .swz = { .r_bits = 5, .g_bits = 6, .b_bits = 5, .a_bits = 8, .r_shift = 0,  .g_shift = 5,  .b_shift = 11, .a_shift = 16}, .bpp = 24, .colorspace = TN_CS_sRGBA     },
    [TN_BGR_565A8]     =                  { .swz = { .r_bits = 5, .g_bits = 6, .b_bits = 5, .a_bits = 8, .r_shift = 0,  .g_shift = 5,  .b_shift = 11, .a_shift = 16}, .bpp = 24, .colorspace = TN_CS_sRGBA     },
    [TN_RGB_A8565]     =                  { .swz = { .r_bits = 5, .g_bits = 6, .b_bits = 5, .a_bits = 8, .r_shift = 11, .g_shift = 5,  .b_shift = 0,  .a_shift = 16}, .bpp = 24, .colorspace = TN_CS_sRGBA     },
    [TN_RGB_565A8]     =                  { .swz = { .r_bits = 5, .g_bits = 6, .b_bits = 5, .a_bits = 8, .r_shift = 11, .g_shift = 5,  .b_shift = 0,  .a_shift = 16}, .bpp = 24, .colorspace = TN_CS_sRGBA     },
    [TN_RGB_888A8]     =                  { .swz = { .r_bits = 8, .g_bits = 8, .b_bits = 8, .a_bits = 8, .r_shift = 16, .g_shift = 8,  .b_shift = 0,  .a_shift = 24}, .bpp = 32, .colorspace = TN_CS_sRGBA     },
    [TN_BGR_888A8]     =                  { .swz = { .r_bits = 8, .g_bits = 8, .b_bits = 8, .a_bits = 8, .r_shift = 0,  .g_shift = 8,  .b_shift = 16, .a_shift = 24}, .bpp = 32, .colorspace = TN_CS_sRGBA     },
    [TN_RGB_A8888]     =                  { .swz = { .r_bits = 8, .g_bits = 8, .b_bits = 8, .a_bits = 8, .r_shift = 16, .g_shift = 8,  .b_shift = 0,  .a_shift = 24}, .bpp = 32, .colorspace = TN_CS_sRGBA     },
    [TN_BGR_A8888]     =                  { .swz = { .r_bits = 8, .g_bits = 8, .b_bits = 8, .a_bits = 8, .r_shift = 0,  .g_shift = 8,  .b_shift = 16, .a_shift = 24}, .bpp = 32, .colorspace = TN_CS_sRGBA     },
    [TN_L_16_LE]       =                  { .swz = { .r_bits = 8, .g_bits = 8, .b_bits = 0, .a_bits = 0, .r_shift = 8,  .g_shift = 0,  .b_shift = 0,  .a_shift = 0 }, .bpp = 16, .colorspace = TN_CS_sLA       },
};



static tn_i32
_tn_format_is_valid(tn_color_format f)
{
    if (f >= TN_MAX_COLOR_FORMAT)
        return 0;
    return 1;
}



void
tn_color_copy(
    tn_color_format format,
    tn_color       *color)
{

    tnEXPECT(_tn_format_is_valid(format));

    *color = color_formats[format];
    color->format = format;
}



/// From "Introduction to Digital Image Processing":
///
/// The red, green and blue signals from video camera sensors typically are
/// linearly proportional to the light striking each sensor. However, the
/// light generated by cathode tube displays is approximately proportional to
/// the display amplitude drive signals raised to a power in the range (2.0,
/// 3.0) [28].  To obtain good-quality display, it is necessary to compensate
/// for this point non-linearity.  The compensation process, called gamma
/// correction, involves passing the camera sensor signals through a point
/// non-linearity with a power, typically, of about 0.45.
///
/// A linear RGB image that has been gamma corrected is called
/// a "gamma RGB image".

/// The specification for sRGB (IEC 61966–2–1) uses BT.709 chromaticity,
/// D65 reference white, a display gamma of 2.2,
/// and linear RGB (8 bits per color).
/// sRGB values have a normalized range of 0–1,
/// with 8-bit digital sRGB values having a range of 0–255 for black–white.
///
/// Rtransmit = Rcamera^(1/2.4)
/// Gtransmit = Gcamera^(1/2.4)
/// Btransmit = Bcamera^(1/2.4)
static tn_f32
_tn_gamma(tn_f32 component)
{
    if (component <= 0.00304f)
        component *= 12.92f;
    else
        component = 1.0556f * tn_powf32(component, 1.0f / 2.4f) - 0.0556f;
    return component;
}



/// Rdisplay=Rcamera^2.4
/// Gdisplay=Gcamera^2.4
/// Bdisplay=Bcamera^2.4
static tn_f32
_tn_invgamma(tn_f32 component)
{
    if (component <= 0.03928f)
        component /= 12.92f;
    else
        component = tn_powf32((component + 0.0556f) / 1.0556f, 2.4f);
    return component;
}



/// Linear grayscale (luminance).
static tn_f32
_tn_lRGB_luminance(
    tn_f32 r,
    tn_f32 g,
    tn_f32 b)
{
    return 0.2126f * r + 0.7152f * g + 0.0722f * b;
}



void
tn_color_clamp_imm(tn_color *color)
{
    color->a = tnCLAMP(color->a, 0.0f, 1.0f);
    tn_f32 u = (color->colorspace & TN_CS_PREMULTIPLIED) ? color->a : (tn_f32)1.0f;

    color->r = tnCLAMP(color->r, 0.0f, u);
    color->g = tnCLAMP(color->g, 0.0f, u);
    color->b = tnCLAMP(color->b, 0.0f, u);
}



static tn_f32
_tn_normalize_u32(
    tn_u32 i,
    tn_u32 maxi)
{
    return (tn_f32)(i & maxi) / (tn_f32)maxi;
}



static tn_u32
_tn_denormalize_f32(
    tn_f32 c,
    tn_u32 maxc)
{
    tn_u32 t = tnMAX( (tn_u32)tn_floorf32(c * (tn_f32)maxc + 0.5f), 0);
    return tnMIN(t, maxc);
}



/// Convert color between colorspaces and update color::colorspace accordingly.
///
/// @note: color::format is not updated.
void
tn_color_csc_imm(
    tn_color      *color,
    tn_color_space dst_colorspace)
{
    if (color->colorspace == dst_colorspace)
        return;

    if (color->colorspace & TN_CS_PREMULTIPLIED)
    {   //unpremultiply
        tn_f32 ooa = (color->a != 0.0f) ? 1.0f / color->a : (tn_f32)0.0f;
        color->r *= ooa;
        color->g *= ooa;
        color->b *= ooa;
    }

    //From Section 3.4.2 of OpenVG spec
    //1: sRGB = gamma(lRGB)
    //2: lRGB = invgamma(sRGB)
    //3: lL = 0.2126 lR + 0.7152 lG + 0.0722 lB
    //4: lRGB = lL
    //5: sL = gamma(lL)
    //6: lL = invgamma(sL)
    //7: sRGB = sL

    //Source/Dest lRGB  sRGB   lL   sL
    //lRGB          -     1    3    3,5
    //sRGB          2     -    2,3  2,3,5
    //lL            4    4,1   -    5
    //sL           7,2    7    6    -

    tn_u8 csc_src = color->colorspace & (TN_CS_NONLINEAR | TN_CS_LUMINANCE);
    tn_u8 csc_dst = dst_colorspace & (TN_CS_NONLINEAR | TN_CS_LUMINANCE);

    if (csc_src == TN_CS_lRGBA && csc_dst == TN_CS_sRGBA)
    {
        //1
        color->r = _tn_gamma(color->r);
        color->g = _tn_gamma(color->g);
        color->b = _tn_gamma(color->b);
    }
    else if (csc_src == TN_CS_lRGBA && csc_dst == TN_CS_lLA)
    {
        //3
        color->r =
            color->g =
                color->b = _tn_lRGB_luminance(color->r, color->g, color->b);
    }
    else if (csc_src == TN_CS_lRGBA && csc_dst == TN_CS_sLA)
    {
        //3,5
        color->r =
            color->g =
                color->b = _tn_gamma(
                    _tn_lRGB_luminance(color->r, color->g, color->b)
                );
    }
    else if (csc_src == TN_CS_sRGBA && csc_dst == TN_CS_lRGBA)
    {
        //2
        color->r = _tn_invgamma(color->r);
        color->g = _tn_invgamma(color->g);
        color->b = _tn_invgamma(color->b);
    }
    else if (csc_src == TN_CS_sRGBA && csc_dst == TN_CS_lLA)
    {
        //2,3
        color->r =
            color->g =
                color->b = _tn_lRGB_luminance(
                    _tn_invgamma(color->r),
                    _tn_invgamma(color->g),
                    _tn_invgamma(color->b)
                );
    }
    else if (csc_src == TN_CS_sRGBA && csc_dst == TN_CS_sLA)
    {
        //2,3,4
        color->r =
            color->g =
                color->b = _tn_gamma(
                    _tn_lRGB_luminance(
                        _tn_invgamma(color->r),
                        _tn_invgamma(color->g),
                        _tn_invgamma(color->b)
                    )
                );
    }
    else if (csc_src == TN_CS_lLA && csc_dst == TN_CS_lRGBA)
    {
        //4
        //do nothing
    }
    else if (csc_src == TN_CS_lLA && csc_dst == TN_CS_sRGBA)
    {
        //4,1
        color->r =
            color->g =
                color->b = _tn_gamma(color->r);
    }
    else if (csc_src == TN_CS_lLA && csc_dst == TN_CS_sLA)
    {
        //5
        color->r =
            color->g =
                color->b = _tn_gamma(color->r);
    }
    else if (csc_src == TN_CS_sLA && csc_dst == TN_CS_lRGBA)
    {
        //7,2
        color->r =
            color->g =
                color->b = _tn_invgamma(color->r);
    }
    else if (csc_src == TN_CS_sLA && csc_dst == TN_CS_sRGBA)
    {
        //7
        //do nothing
    }
    else if (csc_src == TN_CS_sLA && csc_dst == TN_CS_lLA)
    {
        //6
        color->r =
            color->g =
                color->b = _tn_invgamma(color->r);
    }
    else
    {
        // nothing to convert
        tnEXPECT((color->colorspace & (TN_CS_LUMINANCE | TN_CS_NONLINEAR)) == (dst_colorspace & (TN_CS_LUMINANCE | TN_CS_NONLINEAR)));
    }

    if (dst_colorspace & TN_CS_PREMULTIPLIED)
    {   //premultiply
        color->r *= color->a;
        color->g *= color->a;
        color->b *= color->a;
    }
    color->colorspace = dst_colorspace;
}



/// color is pixel-packed according to its color::format.
/// @return pixel-packed color.
tn_u32
tn_color_pack(tn_color *color)
{
    //assertConsistency();
    tn_color colorI;
    tn_color_copy(color->format, &colorI);

    tn_i32 r_bits = colorI.swz.r_bits;
    tn_i32 g_bits = colorI.swz.g_bits;
    tn_i32 b_bits = colorI.swz.b_bits;
    tn_i32 a_bits = colorI.swz.a_bits;
    // tn_i32 lb = outputDesc.luminanceBits;
    tn_i32 r_shift = colorI.swz.r_shift;
    tn_i32 g_shift = colorI.swz.g_shift;
    tn_i32 b_shift = colorI.swz.b_shift;
    tn_i32 a_shift = colorI.swz.a_shift;
    //tn_i32 ls = outputDesc.luminanceShift;
    // if(lb)
    // {   //luminance
    //     RI_ASSERT(isLuminance());
    //     return colorToInt(r, (1<<lb)-1) << ls;
    // }
    // else
    {   //rgb
        // RI_ASSERT(!isLuminance());
        tn_u32 r = r_bits ? _tn_denormalize_f32(color->r, 255) >> (8 - r_bits) : 0;
        tn_u32 g = g_bits ? _tn_denormalize_f32(color->g, 255) >> (8 - g_bits) : 0;
        tn_u32 b = b_bits ? _tn_denormalize_f32(color->b, 255) >> (8 - b_bits) : 0;
        tn_u32 a = a_bits ? _tn_denormalize_f32(color->a, 255) >> (8 - a_bits) : 0;
        return (r << r_shift) | (g << g_shift) | (b << b_shift) | (a << a_shift);
    }
}



/// color is pixel-packed according to its color::format.
/// @return pixel-packed color.
tn_u32
tn_color_pack_to(
    tn_color       *color,
    const tn_color *colorI)
{
    tn_i32 r_bits = colorI->swz.r_bits;
    tn_i32 g_bits = colorI->swz.g_bits;
    tn_i32 b_bits = colorI->swz.b_bits;
    tn_i32 a_bits = colorI->swz.a_bits;
    // tn_i32 lb = outputDesc.luminanceBits;
    tn_i32 r_shift = colorI->swz.r_shift;
    tn_i32 g_shift = colorI->swz.g_shift;
    tn_i32 b_shift = colorI->swz.b_shift;
    tn_i32 a_shift = colorI->swz.a_shift;
    //tn_i32 ls = outputDesc.luminanceShift;
    // if(lb)
    // {   //luminance
    //     RI_ASSERT(isLuminance());
    //     return colorToInt(r, (1<<lb)-1) << ls;
    // }
    // else
    {   //rgb
        // RI_ASSERT(!isLuminance());
        tn_u32 r = r_bits ? _tn_denormalize_f32(color->r, 255) >> (8 - r_bits) : 0;
        tn_u32 g = g_bits ? _tn_denormalize_f32(color->g, 255) >> (8 - g_bits) : 0;
        tn_u32 b = b_bits ? _tn_denormalize_f32(color->b, 255) >> (8 - b_bits) : 0;
        tn_u32 a = a_bits ? _tn_denormalize_f32(color->a, 255) >> (8 - a_bits) : 0;
        return (r << r_shift) | (g << g_shift) | (b << b_shift) | (a << a_shift);
    }
}



tn_u32
tn_color_rgba8888_to(
    tn_u32          rgba_color,
    const tn_color *colorI)
{
    tn_u32 r_ch, g_ch, b_ch, a_ch;
    tn_u32 r_mask, g_mask, b_mask, a_mask;

    r_ch = (rgba_color >> 24) & 0xff;
    g_ch = (rgba_color >> 16) & 0xff;
    b_ch = (rgba_color >> 8) & 0xff;
    a_ch = (rgba_color >> 0) & 0xff;

    r_mask = (1 << colorI->swz.r_bits) - 1;
    g_mask = (1 << colorI->swz.g_bits) - 1;
    b_mask = (1 << colorI->swz.b_bits) - 1;
    a_mask = (1 << colorI->swz.a_bits) - 1;

    if (((colorI->format >= TN_sRGBA_2222 && colorI->format <= TN_lARGB_2222)
         || (colorI->format >= TN_RGBA_2222 && colorI->format <= TN_ARGB_2222))
        && a_ch == 0xff)
    {
        tn_u8 new_r, new_g, new_b, new_a;
        new_r = (r_ch * 3 + 127) / 255;
        new_g = (g_ch * 3 + 127) / 255;
        new_b = (b_ch * 3 + 127) / 255;
        new_a = (a_ch * 3 + 127) / 255;

        rgba_color = ( new_r << colorI->swz.r_shift ) |
                     ( new_g << colorI->swz.g_shift ) |
                     ( new_b << colorI->swz.b_shift ) |
                     ( new_a << colorI->swz.a_shift );
    }
    else
    {
        rgba_color = ((( r_ch >> ( 8 - colorI->swz.r_bits ) ) & r_mask ) << colorI->swz.r_shift) |
                     ((( g_ch >> ( 8 - colorI->swz.g_bits ) ) & g_mask ) << colorI->swz.g_shift) |
                     ((( b_ch >> ( 8 - colorI->swz.b_bits ) ) & b_mask ) << colorI->swz.b_shift) |
                     ((( a_ch >> ( 8 - colorI->swz.a_bits ) ) & a_mask ) << colorI->swz.a_shift);
    }

    return rgba_color;
}



/// unpack a pixel into a color.
void
tn_color_unpack(
    tn_u32    packed_pixel,
    tn_color *color)
{
    tn_color colorI;
    tn_color_copy(color->format, &colorI);

    tn_i32 r_bits = colorI.swz.r_bits;
    tn_i32 g_bits = colorI.swz.g_bits;
    tn_i32 b_bits = colorI.swz.b_bits;
    tn_i32 a_bits = colorI.swz.a_bits;
    // int lb = inputDesc.luminanceBits;
    tn_i32 r_shift = colorI.swz.r_shift;
    tn_i32 g_shift = colorI.swz.g_shift;
    tn_i32 b_shift = colorI.swz.b_shift;
    tn_i32 a_shift = colorI.swz.a_shift;
    // int ls = inputDesc.luminanceShift;

    // m_format = inputDesc.internalFormat;
    // if(lb)
    // {   //luminance
    //     r = g = b = intToColor(inputData >> ls, (1<<lb)-1);
    //     a = 1.0f;
    // }
    // else
    {   //rgba
        color->r = r_bits ? _tn_normalize_u32(packed_pixel >> r_shift, (1U << r_bits) - 1) : (tn_f32)1.0f;
        color->g = g_bits ? _tn_normalize_u32(packed_pixel >> g_shift, (1U << g_bits) - 1) : (tn_f32)1.0f;
        color->b = b_bits ? _tn_normalize_u32(packed_pixel >> b_shift, (1U << b_bits) - 1) : (tn_f32)1.0f;
        color->a = a_bits ? _tn_normalize_u32(packed_pixel >> a_shift, (1U << a_bits) - 1) : (tn_f32)1.0f;

        color->colorspace =  colorI.colorspace;

        if (colorI.colorspace & TN_CS_PREMULTIPLIED)
        {   //clamp premultiplied color to alpha to enforce consistency
            color->r = tnMIN(color->r, color->a);
            color->g = tnMIN(color->g, color->a);
            color->b = tnMIN(color->b, color->a);
        }
    }
    // assertConsistency();
}



void
tn_color_premultiply_imm(tn_color *color)
{
    color->r *= color->a;
    color->g *= color->a;
    color->b *= color->a;
}
