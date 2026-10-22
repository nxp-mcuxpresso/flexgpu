//
// Copyright 2025-2026 NXP
//
// SPDX-License-Identifier: MIT
//

#pragma once

#include "tn_types.h"
#include "tn_color.h"
#include "tn_gbuf.h"
#include "tn_m3f32.h"
#include "tn_dynarr.h"


typedef enum
{
    S1,
    S2,
    DST,
    MASK
} tn_chan_type;

typedef enum
{
    SCISSOR0,
    SCISSOR1,
    SCISSOR2,
    SCISSOR3,
    SCISSOR_NUM
} tn_scissor_id;

typedef enum
{
    KEY0,
    KEY1,
    KEY2,
    KEY3
} tn_col_key_id;

#define MSK_FMT_A4    0
#define MSK_FMT_A8    1
#define MSK_FMT_A1    2

#define TN_REGS_IMAGE_FMT_TYPE1_RGBA        0
#define TN_REGS_IMAGE_FMT_TYPE1_INDEX       1
#define TN_REGS_IMAGE_FMT_TYPE1_YUV         2
#define TN_REGS_IMAGE_FMT_TYPE1_COMPRESSED  3

// these apply when TYPE1=RGBA
#define TN_REGS_IMAGE_FMT_TYPE2_PACKED 0         // all colors in plane0
#define TN_REGS_IMAGE_FMT_TYPE2_RGB_A  1         // RGB is in plane0,  A is in plane1
#define TN_REGS_IMAGE_FMT_TYPE2_GRAY8  2         //
#define TN_REGS_IMAGE_FMT_TYPE2_GRAY16 3         //
// these apply when TYPE1=RGBA, and WIDTH_24/32 also used for YUV444 1PLANE
#define TN_REGS_IMAGE_FMT_WORD_WIDTH_1  0
#define TN_REGS_IMAGE_FMT_WORD_WIDTH_2  1
#define TN_REGS_IMAGE_FMT_WORD_WIDTH_4  2
#define TN_REGS_IMAGE_FMT_WORD_WIDTH_8  3
#define TN_REGS_IMAGE_FMT_WORD_WIDTH_16 4
#define TN_REGS_IMAGE_FMT_WORD_WIDTH_24 5
#define TN_REGS_IMAGE_FMT_WORD_WIDTH_32 6

// these apply when TYPE1=YUV
#define TN_REGS_IMAGE_FMT_TYPE2_444     0         // these are in the 2 LSBs of "type2"
#define TN_REGS_IMAGE_FMT_TYPE2_422     1
#define TN_REGS_IMAGE_FMT_TYPE2_420     2
#define TN_REGS_IMAGE_FMT_TYPE2_1PLANE  (0 << 2)    // these are in the 2 MSBs of "type2"
#define TN_REGS_IMAGE_FMT_TYPE2_2PLANE  (1 << 2)
#define TN_REGS_IMAGE_FMT_TYPE2_3PLANE  (2 << 2)
#define TN_REGS_IMAGE_FMT_TYPE2_4PLANE  (3 << 2)

// these apply when TYPE1=INDEX
#define TN_REGS_IMAGE_FMT_TYPE2_INDEX1  0
#define TN_REGS_IMAGE_FMT_TYPE2_INDEX2  1
#define TN_REGS_IMAGE_FMT_TYPE2_INDEX4  2
#define TN_REGS_IMAGE_FMT_TYPE2_INDEX8  3

