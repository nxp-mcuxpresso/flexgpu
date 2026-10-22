//
// Copyright 2025-2026 NXP
//
// SPDX-License-Identifier: MIT
//

#include "tn_fmt.h"
#include "tn_defs.h"
#include "tn_regs_image.h"


tn_u8
tn_fmt_get_image_word_width(const tn_color colI)
{
    switch (colI.bpp)
    {
    case 32:
        return TN_REGS_IMAGE_FMT_WORD_WIDTH_32;
    case 24:
        return TN_REGS_IMAGE_FMT_WORD_WIDTH_24;
    case 16:
        return TN_REGS_IMAGE_FMT_WORD_WIDTH_16;
    case 8:
        return TN_REGS_IMAGE_FMT_WORD_WIDTH_8;
    case 4:
        return TN_REGS_IMAGE_FMT_WORD_WIDTH_4;
    case 2:
        return TN_REGS_IMAGE_FMT_WORD_WIDTH_2;
    case 1:
        return TN_REGS_IMAGE_FMT_WORD_WIDTH_1;
    default:
        // the callers shift the result straight into a register field, they
        // cannot act on an error value
        tnABORT(TN_FAULT_UNSUPPORTED, "Unsupported color width");
    }
}



tn_bool
tn_fmt_is_mask_fmt(const tn_color_format fmt)
{
    switch (fmt)
    {
    case TN_A_8:
    case TN_A_4:
    case TN_A_1:
        return tnTRUE;
    default:
        return tnFALSE;
    }
}



tn_u8
tn_fmt_get_mask_fmt_reg(const tn_color_format fmt)
{
    switch (fmt)
    {
    case TN_A_8:
        return MSK_FMT_A8;
    case TN_A_4:
        return MSK_FMT_A4;
    case TN_A_1:
        return MSK_FMT_A1;
    default:
        tnABORT(TN_FAULT_UNSUPPORTED, "Unsupported mask format");
    }
}



tn_bool
tn_fmt_is_compressed(const tn_color_format fmt)
{
    tn_color colorI;
    tn_color_copy(fmt, &colorI);
    if (colorI.colorspace & TN_CS_COMPRESSED)
    {
        return tnTRUE;
    }

    return tnFALSE;
}



tn_bool
tn_fmt_is_ETC2_compressed(const tn_color_format fmt)
{
    switch (fmt)
    {
    case TN_COMPRESSED_ETC2_RGB:
    case TN_COMPRESSED_ETC2_RGBA:
    case TN_COMPRESSED_ETC2_RGBA1:
        return tnTRUE;
    }

    return tnFALSE;
}



tn_bool
tn_fmt_is_IBC32x1_compressed(const tn_color_format fmt)
{
    switch (fmt)
    {
    case TN_COMPRESSED_IBC_RGBA_32x1_16BPP:
    case TN_COMPRESSED_IBC_RGB_32x1_12BPP:
    case TN_COMPRESSED_IBC_RGBA_32x1_13BPP:
    case TN_COMPRESSED_IBC_RGB_32x1_10BPP:
    case TN_COMPRESSED_IBC_RGBA_32x1_11BPP:
    case TN_COMPRESSED_IBC_RGB_32x1_8BPP:
        return tnTRUE;
    }

    return tnFALSE;
}



tn_bool
tn_fmt_is_IBC8x8_compressed(const tn_color_format fmt)
{
    switch (fmt)
    {
    case TN_COMPRESSED_IBC_RGBA_8x8_16BPP:
    case TN_COMPRESSED_IBC_RGB_8x8_12BPP:
    case TN_COMPRESSED_IBC_RGBA_8x8_13BPP:
    case TN_COMPRESSED_IBC_RGB_8x8_10BPP:
    case TN_COMPRESSED_IBC_RGBA_8x8_11BPP:
    case TN_COMPRESSED_IBC_RGB_8x8_8BPP:
        return tnTRUE;
    }

    return tnFALSE;
}



tn_bool
tn_fmt_is_yuv(const tn_color_format fmt)
{
    tn_color colorI;
    tn_color_copy(fmt, &colorI);
    if (colorI.colorspace & TN_CS_YUV)
        return tnTRUE;
    return tnFALSE;
}



tn_bool
tn_fmt_is_yuv_alpha(const tn_color_format fmt)
{
    if (fmt == TN_AYUV32 || fmt == TN_VUYA32 ||
        fmt == TN_YUVA32 || fmt == TN_AVUY32 ||
        fmt == TN_XYUV32 || fmt == TN_VUYX32 ||
        fmt == TN_YUVX32)
        return tnTRUE;
    return tnFALSE;
}



