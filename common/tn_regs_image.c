//
// Copyright 2025-2026 NXP
//
// SPDX-License-Identifier: MIT
//

#include "tn_regs_image.h"
#include "tn_gpu_regs.h"
#include "tin.h"
#include "tn_hw.h"
#include "tn_fxp.h"
#include "tn_ctx_img.h"
#include "tn_os.h"
#include "tn_cmd_buf_ccdma.h"
#include "tn_fmt.h"
#include "tn_obj.h"
#include "tn_mspace.h"
#include "tn_mem.h"



// matrix types
#define TN_MATRIX_2x2 0
#define TN_MATRIX_4x4 1
#define TN_MATRIX_8x8 2
#define TN_MATRIX_16x16 3


typedef struct
{
    tn_u32   reg[TINGPU_S1_STRIDE_NUM];
    tn_u32   reg_shift;
} tn_stride_regs;

typedef struct
{
    tn_u32   reg[TINGPU_S1_ADDR_NUM];
    tn_u32   reg_shift;
} tn_addr_regs;


// The other CTRL/SIZE/COLOR_SHIFT/COLOR_BITS fields that used to live here
// (enabled/word_width/premul/gamma shifts, fmt_type, size, swizzel, width)
// are now read directly through the matching TINGPU_<CH>_...(x) macros in
// each tn_regs_set_* helper below, so only the array-style ADDR/STRIDE
// registers - which still need a per-channel base/shift lookup - remain here.
typedef struct
{
    tn_addr_regs           addr;
    tn_stride_regs         stride;
} tn_chan_regs;


static const tn_chan_regs ch_regs[3]  =
{
    [S1]  = { .addr        = { .reg = { TINGPU_S1_ADDR_IDX(0), TINGPU_S1_ADDR_IDX(1),
                                         TINGPU_S1_ADDR_IDX(2), TINGPU_S1_ADDR_IDX(3) },
                               .reg_shift = TINGPU_S1_ADDR_ADDR_SHIFT},
              .stride      = { .reg = { TINGPU_S1_STRIDE_IDX(0), TINGPU_S1_STRIDE_IDX(1),
                                         TINGPU_S1_STRIDE_IDX(2), TINGPU_S1_STRIDE_IDX(3) },
                               .reg_shift = TINGPU_S1_STRIDE_STRIDE_SHIFT}},
    [S2]  = { .addr        = { .reg = { TINGPU_S2_ADDR_IDX(0), TINGPU_S2_ADDR_IDX(1),
                                         TINGPU_S2_ADDR_IDX(2), TINGPU_S2_ADDR_IDX(3) },
                               .reg_shift = TINGPU_S2_ADDR_ADDR_SHIFT},
              .stride      = { .reg = { TINGPU_S2_STRIDE_IDX(0), TINGPU_S2_STRIDE_IDX(1),
                                         TINGPU_S2_STRIDE_IDX(2), TINGPU_S2_STRIDE_IDX(3) },
                               .reg_shift = TINGPU_S2_STRIDE_STRIDE_SHIFT}},
    [DST] = { .addr        = { .reg = { TINGPU_DST_ADDR_IDX(0), TINGPU_DST_ADDR_IDX(1),
                                         TINGPU_DST_ADDR_IDX(2), TINGPU_DST_ADDR_IDX(3) },
                               .reg_shift = TINGPU_DST_ADDR_ADDR_SHIFT},
              .stride      = { .reg = { TINGPU_DST_STRIDE_IDX(0), TINGPU_DST_STRIDE_IDX(1),
                                         TINGPU_DST_STRIDE_IDX(2), TINGPU_DST_STRIDE_IDX(3) },
                               .reg_shift = TINGPU_DST_STRIDE_STRIDE_SHIFT}},
};


typedef struct
{
    tn_u32   low[TINGPU_S1_CLRKEY_LOW_NUM];
    tn_u32   high[TINGPU_S1_CLRKEY_HIGH_NUM];
} tn_color_key_regs;

static const tn_color_key_regs col_keys[2]  =
{
    [S1] = { .low  = { TINGPU_S1_CLRKEY_LOW_IDX(0), TINGPU_S1_CLRKEY_LOW_IDX(1),
                        TINGPU_S1_CLRKEY_LOW_IDX(2), TINGPU_S1_CLRKEY_LOW_IDX(3) },
             .high = { TINGPU_S1_CLRKEY_HIGH_IDX(0), TINGPU_S1_CLRKEY_HIGH_IDX(1),
                        TINGPU_S1_CLRKEY_HIGH_IDX(2), TINGPU_S1_CLRKEY_HIGH_IDX(3) }},
    [S2] = { .low  = { TINGPU_S2_CLRKEY_LOW_IDX(0), TINGPU_S2_CLRKEY_LOW_IDX(1),
                        TINGPU_S2_CLRKEY_LOW_IDX(2), TINGPU_S2_CLRKEY_LOW_IDX(3) },
             .high = { TINGPU_S2_CLRKEY_HIGH_IDX(0), TINGPU_S2_CLRKEY_HIGH_IDX(1),
                        TINGPU_S2_CLRKEY_HIGH_IDX(2), TINGPU_S2_CLRKEY_HIGH_IDX(3) }}
};

typedef struct
{
    tn_u8   mode;
    tn_u8   additional;
    tn_u8   s1_factor;
    tn_u8   s2_factor;
} tn_blend_config;

typedef struct
{
    tn_blend_config   cfg[15];
} tn_blend_regs;

