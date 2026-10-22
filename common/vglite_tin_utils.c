//
// Copyright 2025-2026 NXP
//
// SPDX-License-Identifier: MIT
//

#include "vglite_tin_utils.h"
#include "tn_mspace.h"
#include "tn_fmt.h"
#include "tn_misc.h"
#include "tn_ctx_api.h"



void
setup_yuv_struct(
    vg_lite_buffer_t *buffer,
    tn_color_format   fmt,
    tn_size          *y,
    tn_size          *u,
    tn_size          *v,
    tn_size          *a)
{
    tnIN();

    tnEXPECT(!tn_is_null(buffer));
    tnEXPECT(!tn_is_null(y));
    tnEXPECT(!tn_is_null(u));
    tnEXPECT(!tn_is_null(v));
    tnEXPECT(!tn_is_null(a));

    switch (fmt)
    {
    case TN_NV12:
    case TN_NV21:
        *y = buffer->width * buffer->height;
        *u = *v = *y / 4;
        buffer->yuv.uv_stride = buffer->width;
        buffer->yuv.v_stride  = 0;
        buffer->yuv.uv_height = buffer->height / 2;
        buffer->yuv.v_height  = 0;
        break;
    case TN_NV16:
    case TN_NV61:
        *y = buffer->width * buffer->height;
        *u = *v = *y / 2;
        buffer->yuv.uv_stride = buffer->width;
        buffer->yuv.v_stride  = 0;
        buffer->yuv.uv_height = buffer->height;
        buffer->yuv.v_height  = 0;
        break;
    case TN_NV24:
    case TN_NV42:
        *y = buffer->width * buffer->height;
        *u = *v = *y;
        buffer->yuv.uv_stride = buffer->width * 2;
        buffer->yuv.v_stride  = 0;
        buffer->yuv.uv_height = buffer->height;
        buffer->yuv.v_height  = 0;
        break;
    case TN_AYU12:
        *a = buffer->width * buffer->height;
        buffer->yuv.alpha_stride = buffer->width;
    //fallthrough
    case TN_YV12:
    case TN_YU12:
        *y = buffer->width * buffer->height;
        *u = *y / 4;
        *v = *y / 4;
        buffer->yuv.uv_stride = buffer->width / 2;
        buffer->yuv.v_stride  = buffer->width / 2;
        buffer->yuv.uv_height = buffer->height / 2;
        buffer->yuv.v_height  = buffer->height / 2;
        break;
    case TN_AYU16:
        *a = buffer->width * buffer->height;
        buffer->yuv.alpha_stride = buffer->width;
    //fallthrough
    case TN_YV16:
    case TN_YU16:
        *y = buffer->width * buffer->height;
        *u = *y / 2;
        *v = *y / 2;
        buffer->yuv.uv_stride = buffer->width / 2;
        buffer->yuv.v_stride  = buffer->width / 2;
        buffer->yuv.uv_height = buffer->height;
        buffer->yuv.v_height  = buffer->height;
        break;
    case TN_AYU24:
        *a = buffer->width * buffer->height;
        buffer->yuv.alpha_stride = buffer->width;
    //fallthrough
    case TN_YV24:
    case TN_YU24:
        *y = buffer->width * buffer->height;
        *u = *y;
        *v = *y;
        buffer->yuv.uv_stride = buffer->width;
        buffer->yuv.v_stride  = buffer->width;
        buffer->yuv.uv_height = buffer->height;
        buffer->yuv.v_height  = buffer->height;
        break;
    case TN_YUYV:
        *y = buffer->width * buffer->height;
        *u = *v = *y / 2;
        buffer->yuv.uv_stride = 0;
        buffer->yuv.v_stride  = 0;
        buffer->yuv.uv_height = 0;
        buffer->yuv.v_height  = 0;
        break;
    case TN_YUV24:
        *y = buffer->width * buffer->height;
        *u = *v = *y;
        buffer->yuv.uv_stride = 0;
        buffer->yuv.v_stride  = 0;
        buffer->yuv.uv_height = 0;
        buffer->yuv.v_height  = 0;
        break;
    case TN_AYUV32:
    case TN_VUYA32:
    case TN_YUVA32:
    case TN_AVUY32:
    case TN_XYUV32:
    case TN_VUYX32:
    case TN_YUVX32:
        *y = buffer->width * buffer->height;
        *a = *u = *v = *y;
        buffer->yuv.uv_stride = 0;
        buffer->yuv.v_stride  = 0;
        buffer->yuv.uv_height = 0;
        buffer->yuv.v_height  = 0;
        break;
    default:
        tnABORT(TN_FAULT_UNREACHABLE, "Unhandled YUV format!");
    }

    tnOUT();
}



tn_color_format
getYVFormat(tn_color_format fmt)
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
getYUFormat(tn_color_format fmt)
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



