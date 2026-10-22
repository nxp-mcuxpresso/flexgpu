//
// Copyright 2025-2026 NXP
//
// SPDX-License-Identifier: MIT
//

#pragma once

#include "vg_lite.h"

/* Tiling mode used by Image Filters. Match OpenVG enum VGTilingMode, the
 * same used for Patterns. */
#define vg_lite_tiling_mode_t vg_lite_pattern_mode_t

/* Features supported by custom platforms. */
vg_lite_error_t vg_lite_enable_dither_ext(
    vg_lite_uint8_t matrix_type,
    vg_lite_uint8_t order_scale_factor_r,
    vg_lite_uint8_t order_scale_factor_g,
    vg_lite_uint8_t order_scale_factor_b,
    vg_lite_uint8_t order_scale_factor_a,
    vg_lite_uint8_t order_scale_factor_bw);

vg_lite_error_t vg_lite_lookup_ext(
    vg_lite_buffer_t      *target,
    vg_lite_buffer_t      *source,
    const vg_lite_uint8_t *red_lut,
    const vg_lite_uint8_t *green_lut,
    const vg_lite_uint8_t *blue_lut,
    const vg_lite_uint8_t *alpha_lut,
    vg_lite_uint16_t       lut_gamma,
    vg_lite_uint16_t       lut_premultiply,
    vg_lite_uint16_t       gamma,
    vg_lite_uint16_t       premultiply,
    vg_lite_uint16_t       channel_mask);

vg_lite_error_t vg_lite_lookup_single_ext(
    vg_lite_buffer_t       *target,
    vg_lite_buffer_t       *source,
    const vg_lite_uint32_t *table,
    vg_lite_uint32_t        lut_channel,
    vg_lite_uint16_t        lut_gamma,
    vg_lite_uint16_t        lut_premultiply,
    vg_lite_uint16_t        gamma,
    vg_lite_uint16_t        premultiply,
    vg_lite_uint16_t        channel_mask);

vg_lite_error_t vg_lite_convolve_ext(
    vg_lite_buffer_t      *target,
    vg_lite_buffer_t      *source,
    vg_lite_uint32_t       k_width,
    vg_lite_uint32_t       k_height,
    vg_lite_int32_t        shift_x,
    vg_lite_int32_t        shift_y,
    const vg_lite_int16_t *kernel,
    vg_lite_float_t        scale,
    vg_lite_float_t        bias,
    vg_lite_tiling_mode_t  tile_mode,
    vg_lite_color_t        tile_color,
    vg_lite_uint16_t       gamma,
    vg_lite_uint16_t       premultiply,
    vg_lite_uint16_t       channel_mask);

vg_lite_error_t vg_lite_separable_convolve_ext(
    vg_lite_buffer_t      *target,
    vg_lite_buffer_t      *source,
    vg_lite_uint32_t       k_width,
    vg_lite_uint32_t       k_height,
    vg_lite_int32_t        shift_x,
    vg_lite_int32_t        shift_y,
    const vg_lite_int16_t *kernel_x,
    const vg_lite_int16_t *kernel_y,
    vg_lite_float_t        scale,
    vg_lite_float_t        bias,
    vg_lite_tiling_mode_t  tile_mode,
    vg_lite_color_t        tile_color,
    vg_lite_uint16_t       gamma,
    vg_lite_uint16_t       premultiply,
    vg_lite_uint16_t       channel_mask);

vg_lite_error_t vg_lite_gaussian_filter_ext(
    vg_lite_buffer_t     *target,
    vg_lite_buffer_t     *source,
    vg_lite_float_t       sx,
    vg_lite_float_t       sy,
    vg_lite_tiling_mode_t tile_mode,
    vg_lite_color_t       tile_color,
    vg_lite_uint16_t      gamma,
    vg_lite_uint16_t      premultiply,
    vg_lite_uint16_t      channel_mask);

vg_lite_error_t vg_lite_color_matrix_ext(
    vg_lite_buffer_t      *target,
    vg_lite_buffer_t      *source,
    vg_lite_pixel_matrix_t matrix,
    vg_lite_uint16_t       gamma,
    vg_lite_uint16_t       premultiply,
    vg_lite_uint16_t       channel_mask);