tn_color_format
tn_fmt_get_YV(const tn_color_format fmt)
{
    switch (fmt)
    {
    case TN_NV12:
        return TN_NV21;
    case TN_NV16:
        return TN_NV61;
    case TN_NV24:
        return TN_NV42;
    default:
        return fmt;     // don't convert
    }
}



tn_color_format
tn_fmt_get_YU(const tn_color_format fmt)
{
    switch (fmt)
    {
    case TN_YV12:
        return TN_YU12;
    case TN_YV16:
        return TN_YU16;
    case TN_YV24:
        return TN_YU24;
    default:
        return fmt;     // don't convert
    }
}



tn_bool
tn_fmt_is_rgb_a_planar(const tn_color_format fmt)
{
    if (((fmt >= TN_sBGR_A8565) && (fmt <= TN_lBGR_A8888)) ||
        ((fmt >= TN_BGR_A8565) && (fmt <= TN_BGR_A8888)))
    {
        return tnTRUE;
    }

    return tnFALSE;
}



tn_u8
tn_fmt_is_gamma(const tn_color colorI)
{
    if (colorI.colorspace & TN_CS_NONLINEAR)
    {
        return 1;
    }

    return 0;
}



tn_bool
tn_fmt_is_indexed(const tn_color_format fmt)
{
    if (fmt == TN_INDEX_1 ||
        fmt == TN_INDEX_2 ||
        fmt == TN_INDEX_4 ||
        fmt == TN_INDEX_8)
        return tnTRUE;
    return tnFALSE;
}



tn_bool
tn_fmt_is_vglite(tn_color_format fmt)
{
    return (fmt >= TN_RGBX_8888 && fmt <= TN_L_16_LE);
}



tn_bool
tn_fmt_is_supported_on_dst(const tn_color colorI)
{
    if (colorI.colorspace & (TN_CS_YUV | TN_CS_LUMINANCE | TN_CS_COMPRESSED))
    {
        return 0;
    }

    if (colorI.bpp == 8 || colorI.bpp == 16 || colorI.bpp == 24 || colorI.bpp == 32)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}



tn_u32
tn_fmt_get_supported_src_chan(const tn_color_format fmt)
{
    tn_color colorI;
    tn_color_copy(fmt, &colorI);

    if (tn_fmt_is_rgb_a_planar(fmt) ||
        tn_fmt_is_compressed(fmt))
        return 0;

    if ((fmt == TN_NV12)  || (fmt == TN_NV21) ||
        (fmt == TN_NV16)  || (fmt == TN_NV61) ||
        (fmt == TN_NV24)  || (fmt == TN_NV42) ||
        (fmt == TN_YV12)  || (fmt == TN_YU12) ||
        (fmt == TN_YV16)  || (fmt == TN_YU16) ||
        (fmt == TN_YV24)  || (fmt == TN_YU24) ||
        (fmt == TN_AYU12) || (fmt == TN_AYU16)||
        (fmt == TN_AYU24))
        return 0;

    return 1;
}



tn_bool
tn_fmt_not_supported_on_s1(const tn_color_format fmt)
{
    if ((fmt == TN_COMPRESSED_IBC_RGBA_32x1_16BPP)    ||
        (fmt == TN_COMPRESSED_IBC_RGBA_32x1_13BPP)    ||
        (fmt == TN_COMPRESSED_IBC_RGBA_32x1_11BPP)    ||
        (fmt == TN_COMPRESSED_IBC_RGB_32x1_12BPP)     ||
        (fmt == TN_COMPRESSED_IBC_RGB_32x1_10BPP)     ||
        (fmt == TN_COMPRESSED_IBC_RGB_32x1_8BPP))
        return tnTRUE;

    return tnFALSE;
}