tn_color_format
vglite_color_2_tn_color(vg_lite_buffer_format_t fmt)
{
/* The switch below also handles the extended formats declared by
 * vg_lite_ext_buffer_format_t and the OpenVG formats. Those constants are not
 * members of vg_lite_buffer_format_t, so the compiler must not warn about case
 * labels outside the switched enumeration. */
#if defined(__GNUC__)
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wswitch"
#elif defined(__IAR_SYSTEMS_ICC__)
    #pragma diag_suppress=Pe047
#elif defined(__ARMCC_VERSION) && (__ARMCC_VERSION < 6000000)
    #pragma diag_suppress 1296
#endif

    switch ((tn_u32)fmt)
    {
    case OPENVG_sRGBX_8888_PRE:
        return TN_sRGBX_8888_PRE;
    case OPENVG_lXBGR_8888:
        return TN_lXBGR_8888;
    case OPENVG_lRGBX_8888_PRE:
        return TN_lRGBX_8888_PRE;
    case OPENVG_sABGR_8888:
        return TN_sABGR_8888;
    case VG_LITE_RGBA8888:      /// 32-bit RGBA format with 8 bits per color channel. Red is in bits 7:0, green in bits 15:8, blue in
        /// bits 23 : 16, and the alpha channel is in bits 31 : 24.
        return TN_ABGR_8888;
    case OPENVG_lABGR_8888:
        return TN_lABGR_8888;
    case OPENVG_sARGB_8888:
        return TN_sARGB_8888;
    case OPENVG_sXBGR_8888:
        return TN_sXBGR_8888;
    case VG_LITE_RGBX8888:      /// 32-bit RGBX format with 8 bits per color channel. Red is in bits 7:0, green in bits 15:8, blue in
        /// bits 23 : 16, and the x channel is in bits 31 : 24.
        return TN_XBGR_8888;
    case OPENVG_lRGBX_8888:
        return TN_lRGBX_8888;
    case VG_LITE_BGRX8888:      /// 32-bit RGBX format with 8 bits per color channel. Red is in bits 23:16, green in bits 15:8, blue in
        /// bits 7 : 0, and the x channel is in bits 31 : 24.
        return TN_XRGB_8888;
    case OPENVG_lXRGB_8888:
        return TN_lXRGB_8888;
    case VG_LITE_RGB565:        /// 16-bit RGB format with 5 and 6 bits per color channel. Red is in bits 4:0, green in bits 10:5, and
        /// the blue color channel is in bits 15 : 11.
        return TN_BGR_565;
    case OPENVG_sBGR_565:
        return TN_sBGR_565;
    case VG_LITE_BGR565:        /// 16-bit RGB format with 5 and 6 bits per color channel. Red is in bits 15:11, green in bits 10:5,
        /// and the blue color channel is in bits 4 : 0.
        return TN_RGB_565;
    case OPENVG_sRGB_565:
        return TN_sRGB_565;
    case OPENVG_sRGB_565_PRE:
        return TN_sRGB_565_PRE;
    case OPENVG_lRGB_565:
        return TN_lRGB_565;
    case OPENVG_lRGB_565_PRE:
        return TN_lRGB_565_PRE;
    case VG_LITE_RGBA4444:      /// 16-bit RGBA format with 4 bits per color channel. Red is in bits 3:0, green in bits 7:4, blue in
        /// bits 11 : 8 and the alpha channel is in bits 15 : 12.
        return TN_ABGR_4444;
    case OPENVG_sABGR_4444:
        return TN_sABGR_4444;
    case VG_LITE_BGRA4444:      /// 16-bit RGBA format with 4 bits per color channel. Red is in bits 11:8, green in bits 7:4, blue in
        /// bits 3 : 0 and the alpha channel is in bits 15 : 12.
        return TN_ARGB_4444;
    case OPENVG_sARGB_4444:
        return TN_sARGB_4444;
    case VG_LITE_BGRA5551:      /// 16-bit RGBA format with 4 bits per color channel. Red is in bits 14:10, green in bits 9:5, blue in
        /// bits 4 : 0 and the alpha channel is in bit 15 : 15.
        return TN_ARGB_1555;
    case OPENVG_sARGB_1555:
        return TN_sARGB_1555;
    case OPENVG_A_1:
        return TN_A_1;
    case VG_LITE_A4:            /// 4-bit alpha format. There are no RGB values.
    case OPENVG_A_4:
        return TN_A_4;
    case VG_LITE_A8:            /// 8-bit alpha format. There are no RGB values.
    case OPENVG_A_8:
        return TN_A_8;
    case OPENVG_sL_8:
        return TN_sL_8;
    case VG_LITE_L8:            /// 8-bit luminance value. There is no alpha value.
        return TN_L_8;
    case OPENVG_lL_8:
        return TN_lL_8;
    case OPENVG_BW_1:
        return TN_lL_1;         /// 1-bit Black and White
    case VG_LITE_L16:           /// GRAY16
        return TN_L_16_LE;
    case VG_LITE_YUY2:
    case VG_LITE_YUYV:          /// Packed YUV format, 32-bit for 2 pixels. Y0 is in bits 7:0 and V is in bits 31:23.
        return TN_YUYV;
    case VG_LITE_NV12:
        return TN_NV12;
    case VG_LITE_NV21:
        return TN_NV21;
    case VG_LITE_NV16:
        return TN_NV16;
    case VG_LITE_NV61:
        return TN_NV61;
    case VG_LITE_NV24:
        return TN_NV24;
    case VG_LITE_NV42:
        return TN_NV42;
    case VG_LITE_ANV12:
        return TN_ANV12;
    case VG_LITE_YV12:
        return TN_YV12;
    case VG_LITE_YV16:
        return TN_YV16;
    case VG_LITE_YV24:
        return TN_YV24;
    case VG_LITE_AYU12:
        return TN_AYU12;
    case VG_LITE_AYU16:
        return TN_AYU16;
    case VG_LITE_AYU24:
        return TN_AYU24;
    case VG_LITE_AYUY2:
        return TN_AYUY2;
    case VG_LITE_YUY2_TILED:
    case VG_LITE_NV12_TILED:
    case VG_LITE_ANV12_TILED:
    case VG_LITE_AYUY2_TILED:
        return TN_MAX_COLOR_FORMAT;
    case VG_LITE_INDEX_1:       /// Indexed format.
        return TN_INDEX_1;
    case VG_LITE_INDEX_2:
        return TN_INDEX_2;
    case VG_LITE_INDEX_4:
        return TN_INDEX_4;
    case VG_LITE_INDEX_8:
        return TN_INDEX_8;
    case VG_LITE_RGBA2222:      /// 8-bit RGBA format with 2 bits per color channel.Red is in bits 1:0,green in bits 3:2,blue in
                                /// bits 5 : 4 and the alpha channel is in bits 7 : 6
        return TN_ABGR_2222;
    case VG_LITE_BGRA2222:      /// 8-bit RGBA format with 2 bits per color channel.Blue is in bits 1:0,green in bits 3:2,red in
                                /// bits 5 : 4 and the alpha channel is in bits 7 : 6
        return TN_ARGB_2222;
    case VG_LITE_ABGR2222:      /// 8-bit RGBA format with 2 bits per color channel.Alpha is in bits 1:0,blue in bits 3:2,green in
                                /// bits 5 : 4 and the red channel is in bits 7 : 6
        return TN_RGBA_2222;
    case VG_LITE_ARGB2222:      /// 8-bit RGBA format with 2 bits per color channel.Alpha is in bits 1:0,red in bits 3:2,green in
                                /// bits 5 : 4 and the blue channel is in bits 7 : 6
        return TN_BGRA_2222;
    case VG_LITE_ABGR4444:      /// 16-bit RGBA format with 4 bits per color channel. Alpha is in bits 3:0, blue in bits 7:4, green in
        /// bits 11 : 8 and the red channel is in bits 15 : 12.
        return TN_RGBA_4444;
    case OPENVG_sRGBA_4444:
        return TN_sRGBA_4444;
    case OPENVG_sRGBA_4444_PRE:
        return TN_sRGBA_4444_PRE;
    case OPENVG_lRGBA_4444:
        return TN_lRGBA_4444;
    case OPENVG_lRGBA_4444_PRE:
        return TN_lRGBA_4444_PRE;
    case VG_LITE_ARGB4444:      /// 16-bit RGBA format with 4 bits per color channel. Alpha is in bits 3:0, red in bits 7:4, green in
        /// bits 11 : 8 and the blue channel is in bits 15 : 12.
        return TN_BGRA_4444;
    case OPENVG_sBGRA_4444:
        return TN_sBGRA_4444;
    case OPENVG_sRGBA_8888:
        return TN_sRGBA_8888;
    case VG_LITE_ABGR8888:      /// 32-bit RGBA format with 8 bits per color channel. Alpha is in bits 7:0, blue in bits 15:8, green in
        /// bits 23 : 16, and the red channel is in bits 31 : 24.
        return TN_RGBA_8888;
    case OPENVG_lRGBA_8888:
        return TN_lRGBA_8888;
    case OPENVG_sBGRA_8888:
        return TN_sBGRA_8888;
    case VG_LITE_ARGB8888:      /// 32-bit RGBA format with 8 bits per color channel. Alpha is in bits 7:0, red in bits 15:8, green in
        /// bits 23 : 16, and the blue channel is in bits 31 : 24.
        return TN_BGRA_8888;
    case OPENVG_lBGRA_8888:
        return TN_lBGRA_8888;
    case VG_LITE_ABGR1555:      /// 16-bit RGBA format with 4 bits per color channel. Red is in bits 15:11, green in bits 10:6, blue in
        /// bits 5 : 1 and the alpha channel is in bit 0 : 0.
        return TN_RGBA_5551;
    case OPENVG_sRGBA_5551:
        return TN_sRGBA_5551;
    case OPENVG_sRGBA_5551_PRE:
        return TN_sRGBA_5551_PRE;
    case OPENVG_lRGBA_5551:
        return TN_lRGBA_5551;
    case OPENVG_lRGBA_5551_PRE:
        return TN_lRGBA_5551_PRE;
    case VG_LITE_RGBA5551:      /// 16-bit RGBA format with 4 bits per color channel. Blue is in bits 14:10, green in bits 9:5, red in
        /// bits 4 : 0 and the alpha channel is in bit 15 : 15.
        return TN_ABGR_1555;
    case OPENVG_sABGR_1555:
        return TN_sABGR_1555;
    case VG_LITE_ARGB1555:      /// 16-bit RGBA format with 4 bits per color channel. Blue is in bits 15:11, green in bits 10:6, red in
        /// bits 5 : 1 and the alpha channel is in bit 0 : 0.
        return TN_BGRA_5551;
    case OPENVG_sBGRA_5551:
        return TN_sBGRA_5551;
    case VG_LITE_XBGR8888:      /// 32-bit RGBX format with 8 bits per color channel. X channel is in bits 7:0, blue in bits 15:8, green in
        /// bits 23 : 16, and the red is in bits 31 : 24.
        return TN_RGBX_8888;
    case OPENVG_sXRGB_8888:
        return TN_sXRGB_8888;
    case OPENVG_sRGBX_8888:
        return TN_sRGBX_8888;
    case OPENVG_sBGRX_8888:
        return TN_sBGRX_8888;
    case VG_LITE_XRGB8888:      /// 32-bit RGBX format with 8 bits per color channel. X channel is in bits 7:0, red in bits 15:8, green in
        /// bits 23 : 16, and the blue is in bits 31 : 24.
        return TN_BGRX_8888;
    case OPENVG_lBGRX_8888:
        return TN_lBGRX_8888;
    case VG_LITE_ABGR8565:      /// 24-bit RGBA format with 4 and 5 bits per color channel. Alpha channel is in bit 7:0, blue in bits 12:8, green in
                                /// bits 18 : 13 and the red in bits 23 : 19.
        return TN_RGBA_5658;
    case VG_LITE_BGRA5658:      /// 24-bit RGBA format with 4 and 5 bits per color channel. Blue is in bits 4:0, green in bits 10:5, red in
                                /// bits 15 : 11,alpha channel is in bit 23 : 16.
        return TN_ARGB_8565;
    case VG_LITE_ARGB8565:      /// 24-bit RGBA format with 4 and 5 bits per color channel. Alpha channel is in bits 7:0, red in bits 12:8, green in
                                /// bits 18 : 13, and the blue channel is in bits 23 : 19.
        return TN_BGRA_5658;
    case VG_LITE_RGBA5658:      /// 24-bit RGBA format with 4 and 5 bits per color channel. Red is in bits 4:0, green in bits 10:5, blue in
                                /// bits 15 : 11 and the alpha channel is in bits 23 : 16
        return TN_ABGR_8565;
    case VG_LITE_ABGR8565_PLANAR:
        return TN_RGB_A8565;   /// A8 is in the first plane
    case VG_LITE_BGRA5658_PLANAR:
        return TN_RGB_565A8;   /// A8 is in the second plane
    case VG_LITE_ARGB8565_PLANAR:
        return TN_BGR_A8565;
    case VG_LITE_RGBA5658_PLANAR:
        return TN_BGR_565A8;
    case VG_LITE_RGBA8888_PLANAR:
        return TN_BGR_888A8;
    case VG_LITE_BGRA8888_PLANAR:
        return TN_RGB_888A8;
    case VG_LITE_ARGB8888_PLANAR:
        return TN_BGR_A8888;
    case VG_LITE_ABGR8888_PLANAR:
        return TN_RGB_A8888;
    case VG_LITE_RGB888:        /// 24-bit RGB format with 8 bits per color channel. Red is in bits 7:0, green in bits 15:8, blue in
                                /// bits 23 : 16.
        return TN_BGR_888;
    case VG_LITE_BGR888:        /// 24-bit RGB format with 8 bits per color channel. Blue is in bits 7:0, green in bits 15:8, red in
                                /// bits 23 : 16.
        return TN_RGB_888;
    case OPENVG_sRGBA_8888_PRE:
        return TN_sRGBA_8888_PRE;
    case OPENVG_lRGBA_8888_PRE:
        return TN_lRGBA_8888_PRE;
    case OPENVG_sARGB_8888_PRE:
        return TN_sARGB_8888_PRE;
    case VG_LITE_BGRA8888:      /// 32-bit RGBA format with 8 bits per color channel. Red is in bits 23:16, green in bits 15:8, blue in
        /// bits 7 : 0, and the alpha channel is in bits 31 : 24.
        return TN_ARGB_8888;
    case OPENVG_lARGB_8888:
        return TN_lARGB_8888;
    case OPENVG_lARGB_8888_PRE:
        return TN_lARGB_8888_PRE;
    case OPENVG_sBGRA_8888_PRE:
        return TN_sBGRA_8888_PRE;
    case OPENVG_lBGRA_8888_PRE:
        return TN_lBGRA_8888_PRE;
    case OPENVG_sABGR_8888_PRE:
        return TN_sABGR_8888_PRE;
    case OPENVG_lABGR_8888_PRE:
        return TN_lABGR_8888_PRE;
    case VG_LITE_YUV24:
        return TN_YUV24;
    case VG_LITE_AYUV32:
        return TN_AYUV32;
    case VG_LITE_VUYA32:
        return TN_VUYA32;
    case VG_LITE_YUVA32:
        return TN_YUVA32;
    case VG_LITE_AVUY32:
        return TN_AVUY32;
    case VG_LITE_XYUV32:
        return TN_XYUV32;
    case VG_LITE_VUYX32:
        return TN_VUYX32;
    case VG_LITE_YUVX32:
        return TN_YUVX32;
    case VG_LITE_RGBA8888_ETC2_EAC:    /** ETC2 RGBA format. */
        return TN_COMPRESSED_ETC2_RGBA;
    case VG_LITE_RGB888_ETC2_EAC:      /** ETC2 RGB format. */
        return TN_COMPRESSED_ETC2_RGB;
    case VG_LITE_RGBA1_ETC2_EAC:
        return TN_COMPRESSED_ETC2_RGBA1;
    case VG_LITE_IBC_RGBA_8x8_16BPP:
        return TN_COMPRESSED_IBC_RGBA_8x8_16BPP;
    case VG_LITE_IBC_RGB_8x8_12BPP:
        return TN_COMPRESSED_IBC_RGB_8x8_12BPP;
    case VG_LITE_IBC_RGBA_8x8_13BPP:
        return TN_COMPRESSED_IBC_RGBA_8x8_13BPP;
    case VG_LITE_IBC_RGB_8x8_10BPP:
        return TN_COMPRESSED_IBC_RGB_8x8_10BPP;
    case VG_LITE_IBC_RGBA_8x8_11BPP:
        return TN_COMPRESSED_IBC_RGBA_8x8_11BPP;
    case VG_LITE_IBC_RGB_8x8_8BPP:
        return TN_COMPRESSED_IBC_RGB_8x8_8BPP;
    case VG_LITE_IBC_RGBA_32x1_16BPP:
        return TN_COMPRESSED_IBC_RGBA_32x1_16BPP;
    case VG_LITE_IBC_RGB_32x1_12BPP:
        return TN_COMPRESSED_IBC_RGB_32x1_12BPP;
    case VG_LITE_IBC_RGBA_32x1_13BPP:
        return TN_COMPRESSED_IBC_RGBA_32x1_13BPP;
    case VG_LITE_IBC_RGB_32x1_10BPP:
        return TN_COMPRESSED_IBC_RGB_32x1_10BPP;
    case VG_LITE_IBC_RGBA_32x1_11BPP:
        return TN_COMPRESSED_IBC_RGBA_32x1_11BPP;
    case VG_LITE_IBC_RGB_32x1_8BPP:
        return TN_COMPRESSED_IBC_RGB_32x1_8BPP;
    default:
    {
        tnLOG_ERR("invalid color format");
        return TN_MAX_COLOR_FORMAT;
    }
    }

#if defined(__GNUC__)
    #pragma GCC diagnostic pop
#elif defined(__IAR_SYSTEMS_ICC__)
    #pragma diag_default=Pe047
#elif defined(__ARMCC_VERSION) && (__ARMCC_VERSION < 6000000)
    #pragma diag_default 1296
#endif
}