static const tn_blend_regs blend_modes[2]  =
{
    [S1] = { .cfg[TN_BLEND_SRC]       = { .mode = PORTER_DUFF,
                                          .additional = DARKEN,
                                          .s1_factor = ZERO_FACTOR,
                                          .s2_factor = FULL_FACTOR},
             .cfg[TN_BLEND_SRC_OVER]  = { .mode = PORTER_DUFF,
                                          .additional = DARKEN,
                                          .s1_factor = INVERSE_ALPHA,
                                          .s2_factor = FULL_FACTOR},
             .cfg[TN_BLEND_DST_OVER]  = { .mode = PORTER_DUFF,
                                          .additional = DARKEN,
                                          .s1_factor = FULL_FACTOR,
                                          .s2_factor = INVERSE_ALPHA},
             .cfg[TN_BLEND_SRC_IN]    = { .mode = PORTER_DUFF,
                                          .additional = DARKEN,
                                          .s1_factor = ZERO_FACTOR,
                                          .s2_factor = STRAIGTH_ALPHA},
             .cfg[TN_BLEND_DST_IN]    = { .mode = PORTER_DUFF,
                                          .additional = DARKEN,
                                          .s1_factor = STRAIGTH_ALPHA,
                                          .s2_factor = ZERO_FACTOR},
             .cfg[TN_BLEND_MULTIPLY]  = { .mode = ADDITIONAL,
                                          .additional = MULTIPLY,
                                          .s1_factor = FULL_FACTOR,
                                          .s2_factor = FULL_FACTOR},
             .cfg[TN_BLEND_SCREEN]    = { .mode = ADDITIONAL,
                                          .additional = SCREEN,
                                          .s1_factor = FULL_FACTOR,
                                          .s2_factor = FULL_FACTOR},
             .cfg[TN_BLEND_DARKEN]    = { .mode = ADDITIONAL,
                                          .additional = DARKEN,
                                          .s1_factor = FULL_FACTOR,
                                          .s2_factor = FULL_FACTOR},
             .cfg[TN_BLEND_LIGHTEN]   = { .mode = ADDITIONAL,
                                          .additional = LIGHTEN,
                                          .s1_factor = FULL_FACTOR,
                                          .s2_factor = FULL_FACTOR},
             .cfg[TN_BLEND_ADDITIVE]  = { .mode = ADDITIVE,
                                          .additional = MULTIPLY,
                                          .s1_factor = FULL_FACTOR,
                                          .s2_factor = FULL_FACTOR},
             .cfg[TN_BLEND_SRC_OUT]   = { .mode = PORTER_DUFF,
                                          .additional = DARKEN,
                                          .s1_factor = ZERO_FACTOR,
                                          .s2_factor = INVERSE_ALPHA},
             .cfg[TN_BLEND_DST_OUT]   = { .mode = PORTER_DUFF,
                                          .additional = DARKEN,
                                          .s1_factor = INVERSE_ALPHA,
                                          .s2_factor = ZERO_FACTOR},
             .cfg[TN_BLEND_SUBTRACT]  = { .mode = PORTER_DUFF,
                                          .additional = DARKEN,
                                          .s1_factor = INVERSE_ALPHA,
                                          .s2_factor = ZERO_FACTOR},
             .cfg[TN_BLEND_XOR]       = { .mode = PORTER_DUFF,
                                          .additional = DARKEN,
                                          .s1_factor = INVERSE_ALPHA,
                                          .s2_factor = INVERSE_ALPHA},
             .cfg[TN_BLEND_SPECIAL]   = { .mode = NEW_MODE,
                                          .additional = DARKEN,
                                          .s1_factor = INVERSE_ALPHA,
                                          .s2_factor = FULL_FACTOR}},
    [S2] = { .cfg[TN_BLEND_SRC]       = { .mode = PORTER_DUFF,
                                          .additional = DARKEN,
                                          .s1_factor = FULL_FACTOR,
                                          .s2_factor = ZERO_FACTOR},
             .cfg[TN_BLEND_SRC_OVER]  = { .mode = PORTER_DUFF,
                                          .additional = DARKEN,
                                          .s1_factor = FULL_FACTOR,
                                          .s2_factor = INVERSE_ALPHA},
             .cfg[TN_BLEND_DST_OVER]  = { .mode = PORTER_DUFF,
                                          .additional = DARKEN,
                                          .s1_factor = INVERSE_ALPHA,
                                          .s2_factor = FULL_FACTOR},
             .cfg[TN_BLEND_SRC_IN]    = { .mode = PORTER_DUFF,
                                          .additional = DARKEN,
                                          .s1_factor = STRAIGTH_ALPHA,
                                          .s2_factor = ZERO_FACTOR},
             .cfg[TN_BLEND_DST_IN]    = { .mode = PORTER_DUFF,
                                          .additional = DARKEN,
                                          .s1_factor = ZERO_FACTOR,
                                          .s2_factor = STRAIGTH_ALPHA},
             .cfg[TN_BLEND_MULTIPLY]  = { .mode = ADDITIONAL,
                                          .additional = MULTIPLY,
                                          .s1_factor = FULL_FACTOR,
                                          .s2_factor = FULL_FACTOR},
             .cfg[TN_BLEND_SCREEN]    = { .mode = ADDITIONAL,
                                          .additional = SCREEN,
                                          .s1_factor = FULL_FACTOR,
                                          .s2_factor = FULL_FACTOR},
             .cfg[TN_BLEND_DARKEN]    = { .mode = ADDITIONAL,
                                          .additional = DARKEN,
                                          .s1_factor = FULL_FACTOR,
                                          .s2_factor = FULL_FACTOR},
             .cfg[TN_BLEND_LIGHTEN]   = { .mode = ADDITIONAL,
                                          .additional = LIGHTEN,
                                          .s1_factor = FULL_FACTOR,
                                          .s2_factor = FULL_FACTOR},
             .cfg[TN_BLEND_ADDITIVE]  = { .mode = ADDITIVE,
                                          .additional = MULTIPLY,
                                          .s1_factor = FULL_FACTOR,
                                          .s2_factor = FULL_FACTOR},
             .cfg[TN_BLEND_SRC_OUT]   = { .mode = PORTER_DUFF,
                                          .additional = DARKEN,
                                          .s1_factor = INVERSE_ALPHA,
                                          .s2_factor = ZERO_FACTOR},
             .cfg[TN_BLEND_DST_OUT]   = { .mode = PORTER_DUFF,
                                          .additional = DARKEN,
                                          .s1_factor = ZERO_FACTOR,
                                          .s2_factor = INVERSE_ALPHA},
             .cfg[TN_BLEND_SUBTRACT]   = { .mode = PORTER_DUFF,
                                          .additional = DARKEN,
                                          .s1_factor = ZERO_FACTOR,
                                          .s2_factor = INVERSE_ALPHA},
             .cfg[TN_BLEND_XOR]        = { .mode = PORTER_DUFF,
                                          .additional = DARKEN,
                                          .s1_factor = INVERSE_ALPHA,
                                          .s2_factor = INVERSE_ALPHA},
             .cfg[TN_BLEND_SPECIAL]    = { .mode = NEW_MODE,
                                          .additional = DARKEN,
                                          .s1_factor = FULL_FACTOR,
                                          .s2_factor = INVERSE_ALPHA}}
};


// Cached values for base_addr.
// This doesn't have to be in vglitectx as it is not init it during vg_lite_init
static tn_reg_32 base_addr_cache[4] = {0};

// cached values for mask state
static tn_u8 mask_enabled_cache = 0;
static tn_u8 mask_fmt_cache = 0;
static tn_u8 mask_num_tiles_cache = 0;

void
tn_regs_enq(
    tn_u32 reg,
    tn_u32 value)
{
    tn_cmd_buf_ccdma_load_reg(reg, value);
}



void
tn_regs_set_base_addr(
    tn_u32             addr,
    const tn_chan_type ch)
{
    base_addr_cache[ch] = addr;

    // The TINGPU_<CH>_ADDR_ADDR(x) macros already combine the field's shift
    // and mask for that channel's ADDR0 register, so there is no need for a
    // runtime reg/shift lookup table here: just dispatch on the channel and
    // call the matching macro directly.
    switch (ch)
    {
    case S1:
        tn_regs_enq(TINGPU_S1_ADDR0, TINGPU_S1_ADDR_ADDR(addr));
        break;
    case S2:
        tn_regs_enq(TINGPU_S2_ADDR0, TINGPU_S2_ADDR_ADDR(addr));
        break;
    case DST:
        tn_regs_enq(TINGPU_DST_ADDR0, TINGPU_DST_ADDR_ADDR(addr));
        break;
    case MASK:
        tn_regs_enq(TINGPU_MASK_ADDR, TINGPU_MASK_ADDR_ADDR(addr));
        break;
    }
}



tn_reg_32
tn_regs_get_base_addr(const tn_chan_type ch)
{
    return base_addr_cache[ch];
}



tn_u8
tn_regs_get_mask_enabled(void)
{
    return mask_enabled_cache;
}



tn_u8
tn_regs_get_mask_fmt(void)
{
    return mask_fmt_cache;
}



tn_u8
tn_regs_get_mask_num_tiles(void)
{
    return mask_num_tiles_cache;
}



void
tn_regs_get_product_info(
    tn_char *name,
    tn_u32  *chip_id,
    tn_u32  *chip_rev)
{
    tnEXPECT(!tn_is_null(name));
    tnEXPECT(!tn_is_null(chip_id));
    tnEXPECT(!tn_is_null(chip_rev));

    tn_u32 reg_value = tn_hw_reg_get(TINGPU_VERID);

    tn_memcpy(name, "FlexGPU", sizeof("FlexGPU"));

    *chip_id  = (reg_value & TINGPU_VERID_FEATURE_MASK) >> TINGPU_VERID_FEATURE_SHIFT;
    *chip_rev = ((reg_value & TINGPU_VERID_MAJOR_MASK) >> (TINGPU_VERID_MAJOR_SHIFT - 8) |
                 (reg_value & TINGPU_VERID_MINOR_MASK) >> TINGPU_VERID_MINOR_SHIFT);
}



void
tn_regs_disable_s1(void)
{
    tn_regs_enq(TINGPU_S1_CTRL, 0);
}



void
tn_regs_disable_s2(void)
{
    tn_regs_enq(TINGPU_S2_CTRL, 0);
}



void
tn_regs_enable_mask(
    tn_u8 fmt,
    tn_u8 num_tiles)
{
    tn_regs_enq(TINGPU_MASK_CTRL,
                TINGPU_MASK_CTRL_EN(1) | (TINGPU_MASK_CTRL_MASK_FMT(fmt)) | (TINGPU_S1_CTRL_ONE_TILE(num_tiles)));

    mask_fmt_cache = fmt;
    mask_num_tiles_cache = num_tiles;
    mask_enabled_cache = 1;
}



void
tn_regs_disable_mask(void)
{
    tn_regs_enq(TINGPU_MASK_CTRL, TINGPU_MASK_CTRL_EN(0));

    mask_fmt_cache = 0;
    mask_num_tiles_cache = 0;
    mask_enabled_cache = 0;
}



void
tn_regs_enable_gpu_ctrl(void)
{
    tn_cmd_buf_ccdma_enable_gpu_ctrl();
}



void
tn_regs_set_s1_bkgnd_color(tn_u32 color)
{
    tn_regs_enq(TINGPU_S1_BKGND, color);
}



void
tn_regs_set_s2_bkgnd_color(tn_u32 color)
{
    tn_regs_enq(TINGPU_S2_BKGND, color);
}



void
tn_regs_set_dst_fill_color(tn_u32 color)
{
    tn_regs_enq(TINGPU_DST_FILL_DATA, color);
}



void
tn_regs_set_lut(
    tn_u32  count,
    tn_u32 *colors)
{
    tnEXPECT(count <= 256);
    tnEXPECT(!tn_is_null(colors));

    tn_u32 start_reg = TINGPU_COLOR_LUT0;
    for (tn_u32 i = 0; i < count; i++) {
        tn_regs_enq(start_reg + i * 4, colors[i]);
    }
}