tn_bool
tn_fmt_is_supported_on_s2(const tn_color_format fmt)
{
    if (tn_fmt_is_rgb_a_planar(fmt))
        return tnFALSE;
    if ((fmt == TN_NV12) || (fmt == TN_NV21) ||
        (fmt == TN_NV16) || (fmt == TN_NV61) ||
        (fmt == TN_NV24) || (fmt == TN_NV42) ||
        (fmt == TN_YV12) || (fmt == TN_YU12) || (fmt == TN_AYU12) ||
        (fmt == TN_YV16) || (fmt == TN_YU16) || (fmt == TN_AYU16) ||
        (fmt == TN_YV24) || (fmt == TN_YU24) || (fmt == TN_AYU24))
        return tnFALSE;
    if ((fmt == TN_COMPRESSED_ETC2_RGBA)  ||
        (fmt == TN_COMPRESSED_ETC2_RGB)      ||
        (fmt == TN_COMPRESSED_ETC2_RGBA1) ||
        (fmt == TN_COMPRESSED_IBC_RGBA_8x8_16BPP)  ||
        (fmt == TN_COMPRESSED_IBC_RGBA_8x8_13BPP)  ||
        (fmt == TN_COMPRESSED_IBC_RGBA_8x8_11BPP)  ||
        (fmt == TN_COMPRESSED_IBC_RGB_8x8_12BPP)   ||
        (fmt == TN_COMPRESSED_IBC_RGB_8x8_10BPP)   ||
        (fmt == TN_COMPRESSED_IBC_RGB_8x8_8BPP)    ||
        (fmt == TN_COMPRESSED_IBC_RGBA_32x1_16BPP) ||
        (fmt == TN_COMPRESSED_IBC_RGBA_32x1_13BPP) ||
        (fmt == TN_COMPRESSED_IBC_RGBA_32x1_11BPP) ||
        (fmt == TN_COMPRESSED_IBC_RGB_32x1_12BPP)  ||
        (fmt == TN_COMPRESSED_IBC_RGB_32x1_10BPP)  ||
        (fmt == TN_COMPRESSED_IBC_RGB_32x1_8BPP))
        return tnFALSE;

    return tnTRUE;
}



tn_u32
tn_fmt_get_yuv_type(const tn_color_format fmt)
{
    switch (fmt)
    {
    case TN_NV12:
    case TN_NV21:
        return TN_REGS_IMAGE_FMT_TYPE2_420 | TN_REGS_IMAGE_FMT_TYPE2_2PLANE;
    case TN_NV16:
    case TN_NV61:
        return TN_REGS_IMAGE_FMT_TYPE2_422 | TN_REGS_IMAGE_FMT_TYPE2_2PLANE;
    case TN_NV24:
    case TN_NV42:
        return TN_REGS_IMAGE_FMT_TYPE2_444 | TN_REGS_IMAGE_FMT_TYPE2_2PLANE;
    case TN_YV12:
    case TN_YU12:
        return TN_REGS_IMAGE_FMT_TYPE2_420 | TN_REGS_IMAGE_FMT_TYPE2_3PLANE;
    case TN_AYU12:
        return TN_REGS_IMAGE_FMT_TYPE2_420 | TN_REGS_IMAGE_FMT_TYPE2_4PLANE;
    case TN_YV16:
    case TN_YU16:
        return TN_REGS_IMAGE_FMT_TYPE2_422 | TN_REGS_IMAGE_FMT_TYPE2_3PLANE;
    case TN_AYU16:
        return TN_REGS_IMAGE_FMT_TYPE2_422 | TN_REGS_IMAGE_FMT_TYPE2_4PLANE;
    case TN_YV24:
    case TN_YU24:
        return TN_REGS_IMAGE_FMT_TYPE2_444 | TN_REGS_IMAGE_FMT_TYPE2_3PLANE;
    case TN_AYU24:
        return TN_REGS_IMAGE_FMT_TYPE2_444 | TN_REGS_IMAGE_FMT_TYPE2_4PLANE;
    case TN_YUYV:
        return TN_REGS_IMAGE_FMT_TYPE2_422 | TN_REGS_IMAGE_FMT_TYPE2_1PLANE;
    case TN_YUV24:
    case TN_AYUV32:
    case TN_VUYA32:
    case TN_YUVA32:
    case TN_AVUY32:
    case TN_XYUV32:
    case TN_VUYX32:
    case TN_YUVX32:
        return TN_REGS_IMAGE_FMT_TYPE2_444 | TN_REGS_IMAGE_FMT_TYPE2_1PLANE;
    default:
        tnABORT(TN_FAULT_UNSUPPORTED, "Unsupported YUV format");
    }
}