// Fill in the per-plane geometry (number of planes, and the height and stride
// of each plane) for the format stored inside 'buffer'. The information is
// derived from the color format together with buffer->width / buffer->height
// and is used to copy planar buffers into video memory allocated with
// vg_lite_allocate.
//
// On unsupported / unknown formats, *num_planes is set to 0 and the height /
// stride arrays are left cleared.
void
get_planes_info(
    vg_lite_buffer_t *buffer,
    tn_u8            *num_planes,
    tn_u32            height[4],
    tn_u32            stride[4])
{
    tnIN();

    tnEXPECT(!tn_is_null(buffer));
    tnEXPECT(!tn_is_null(num_planes));

    tn_u32 w = (tn_u32)buffer->width;
    tn_u32 h = (tn_u32)buffer->height;

    // Start from a clean slate so unused planes report zero geometry.
    *num_planes = 0;
    for (tn_u8 i = 0; i < 4; i++)
    {
        height[i] = 0;
        stride[i] = 0;
    }

/* The switch below also handles the extended formats declared by
 * vg_lite_ext_buffer_format_t. Those constants are not members of
 * vg_lite_buffer_format_t, so the compiler must not warn about case labels
 * outside the switched enumeration. */
#if defined(__GNUC__)
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wswitch"
#elif defined(__IAR_SYSTEMS_ICC__)
    #pragma diag_suppress=Pe047
#elif defined(__ARMCC_VERSION) && (__ARMCC_VERSION < 6000000)
    #pragma diag_suppress 1296
#endif

    switch ((tn_u32)buffer->format)
    {
    // Semi-planar YUV: plane 0 = Y (1 byte/pel), plane 1 = interleaved UV.
    case VG_LITE_NV12:
    case VG_LITE_NV21:
        *num_planes = 2;
        stride[0] = w;      height[0] = h;
        stride[1] = w;      height[1] = h / 2;
        break;
    case VG_LITE_NV16:
    case VG_LITE_NV61:
        *num_planes = 2;
        stride[0] = w;      height[0] = h;
        stride[1] = w;      height[1] = h;
        break;
    case VG_LITE_NV24:
    case VG_LITE_NV42:
        *num_planes = 2;
        stride[0] = w;      height[0] = h;
        stride[1] = w * 2;  height[1] = h;
        break;

    // Alpha-last planar RGB565 (RGBA/BGRA prefix): plane 0 = RGB565 (2 bytes/pel),
    // plane 1 = A8. RGB plane is physically first in memory.
    case VG_LITE_BGRA5658_PLANAR:
    case VG_LITE_RGBA5658_PLANAR:
        *num_planes = 2;
        stride[0] = w * 2;  height[0] = h;
        stride[1] = w;      height[1] = h;
        break;

    // Alpha-first planar RGB565 (ARGB/ABGR prefix): plane 0 = A8,
    // plane 1 = RGB565 (2 bytes/pel). Alpha plane is physically first in memory.
    case VG_LITE_ABGR8565_PLANAR:
    case VG_LITE_ARGB8565_PLANAR:
        *num_planes = 2;
        stride[0] = w;      height[0] = h;
        stride[1] = w * 2;  height[1] = h;
        break;

    // Alpha-last planar RGB888 (RGBA/BGRA prefix): plane 0 = RGB888 (3 bytes/pel),
    // plane 1 = A8. RGB plane is physically first in memory.
    case VG_LITE_RGBA8888_PLANAR:
    case VG_LITE_BGRA8888_PLANAR:
        *num_planes = 2;
        stride[0] = w * 3;  height[0] = h;
        stride[1] = w;      height[1] = h;
        break;

    // Alpha-first planar RGB888 (ARGB/ABGR prefix): plane 0 = A8,
    // plane 1 = RGB888 (3 bytes/pel). Alpha plane is physically first in memory.
    case VG_LITE_ARGB8888_PLANAR:
    case VG_LITE_ABGR8888_PLANAR:
        *num_planes = 2;
        stride[0] = w;      height[0] = h;
        stride[1] = w * 3;  height[1] = h;
        break;

    // Planar YUV 4:2:0: Y + U + V, U/V subsampled in both directions.
    case VG_LITE_YV12:
        *num_planes = 3;
        stride[0] = w;      height[0] = h;
        stride[1] = w / 2;  height[1] = h / 2;
        stride[2] = w / 2;  height[2] = h / 2;
        break;
    // Planar YUV 4:2:2: Y + U + V, U/V subsampled horizontally only.
    case VG_LITE_YV16:
        *num_planes = 3;
        stride[0] = w;      height[0] = h;
        stride[1] = w / 2;  height[1] = h;
        stride[2] = w / 2;  height[2] = h;
        break;
    // Planar YUV 4:4:4: Y + U + V, full resolution.
    case VG_LITE_YV24:
        *num_planes = 3;
        stride[0] = w;      height[0] = h;
        stride[1] = w;      height[1] = h;
        stride[2] = w;      height[2] = h;
        break;

    // ANV12: Y + interleaved UV + A planes.
    case VG_LITE_ANV12:
        *num_planes = 3;
        stride[0] = w;      height[0] = h;
        stride[1] = w;      height[1] = h / 2;
        stride[2] = w;      height[2] = h;
        break;

    // Alpha + planar YUV 4:2:0: A + Y + U + V planes.
    case VG_LITE_AYU12:
        *num_planes = 4;
        stride[0] = w;      height[0] = h;
        stride[1] = w;      height[1] = h;
        stride[2] = w / 2;  height[2] = h / 2;
        stride[3] = w / 2;  height[3] = h / 2;
        break;
    // Alpha + planar YUV 4:2:2: A + Y + U + V planes.
    case VG_LITE_AYU16:
        *num_planes = 4;
        stride[0] = w;      height[0] = h;
        stride[1] = w;      height[1] = h;
        stride[2] = w / 2;  height[2] = h;
        stride[3] = w / 2;  height[3] = h;
        break;
    // Alpha + planar YUV 4:4:4: A + Y + U + V planes.
    case VG_LITE_AYU24:
        *num_planes = 4;
        stride[0] = w;      height[0] = h;
        stride[1] = w;      height[1] = h;
        stride[2] = w;      height[2] = h;
        stride[3] = w;      height[3] = h;
        break;

    // Packed formats: single plane. Use the already computed buffer stride.
    case VG_LITE_RGBA8888:
    case VG_LITE_RGBX8888:
    case VG_LITE_BGRA8888:
    case VG_LITE_BGRX8888:
    case VG_LITE_ABGR8888:
    case VG_LITE_XBGR8888:
    case VG_LITE_ARGB8888:
    case VG_LITE_XRGB8888:
    case VG_LITE_RGB565:
    case VG_LITE_BGR565:
    case VG_LITE_RGBA4444:
    case VG_LITE_BGRA4444:
    case VG_LITE_ABGR4444:
    case VG_LITE_ARGB4444:
    case VG_LITE_RGBA5551:
    case VG_LITE_BGRA5551:
    case VG_LITE_ABGR1555:
    case VG_LITE_ARGB1555:
    case VG_LITE_RGBA2222:
    case VG_LITE_BGRA2222:
    case VG_LITE_ABGR2222:
    case VG_LITE_ARGB2222:
    case VG_LITE_ABGR8565:
    case VG_LITE_BGRA5658:
    case VG_LITE_ARGB8565:
    case VG_LITE_RGBA5658:
    case VG_LITE_RGB888:
    case VG_LITE_BGR888:
    case VG_LITE_A4:
    case VG_LITE_A8:
    case VG_LITE_L8:
    case VG_LITE_L16:
    case VG_LITE_INDEX_1:
    case VG_LITE_INDEX_2:
    case VG_LITE_INDEX_4:
    case VG_LITE_INDEX_8:
    // Packed YUV
    case VG_LITE_YUYV:
    case VG_LITE_YUY2:
    case VG_LITE_YUV24:
    case VG_LITE_AYUV32:
    case VG_LITE_VUYA32:
    case VG_LITE_YUVA32:
    case VG_LITE_AVUY32:
    case VG_LITE_XYUV32:
    case VG_LITE_VUYX32:
    case VG_LITE_YUVX32:
    case VG_LITE_AYUY2:
    // Compressed formats
    case VG_LITE_RGBA8888_ETC2_EAC:
    case VG_LITE_RGB888_ETC2_EAC:
    case VG_LITE_RGBA1_ETC2_EAC:
    case VG_LITE_IBC_RGBA_8x8_16BPP:
    case VG_LITE_IBC_RGB_8x8_12BPP:
    case VG_LITE_IBC_RGBA_8x8_13BPP:
    case VG_LITE_IBC_RGB_8x8_10BPP:
    case VG_LITE_IBC_RGBA_8x8_11BPP:
    case VG_LITE_IBC_RGB_8x8_8BPP:
    case VG_LITE_IBC_RGBA_32x1_16BPP:
    case VG_LITE_IBC_RGB_32x1_12BPP:
    case VG_LITE_IBC_RGBA_32x1_13BPP:
    case VG_LITE_IBC_RGB_32x1_10BPP:
    case VG_LITE_IBC_RGBA_32x1_11BPP:
    case VG_LITE_IBC_RGB_32x1_8BPP:
        *num_planes = 1;
        stride[0] = (tn_u32)buffer->stride;
        height[0] = h;
        break;

    // Tiled formats: not supported by this HW.
    case VG_LITE_NV12_TILED:
    case VG_LITE_ANV12_TILED:
    case VG_LITE_YUY2_TILED:
    case VG_LITE_AYUY2_TILED:
        *num_planes = 0;
        break;

    default:
        *num_planes = 0;
        break;
    }

#if defined(__GNUC__)
    #pragma GCC diagnostic pop
#elif defined(__IAR_SYSTEMS_ICC__)
    #pragma diag_default=Pe047
#elif defined(__ARMCC_VERSION) && (__ARMCC_VERSION < 6000000)
    #pragma diag_default 1296
#endif
    tnOUT();
}