void
tn_regs_enable_dither(void)
{
    tn_reg_32 reg_val;
    const tn_u8 matrix_type = TN_MATRIX_2x2;

    //0:1/128, 1:1/64, 2:1/32 ,3:1/16, 4:1/8, 5:1/4, 6:1/2, 7:1, 8:2, 9:4, 10:8, 11:16, others:32
    const tn_u8 order_scale_factor_r = 7;
    const tn_u8 order_scale_factor_g = 7;
    const tn_u8 order_scale_factor_b = 7;
    const tn_u8 order_scale_factor_a = 7;

    const tn_u8 order_scale_factor_bw = 0; //0:right shift +0; 1:right shift +1; 2: right shift +2; 3: right shift +3

    // enable dither
    reg_val = TINGPU_DITHER_CTRL_DITHER_EN(1);
    // set matrix mode
    reg_val |= (TINGPU_DITHER_CTRL_ORDER_MATRIX_TYPE(matrix_type & 0x03));
    // set scaling factors
    reg_val |= (TINGPU_DITHER_CTRL_ORDER_SCALE_FACTOR_BW(order_scale_factor_bw & 0x03));
    reg_val |= (TINGPU_DITHER_CTRL_ORDER_SCALE_A(order_scale_factor_a & 0x0F));
    reg_val |= (TINGPU_DITHER_CTRL_ORDER_SCALE_B(order_scale_factor_b & 0x0F));
    reg_val |= (TINGPU_DITHER_CTRL_ORDER_SCALE_G(order_scale_factor_g & 0x0F));
    reg_val |= (TINGPU_DITHER_CTRL_ORDER_SCALE_R(order_scale_factor_r & 0x0F));

    tn_regs_enq(TINGPU_DITHER_CTRL, reg_val);
}



void
tn_regs_enable_dither_ext(
    tn_u8 matrix_type,
    tn_u8 order_scale_factor_r,
    tn_u8 order_scale_factor_g,
    tn_u8 order_scale_factor_b,
    tn_u8 order_scale_factor_a,
    tn_u8 order_scale_factor_bw)
{
    tn_reg_32 reg_val;

    // enable dither
    reg_val = TINGPU_DITHER_CTRL_DITHER_EN(1);
    // set matrix mode
    reg_val |= (TINGPU_DITHER_CTRL_ORDER_MATRIX_TYPE(matrix_type & 0x03));
    // set scaling factors
    // BW 0:right shift +0; 1:right shift +1; 2: right shift +2; 3: right shift +3
    reg_val |= (TINGPU_DITHER_CTRL_ORDER_SCALE_FACTOR_BW(order_scale_factor_bw & 0x03));
    // RGB 0:1/128, 1:1/64, 2:1/32 ,3:1/16, 4:1/8, 5:1/4, 6:1/2, 7:1, 8:2, 9:4, 10:8, 11:16, others:32
    reg_val |= (TINGPU_DITHER_CTRL_ORDER_SCALE_A(order_scale_factor_a & 0x0F));
    reg_val |= (TINGPU_DITHER_CTRL_ORDER_SCALE_B(order_scale_factor_b & 0x0F));
    reg_val |= (TINGPU_DITHER_CTRL_ORDER_SCALE_G(order_scale_factor_g & 0x0F));
    reg_val |= (TINGPU_DITHER_CTRL_ORDER_SCALE_R(order_scale_factor_r & 0x0F));

    tn_regs_enq(TINGPU_DITHER_CTRL, reg_val);
}



void
tn_regs_disable_dither(void)
{
    tn_regs_enq(TINGPU_DITHER_CTRL, 0);
}



void
tn_regs_set_mask_stride(tn_reg_32 reg_val)
{
    tn_regs_enq(TINGPU_MASK_STRIDE,
                TINGPU_MASK_STRIDE_STRIDE(reg_val));
}



void
tn_regs_set_dst_csc()
{
    tn_reg_32 reg_val = 0;
    tn_reg_32 a1, a2, a3, b1, b2, b3, c1, c2, c3, d1, d2, d3;
    // presets for TN_YUV601
    a1 = 0x42;
    a2 = 0x81;
    a3 = 0x19;
    b1 = 0x7da;
    b2 = 0x7b6;
    b3 = 0x70;
    c1 = 0x70;
    c2 = 0x7a2;
    c3 = 0x7ee;
    d1 = 0x10;
    d2 = 0x80;
    d3 = 0x80;
    reg_val = (TINGPU_DST_CSC0_A1(a1)) |
              (TINGPU_DST_CSC0_A2(a2));
    tn_regs_enq(TINGPU_DST_CSC0, reg_val);

    reg_val = (TINGPU_DST_CSC1_A3(a3)) |
              (TINGPU_DST_CSC1_B1(b1));
    tn_regs_enq(TINGPU_DST_CSC1, reg_val);

    reg_val = (TINGPU_DST_CSC2_B2(b2)) |
              (TINGPU_DST_CSC2_B3(b3));
    tn_regs_enq(TINGPU_DST_CSC2, reg_val);

    reg_val = (TINGPU_DST_CSC3_C1(c1)) |
              (TINGPU_DST_CSC3_C2(c2));
    tn_regs_enq(TINGPU_DST_CSC3, reg_val);

    reg_val = (TINGPU_DST_CSC4_C3(c3)) |
              (TINGPU_DST_CSC4_D1(d1));
    tn_regs_enq(TINGPU_DST_CSC4, reg_val);

    reg_val = (TINGPU_DST_CSC5_D2(d2)) |
              (TINGPU_DST_CSC5_D3(d3));
    tn_regs_enq(TINGPU_DST_CSC5, reg_val);
}



void
tn_regs_set_dst_color_format(
    const tn_gbuf *imgI,
    tn_u32         en_dst_fill,
    tn_u32         src_yuv)
{
    tn_color colI;
    tnEXPECT(!tn_is_null(imgI));
    tn_color_copy(imgI->color_format, &colI);
    if (tn_fmt_is_vglite(imgI->color_format) && g_img_ctx.gamma_en)
    {
        colI.colorspace = (colI.colorspace & ~(tn_color_space)TN_CS_NONLINEAR) | g_img_ctx.gamma_cs;
    }
    tn_reg_32 reg_val = 0;

    // YUV source images are derived from sRGB images, gamma-correction is
    // already applied. Do not re-apply gamma on the destination.
    if (src_yuv)
        colI.colorspace = (colI.colorspace & ~(tn_color_space)TN_CS_NONLINEAR);

    reg_val = tn_regs_set_fmt_type(imgI, DST) |
              tn_regs_set_word_width(imgI, colI, DST) |
              tn_regs_set_premul(colI, DST) |
              tn_regs_set_gamma(colI, DST) |
              tn_regs_en_dst_fill(en_dst_fill);

    tn_regs_enq(TINGPU_DST_CTRL, reg_val);

    tn_regs_set_swizzel(colI, DST);

    tn_regs_set_color_width(colI, DST);

    if (tn_fmt_is_yuv(imgI->color_format))
    {
        tn_regs_set_dst_csc();
    }
}



void
tn_regs_set_dst_fill(
    const tn_gbuf *dstI,
    tn_u32         color,
    tn_u32         w,
    tn_u32         h)
{
    tn_color colI;
    tnEXPECT(!tn_is_null(dstI));
    tn_color_copy(dstI->color_format, &colI);

    tn_regs_set_size(w, h, DST);
    tn_regs_set_dst_fill_color(color);
    tn_regs_enq(TINGPU_DST_STRIDE0, dstI->stride);

    tn_regs_enq(TINGPU_DST_CTRL, tn_regs_set_fmt_type(dstI, DST) |
                tn_regs_set_word_width(dstI, colI, DST) |
                tn_regs_set_premul(colI, DST) |
                TINGPU_DST_CTRL_FILL_EN_MASK |
                tn_regs_set_gamma(colI, DST));

    tn_regs_set_swizzel(colI, DST);
    tn_regs_set_color_width(colI, DST);

    tn_regs_set_dst_color_format(dstI, 0, tn_fmt_is_yuv(dstI->color_format));
    tn_regs_disable_s1();
    tn_regs_disable_s2();
}