// these apply when TYPE1=COMPRESSED
#define TN_REGS_IMAGE_FMT_TYPE2_ETC2_RGB                    0
#define TN_REGS_IMAGE_FMT_TYPE2_ETC2_RGBA                   1
#define TN_REGS_IMAGE_FMT_TYPE2_ETC2_RGBA1                  2
#define TN_REGS_IMAGE_FMT_TYPE2_IBC_RGBA_8x8_16BPP          4
#define TN_REGS_IMAGE_FMT_TYPE2_IBC_RGB_8x8_12BPP           5
#define TN_REGS_IMAGE_FMT_TYPE2_IBC_RGBA_8x8_13BPP          6
#define TN_REGS_IMAGE_FMT_TYPE2_IBC_RGB_8x8_10BPP           7
#define TN_REGS_IMAGE_FMT_TYPE2_IBC_RGBA_8x8_11BPP          8
#define TN_REGS_IMAGE_FMT_TYPE2_IBC_RGB_8x8_8BPP            9
#define TN_REGS_IMAGE_FMT_TYPE2_IBC_RGBA_32x1_16BPP         10
#define TN_REGS_IMAGE_FMT_TYPE2_IBC_RGB_32x1_12BPP          11
#define TN_REGS_IMAGE_FMT_TYPE2_IBC_RGBA_32x1_13BPP         12
#define TN_REGS_IMAGE_FMT_TYPE2_IBC_RGB_32x1_10BPP          13
#define TN_REGS_IMAGE_FMT_TYPE2_IBC_RGBA_32x1_11BPP         14
#define TN_REGS_IMAGE_FMT_TYPE2_IBC_RGB_32x1_8BPP           15


#define TN_REGS_S2_XFORM_MODE_AFFINE     0
#define TN_REGS_S2_XFORM_MODE_PROJECTIVE 1

#define TN_REGS_VG_IMAGE_QUALITY_NONANTIALIASED 0
#define TN_REGS_VG_IMAGE_QUALITY_FASTER         1
#define TN_REGS_VG_IMAGE_QUALITY_BETTER         2

#define TN_REGS_VG_TILING_MODE_FILL    0
#define TN_REGS_VG_TILING_MODE_PAD     1
#define TN_REGS_VG_TILING_MODE_REPEAT  2
#define TN_REGS_VG_TILING_MODE_REFLECT 3

#define TN_RECT_MODE_INVALID 0  // pixels outside of scissoring rectangles are marked as invalid.      <-- default behavior
#define TN_RECT_MODE_OOB     1  // pixels outside of scissoring rectangles are marked as out-of-bounds

#define TN_GLOBAL_ALPHA 0
#define TN_LOCAL_ALPHA  1
#define TN_SCALED_ALPHA 2

//Blending mode
#define PORTER_DUFF 0
#define ADDITIONAL 1
#define ADDITIVE 2
#define NEW_MODE 3

//Additional mode
#define DARKEN 0
#define LIGHTEN 1
#define MULTIPLY 2
#define SCREEN 3

//Factor
#define ZERO_FACTOR 1
#define FULL_FACTOR 0
#define STRAIGTH_ALPHA 2
#define INVERSE_ALPHA 3

typedef struct
{
    tn_u32   low, high;
} color_key_pair;


typedef_dynarr (tn_u32, tn_reg_addrs_list, static inline)
typedef_dynarr (tn_u32, tn_reg_values_list, static inline)

void tn_regs_enq(
    tn_u32 reg,
    tn_u32 value);

void tn_regs_set_base_addr(
    tn_u32             addr,
    const tn_chan_type ch);

tn_reg_32 tn_regs_get_base_addr(const tn_chan_type ch);

tn_u8 tn_regs_get_mask_enabled(void);

tn_u8 tn_regs_get_mask_fmt(void);

tn_u8 tn_regs_get_mask_num_tiles(void);

void tn_regs_get_product_info(
    tn_char *name,
    tn_u32  *chip_id,
    tn_u32  *chip_rev);

void tn_regs_disable_s1(void);

void tn_regs_disable_s2(void);

void tn_regs_enable_mask(
    tn_u8 fmt,
    tn_u8 num_tiles);

void tn_regs_disable_mask(void);

void tn_regs_enable_gpu_ctrl(void);

void tn_regs_set_s1_bkgnd_color(tn_u32 color);

void tn_regs_set_s2_bkgnd_color(tn_u32 color);