tn_bool
vglite_planes_total_bytes(
    tn_u8        num_planes,
    const tn_u32 height[4],
    const tn_u32 stride[4],
    tn_size     *total_out)
{
    tnIN();

    tnEXPECT(!tn_is_null(height));
    tnEXPECT(!tn_is_null(stride));
    tnEXPECT(!tn_is_null(total_out));

    if (num_planes > 4)
    {
        tnOUT();
        return tnFALSE;
    }

    tn_size total = 0;

    for (tn_u8 i = 0; i < num_planes; i++)
    {
        if (stride[i] != 0 && height[i] > (TN_SIZE_MAX / stride[i]))
        {
            tnOUT();
            return tnFALSE;
        }

        tn_size plane = (tn_size)height[i] * (tn_size)stride[i];

        if (plane > (TN_SIZE_MAX - total))
        {
            tnOUT();
            return tnFALSE;
        }

        total += plane;
    }

    *total_out = total;

    tnOUT();
    return tnTRUE;
}



tn_i32
vglite_blend_to_tn_blend(vg_lite_blend_t blend)
{
#if defined(__GNUC__)
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wswitch"
#elif defined(__IAR_SYSTEMS_ICC__)
    #pragma diag_suppress=Pe047
#elif defined(__ARMCC_VERSION) && (__ARMCC_VERSION < 6000000)
    #pragma diag_suppress 1296
#endif

    /* This implements mapping between blend modes defined in VGLite.
     * The VG_LITE_BLEND_ and OPENVG_BLEND_ modes have the same blending formula,
     * the difference is OPENVG_BLEND_ is enforcing premultiplication, while
     * VG_LITE_BLEND expects to use straight alpha. This difference is not handled in this function.
     */
    switch ((tn_u32)blend)
    {
    case VG_LITE_BLEND_NONE:    /// S, i.e. no blending.
        return TN_BLEND_VGLITE | TN_BLEND_SRC;
    case OPENVG_BLEND_SRC:
        return TN_BLEND_SRC;
    case VG_LITE_BLEND_SRC_OVER:/// S + (1 - Sa) * D
        return TN_BLEND_VGLITE | TN_BLEND_SRC_OVER;
    case OPENVG_BLEND_SRC_OVER:
        return TN_BLEND_SRC_OVER;
    case VG_LITE_BLEND_DST_OVER:/// (1 - Da) * S + D
        return TN_BLEND_VGLITE | TN_BLEND_DST_OVER;
    case OPENVG_BLEND_DST_OVER:
        return TN_BLEND_DST_OVER;
    case VG_LITE_BLEND_SRC_IN:  /// Da * S
        return TN_BLEND_VGLITE | TN_BLEND_SRC_IN;
    case OPENVG_BLEND_SRC_IN:
        return TN_BLEND_SRC_IN;
    case VG_LITE_BLEND_DST_IN:  /// Sa * D
        return TN_BLEND_VGLITE | TN_BLEND_DST_IN;
    case OPENVG_BLEND_DST_IN:
        return TN_BLEND_DST_IN;
    case VG_LITE_BLEND_SCREEN:  /// S + D - S * D
        return TN_BLEND_VGLITE | TN_BLEND_SCREEN;
    case OPENVG_BLEND_SCREEN:
        return TN_BLEND_SCREEN;
    case VG_LITE_BLEND_MULTIPLY:/// S * (1 - Da) + D * (1 - Sa) + S * D
        return TN_BLEND_VGLITE | TN_BLEND_MULTIPLY;
    case OPENVG_BLEND_MULTIPLY:
        return TN_BLEND_MULTIPLY;
    case VG_LITE_BLEND_ADDITIVE:/// S + D
        return TN_BLEND_VGLITE | TN_BLEND_ADDITIVE;
    case OPENVG_BLEND_ADDITIVE:
        return TN_BLEND_ADDITIVE;
    case VG_LITE_BLEND_SUBTRACT:/// D * (1 - S)
        return TN_BLEND_SUBTRACT;
    case VG_LITE_BLEND_DARKEN:  /// min(SrcOver, DstOver)
        return TN_BLEND_VGLITE | TN_BLEND_DARKEN;
    case OPENVG_BLEND_DARKEN:
        return TN_BLEND_DARKEN;
    case VG_LITE_BLEND_LIGHTEN: /// max(SrcOver, DstOver)
        return TN_BLEND_VGLITE | TN_BLEND_LIGHTEN;
    case OPENVG_BLEND_LIGHTEN:
        return TN_BLEND_LIGHTEN;
    case VG_LITE_BLEND_SRC_OUT: /// S - (Da * S) , Non-premultiplied
        return TN_BLEND_VGLITE | TN_BLEND_SRC_OUT;
    case VG_LITE_BLEND_DST_OUT: /// D - (Sa * D) , Non-premultiplied
        return TN_BLEND_VGLITE | TN_BLEND_DST_OUT;
    case VG_LITE_BLEND_SPECIAL:
        return TN_BLEND_SPECIAL;
    default:
        tnABORT(TN_FAULT_UNSUPPORTED, "Unsupported VGLite blending mode");
    }

#if defined(__GNUC__)
    #pragma GCC diagnostic pop
#elif defined(__IAR_SYSTEMS_ICC__)
    #pragma diag_default=Pe047
#elif defined(__ARMCC_VERSION) && (__ARMCC_VERSION < 6000000)
    #pragma diag_default 1296
#endif

    return TN_BLEND_SRC;
}



tn_u8
vglite_blend_to_attr_blend(vg_lite_blend_t blend)
{
    switch (blend)
    {
    case VG_LITE_BLEND_NONE:
        return 0;
    case OPENVG_BLEND_SRC:
        return 0;
    case VG_LITE_BLEND_SRC_OVER:
    case OPENVG_BLEND_SRC_OVER:
        return 2;
    case VG_LITE_BLEND_DST_OVER:
    case OPENVG_BLEND_DST_OVER:
        return 3;
    case VG_LITE_BLEND_SRC_IN:
    case OPENVG_BLEND_SRC_IN:
        return 4;
    case VG_LITE_BLEND_DST_IN:
    case OPENVG_BLEND_DST_IN:
        return 5;
    case VG_LITE_BLEND_SRC_OUT:
        return 6;
    case VG_LITE_BLEND_DST_OUT:
        return 7;
    /* TODO
    SRC_ATOP=8
    DST_ATOP=9
    XOR=10
    */
    case VG_LITE_BLEND_DARKEN:
    case OPENVG_BLEND_DARKEN:
        return 11;
    case VG_LITE_BLEND_LIGHTEN:
    case OPENVG_BLEND_LIGHTEN:
        return 12;
    case VG_LITE_BLEND_MULTIPLY:
    case OPENVG_BLEND_MULTIPLY:
        return 13;
    case VG_LITE_BLEND_SCREEN:
    case OPENVG_BLEND_SCREEN:
        return 14;
    case VG_LITE_BLEND_ADDITIVE:
    case OPENVG_BLEND_ADDITIVE:
        return 15;
    default:
        tnABORT(TN_FAULT_UNSUPPORTED, "Unsupported VGLite blending mode");
    }
}