void
tn_regs_set_s1_csc(tn_i32 conv_standard)
{
    tn_reg_32 reg_val = 0;
    tn_reg_32 ycbcr_mode = 0, y_offset = 0, uv_offset = 0;
    tn_reg_32 c0 = 0, c1 = 0, c2 = 0, c3 = 0, c4 = 0;

    switch (conv_standard)
    {
    case TN_YUV601:
        ycbcr_mode = 1;
        y_offset   = 0x1F0;
        uv_offset  = 0x180;
        c0         = 0x12A;
        c1         = 0x198;
        c2         = 0x730;
        c3         = 0x79C;
        c4         = 0x204;
        break;
    case TN_YUV709:
        ycbcr_mode = 1;
        y_offset   = 0x1F0;
        uv_offset  = 0x180;
        c0         = 0x12A;
        c1         = 0x1CB;
        c2         = 0x778;
        c3         = 0x7C9;
        c4         = 0x21C;
        break;
    case TN_YUV470:
        ycbcr_mode = 0;
        y_offset   = 0x000;
        uv_offset  = 0x000;
        c0         = 0x100;
        c1         = 0x123;
        c2         = 0x76B;
        c3         = 0x79B;
        c4         = 0x208;
        break;
    default:
        tnABORT(TN_FAULT_UNSUPPORTED, "Unsupported conversion standard");
    }

    reg_val = (TINGPU_S1_CSC0_YCBCR_MODE(ycbcr_mode)) |
              (TINGPU_S1_CSC0_Y_OFFSET(y_offset)) |
              (TINGPU_S1_CSC0_UV_OFFSET(uv_offset)) |
              (TINGPU_S1_CSC0_C0(c0));
    tn_regs_enq(TINGPU_S1_CSC0, reg_val);

    reg_val = (TINGPU_S1_CSC1_C1(c1)) |
              (TINGPU_S1_CSC1_C4(c4));
    tn_regs_enq(TINGPU_S1_CSC1, reg_val);

    reg_val = (TINGPU_S1_CSC2_C2(c2)) |
              (TINGPU_S1_CSC2_C3(c3));
    tn_regs_enq(TINGPU_S1_CSC2, reg_val);
}



void
tn_regs_set_s1_color_format(const tn_gbuf *imgI)
{
    tn_color colI;
    tnEXPECT(!tn_is_null(imgI));
    tn_color_copy(imgI->color_format, &colI);
    if (tn_fmt_is_vglite(imgI->color_format) && g_img_ctx.gamma_en)
    {
        colI.colorspace = (colI.colorspace & ~(tn_color_space)TN_CS_NONLINEAR) | g_img_ctx.gamma_cs;
    }
    tn_reg_32 reg_val = 0;

    reg_val = TINGPU_S1_CTRL_EN(1) |
              tn_regs_set_fmt_type(imgI, S1) |
              tn_regs_set_word_width(imgI, colI, S1) |
              TINGPU_S1_CTRL_ONE_TILE(1) |
              tn_regs_set_premul(colI, S1) |
              tn_regs_set_gamma(colI, S1);

    tn_regs_enq(TINGPU_S1_CTRL, reg_val);

    tn_regs_set_swizzel(colI, S1);

    tn_regs_set_color_width(colI, S1);

    if (tn_fmt_is_yuv(imgI->color_format))
    {
        tn_regs_set_s1_csc(imgI->yuv2rgb);
    }
}



void
tn_regs_set_s2_csc(tn_i32 conv_standard)
{
    tn_reg_32 reg_val = 0;
    tn_reg_32 ycbcr_mode = 0, y_offset = 0, uv_offset = 0;
    tn_reg_32 c0 = 0, c1 = 0, c2 = 0, c3 = 0, c4 = 0;

    switch (conv_standard)
    {
    case TN_YUV601:
        ycbcr_mode = 1;
        y_offset   = 0x1F0;
        uv_offset  = 0x180;
        c0         = 0x12A;
        c1         = 0x198;
        c2         = 0x730;
        c3         = 0x79C;
        c4         = 0x204;
        break;
    case TN_YUV709:
        ycbcr_mode = 1;
        y_offset   = 0x1F0;
        uv_offset  = 0x180;
        c0         = 0x12A;
        c1         = 0x1CB;
        c2         = 0x778;
        c3         = 0x7C9;
        c4         = 0x21C;
        break;
    case TN_YUV470:
        ycbcr_mode = 0;
        y_offset   = 0x000;
        uv_offset  = 0x000;
        c0         = 0x100;
        c1         = 0x123;
        c2         = 0x76B;
        c3         = 0x79B;
        c4         = 0x208;
        break;
    default:
        tnABORT(TN_FAULT_UNSUPPORTED, "Unsupported conversion standard");
    }

    reg_val = (TINGPU_S2_CSC0_YCBCR_MODE(ycbcr_mode)) |
              (TINGPU_S2_CSC0_Y_OFFSET(y_offset)) |
              (TINGPU_S2_CSC0_UV_OFFSET(uv_offset)) |
              (TINGPU_S2_CSC0_C0(c0));
    tn_regs_enq(TINGPU_S2_CSC0, reg_val);

    reg_val = (TINGPU_S2_CSC1_C1(c1)) |
              (TINGPU_S2_CSC1_C4(c4));
    tn_regs_enq(TINGPU_S2_CSC1, reg_val);

    reg_val = (TINGPU_S2_CSC2_C2(c2)) |
              (TINGPU_S2_CSC2_C3(c3));
    tn_regs_enq(TINGPU_S2_CSC2, reg_val);
}



void
tn_regs_set_s2_color_format(
    const tn_gbuf *imgI,
    tn_u32         quality)
{
    tn_color colI;
    tnEXPECT(!tn_is_null(imgI));
    tn_color_copy(imgI->color_format, &colI);
    if (tn_fmt_is_vglite(imgI->color_format) && g_img_ctx.gamma_en)
    {
        colI.colorspace = (colI.colorspace & ~(tn_color_space)TN_CS_NONLINEAR) | g_img_ctx.gamma_cs;
    }
    tn_reg_32 reg_val = 0;

    reg_val = TINGPU_S2_CTRL_EN(1) |
              tn_regs_set_fmt_type(imgI, S2) |
              tn_regs_set_word_width(imgI, colI, S2) |
              tn_regs_set_premul(colI, S2) |
              tn_regs_set_gamma(colI, S2) |
              (TINGPU_S2_CTRL_RECT_MODE(TN_RECT_MODE_OOB)) |
              (TINGPU_S2_CTRL_IMAGE_QUALITY(tn_fmt_quality_to_reg_value(quality)));

    tn_regs_enq(TINGPU_S2_CTRL, reg_val);

    tn_regs_set_swizzel(colI, S2);

    tn_regs_set_color_width(colI, S2);

    if (tn_fmt_is_yuv(imgI->color_format))
    {
        tn_regs_set_s2_csc(imgI->yuv2rgb);
    }
}



void
tn_regs_enable_signed_fmt(
    tn_u8  fmt,
    tn_u32 scale,
    tn_u16 zero_point)
{
    tn_reg_32 reg_val;

    //set "output_signed_en" bit
    reg_val = TINGPU_DST_SIGN0_OUTPUT_SIGNED_EN(1);
    //set signed zero_point value
    reg_val |= TINGPU_DST_SIGN0_ZERO_POINT(zero_point);
    tn_regs_enq(TINGPU_DST_SIGN0, reg_val);

    tn_u32 inv_scale_factor = (fmt == TN_16BIT_SIGNED_FMT) ? 262144 : 1023;

    reg_val = TINGPU_DST_SIGN1_INV_SCALE_INT(lround(inv_scale_factor / scale));
    if (scale == 1) // no scaling use case
        reg_val = TINGPU_DST_SIGN1_INV_SCALE_INT(lround(inv_scale_factor / scale - 1));
    tn_regs_enq(TINGPU_DST_SIGN1, reg_val);
}



void
tn_regs_disable_signed_fmt()
{
    tn_regs_enq(TINGPU_DST_SIGN0, 0);
}



void
tn_regs_disable_crt_scissor(tn_u8 id)
{
    // id arrives as a raw integer, so range check it before it is used to
    // index the register array.
    tnEXPECT(id < SCISSOR_NUM);
    tn_regs_enq(TINGPU_S2_SCISSOR_CTRL_IDX(id), 0);
}



void
tn_regs_disable_scissors()
{
    for (int i = 0; i < SCISSOR_NUM; ++i)
        tn_regs_enq(TINGPU_S2_SCISSOR_CTRL_IDX(i), 0);
}



void
tn_regs_set_scissor(
    tn_scissor_id id,
    tn_u16        start_x,
    tn_u16        start_y,
    tn_u16        width,
    tn_u16        height)
{
    tn_regs_enq(TINGPU_S2_SCISSOR_CTRL_IDX(id),
                TINGPU_S2_SCISSOR_CTRL_SCISSOR_EN(1) |
                TINGPU_S2_SCISSOR_CTRL_MINX(start_x) |
                TINGPU_S2_SCISSOR_CTRL_MINY(start_y));

    tn_regs_enq(TINGPU_S2_SCISSOR_SIZE_IDX(id),
                TINGPU_S2_SCISSOR_SIZE_WIDTH(width) |
                TINGPU_S2_SCISSOR_SIZE_HEIGHT(height));
}