void tn_regs_set_dst_fill_color(tn_u32 color);

void tn_regs_set_lut(
    tn_u32  count,
    tn_u32 *colors);

void tn_regs_enable_dither(void);

void tn_regs_enable_dither_ext(
    tn_u8 matrix_type,
    tn_u8 order_scale_factor_r,
    tn_u8 order_scale_factor_g,
    tn_u8 order_scale_factor_b,
    tn_u8 order_scale_factor_a,
    tn_u8 order_scale_factor_bw);

void tn_regs_disable_dither(void);

void tn_regs_set_mask_stride(tn_reg_32 reg_val);

void tn_regs_set_dst_csc(void);

void tn_regs_set_dst_color_format(
    const tn_gbuf *img,
    tn_u32         en_dst_fill,
    tn_u32         src_yuv);

void tn_regs_set_dst_fill(
    const tn_gbuf *dstI,
    tn_u32         color,
    tn_u32         w,
    tn_u32         h);

void tn_regs_set_s1_csc(tn_i32 conv_standard);

void tn_regs_set_s1_color_format(const tn_gbuf *img);

void tn_regs_set_s2_csc(tn_i32 conv_standard);

void tn_regs_set_s2_color_format(
    const tn_gbuf *img,
    tn_u32         quality);

void tn_regs_enable_signed_fmt(
    tn_u8  fmt,
    tn_u32 scale,
    tn_u16 zero_point);

void tn_regs_disable_signed_fmt(void);

void tn_regs_disable_crt_scissor(tn_u8 id);

void tn_regs_disable_scissors(void);

void tn_regs_set_scissor(
    tn_scissor_id id,
    tn_u16        start_x,
    tn_u16        start_y,
    tn_u16        width,
    tn_u16        height);

void tn_regs_set_size(
    tn_reg_32          width,
    tn_reg_32          height,
    const tn_chan_type ch);

void tn_regs_set_swizzel(
    tn_color           col,
    const tn_chan_type ch);

void tn_regs_set_color_width(
    tn_color           col,
    const tn_chan_type ch);

tn_reg_32 tn_regs_set_fmt_type(
    const tn_gbuf     *img,
    const tn_chan_type ch);

tn_reg_32 tn_regs_set_premul(
    tn_color           col,
    const tn_chan_type ch);

tn_reg_32 tn_regs_set_gamma(
    tn_color           col,
    const tn_chan_type ch);

tn_reg_32 tn_regs_set_word_width(
    const tn_gbuf     *img,
    tn_color           col,
    const tn_chan_type ch);

tn_u32 tn_regs_en_dst_fill(tn_u32 enable);

void tn_regs_enable_color_transform(void);

void tn_regs_disable_color_transform(void);

void tn_regs_set_color_transform(tn_color_transform *colorX);

void tn_regs_set_blending_mode(
    tn_i32  blend,
    tn_bool S2_supported_chan);

void tn_regs_set_xform_mode(tn_reg_32 mode);

void tn_regs_set_xform(tn_m3f32 *mat);

// Set the stride and the start address of each plane.
// base_addr:           physical address of the allocated image.
// offset_x & offset_y: offset inside the image. This is used to
//                      optimize the case where only a smaller portion of the
//                      destination buffer needs GPU processing.
// img:                 pixel-buffer descriptor
// col:                 color format information
// ch:                  GPU channel (S1, S2, DST or MASK)
tn_size tn_regs_set_stride_and_addr(
    tn_u32             base_addr,
    tn_u32             offset_x,
    tn_u32             offset_y,
    const tn_gbuf     *img,
    tn_color           col,
    const tn_chan_type ch);

void tn_regs_set_color_key(
    const tn_col_key_id key,
    tn_u8               enable,
    tn_u32              low,
    tn_u32              high,
    tn_u32              alpha);

void tn_regs_reset_color_key(const tn_chan_type ch);

void tn_regs_transfer_color_key(const tn_chan_type ch);