tn_u8
vglite_quality_to_vg_quality(vg_lite_quality_t quality)
{
    switch (quality)
    {
    case VG_LITE_HIGH:       /*! High quality 16x anti-aliasing path. */
        return 3;
    case VG_LITE_UPPER:      /*! Upper quality 8x anti-aliasing path. */
        return 2;
    case VG_LITE_MEDIUM:     /*! Medium quality 4x anti-aliasing path. */
        return 1;
    case VG_LITE_LOW:        /*! Low quality path without any anti-aliasing. */
        return 0;
    default:
        tnABORT(TN_FAULT_UNSUPPORTED, "Unsupported VGLite path quality");
    }
}



tn_i32
vglite_mask_op_to_tn_blend(vg_lite_mask_operation_t op)
{
    switch (op)
    {
    case VG_LITE_SET_MASK:
        return TN_BLEND_SRC;
    case VG_LITE_UNION_MASK:
        return TN_BLEND_SCREEN;
    case VG_LITE_INTERSECT_MASK:
        return TN_BLEND_SRC_IN;
    case VG_LITE_SUBTRACT_MASK:
        return TN_BLEND_DST_OUT;
    default:
        tnABORT(TN_FAULT_UNSUPPORTED, "Unsupported VGLite mask operation");
    }
}



tn_i32
vglite_image_mode_to_tn_image_mode(vg_lite_image_mode_t mode)
{
    switch (mode)
    {
    case VG_LITE_NORMAL_IMAGE_MODE:
        return TN_DRAW_IMAGE_NORMAL;
    case VG_LITE_NONE_IMAGE_MODE:
        tnABORT(TN_FAULT_UNSUPPORTED, "VG_LITE_NONE_IMAGE_MODE not supported yet");
    case VG_LITE_MULTIPLY_IMAGE_MODE:
        return TN_DRAW_IMAGE_MULTIPLY;
    case VG_LITE_STENCIL_MODE:
        return TN_DRAW_IMAGE_STENCIL;
    default:
        return TN_DRAW_IMAGE_NORMAL;
    }
}



tn_i32
vglite_yuv2rgb_to_tn(vg_lite_yuv2rgb_t yuv2rgb)
{
    switch ((tn_u32)yuv2rgb)
    {
    case VG_LITE_YUV601:
        return TN_YUV601;
    case VG_LITE_YUV709:
        return TN_YUV709;
    case VG_LITE_YUV470:
        return TN_YUV470;
    default:
        tnABORT(TN_FAULT_UNSUPPORTED, "Unsupported VGLite YUV->RGB conversion standard");
    }
}



static tn_result
vglite_buffer_layout(
    vg_lite_buffer_t *buffer,
    tn_color_format  *color_format_out,
    tn_size          *buffer_size_out,
    tn_u32           *uv_offset_out,
    tn_u32           *v_offset_out)
{
    tnIN();

    tnEXPECT(!tn_is_null(buffer));
    tnEXPECT(!tn_is_null(color_format_out));
    tnEXPECT(!tn_is_null(buffer_size_out));
    tnEXPECT(!tn_is_null(uv_offset_out));
    tnEXPECT(!tn_is_null(v_offset_out));

    tn_size buffer_size = 0, y_size = 0, u_size = 0, v_size = 0, a_size = 0;
    tn_color_format color_format = vglite_color_2_tn_color(buffer->format);
    tnRET_IF(color_format == TN_MAX_COLOR_FORMAT, TN_UNSUPPORTED_IMAGE_FORMAT_ERROR);

    tn_color colorI;
    tn_color_copy(color_format, &colorI);

    if (buffer->stride == 0)
    {
        int extended_width = buffer->width;
        if (tn_fmt_is_ETC2_compressed(color_format))
        {
            // ETC2 uses 4x4 blocks choose next multiple of 4 to calculate stride
            extended_width = ((buffer->width + 3) / 4) * 4;
        }
        else if (tn_fmt_is_IBC32x1_compressed(color_format))
        {
            // IBC32x1 uses 32x1 blocks choose next multiple of 32 to calculate stride
            extended_width = ((buffer->width + 31) / 32) * 32;
        }
        else if (tn_fmt_is_IBC8x8_compressed(color_format))
        {
            // IBC8x8 uses 8x8 blocks choose next multiple of 8 to calculate stride
            extended_width = ((buffer->width + 7) / 8) * 8;
        }

        buffer->stride = tnALIGN((extended_width * colorI.bpp), 8) / 8;
    }

    if (tn_fmt_is_yuv(color_format))
    {
        if (buffer->yuv.swizzle == VG_LITE_SWIZZLE_VU)
        {
            color_format = tn_fmt_get_YV(color_format);
        }
        else     // (buffer->yuv.swizzle == VG_LITE_SWIZZLE_UV)
        {
            color_format = tn_fmt_get_YU(color_format);
        }
        setup_yuv_struct(buffer, color_format, &y_size, &u_size, &v_size, &a_size);
        buffer_size = y_size + u_size + v_size + a_size;
    }
    else
    {
        buffer_size = buffer->stride * buffer->height;
    }

    *color_format_out = color_format;
    *buffer_size_out  = buffer_size;
    *uv_offset_out    = (tn_u32)((u_size != 0) ? y_size : 0);
    *v_offset_out     = (tn_u32)((v_size != 0) ? (y_size + u_size) : 0);

    tnOUT();
    return TN_NO_ERROR;
}



static tn_result
vglite_buffer_gbuf_layout(
    vg_lite_buffer_t *buffer,
    tn_gbuf_layout   *layout)
{
    tnIN();

    tn_size buffer_size = 0;
    tn_color_format color_format = 0;
    tn_u32 uv_offset = 0, v_offset = 0;

    tn_result res = vglite_buffer_layout(buffer,
                                         &color_format,
                                         &buffer_size,
                                         &uv_offset,
                                         &v_offset);
    tnRET_IF(res != TN_NO_ERROR, res);

    layout->width        = (tn_map_coord)buffer->width;
    layout->height       = (tn_map_coord)buffer->height;
    layout->stride       = (tn_map_coord)buffer->stride;
    layout->color_format = color_format;
    layout->yuv2rgb      = vglite_yuv2rgb_to_tn(buffer->yuv.yuv2rgb);
    layout->size         = buffer_size;
    layout->uv_offset    = uv_offset;
    layout->v_offset     = v_offset;

    tnOUT();
    return TN_NO_ERROR;
}



tn_result
vglite_buffer_gbuf_create(
    tn_gbuf          *gbuf,
    vg_lite_buffer_t *buffer)
{
    tnIN();

    tnEXPECT(!tn_is_null(buffer));
    tnEXPECT(!tn_is_null(gbuf));

    tn_gbuf_layout layout;
    tn_result res = vglite_buffer_gbuf_layout(buffer, &layout);
    tnRET_IF(res != TN_NO_ERROR, res);
    res = tn_gbuf_create(gbuf, &layout);

    tnOUT();
    return res;
}



// ---------------------------------------------------------------------------
// Source-buffer alignment checks (VG_LITE_SRC_BUF_ALIGNED_CHECK)
//
// Each hardware constraint is gated at runtime with
// tn_ctx_api_has_feature(gcFEATURE_BIT_VG_*), so a constraint only
// applies when the running GPU generation advertises it. All helpers return
// VG_LITE_SUCCESS when satisfied and VG_LITE_INVALID_ARGUMENT otherwise.
// ---------------------------------------------------------------------------

// Bytes-per-pixel numerator/denominator for a format: bpp = 8 * mul / div.
static void
tin_get_format_bytes(
    vg_lite_buffer_format_t format,
    tn_u32                 *mul,
    tn_u32                 *div)
{
    tnIN();

    *mul = 1;
    *div = 1;

    switch ((tn_u32)format)
    {
    case VG_LITE_A1:
    case VG_LITE_INDEX_1:
        *div = 8;
        break;
    case VG_LITE_A2:
    case VG_LITE_INDEX_2:
        *div = 4;
        break;
    case VG_LITE_L4:
    case VG_LITE_A4:
    case VG_LITE_INDEX_4:
    case VG_LITE_RGB888_ETC2_EAC:
        *div = 2;
        break;
    case VG_LITE_L8:
    case VG_LITE_A8:
    case VG_LITE_INDEX_8:
    case VG_LITE_RGBA8888_ETC2_EAC:
    case VG_LITE_NV12:
    case VG_LITE_NV12_TILED:
    case VG_LITE_RGBA2222:
    case VG_LITE_BGRA2222:
    case VG_LITE_ABGR2222:
    case VG_LITE_ARGB2222:
        *mul = 1;
        break;
    case VG_LITE_ABGR1555:
    case VG_LITE_ARGB1555:
    case VG_LITE_BGRA5551:
    case VG_LITE_RGBA5551:
    case VG_LITE_RGBA4444:
    case VG_LITE_BGRA4444:
    case VG_LITE_ABGR4444:
    case VG_LITE_ARGB4444:
    case VG_LITE_RGB565:
    case VG_LITE_BGR565:
    case VG_LITE_YUYV:
    case VG_LITE_YUY2:
    case VG_LITE_YUY2_TILED:
    case VG_LITE_AYUY2:
    case VG_LITE_AYUY2_TILED:
    case VG_LITE_ABGR8565_PLANAR:
    case VG_LITE_ARGB8565_PLANAR:
    case VG_LITE_RGBA5658_PLANAR:
    case VG_LITE_BGRA5658_PLANAR:
    case VG_LITE_A8L8:
        *mul = 2;
        break;
    case VG_LITE_RGB888:
    case VG_LITE_BGR888:
    case VG_LITE_ABGR8565:
    case VG_LITE_BGRA5658:
    case VG_LITE_ARGB8565:
    case VG_LITE_RGBA5658:
        *mul = 3;
        break;
    case VG_LITE_RGBA8888:
    case VG_LITE_BGRA8888:
    case VG_LITE_ABGR8888:
    case VG_LITE_ARGB8888:
    case VG_LITE_RGBX8888:
    case VG_LITE_BGRX8888:
    case VG_LITE_XBGR8888:
    case VG_LITE_XRGB8888:
    case VG_LITE_ANV12:
    case VG_LITE_ANV12_TILED:
        *mul = 4;
        break;
    default:
        // Leave 1/1 for formats not covered by the reference table.
        break;
    }

    tnOUT();
}