/**
* @brief Buffer formats supported by custom platforms
*
*/
typedef enum vg_lite_ext_buffer_format
{
    VG_LITE_RGBA1_ETC2_EAC                  =  0 | (1 << 16),   /** ETC2 punchthrough alpha compressed format */
    VG_LITE_IBC_RGBA_8x8_16BPP              =  1 | (1 << 16),   /** IBC RGBA 16bpp compressed image */
    VG_LITE_IBC_RGB_8x8_12BPP               =  2 | (1 << 16),   /** IBC RGB 12bpp compressed image */
    VG_LITE_IBC_RGBA_8x8_13BPP              =  3 | (1 << 16),   /** IBC RGBA 13bpp compressed image */
    VG_LITE_IBC_RGB_8x8_10BPP               =  4 | (1 << 16),   /** IBC RGB 10bpp compressed image */
    VG_LITE_IBC_RGBA_8x8_11BPP              =  5 | (1 << 16),   /** IBC RGBA 11bpp compressed image */
    VG_LITE_IBC_RGB_8x8_8BPP                =  6 | (1 << 16),   /** IBC RGB 8bpp compressed image */
    VG_LITE_IBC_RGBA_32x1_16BPP             =  7 | (1 << 16),   /** IBC RGBA 16bpp compressed image */
    VG_LITE_IBC_RGB_32x1_12BPP              =  8 | (1 << 16),   /** IBC RGB 12bpp compressed image */
    VG_LITE_IBC_RGBA_32x1_13BPP             =  9 | (1 << 16),   /** IBC RGBA 13bpp compressed image */
    VG_LITE_IBC_RGB_32x1_10BPP              = 10 | (1 << 16),   /** IBC RGB 10bpp compressed image */
    VG_LITE_IBC_RGBA_32x1_11BPP             = 11 | (1 << 16),   /** IBC RGBA 11bpp compressed image */
    VG_LITE_IBC_RGB_32x1_8BPP               = 12 | (1 << 16),   /** IBC RGB 8bpp compressed image */

    VG_LITE_L16                             = 13 | (1 << 16),

    /** Non-subsampled packed Y'CrCb formats*/
    VG_LITE_YUV24                           = 14 | (1 << 16),
    VG_LITE_AYUV32                          = 15 | (1 << 16),
    VG_LITE_VUYA32                          = 16 | (1 << 16),
    VG_LITE_YUVA32                          = 17 | (1 << 16),
    VG_LITE_AVUY32                          = 18 | (1 << 16),
    VG_LITE_XYUV32                          = 19 | (1 << 16),
    VG_LITE_VUYX32                          = 20 | (1 << 16),
    VG_LITE_YUVX32                          = 21 | (1 << 16),

    /** 3 planes Y'CbCr formats*/
    VG_LITE_YU12                            = 22 | (1 << 16),
    VG_LITE_YU16                            = 23 | (1 << 16),
    VG_LITE_YU24                            = 24 | (1 << 16),

    /** 4 planes Y'CbCr formats*/
    VG_LITE_AYU12                            = 25 | (1 << 16),
    VG_LITE_AYU16                            = 26 | (1 << 16),
    VG_LITE_AYU24                            = 27 | (1 << 16),

    /** 2 planes Y'CrCb format, not explicitly supported by VGLite */
    VG_LITE_NV21                             = 28 | (1 << 16),
    VG_LITE_NV61                             = 29 | (1 << 16),
    VG_LITE_NV42                             = 30 | (1 << 16),

    /** RGBA8888 2 planes, alpha on the second plane  */
    VG_LITE_RGBA8888_PLANAR                  = 31 | (1 << 16),
    VG_LITE_BGRA8888_PLANAR                  = 32 | (1 << 16),
    VG_LITE_ARGB8888_PLANAR                  = 33 | (1 << 16),
    VG_LITE_ABGR8888_PLANAR                  = 34 | (1 << 16)
} vg_lite_ext_buffer_format_t;


typedef enum vg_lite_ext_blend
{
    VG_LITE_BLEND_XOR                        = 0 | (1 << 16),
    VG_LITE_BLEND_SPECIAL                    = 1 | (1 << 16)
} vg_lite_ext_blend_t;


/* Extended YUV<->RGB conversion rule. */
typedef enum vg_lite_ext_yuv2rgb
{
    VG_LITE_YUV470 = 0 | (1 << 16)
} vg_lite_ext_yuv2rgb_t;

/* Chip features supported by custom platforms. */
typedef enum vg_lite_ext_feature
{
    gcFEATURE_EXT_FIRST = gcFEATURE_COUNT - 1,
    gcFEATURE_BIT_VG_RGBA1_ETC2_EAC,
    gcFEATURE_BIT_VG_IBC_RGBA_8x8_16BPP,
    gcFEATURE_BIT_VG_IBC_RGB_8x8_12BPP,
    gcFEATURE_BIT_VG_IBC_RGBA_8x8_13BPP,
    gcFEATURE_BIT_VG_IBC_RGB_8x8_10BPP,
    gcFEATURE_BIT_VG_IBC_RGBA_8x8_11BPP,
    gcFEATURE_BIT_VG_IBC_RGB_8x8_8BPP,
    gcFEATURE_BIT_VG_IBC_RGBA_32x1_16BPP,
    gcFEATURE_BIT_VG_IBC_RGB_32x1_12BPP,
    gcFEATURE_BIT_VG_IBC_RGBA_32x1_13BPP,
    gcFEATURE_BIT_VG_IBC_RGB_32x1_10BPP,
    gcFEATURE_BIT_VG_IBC_RGBA_32x1_11BPP,
    gcFEATURE_BIT_VG_IBC_RGB_32x1_8BPP,
    gcFEATURE_EXT_LAST                   // last extended feature
} vg_lite_ext_feature_t;