tn_u32
tn_fmt_get_compressed_type(const tn_color_format fmt)
{
    switch (fmt)
    {
    case TN_COMPRESSED_ETC2_RGB:
        return TN_REGS_IMAGE_FMT_TYPE2_ETC2_RGB;
    case TN_COMPRESSED_ETC2_RGBA:
        return TN_REGS_IMAGE_FMT_TYPE2_ETC2_RGBA;
    case TN_COMPRESSED_ETC2_RGBA1:
        return TN_REGS_IMAGE_FMT_TYPE2_ETC2_RGBA1;
    case TN_COMPRESSED_IBC_RGBA_8x8_16BPP:
        return TN_REGS_IMAGE_FMT_TYPE2_IBC_RGBA_8x8_16BPP;
    case TN_COMPRESSED_IBC_RGB_8x8_12BPP:
        return TN_REGS_IMAGE_FMT_TYPE2_IBC_RGB_8x8_12BPP;
    case TN_COMPRESSED_IBC_RGBA_8x8_13BPP:
        return TN_REGS_IMAGE_FMT_TYPE2_IBC_RGBA_8x8_13BPP;
    case TN_COMPRESSED_IBC_RGB_8x8_10BPP:
        return TN_REGS_IMAGE_FMT_TYPE2_IBC_RGB_8x8_10BPP;
    case TN_COMPRESSED_IBC_RGBA_8x8_11BPP:
        return TN_REGS_IMAGE_FMT_TYPE2_IBC_RGBA_8x8_11BPP;
    case TN_COMPRESSED_IBC_RGB_8x8_8BPP:
        return TN_REGS_IMAGE_FMT_TYPE2_IBC_RGB_8x8_8BPP;
    case TN_COMPRESSED_IBC_RGBA_32x1_16BPP:
        return TN_REGS_IMAGE_FMT_TYPE2_IBC_RGBA_32x1_16BPP;
    case TN_COMPRESSED_IBC_RGB_32x1_12BPP:
        return TN_REGS_IMAGE_FMT_TYPE2_IBC_RGB_32x1_12BPP;
    case TN_COMPRESSED_IBC_RGBA_32x1_13BPP:
        return TN_REGS_IMAGE_FMT_TYPE2_IBC_RGBA_32x1_13BPP;
    case TN_COMPRESSED_IBC_RGB_32x1_10BPP:
        return TN_REGS_IMAGE_FMT_TYPE2_IBC_RGB_32x1_10BPP;
    case TN_COMPRESSED_IBC_RGBA_32x1_11BPP:
        return TN_REGS_IMAGE_FMT_TYPE2_IBC_RGBA_32x1_11BPP;
    case TN_COMPRESSED_IBC_RGB_32x1_8BPP:
        return TN_REGS_IMAGE_FMT_TYPE2_IBC_RGB_32x1_8BPP;
    default:
        tnABORT(TN_FAULT_UNSUPPORTED, "Unsupported COMPRESSED format");
    }
}



tn_u8
tn_fmt_is_premul(const tn_color colorI)
{
    if (colorI.colorspace & TN_CS_PREMULTIPLIED)
    {
        return 1;
    }

    return 0;
}



tn_u8
tn_fmt_get_lut_width(const tn_color_format fmt)
{
    switch (fmt)
    {
    case TN_INDEX_1:
        return TN_REGS_IMAGE_FMT_TYPE2_INDEX1;
    case TN_INDEX_2:
        return TN_REGS_IMAGE_FMT_TYPE2_INDEX2;
    case TN_INDEX_4:
        return TN_REGS_IMAGE_FMT_TYPE2_INDEX4;
    case TN_INDEX_8:
        return TN_REGS_IMAGE_FMT_TYPE2_INDEX8;
    default:
        tnABORT(TN_FAULT_UNSUPPORTED, "Unsupported LUT width");
    }
}



tn_reg_32
tn_fmt_quality_to_reg_value(tn_reg_32 q)
{
    tn_reg_32 quality = TN_REGS_VG_IMAGE_QUALITY_NONANTIALIASED; // default value
    switch (q)
    {
    case TN_RENDERING_QUALITY_NONANTIALIASED:
        quality = TN_REGS_VG_IMAGE_QUALITY_NONANTIALIASED;
        break;
    case TN_RENDERING_QUALITY_FASTER:
        quality = TN_REGS_VG_IMAGE_QUALITY_FASTER;
        break;
    case TN_RENDERING_QUALITY_BETTER:
        quality = TN_REGS_VG_IMAGE_QUALITY_BETTER;
        break;
    default:
        tnABORT(TN_FAULT_UNSUPPORTED, "Unsupported quality");
    }

    return quality;
}