// Primary format support allow-list.
static vg_lite_error_t
tin_check_format_support_1(vg_lite_buffer_format_t format)
{
    tnIN();

    switch ((tn_u32)format)
    {
    case VG_LITE_A8:
    case VG_LITE_L8:
    case VG_LITE_RGBA2222:
    case VG_LITE_BGRA2222:
    case VG_LITE_ABGR2222:
    case VG_LITE_ARGB2222:
    case VG_LITE_RGBA4444:
    case VG_LITE_BGRA4444:
    case VG_LITE_ABGR4444:
    case VG_LITE_ARGB4444:
    case VG_LITE_BGRA5551:
    case VG_LITE_RGBA5551:
    case VG_LITE_ABGR1555:
    case VG_LITE_ARGB1555:
    case VG_LITE_RGB565:
    case VG_LITE_BGR565:
    case VG_LITE_RGB888:
    case VG_LITE_BGR888:
    case VG_LITE_ABGR8565:
    case VG_LITE_BGRA5658:
    case VG_LITE_ARGB8565:
    case VG_LITE_RGBA5658:
    case VG_LITE_RGBA8888:
    case VG_LITE_BGRA8888:
    case VG_LITE_ABGR8888:
    case VG_LITE_ARGB8888:
    case VG_LITE_RGBX8888:
    case VG_LITE_BGRX8888:
    case VG_LITE_XBGR8888:
    case VG_LITE_XRGB8888:
        tnOUT();
        return VG_LITE_SUCCESS;
    default:
        tnOUT();
        return VG_LITE_NOT_SUPPORT;
    }
}



// Secondary format support allow-list.
static vg_lite_error_t
tin_check_format_support_2(vg_lite_buffer_format_t format)
{
    tnIN();

    switch ((tn_u32)format)
    {
    case VG_LITE_INDEX_1:
    case VG_LITE_INDEX_2:
    case VG_LITE_INDEX_4:
    case VG_LITE_INDEX_8:
    case VG_LITE_A4:
    case VG_LITE_YUY2:
    case VG_LITE_YUY2_TILED:
    case VG_LITE_RGBA8888_ETC2_EAC:
        tnOUT();
        return VG_LITE_SUCCESS;
    default:
        tnOUT();
        return VG_LITE_NOT_SUPPORT;
    }
}



// Per-format source stride alignment table (gated by VG_16PIXELS_ALIGN).
// Requires stride to be a multiple of the byte count listed for the format.
vg_lite_error_t
vglite_source_stride_align_check(
    vg_lite_buffer_format_t format,
    tn_u32                  stride)
{
    tnIN();

    tn_u32 align;

    switch ((tn_u32)format)
    {
    case VG_LITE_A4:
    case VG_LITE_INDEX_1:
    case VG_LITE_INDEX_2:
    case VG_LITE_INDEX_4:
        align = 8;
        break;
    case VG_LITE_L8:
    case VG_LITE_A8:
    case VG_LITE_INDEX_8:
    case VG_LITE_RGBA2222:
    case VG_LITE_BGRA2222:
    case VG_LITE_ABGR2222:
    case VG_LITE_ARGB2222:
    case VG_LITE_RGBA8888_ETC2_EAC:
        align = 16;
        break;
    case VG_LITE_RGBA4444:
    case VG_LITE_BGRA4444:
    case VG_LITE_ABGR4444:
    case VG_LITE_ARGB4444:
    case VG_LITE_RGB565:
    case VG_LITE_BGR565:
    case VG_LITE_BGRA5551:
    case VG_LITE_RGBA5551:
    case VG_LITE_ABGR1555:
    case VG_LITE_ARGB1555:
    case VG_LITE_YUYV:
    case VG_LITE_YUY2:
    case VG_LITE_NV12:
    case VG_LITE_YV12:
    case VG_LITE_YV24:
    case VG_LITE_YV16:
    case VG_LITE_NV16:
    case VG_LITE_NV24:
    case VG_LITE_ABGR8565_PLANAR:
    case VG_LITE_BGRA5658_PLANAR:
    case VG_LITE_ARGB8565_PLANAR:
    case VG_LITE_RGBA5658_PLANAR:
        align = 32;
        break;
    case VG_LITE_RGB888:
    case VG_LITE_BGR888:
    case VG_LITE_ABGR8565:
    case VG_LITE_BGRA5658:
    case VG_LITE_ARGB8565:
    case VG_LITE_RGBA5658:
        align = 48;
        break;
    case VG_LITE_RGBA8888:
    case VG_LITE_BGRA8888:
    case VG_LITE_ABGR8888:
    case VG_LITE_ARGB8888:
    case VG_LITE_RGBX8888:
    case VG_LITE_BGRX8888:
    case VG_LITE_XBGR8888:
    case VG_LITE_XRGB8888:
        align = 64;
        break;
    default:
        tnOUT();
        return VG_LITE_SUCCESS;
    }

    tnOUT();
    return (stride % align) ? VG_LITE_INVALID_ARGUMENT : VG_LITE_SUCCESS;
}



// Linear source stride alignment table (gated by VG_SRC_BUF_ALINGED for linear buffers).
static vg_lite_error_t
vglite_source_stride_align_check_2(
    vg_lite_buffer_format_t format,
    tn_u32                  stride)
{
    tnIN();

    tn_u32 align;

    switch ((tn_u32)format)
    {
    case VG_LITE_A1:
    case VG_LITE_A2:
    case VG_LITE_A4:
    case VG_LITE_A8:
    case VG_LITE_L4:
    case VG_LITE_L8:
    case VG_LITE_INDEX_1:
    case VG_LITE_INDEX_2:
    case VG_LITE_INDEX_4:
    case VG_LITE_INDEX_8:
    case VG_LITE_RGBA2222:
    case VG_LITE_BGRA2222:
    case VG_LITE_ABGR2222:
    case VG_LITE_ARGB2222:
    case VG_LITE_RGBA8888_ETC2_EAC:
        align = 1;
        break;
    case VG_LITE_A8L8:
    case VG_LITE_RGBA4444:
    case VG_LITE_BGRA4444:
    case VG_LITE_ABGR4444:
    case VG_LITE_ARGB4444:
    case VG_LITE_RGB565:
    case VG_LITE_BGR565:
    case VG_LITE_BGRA5551:
    case VG_LITE_RGBA5551:
    case VG_LITE_ABGR1555:
    case VG_LITE_ARGB1555:
    case VG_LITE_YV16:
    case VG_LITE_NV16:
    case VG_LITE_YV12:
    case VG_LITE_NV12:
    case VG_LITE_YV24:
    case VG_LITE_ABGR8565_PLANAR:
    case VG_LITE_BGRA5658_PLANAR:
    case VG_LITE_ARGB8565_PLANAR:
    case VG_LITE_RGBA5658_PLANAR:
        align = 2;
        break;
    case VG_LITE_RGB888:
    case VG_LITE_BGR888:
    case VG_LITE_ABGR8565:
    case VG_LITE_BGRA5658:
    case VG_LITE_ARGB8565:
    case VG_LITE_RGBA5658:
        align = 3;
        break;
    case VG_LITE_YUY2:
    case VG_LITE_RGBA8888:
    case VG_LITE_BGRA8888:
    case VG_LITE_ABGR8888:
    case VG_LITE_ARGB8888:
    case VG_LITE_RGBX8888:
    case VG_LITE_BGRX8888:
    case VG_LITE_XBGR8888:
    case VG_LITE_XRGB8888:
        align = 4;
        break;
    default:
        tnOUT();
        return VG_LITE_SUCCESS;
    }

    tnOUT();
    return (stride % align) ? VG_LITE_INVALID_ARGUMENT : VG_LITE_SUCCESS;
}