void
tn_regs_set_size(
    tn_reg_32          width,
    tn_reg_32          height,
    const tn_chan_type ch)
{
    switch (ch)
    {
    case S1:
        tn_regs_enq(TINGPU_S1_SIZE, TINGPU_S1_SIZE_WIDTH(width) | TINGPU_S1_SIZE_HEIGHT(height));
        break;
    case S2:
        tn_regs_enq(TINGPU_S2_SIZE, TINGPU_S2_SIZE_WIDTH(width) | TINGPU_S2_SIZE_HEIGHT(height));
        break;
    case DST:
        tn_regs_enq(TINGPU_DST_SIZE, TINGPU_DST_SIZE_WIDTH(width) | TINGPU_DST_SIZE_HEIGHT(height));
        break;
    case MASK:
        tnABORT(TN_FAULT_UNSUPPORTED, "MASK has no SIZE register");
    }
}



void
tn_regs_set_swizzel(
    tn_color           colI,
    const tn_chan_type ch)
{
    switch (ch)
    {
    case S1:
        tn_regs_enq(TINGPU_S1_COLOR_SHIFT,
                    TINGPU_S1_COLOR_SHIFT_R_SHIFT(colI.swz.r_shift) |
                    TINGPU_S1_COLOR_SHIFT_G_SHIFT(colI.swz.g_shift) |
                    TINGPU_S1_COLOR_SHIFT_B_SHIFT(colI.swz.b_shift) |
                    TINGPU_S1_COLOR_SHIFT_A_SHIFT(colI.swz.a_shift));
        break;
    case S2:
        tn_regs_enq(TINGPU_S2_COLOR_SHIFT,
                    TINGPU_S2_COLOR_SHIFT_R_SHIFT(colI.swz.r_shift) |
                    TINGPU_S2_COLOR_SHIFT_G_SHIFT(colI.swz.g_shift) |
                    TINGPU_S2_COLOR_SHIFT_B_SHIFT(colI.swz.b_shift) |
                    TINGPU_S2_COLOR_SHIFT_A_SHIFT(colI.swz.a_shift));
        break;
    case DST:
        tn_regs_enq(TINGPU_DST_COLOR_SHIFT,
                    TINGPU_DST_COLOR_SHIFT_R_SHIFT(colI.swz.r_shift) |
                    TINGPU_DST_COLOR_SHIFT_G_SHIFT(colI.swz.g_shift) |
                    TINGPU_DST_COLOR_SHIFT_B_SHIFT(colI.swz.b_shift) |
                    TINGPU_DST_COLOR_SHIFT_A_SHIFT(colI.swz.a_shift));
        break;
    case MASK:
        tnABORT(TN_FAULT_UNSUPPORTED, "MASK has no COLOR_SHIFT register");
    }
}



void
tn_regs_set_color_width(
    tn_color           colI,
    const tn_chan_type ch)
{
    switch (ch)
    {
    case S1:
        tn_regs_enq(TINGPU_S1_COLOR_BITS,
                    TINGPU_S1_COLOR_BITS_R_WIDTH(colI.swz.r_bits) |
                    TINGPU_S1_COLOR_BITS_G_WIDTH(colI.swz.g_bits) |
                    TINGPU_S1_COLOR_BITS_B_WIDTH(colI.swz.b_bits) |
                    TINGPU_S1_COLOR_BITS_A_WIDTH(colI.swz.a_bits));
        break;
    case S2:
        tn_regs_enq(TINGPU_S2_COLOR_BITS,
                    TINGPU_S2_COLOR_BITS_R_WIDTH(colI.swz.r_bits) |
                    TINGPU_S2_COLOR_BITS_G_WIDTH(colI.swz.g_bits) |
                    TINGPU_S2_COLOR_BITS_B_WIDTH(colI.swz.b_bits) |
                    TINGPU_S2_COLOR_BITS_A_WIDTH(colI.swz.a_bits));
        break;
    case DST:
        tn_regs_enq(TINGPU_DST_COLOR_BITS,
                    TINGPU_DST_COLOR_BITS_R_WIDTH(colI.swz.r_bits) |
                    TINGPU_DST_COLOR_BITS_G_WIDTH(colI.swz.g_bits) |
                    TINGPU_DST_COLOR_BITS_B_WIDTH(colI.swz.b_bits) |
                    TINGPU_DST_COLOR_BITS_A_WIDTH(colI.swz.a_bits));
        break;
    case MASK:
        tnABORT(TN_FAULT_UNSUPPORTED, "MASK has no COLOR_BITS register");
    }
}



tn_reg_32
tn_regs_set_fmt_type(
    const tn_gbuf      *imgI,
    const tn_chan_type  ch)
{
    tn_u32 type1, type2;
    tnEXPECT(!tn_is_null(imgI));

    if (tn_fmt_is_indexed(imgI->color_format))
    {
        type1 = TN_REGS_IMAGE_FMT_TYPE1_INDEX;
        type2 = tn_fmt_get_lut_width(imgI->color_format);
    }
    else if (tn_fmt_is_yuv(imgI->color_format))
    {
        type1 = TN_REGS_IMAGE_FMT_TYPE1_YUV;
        type2 = tn_fmt_get_yuv_type(imgI->color_format);
    }
    else if (tn_fmt_is_compressed(imgI->color_format))
    {
        type1 = TN_REGS_IMAGE_FMT_TYPE1_COMPRESSED;
        type2 = tn_fmt_get_compressed_type(imgI->color_format);
    }
    else
    {
        type1 = TN_REGS_IMAGE_FMT_TYPE1_RGBA;
        if (imgI->color_format == TN_sL_8 ||
            imgI->color_format == TN_lL_8 ||
            imgI->color_format == TN_L_8 ||
            imgI->color_format == TN_L_1 ||
            imgI->color_format == TN_lL_1)
        {
            type2 = TN_REGS_IMAGE_FMT_TYPE2_GRAY8;
        }
        else if (imgI->color_format == TN_lL_16_LE || imgI->color_format == TN_L_16_LE)
        {
            type2 = TN_REGS_IMAGE_FMT_TYPE2_GRAY16;
        }
        else if (tn_fmt_is_rgb_a_planar(imgI->color_format))
        {
            type2 = TN_REGS_IMAGE_FMT_TYPE2_RGB_A;
        }
        else
        {
            type2 = TN_REGS_IMAGE_FMT_TYPE2_PACKED;
        }
    }

    switch (ch)
    {
    case S1:
        return TINGPU_S1_CTRL_TYPE1(type1) | TINGPU_S1_CTRL_TYPE2(type2);
    case S2:
        return TINGPU_S2_CTRL_TYPE1(type1) | TINGPU_S2_CTRL_TYPE2(type2);
    case DST:
        return TINGPU_DST_CTRL_TYPE1(type1) | TINGPU_DST_CTRL_TYPE2(type2);
    case MASK:
        tnABORT(TN_FAULT_UNSUPPORTED, "MASK has no CTRL TYPE1/TYPE2 fields");
    }
    return 0;
}



tn_reg_32
tn_regs_set_premul(
    tn_color           colI,
    const tn_chan_type ch)
{
    tn_u32 premul = tn_fmt_is_premul(colI);

    switch (ch)
    {
    case S1:
        return TINGPU_S1_CTRL_PREMULTIPLY(premul);
    case S2:
        return TINGPU_S2_CTRL_PREMULTIPLY(premul);
    case DST:
        return TINGPU_DST_CTRL_PREMULTIPLY(premul);
    case MASK:
        tnABORT(TN_FAULT_UNSUPPORTED, "MASK has no CTRL PREMULTIPLY field");
    }
    return 0;
}



tn_reg_32
tn_regs_set_gamma(
    tn_color           colI,
    const tn_chan_type ch)
{
    // Force gamma off for TN_L_16_LE for GPU versions without gray16 gamma LUT
    if ((g_img_ctx.gpu_major == 2) && (colI.format == TN_L_16_LE))
        return 0;

    tn_u32 gamma = tn_fmt_is_gamma(colI);

    switch (ch)
    {
    case S1:
        return TINGPU_S1_CTRL_GAMMA(gamma);
    case S2:
        return TINGPU_S2_CTRL_GAMMA(gamma);
    case DST:
        return TINGPU_DST_CTRL_GAMMA(gamma);
    case MASK:
        tnABORT(TN_FAULT_UNSUPPORTED, "MASK has no CTRL GAMMA field");
    }
    return 0;
}