/**
 * @brief Signed format type
 *
 * @discussion
 * Indicate the type of the signed output format
 */
typedef enum vg_lite_ext_signed_fmt
{
    VG_LITE_EXT_16BIT_SIGNED,
    VG_LITE_EXT_8BIT_SIGNED,
} vg_lite_ext_signed_fmt_t;

vg_lite_error_t vg_lite_enable_signed_fmt_ext(
    vg_lite_ext_signed_fmt_t fmt,
    vg_lite_uint32_t         scale,
    vg_lite_uint16_t         zero_point);

vg_lite_error_t vg_lite_disable_signed_fmt_ext(void);

/**
 * @brief Wrap a caller-owned memory block into a vg_lite_buffer_t.
 *
 * The caller fills in the buffer property fields (width, height, stride,
 * format, yuv details, ...) and provides a pointer to an already-allocated
 * memory block. This function wraps the memory block with the provided
 * vg_lite_buffer_t buffer: buffer.memory will point to the memory block
 *
 * Ownership of user_buffer remains with the caller. After the wrapped buffer
 * is no longer needed, call vg_lite_unwrap_buffer_ext() to release the internal
 * driver objects, then free user_buffer.
 *
 * user_buffer_size states how many bytes are readable and writable at
 * user_buffer. The driver cannot discover the length of an allocation it did
 * not make, so the caller must declare it: it becomes the buffer's recorded
 * capacity and is what bounds later writes such as vg_lite_upload_buffer().
 * Declaring more than the real allocation defeats that bound, so it must be
 * the true size. The call fails with VG_LITE_INVALID_ARGUMENT when
 * user_buffer_size is 0 or is too small for the geometry in buffer.
 *
 * @param[in,out] buffer            Buffer descriptor with property fields set.
 * @param[in]     user_buffer       Caller-owned backing memory to wrap.
 * @param[in]     user_buffer_size  Size of user_buffer in bytes.
 */
vg_lite_error_t vg_lite_wrap_buffer_ext(
    vg_lite_buffer_t *buffer,
    vg_lite_void     *user_buffer,
    vg_lite_uint32_t  user_buffer_size);

/**
 * @brief Release the internal driver objects created by vg_lite_wrap_buffer_ext.
 *
 * The caller-owned backing memory is left intact and must
 * be freed by the caller.
 *
 * @param[in,out] buffer  Buffer previously wrapped with vg_lite_wrap_buffer_ext.
 */
vg_lite_error_t vg_lite_unwrap_buffer_ext(vg_lite_buffer_t *buffer);

/**
 * @brief Upload pixel data into a pre-allocated vg_lite_buffer_t, with support
 *        for 4-plane AYUV formats and per-plane source strides.
 *
 * Compared to vg_lite_upload_buffer, this function differs in two ways:
 * - It supports up to 4 planes (Y, Cb, Cr, A), covering AYUV formats that
 *   vg_lite_upload_buffer cannot handle (3-plane maximum).
 * - It accepts a per-plane source stride, so the caller may pass a source
 *   buffer whose row pitch differs from the tightly-packed destination;
 *   only the destination plane width is copied per row.
 *
 * The caller is responsible for allocating the destination video memory
 * (e.g. via vg_lite_allocate()) before calling this function, and for
 * releasing it afterwards (e.g. via vg_lite_free()).
 *
 * @param[in,out] buffer  Buffer descriptor with property fields (width, height,
 *                        format, ...) already set and video memory already
 *                        allocated by the caller (e.g. via vg_lite_allocate()).
 * @param[in]     data    Array of source plane pointers (one per plane, up to 4).
 *                        Each pointer must address the first byte of the
 *                        corresponding plane.
 * @param[in]     stride  Array of source strides in bytes per row, one per plane.
 *                        Pass NULL, or set an individual entry to 0, to indicate
 *                        that the corresponding source plane is tightly packed;
 *                        the destination stride is then used for that plane.
 */
vg_lite_error_t vg_lite_upload_buffer_ext(
    vg_lite_buffer_t *buffer,
    vg_lite_uint8_t  *data[4],
    vg_lite_uint32_t  stride[4]);

typedef struct vg_lite_ext_prof_counters
{
    tn_u64   driver_cycles;
    tn_u64   gpu_cycles;
    tn_u64   fw_cycles;
    tn_u64   vg_cycles;
} vg_lite_ext_prof_counters_t;

typedef enum vg_lite_ext_param_type
{
    VG_LITE_EXT_CYCLE_COUNTERS                 = 0 | (1 << 16)   /*! count must be 1 */
} vg_lite_ext_param_type_t;

/**
 * @brief IRQ Handler callback
 */
void vg_lite_irq_handler_ext(void);