// Tiled source stride alignment table (gated by VG_SRC_BUF_ALINGED for tiled buffers).
static vg_lite_error_t
vglite_source_stride_align_check_3(
    vg_lite_buffer_format_t format,
    tn_u32                  stride)
{
    tnIN();

    tn_u32 align;

    switch ((tn_u32)format)
    {
    case VG_LITE_A1:
    case VG_LITE_A2:
    case VG_LITE_INDEX_1:
    case VG_LITE_INDEX_2:
        align = 1;
        break;
    case VG_LITE_A4:
    case VG_LITE_L4:
    case VG_LITE_INDEX_4:
    case VG_LITE_RGB888_ETC2_EAC:
        align = 2;
        break;
    case VG_LITE_A8:
    case VG_LITE_L8:
    case VG_LITE_YV24:
    case VG_LITE_INDEX_8:
    case VG_LITE_RGBA2222:
    case VG_LITE_BGRA2222:
    case VG_LITE_ABGR2222:
    case VG_LITE_ARGB2222:
    case VG_LITE_RGBA8888_ETC2_EAC:
        align = 4;
        break;
    case VG_LITE_A8L8:
    case VG_LITE_RGBA4444:
    case VG_LITE_BGRA4444:
    case VG_LITE_ABGR4444:
    case VG_LITE_ARGB4444:
    case VG_LITE_BGRA5551:
    case VG_LITE_RGBA5551:
    case VG_LITE_ABGR1555:
    case VG_LITE_ARGB1555:
    case VG_LITE_RGB565:
    case VG_LITE_BGR565:
    case VG_LITE_YUY2_TILED:
    case VG_LITE_NV12_TILED:
    case VG_LITE_ABGR8565_PLANAR:
    case VG_LITE_BGRA5658_PLANAR:
    case VG_LITE_ARGB8565_PLANAR:
    case VG_LITE_RGBA5658_PLANAR:
        align = 8;
        break;
    case VG_LITE_RGB888:
    case VG_LITE_BGR888:
    case VG_LITE_ABGR8565:
    case VG_LITE_BGRA5658:
    case VG_LITE_ARGB8565:
    case VG_LITE_RGBA5658:
        align = 12;
        break;
    case VG_LITE_RGBA8888:
    case VG_LITE_BGRA8888:
    case VG_LITE_ABGR8888:
    case VG_LITE_ARGB8888:
    case VG_LITE_RGBX8888:
    case VG_LITE_BGRX8888:
    case VG_LITE_XBGR8888:
    case VG_LITE_XRGB8888:
        align = 16;
        break;
    default:
        tnOUT();
        return VG_LITE_SUCCESS;
    }

    tnOUT();
    return (stride % align) ? VG_LITE_INVALID_ARGUMENT : VG_LITE_SUCCESS;
}



// Aggregate source-buffer alignment check.
// Every constraint is gated on the runtime feature table; with all alignment
// bits disabled this reduces to the format-support check only.
vg_lite_error_t
vglite_srcbuf_align_check(vg_lite_buffer_t *source)
{
    tnIN();

    tn_u32 mul, div, bpp;
    tin_get_format_bytes(source->format, &mul, &div);
    bpp = 8 * mul / div;

    if (tn_ctx_api_has_feature(gcFEATURE_BIT_VG_FORMAT_SUPPORT_CHECK))
    {
        if (tin_check_format_support_1(source->format) != VG_LITE_SUCCESS
            && tin_check_format_support_2(source->format) != VG_LITE_SUCCESS)
        {
            tnOUT();
            return VG_LITE_INVALID_ARGUMENT;
        }
    }

    if (tn_ctx_api_has_feature(gcFEATURE_BIT_VG_SRC_ADDRESS_64BYTES_ALIGNED))
    {
        if ((tn_u32)source->address % 64 != 0)
        {
            tnOUT();
            return VG_LITE_INVALID_ARGUMENT;
        }
    }

    if (tn_ctx_api_has_feature(gcFEATURE_BIT_VG_SRC_ADDRESS_DETAIL_ALIGNED))
    {
        if (bpp == 8)
        {
            if ((tn_u32)source->address % 16 != 0)
            {
                tnOUT();
                return VG_LITE_INVALID_ARGUMENT;
            }
        }
        else if (bpp == 16)
        {
            if ((tn_u32)source->address % 32 != 0)
            {
                tnOUT();
                return VG_LITE_INVALID_ARGUMENT;
            }
        }
        else if (bpp == 24 || bpp == 32)
        {
            if ((tn_u32)source->address % 64 != 0)
            {
                tnOUT();
                return VG_LITE_INVALID_ARGUMENT;
            }
        }
        else
        {
            if ((tn_u32)source->address % 8 != 0)
            {
                tnOUT();
                return VG_LITE_INVALID_ARGUMENT;
            }
        }
    }

    if (tn_ctx_api_has_feature(gcFEATURE_BIT_VG_SRC_ADDRESS_DETAIL_ALIGNED_1))
    {
        if (bpp == 32)
        {
            if ((tn_u32)source->address % 16 != 0)
            {
                tnOUT();
                return VG_LITE_INVALID_ARGUMENT;
            }
        }
        else
        {
            if ((tn_u32)source->address % 8 != 0)
            {
                tnOUT();
                return VG_LITE_INVALID_ARGUMENT;
            }
        }
    }

    if (tn_ctx_api_has_feature(gcFEATURE_BIT_VG_SRC_BUF_ALINGED))
    {
        if (tn_ctx_api_has_feature(gcFEATURE_BIT_VG_SRC_ADDRESS_16BYTES_ALIGNED)
            && (source->format == VG_LITE_ARGB8888
                || source->format == VG_LITE_BGRA8888
                || source->format == VG_LITE_ABGR8888))
        {
            if ((tn_u32)source->address % 16 != 0)
            {
                tnOUT();
                return VG_LITE_INVALID_ARGUMENT;
            }
        }
        else
        {
            if ((tn_u32)source->address % 8 != 0)
            {
                tnOUT();
                return VG_LITE_INVALID_ARGUMENT;
            }
        }
    }

    if (source->tiled == VG_LITE_TILED)
    {
        if (tn_ctx_api_has_feature(gcFEATURE_BIT_VG_SRC_TILE_4PIXELS_ALIGNED))
        {
            if ((source->stride % (4 * mul / div) != 0) || (source->height % 4 != 0))
            {
                tnOUT();
                return VG_LITE_INVALID_ARGUMENT;
            }
        }

        if (tn_ctx_api_has_feature(gcFEATURE_BIT_VG_SRC_BUF_ALINGED))
        {
            if (vglite_source_stride_align_check_3(source->format, source->stride) != VG_LITE_SUCCESS)
            {
                tnOUT();
                return VG_LITE_INVALID_ARGUMENT;
            }
        }

        if (tn_ctx_api_has_feature(gcFEATURE_BIT_VG_YUV_ALIGNED_CHECK)
            && source->format == VG_LITE_YUY2)
        {
            if (!tn_ctx_api_has_feature(gcFEATURE_BIT_VG_SRC_ADDRESS_DETAIL_ALIGNED_1))
            {
                if (source->address % 32 != 0)
                {
                    tnOUT();
                    return VG_LITE_INVALID_ARGUMENT;
                }
            }
            // tiled path: stride multiple of 8
            if (source->stride % 8 != 0)
            {
                tnOUT();
                return VG_LITE_INVALID_ARGUMENT;
            }
        }
        // Note: the reference also has YUV_DETAIL_ALIGNED_CHECK sub-checks here,
        // gated on a feature bit that does not exist in tin, so they are omitted.
    }

    if (source->tiled == VG_LITE_LINEAR)
    {
        if (tn_ctx_api_has_feature(gcFEATURE_BIT_VG_16PIXELS_ALIGN))
        {
            if (source->stride % (16 * mul / div) != 0)
            {
                tnOUT();
                return VG_LITE_INVALID_ARGUMENT;
            }
        }

        if (tn_ctx_api_has_feature(gcFEATURE_BIT_VG_SRC_BUF_ALINGED))
        {
            if (vglite_source_stride_align_check_2(source->format, source->stride) != VG_LITE_SUCCESS)
            {
                tnOUT();
                return VG_LITE_INVALID_ARGUMENT;
            }
        }

        if (tn_ctx_api_has_feature(gcFEATURE_BIT_VG_YUV_ALIGNED_CHECK)
            && !tn_ctx_api_has_feature(gcFEATURE_BIT_VG_SRC_ADDRESS_DETAIL_ALIGNED_1))
        {
            if (source->format == VG_LITE_NV12 || source->format == VG_LITE_NV16)
            {
                if (source->address % 32 != 0 || source->stride % 32 != 0)
                {
                    tnOUT();
                    return VG_LITE_INVALID_ARGUMENT;
                }
                if (source->yuv.uv_planar % 32 != 0 || source->yuv.uv_stride % 32 != 0)
                {
                    tnOUT();
                    return VG_LITE_INVALID_ARGUMENT;
                }
            }
            if (source->format == VG_LITE_YV12 || source->format == VG_LITE_YV16)
            {
                if (source->address % 32 != 0 || source->stride % 32 != 0)
                {
                    tnOUT();
                    return VG_LITE_INVALID_ARGUMENT;
                }
                if (source->yuv.uv_planar % 16 != 0 || source->yuv.uv_stride % 16 != 0)
                {
                    tnOUT();
                    return VG_LITE_INVALID_ARGUMENT;
                }
                if (source->yuv.v_planar % 16 != 0 || source->yuv.v_stride % 16 != 0)
                {
                    tnOUT();
                    return VG_LITE_INVALID_ARGUMENT;
                }
            }
            if (source->format == VG_LITE_YV24)
            {
                if (source->address % 32 != 0 || source->stride % 32 != 0)
                {
                    tnOUT();
                    return VG_LITE_INVALID_ARGUMENT;
                }
                if (source->yuv.uv_planar % 32 != 0 || source->yuv.uv_stride % 32 != 0)
                {
                    tnOUT();
                    return VG_LITE_INVALID_ARGUMENT;
                }
                if (source->yuv.v_planar % 32 != 0 || source->yuv.v_stride % 32 != 0)
                {
                    tnOUT();
                    return VG_LITE_INVALID_ARGUMENT;
                }
            }
        }
        // Note: the reference also has YUV_DETAIL_ALIGNED_CHECK and
        // YUV_HEIGHT_ALIGNED_CHECK sub-checks here, gated on feature bits that
        // do not exist in tin, so they are omitted.
    }

    // The reference validates DEC-compressed stride alignment here when
    // compress_mode != VG_LITE_DEC_DISABLE. FlexGPU does not support DEC
    // compression (compress features are disabled and compress_mode defaults to
    // VG_LITE_DEC_DISABLE), so there is nothing to check.

    tnOUT();
    return VG_LITE_SUCCESS;
}