tn_reg_32
tn_regs_set_word_width(
    const tn_gbuf      *imgI,
    tn_color            colI,
    const tn_chan_type  ch)
{
    tn_u8 fmt_word_width;

    tnEXPECT(!tn_is_null(imgI));
    if (tn_fmt_is_compressed(imgI->color_format))
    {
        fmt_word_width = 0;
    }
    else if (tn_fmt_is_yuv(imgI->color_format))
    {
        if (tn_fmt_is_yuv_alpha(imgI->color_format))
            fmt_word_width = TN_REGS_IMAGE_FMT_WORD_WIDTH_32;
        else
            fmt_word_width = TN_REGS_IMAGE_FMT_WORD_WIDTH_24;
    }
    else
    {
        fmt_word_width = tn_fmt_get_image_word_width(colI);
    }

    switch (ch)
    {
    case S1:
        return TINGPU_S1_CTRL_WORD_WIDTH(fmt_word_width);
    case S2:
        return TINGPU_S2_CTRL_WORD_WIDTH(fmt_word_width);
    case DST:
        return TINGPU_DST_CTRL_WORD_WIDTH(fmt_word_width);
    case MASK:
        tnABORT(TN_FAULT_UNSUPPORTED, "MASK has no CTRL WORD_WIDTH field");
    }
    return 0;
}



void
tn_regs_enable_color_transform(void)
{
    tn_regs_enq(TINGPU_COLOR_TRANSFORM_CTRL,
                TINGPU_COLOR_TRANSFORM_CTRL_CT_EN(1));
}



void
tn_regs_disable_color_transform(void)
{
    tn_regs_enq(TINGPU_COLOR_TRANSFORM_CTRL,
                TINGPU_COLOR_TRANSFORM_CTRL_CT_EN(0));
}



void
tn_regs_set_color_transform(tn_color_transform *colorX)
{
    tn_reg_32 reg_val;
    tn_u16 scale, bias;
    reg_val = 0;
    scale = S7Q8(colorX->r_scale);
    bias  = S0Q8_sat(colorX->r_bias);

    reg_val |= TINGPU_COLOR_TRANSFORM0_SCALE_R(scale & 0xFFFF);
    reg_val |= TINGPU_COLOR_TRANSFORM0_BIAS_R(bias & 0xFFFF);
    tn_regs_enq(TINGPU_COLOR_TRANSFORM0, reg_val);

    reg_val = 0;
    scale = S7Q8(colorX->g_scale);
    bias  = S0Q8_sat(colorX->g_bias);
    reg_val |= TINGPU_COLOR_TRANSFORM1_SCALE_G(scale & 0xFFFF);
    reg_val |= TINGPU_COLOR_TRANSFORM1_BIAS_G(bias & 0xFFFF);
    tn_regs_enq(TINGPU_COLOR_TRANSFORM1, reg_val);

    reg_val = 0;
    scale = S7Q8(colorX->b_scale);
    bias  = S0Q8_sat(colorX->b_bias);
    reg_val |= TINGPU_COLOR_TRANSFORM2_SCALE_B(scale & 0xFFFF);
    reg_val |= TINGPU_COLOR_TRANSFORM2_BIAS_B(bias & 0xFFFF);
    tn_regs_enq(TINGPU_COLOR_TRANSFORM2, reg_val);

    reg_val = 0;
    scale = S7Q8(colorX->a_scale);
    bias  = S0Q8_sat(colorX->a_bias);
    reg_val |= TINGPU_COLOR_TRANSFORM3_SCALE_A(scale & 0xFFFF);
    reg_val |= TINGPU_COLOR_TRANSFORM3_BIAS_A(bias & 0xFFFF);
    tn_regs_enq(TINGPU_COLOR_TRANSFORM3, reg_val);
}



void
tn_regs_set_blending_mode(
    tn_i32  blend,
    tn_bool S2_supported_chan)
{
    tn_u8 src_alpha_mode, dst_alpha_mode, src_alpha_value, dst_alpha_value;
    tn_chan_type supported_chan = S1;
    blend = blend & TN_BLEND_MODE_MASK;
    // TN_BLEND_MODE_MASK (0x0F) admits values 0..15, but cfg[] only holds
    // entries 0..14 (up to TN_BLEND_SPECIAL). Value 15 is undefined and would
    // index one past the end, so clamp it to the default blend mode to keep
    // the cfg[blend] accesses below in bounds.
    if (blend >= (tn_i32)(sizeof(blend_modes[0].cfg) / sizeof(blend_modes[0].cfg[0])))
    {
        blend = TN_BLEND_SRC;
    }

    if (S2_supported_chan)
    {
        supported_chan = S2;
        src_alpha_mode  = g_img_ctx.src_alpha_mode;
        src_alpha_value = g_img_ctx.src_alpha_value;
        dst_alpha_mode  = g_img_ctx.dst_alpha_mode;
        dst_alpha_value = g_img_ctx.dst_alpha_value;
    }
    else
    {
        src_alpha_mode  = g_img_ctx.dst_alpha_mode;
        src_alpha_value = g_img_ctx.dst_alpha_value;
        dst_alpha_mode  = g_img_ctx.src_alpha_mode;
        dst_alpha_value = g_img_ctx.src_alpha_value;
    }

    tn_regs_enq(TINGPU_ALPHA_CTRL,
                (TINGPU_ALPHA_CTRL_S1_GLOBAL_ALPHA_MODE(dst_alpha_mode)) |
                (TINGPU_ALPHA_CTRL_S1_GLOBAL_ALPHA(dst_alpha_value)) |
                (TINGPU_ALPHA_CTRL_S2_GLOBAL_ALPHA_MODE(src_alpha_mode)) |
                (TINGPU_ALPHA_CTRL_S2_GLOBAL_ALPHA(src_alpha_value)));

    tn_reg_32 blend_ctrl_val =
                (TINGPU_BLEND_CTRL_BLENDING_MODE(blend_modes[supported_chan].cfg[blend].mode)) |
                (TINGPU_BLEND_CTRL_ADDITIONAL_MODE(blend_modes[supported_chan].cfg[blend].additional)) |
                (TINGPU_BLEND_CTRL_S1_FACTOR_MODE(blend_modes[supported_chan].cfg[blend].s1_factor)) |
                (TINGPU_BLEND_CTRL_S2_FACTOR_MODE(blend_modes[supported_chan].cfg[blend].s2_factor));

    tn_regs_enq(TINGPU_BLEND_CTRL, blend_ctrl_val);
}



void
tn_regs_set_xform_mode(tn_reg_32 mode)
{
    tn_regs_enq(TINGPU_S2_TRANSFORM_CTRL,
                TINGPU_S2_TRANSFORM_CTRL_MODE(mode));
}



void
tn_regs_set_xform(tn_m3f32 *mat)
{
    tnEXPECT(!tn_is_null(mat));
    const uint8_t NUM_ELEMENTS = 9; // 9 elements in matrix
    tn_reg_addrs_list reg_addrs = tn_reg_addrs_list_create_a(tn_mspace_realloc,
                                                             TN_MSPACE_SHMEM_0);
    tn_reg_values_list reg_values = tn_reg_values_list_create_a(tn_mspace_realloc,
                                                                TN_MSPACE_SHMEM_0);
    // tn_dynarr_reserve returns 0 when the backing allocation fails. This
    // function returns void, so bail out early rather than pushing into
    // under-sized buffers below.
    if (!tn_dynarr_reserve(&reg_addrs.dynarr, NUM_ELEMENTS) ||
        !tn_dynarr_reserve(&reg_values.dynarr, NUM_ELEMENTS))
    {
        tnLOG_ERR("out-of-memory");
        return;
    }

    union { uint32_t   u; tn_f32 f; } uf;
    tn_reg_32 reg_name;

    uf.f = mat->m[0][0];
    reg_name = TINGPU_S2_PROJ_TF0;
    tn_dynarr_push_back(&reg_addrs.dynarr, &reg_name);
    tn_dynarr_push_back(&reg_values.dynarr, &uf.u);

    uf.f = mat->m[0][1];
    reg_name = TINGPU_S2_PROJ_TF1;
    tn_dynarr_push_back(&reg_addrs.dynarr, &reg_name);
    tn_dynarr_push_back(&reg_values.dynarr, &uf.u);

    uf.f = mat->m[0][2];
    reg_name = TINGPU_S2_PROJ_TF2;
    tn_dynarr_push_back(&reg_addrs.dynarr, &reg_name);
    tn_dynarr_push_back(&reg_values.dynarr, &uf.u);

    uf.f = mat->m[1][0];
    reg_name = TINGPU_S2_PROJ_TF3;
    tn_dynarr_push_back(&reg_addrs.dynarr, &reg_name);
    tn_dynarr_push_back(&reg_values.dynarr, &uf.u);

    uf.f = mat->m[1][1];
    reg_name = TINGPU_S2_PROJ_TF4;
    tn_dynarr_push_back(&reg_addrs.dynarr, &reg_name);
    tn_dynarr_push_back(&reg_values.dynarr, &uf.u);

    uf.f = mat->m[1][2];
    reg_name = TINGPU_S2_PROJ_TF5;
    tn_dynarr_push_back(&reg_addrs.dynarr, &reg_name);
    tn_dynarr_push_back(&reg_values.dynarr, &uf.u);

    uf.f = mat->m[2][0];
    reg_name = TINGPU_S2_PROJ_TF6;
    tn_dynarr_push_back(&reg_addrs.dynarr, &reg_name);
    tn_dynarr_push_back(&reg_values.dynarr, &uf.u);

    uf.f = mat->m[2][1];
    reg_name = TINGPU_S2_PROJ_TF7;
    tn_dynarr_push_back(&reg_addrs.dynarr, &reg_name);
    tn_dynarr_push_back(&reg_values.dynarr, &uf.u);

#ifdef TN_DQ_CT
    uf.f = 0.5 * (mat->m[2][0] + mat->m[2][1]) + mat->m[2][2];
#else
    uf.f = mat->m[2][2];
#endif
    reg_name = TINGPU_S2_PROJ_TF8;
    tn_dynarr_push_back(&reg_addrs.dynarr, &reg_name);
    tn_dynarr_push_back(&reg_values.dynarr, &uf.u);

    // Read the backing storage through the .dynarr member (the same union
    // member written above via tn_dynarr_push_back) so the union is
    // accessed consistently. dynarr.range.ptr aliases the typed .arr view.
    tn_cmd_buf_ccdma_loadNamedRegValues(NUM_ELEMENTS,
                                        (tn_u32*)reg_addrs.dynarr.range.ptr,
                                        (tn_u32*)reg_values.dynarr.range.ptr);

    tn_dynarr_destroy(&reg_addrs.dynarr);
    tn_dynarr_destroy(&reg_values.dynarr);
}