// ---------------------------------------------------------------------------
// Destination-buffer alignment checks (VG_LITE_DST_BUF_ALIGNED_CHECK)
//
// Every hardware constraint is gated at runtime through tn_ctx_api_has_feature() calls.
// Both return VG_LITE_SUCCESS when satisfied and VG_LITE_INVALID_ARGUMENT otherwise.
// ---------------------------------------------------------------------------

// Aggregate destination-buffer alignment check.
vg_lite_error_t
vglite_dstbuf_align_check(vg_lite_buffer_t *target)
{
    tnIN();

    tn_u32 mul, div, bpp;
    tin_get_format_bytes(target->format, &mul, &div);
    bpp = 8 * mul / div;

    if (tn_ctx_api_has_feature(gcFEATURE_BIT_VG_FORMAT_SUPPORT_CHECK))
    {
        if (tin_check_format_support_1(target->format) != VG_LITE_SUCCESS)
        {
            tnOUT();
            return VG_LITE_INVALID_ARGUMENT;
        }
    }

    if (tn_ctx_api_has_feature(gcFEATURE_BIT_VG_DST_TILE_4PIXELS_ALIGNED))
    {
        if (target->tiled == VG_LITE_TILED)
        {
            if ((target->stride % (4 * mul / div) != 0) || (target->height % 4 != 0))
            {
                tnOUT();
                return VG_LITE_INVALID_ARGUMENT;
            }
        }
    }

    if (target->compress_mode == VG_LITE_DEC_DISABLE)
    {
        if (tn_ctx_api_has_feature(gcFEATURE_BIT_VG_DST_BUF_ALIGNED))
        {
            if (target->tiled == VG_LITE_TILED)
            {
                if (bpp == 8 || bpp == 16 || bpp == 32)
                {
                    if (target->stride % (4 * mul / div))
                    {
                        tnOUT();
                        return VG_LITE_INVALID_ARGUMENT;
                    }
                }
                if (target->format >= VG_LITE_RGB888 && target->format <= VG_LITE_RGBA5658)
                {
                    if (target->stride % 12 != 0)
                    {
                        tnOUT();
                        return VG_LITE_INVALID_ARGUMENT;
                    }
                }
                if (target->format >= VG_LITE_ABGR8565_PLANAR && target->format <= VG_LITE_RGBA5658_PLANAR)
                {
                    if (target->stride % 8 != 0)
                    {
                        tnOUT();
                        return VG_LITE_INVALID_ARGUMENT;
                    }
                }
            }
            else
            {
                if (bpp == 8 || bpp == 16 || bpp == 32)
                {
                    if (target->stride % (mul / div))
                    {
                        tnOUT();
                        return VG_LITE_INVALID_ARGUMENT;
                    }
                }
                if (target->format >= VG_LITE_RGB888 && target->format <= VG_LITE_RGBA5658)
                {
                    if (target->stride % 3 != 0)
                    {
                        tnOUT();
                        return VG_LITE_INVALID_ARGUMENT;
                    }
                }
                if (target->format >= VG_LITE_ABGR8565_PLANAR && target->format <= VG_LITE_RGBA5658_PLANAR)
                {
                    if (target->stride % 2 != 0)
                    {
                        tnOUT();
                        return VG_LITE_INVALID_ARGUMENT;
                    }
                }
            }

            if (bpp == 8 || bpp == 16 || bpp == 32)
            {
                if ((tn_u32)target->address % 4 != 0)
                {
                    tnOUT();
                    return VG_LITE_INVALID_ARGUMENT;
                }
            }
            if (target->format >= VG_LITE_RGB888 && target->format <= VG_LITE_RGBA5658)
            {
                if ((tn_u32)target->address % 64 != 0)
                {
                    tnOUT();
                    return VG_LITE_INVALID_ARGUMENT;
                }
            }
        }

        if (tn_ctx_api_has_feature(gcFEATURE_BIT_VG_DST_ADDRESS_64BYTES_ALIGNED))
        {
            if ((tn_u32)target->address % 64 != 0)
            {
                tnOUT();
                return VG_LITE_INVALID_ARGUMENT;
            }
        }

        if (tn_ctx_api_has_feature(gcFEATURE_BIT_VG_DST_24BIT_PLANAR_ALIGNED))
        {
            if (target->format >= VG_LITE_ABGR8565_PLANAR && target->format <= VG_LITE_RGBA5658_PLANAR)
            {
                if ((tn_u32)target->address % 32 != 0)
                {
                    tnOUT();
                    return VG_LITE_INVALID_ARGUMENT;
                }
                if ((tn_u32)target->yuv.alpha_planar % 16 != 0)
                {
                    tnOUT();
                    return VG_LITE_INVALID_ARGUMENT;
                }
            }
        }
        // Note: the reference also has a DST_24BIT_PLANAR_ALIGNED_1 block here,
        // gated on a feature bit that does not exist in tin, so it is omitted.
    }
    else
    {
        // DEC-compressed destinations require compress-specific address/stride
        // alignment in the reference. FlexGPU does not support DEC compression
        // (compress features are disabled and compress_mode defaults to
        // VG_LITE_DEC_DISABLE), so there is nothing to check here.
    }

    if (tn_ctx_api_has_feature(gcFEATURE_BIT_VG_DST_ADDRESS_DETAIL_ALIGNED))
    {
        if (bpp == 24 || target->tiled == VG_LITE_TILED)
        {
            if ((tn_u32)target->address % 64 != 0)
            {
                tnOUT();
                return VG_LITE_INVALID_ARGUMENT;
            }
        }
        else
        {
            if ((tn_u32)target->address % 4 != 0)
            {
                tnOUT();
                return VG_LITE_INVALID_ARGUMENT;
            }
        }

        if (target->tiled == VG_LITE_TILED)
        {
            if (target->height % 4 != 0)
            {
                tnOUT();
                return VG_LITE_INVALID_ARGUMENT;
            }
            if (bpp == 24)
            {
                if ((tn_u32)target->stride % 12 != 0)
                {
                    tnOUT();
                    return VG_LITE_INVALID_ARGUMENT;
                }
            }
            else
            {
                if ((tn_u32)target->stride % 16 != 0)
                {
                    tnOUT();
                    return VG_LITE_INVALID_ARGUMENT;
                }
            }
        }
        else
        {
            if ((tn_u32)target->stride % (bpp / 8) != 0)
            {
                tnOUT();
                return VG_LITE_INVALID_ARGUMENT;
            }
        }
    }

    if (tn_ctx_api_has_feature(gcFEATURE_BIT_VG_TILED_LIMIT))
    {
        // The reference supports several TILED_LIMIT variants selected by the
        // macro's integer value; tin exposes it as a boolean feature bit, so the
        // most common variant (== 1) is implemented here. tile_flag reflects a
        // tiled target; tile_flag1 additionally requires RECTANGLE_TILED_OUT.
        tn_u32 tile_flag = 0;
        tn_u32 tile_flag1 = 0;
        if (target->tiled == VG_LITE_TILED)
        {
            tile_flag = 1;
            tile_flag1 = tn_ctx_api_has_feature(gcFEATURE_BIT_VG_RECTANGLE_TILED_OUT) ? 1 : 0;
        }

        if (tile_flag1 ^ tile_flag)
        {
            if (bpp != 24)
            {
                if (target->stride % 64 != 0)
                {
                    tnOUT();
                    return VG_LITE_INVALID_ARGUMENT;
                }
            }
            else
            {
                if (target->stride % 48 != 0)
                {
                    tnOUT();
                    return VG_LITE_INVALID_ARGUMENT;
                }
            }
        }
    }

    tnOUT();
    return VG_LITE_SUCCESS;
}



// PE-clear destination alignment check.
vg_lite_error_t
vglite_peclear_align_check(
    vg_lite_buffer_t *target,
    tn_i32            y)
{
    tnIN();

    tnEXPECT(!tn_is_null(target));

    tn_u32 mul, div;

    // Height must be positive, guard before casting y to unsigned.
    if (y <= 0)
    {
        tnOUT();
        return VG_LITE_INVALID_ARGUMENT;
    }

    tin_get_format_bytes(target->format, &mul, &div);

    if (tn_ctx_api_has_feature(gcFEATURE_BIT_VG_DST_ADDRESS_DETAIL_ALIGNED))
    {
        if ((tn_u32)target->address % 64 != 0)
        {
            tnOUT();
            return VG_LITE_INVALID_ARGUMENT;
        }
    }

    if (target->compress_mode == VG_LITE_DEC_DISABLE)
    {
        if (tn_ctx_api_has_feature(gcFEATURE_BIT_VG_DST_BUFLEN_ALIGNED))
        {
            if ((mul / div != 3) && (((tn_u32)target->stride * (tn_u32)y) % 64 != 0))
            {
                tnOUT();
                return VG_LITE_INVALID_ARGUMENT;
            }
        }
        if (tn_ctx_api_has_feature(gcFEATURE_BIT_VG_24BIT))
        {
            if ((mul / div == 3) && (((tn_u32)target->stride * (tn_u32)y) % 48 != 0))
            {
                tnOUT();
                return VG_LITE_INVALID_ARGUMENT;
            }
        }
    }
    else
    {
        // DEC-compressed destination stride alignment is not applicable on
        // FlexGPU (DEC compression unsupported); nothing to check.
    }

    tnOUT();
    return VG_LITE_SUCCESS;
}



tn_result
vglite_buffer_gbuf_wrap(
    tn_gbuf          *gbuf,
    vg_lite_buffer_t *buffer,
    tn_vptr           user_vaddr,
    tn_size           user_len)
{
    tnIN();

    tnEXPECT(!tn_is_null(gbuf));
    tnEXPECT(!tn_is_null(buffer));
    tnEXPECT(user_vaddr != tnNULL);

    tn_gbuf_layout layout;
    tn_result res = vglite_buffer_gbuf_layout(buffer, &layout);
    tnRET_IF(res != TN_NO_ERROR, res);

    res = tn_gbuf_wrap(gbuf, &layout, user_vaddr, user_len);

    tnOUT();
    return res;
}