tn_size
tn_regs_set_stride_and_addr(
    tn_u32              base_addr,
    tn_u32              offset_x,
    tn_u32              offset_y,
    const tn_gbuf      *imgI,
    tn_color            colI,
    const tn_chan_type  ch)
{
    tnEXPECT(!tn_is_null(imgI));
    //size (in bytes) for each plane
    tn_size y, u, v, a;
    y = u = v = a = 0;

    tn_u32 offset;

    if (tn_fmt_is_compressed(imgI->color_format))
    {
        y = imgI->stride * imgI->height;
        tn_u32 block_x = 0, block_y = 0;
        if (tn_fmt_is_IBC8x8_compressed(imgI->color_format))
        {
            block_x = (offset_x >> 3) * (64 * colI.bpp >> 3);
            block_y = offset_y * imgI->stride;
        }

        offset = block_y + block_x;
        tn_regs_set_base_addr(base_addr + offset, ch);
        tn_regs_enq(ch_regs[ch].stride.reg[0], imgI->stride << ch_regs[ch].stride.reg_shift);
    }
    else if (tn_fmt_is_yuv(imgI->color_format))
    {
        y = imgI->width * imgI->height;
        offset = offset_y * imgI->width + offset_x;
        switch (imgI->color_format)
        {
        case TN_NV12:
        case TN_NV21:
            u = v = y / 4;
            tn_regs_set_base_addr(base_addr + offset, ch);
            offset = (offset_y >> 1) * imgI->width + offset_x;
            tn_regs_enq(ch_regs[ch].addr.reg[1], (base_addr + y + offset) << ch_regs[ch].addr.reg_shift);
            tn_regs_enq(ch_regs[ch].stride.reg[0], imgI->width << ch_regs[ch].stride.reg_shift);
            tn_regs_enq(ch_regs[ch].stride.reg[1], (imgI->width >> 1) << ch_regs[ch].stride.reg_shift);
            break;
        case TN_NV16:
        case TN_NV61:
            u = v = y / 2;
            tn_regs_set_base_addr(base_addr + offset, ch);
            tn_regs_enq(ch_regs[ch].addr.reg[1], (base_addr + y + offset) << ch_regs[ch].addr.reg_shift);
            tn_regs_enq(ch_regs[ch].stride.reg[0], imgI->width << ch_regs[ch].stride.reg_shift);
            tn_regs_enq(ch_regs[ch].stride.reg[1], imgI->width << ch_regs[ch].stride.reg_shift);
            break;
        case TN_NV24:
        case TN_NV42:
            u = v = y;
            tn_regs_set_base_addr(base_addr + offset, ch);
            offset = offset_y * (imgI->width << 1) + (offset_x << 1);
            tn_regs_enq(ch_regs[ch].addr.reg[1], (base_addr + y + offset) << ch_regs[ch].addr.reg_shift);
            tn_regs_enq(ch_regs[ch].stride.reg[0], imgI->width << ch_regs[ch].stride.reg_shift);
            tn_regs_enq(ch_regs[ch].stride.reg[1], (imgI->width << 1) << ch_regs[ch].stride.reg_shift);
            break;
        case TN_YV12:
        case TN_YU12:
            u = y / 4;
            v = y / 4;
            tn_regs_set_base_addr(base_addr + offset, ch);
            offset = (offset_y >> 1) * (imgI->width >> 1) + (offset_x >> 1);
            tn_regs_enq(ch_regs[ch].addr.reg[1], (base_addr + y + offset) << ch_regs[ch].addr.reg_shift);
            tn_regs_enq(ch_regs[ch].addr.reg[2], (base_addr + y + u + offset) << ch_regs[ch].addr.reg_shift);
            tn_regs_enq(ch_regs[ch].stride.reg[0], imgI->width << ch_regs[ch].stride.reg_shift);
            tn_regs_enq(ch_regs[ch].stride.reg[1], (imgI->width >> 2) << ch_regs[ch].stride.reg_shift);
            tn_regs_enq(ch_regs[ch].stride.reg[2], (imgI->width >> 2) << ch_regs[ch].stride.reg_shift);
            break;
        case TN_AYU12:
            u = y / 4;
            v = y / 4;
            a = y;
            //y plane
            tn_regs_set_base_addr(base_addr + offset, ch);
            //alpha plane
            tn_regs_enq(ch_regs[ch].addr.reg[3], (base_addr + y + u + v + offset) << ch_regs[ch].addr.reg_shift);
            //offset inside u & v planes
            offset = (offset_y >> 1) * (imgI->width >> 1) + (offset_x >> 1);
            tn_regs_enq(ch_regs[ch].addr.reg[1], (base_addr + y + offset) << ch_regs[ch].addr.reg_shift);
            tn_regs_enq(ch_regs[ch].addr.reg[2], (base_addr + y + u + offset) << ch_regs[ch].addr.reg_shift);
            tn_regs_enq(ch_regs[ch].stride.reg[0], imgI->width << ch_regs[ch].stride.reg_shift);
            tn_regs_enq(ch_regs[ch].stride.reg[1], (imgI->width >> 2) << ch_regs[ch].stride.reg_shift);
            tn_regs_enq(ch_regs[ch].stride.reg[2], (imgI->width >> 2) << ch_regs[ch].stride.reg_shift);
            tn_regs_enq(ch_regs[ch].stride.reg[3], imgI->width << ch_regs[ch].stride.reg_shift);
            break;
        case TN_YV16:
        case TN_YU16:
            u = y / 2;
            v = y / 2;
            tn_regs_set_base_addr(base_addr + offset, ch);
            //offset inside u & v planes
            offset = offset_y * (imgI->width >> 1) + (offset_x >> 1);
            tn_regs_enq(ch_regs[ch].addr.reg[1], (base_addr + y + offset) << ch_regs[ch].addr.reg_shift);
            tn_regs_enq(ch_regs[ch].addr.reg[2], (base_addr + y + u + offset) << ch_regs[ch].addr.reg_shift);
            tn_regs_enq(ch_regs[ch].stride.reg[0], imgI->width << ch_regs[ch].stride.reg_shift);
            tn_regs_enq(ch_regs[ch].stride.reg[1], (imgI->width >> 1) << ch_regs[ch].stride.reg_shift);
            tn_regs_enq(ch_regs[ch].stride.reg[2], (imgI->width >> 1) << ch_regs[ch].stride.reg_shift);
            break;
        case TN_AYU16:
            u = y / 2;
            v = y / 2;
            a = y;
            tn_regs_set_base_addr(base_addr + offset, ch);
            tn_regs_enq(ch_regs[ch].addr.reg[3], (base_addr + y + u + v + offset) << ch_regs[ch].addr.reg_shift);
            //offset inside u & v planes
            offset = offset_y * (imgI->width >> 1) + (offset_x >> 1);
            tn_regs_enq(ch_regs[ch].addr.reg[1], (base_addr + y + offset) << ch_regs[ch].addr.reg_shift);
            tn_regs_enq(ch_regs[ch].addr.reg[2], (base_addr + y + u + offset) << ch_regs[ch].addr.reg_shift);
            tn_regs_enq(ch_regs[ch].stride.reg[0], imgI->width << ch_regs[ch].stride.reg_shift);
            tn_regs_enq(ch_regs[ch].stride.reg[1], (imgI->width >> 1) << ch_regs[ch].stride.reg_shift);
            tn_regs_enq(ch_regs[ch].stride.reg[2], (imgI->width >> 1) << ch_regs[ch].stride.reg_shift);
            tn_regs_enq(ch_regs[ch].stride.reg[3], imgI->width << ch_regs[ch].stride.reg_shift);
            break;
        case TN_YV24:
        case TN_YU24:
            v = u = y;
            tn_regs_set_base_addr(base_addr + offset, ch);
            tn_regs_enq(ch_regs[ch].addr.reg[1], (base_addr + y + offset) << ch_regs[ch].addr.reg_shift);
            tn_regs_enq(ch_regs[ch].addr.reg[2], (base_addr + y + u + offset) << ch_regs[ch].addr.reg_shift);
            tn_regs_enq(ch_regs[ch].stride.reg[0], imgI->width << ch_regs[ch].stride.reg_shift);
            tn_regs_enq(ch_regs[ch].stride.reg[1], imgI->width << ch_regs[ch].stride.reg_shift);
            tn_regs_enq(ch_regs[ch].stride.reg[2], imgI->width << ch_regs[ch].stride.reg_shift);
            break;
        case TN_AYU24:
            a = v = u = y;
            tn_regs_set_base_addr(base_addr + offset, ch);
            tn_regs_enq(ch_regs[ch].addr.reg[1], (base_addr + y + offset) << ch_regs[ch].addr.reg_shift);
            tn_regs_enq(ch_regs[ch].addr.reg[2], (base_addr + y + u + offset) << ch_regs[ch].addr.reg_shift);
            tn_regs_enq(ch_regs[ch].addr.reg[3], (base_addr + y + u + v + offset) << ch_regs[ch].addr.reg_shift);
            tn_regs_enq(ch_regs[ch].stride.reg[0], imgI->width << ch_regs[ch].stride.reg_shift);
            tn_regs_enq(ch_regs[ch].stride.reg[1], imgI->width << ch_regs[ch].stride.reg_shift);
            tn_regs_enq(ch_regs[ch].stride.reg[2], imgI->width << ch_regs[ch].stride.reg_shift);
            tn_regs_enq(ch_regs[ch].stride.reg[3], imgI->width << ch_regs[ch].stride.reg_shift);
            break;

        //non-plannar Y'CbCr
        case TN_YUYV:
            u = v = y / 2;
            offset = (offset_y * imgI->width + offset_x) * colI.bpp >> 3;
            tn_regs_set_base_addr(base_addr + offset, ch);
            tn_regs_enq(ch_regs[ch].stride.reg[0], imgI->stride << ch_regs[ch].stride.reg_shift);
            break;
        case TN_YUV24:
            u = v = y;
            offset = (offset_y * imgI->width + offset_x) * colI.bpp >> 3;
            tn_regs_set_base_addr(base_addr + offset, ch);
            tn_regs_enq(ch_regs[ch].stride.reg[0], imgI->stride << ch_regs[ch].stride.reg_shift);
            break;
        case TN_AYUV32:
        case TN_VUYA32:
        case TN_YUVA32:
        case TN_AVUY32:
        case TN_XYUV32:
        case TN_VUYX32:
        case TN_YUVX32:
            u = v = a = y;
            offset = (offset_y * imgI->width + offset_x) * colI.bpp >> 3;
            tn_regs_set_base_addr(base_addr + offset, ch);
            tn_regs_enq(ch_regs[ch].stride.reg[0], imgI->stride << ch_regs[ch].stride.reg_shift);
            break;
        default:
            tnABORT(TN_FAULT_UNSUPPORTED, "Unsupported Y'UV format");
        } // switch

    }
    else if (tn_fmt_is_rgb_a_planar(imgI->color_format))
    {
        y = imgI->stride * imgI->height; //imgI->stride considers also alpha plane
        tn_u32 colors_plane_stride = imgI->width * (colI.bpp - 8) / 8;
        offset = offset_y * imgI->width + offset_x;
        tn_u32 offset_bytes = offset * (colI.bpp - 8) / 8;
        if (imgI->color_format == TN_sBGR_A8565 || imgI->color_format == TN_sRGB_A8565 ||
            imgI->color_format == TN_sBGR_A8888 || imgI->color_format == TN_sRGB_A8888 ||
            imgI->color_format == TN_lBGR_A8565 || imgI->color_format == TN_lRGB_A8565 ||
            imgI->color_format == TN_lBGR_A8888 || imgI->color_format == TN_lRGB_A8888 ||
            imgI->color_format == TN_BGR_A8565 || imgI->color_format == TN_RGB_A8565 ||
            imgI->color_format == TN_BGR_A8888 || imgI->color_format == TN_RGB_A8888) // alpha is the first plane
        {
            tn_regs_enq(ch_regs[ch].addr.reg[1], (base_addr + offset_bytes) << ch_regs[ch].addr.reg_shift);
            tn_regs_enq(ch_regs[ch].addr.reg[0], (base_addr + imgI->width * imgI->height + offset) << ch_regs[ch].addr.reg_shift);
        }
        else
        {
            tn_regs_enq(ch_regs[ch].addr.reg[0], (base_addr + offset_bytes) << ch_regs[ch].addr.reg_shift);
            tn_regs_enq(ch_regs[ch].addr.reg[1], (base_addr + colors_plane_stride * imgI->height + offset) << ch_regs[ch].addr.reg_shift);
        }
        tn_regs_enq(ch_regs[ch].stride.reg[0], colors_plane_stride << ch_regs[ch].stride.reg_shift);
        tn_regs_enq(ch_regs[ch].stride.reg[1], imgI->width << ch_regs[ch].stride.reg_shift);
    }
    else   /*non planar*/
    {
        y = imgI->stride * imgI->height;
        tn_u32 offset_bytes = (offset_y * imgI->stride + (offset_x * colI.bpp >> 3));
        tn_regs_enq(ch_regs[ch].addr.reg[0], (base_addr + offset_bytes) << ch_regs[ch].addr.reg_shift);
        tn_regs_enq(ch_regs[ch].stride.reg[0], imgI->stride << ch_regs[ch].stride.reg_shift);
    }

    /* Offset the mask layer start address also, whenever DST address was offset */
    if (ch == DST && (g_img_ctx.mask_color_gbuf != tnNULL) && (offset_x || offset_y))
    {
        const tn_color_format msk_fmt =
            (tn_color_format)g_img_ctx.mask_color_gbuf->color_format;
        const tn_map_coord msk_stride = g_img_ctx.mask_color_gbuf->stride;
        tn_paddr src_mask_addr = g_img_ctx.mask_color_gbuf->paddr;

        tn_color msk_colI;
        tn_color_copy(msk_fmt, &msk_colI);

        tn_u32 off_x_bits = offset_x * msk_colI.bpp;
        tnEXPECT((off_x_bits & 0x7) == 0); // can only skip entire bytes through mask memory

        tn_u32 offset_bytes = offset_y * msk_stride + (off_x_bits >> 3);
        src_mask_addr += offset_bytes;
        tn_regs_set_base_addr(src_mask_addr, MASK);
    }

    return y + u + v + a;
}



void
tn_regs_set_color_key(
    const tn_col_key_id key,
    tn_u8               enable,
    tn_u32              low,
    tn_u32              high,
    tn_u32              alpha)
{
    tn_reg_32 color_start, color_end;
    color_start = low | (enable << 31);
    color_end = high | ((alpha & 0x000000FF) << 24);

    tn_regs_enq(col_keys[S2].low[key], color_start);
    tn_regs_enq(col_keys[S2].high[key], color_end);

    //cache the regs
    g_img_ctx.color_keys[key] = (color_key_pair){color_start, color_end};
}



void
tn_regs_reset_color_key(const tn_chan_type ch)
{
    for (int key = 0; key < 4; key++) {
        tn_regs_enq(col_keys[ch].low[key], 0);
        tn_regs_enq(col_keys[ch].high[key], 0);
    }

}



void
tn_regs_transfer_color_key(const tn_chan_type ch)
{
    for (int key = 0; key < 4; key++) {
        tn_regs_enq(col_keys[ch].low[key], g_img_ctx.color_keys[key].low);
        tn_regs_enq(col_keys[ch].high[key], g_img_ctx.color_keys[key].high);
    }
}



tn_u32
tn_regs_en_dst_fill(tn_u32 enable)
{
    return TINGPU_DST_CTRL_FILL_EN(!!enable);
}
