//
// Copyright 2025-2026 NXP
//
// SPDX-License-Identifier: MIT
//

#ifndef   __TN_FMT_H__
#define   __TN_FMT_H__

#include "tn_color.h"

tn_u8 tn_fmt_get_image_word_width(const tn_color colI);

tn_bool tn_fmt_is_mask_fmt(const tn_color_format fmt);

tn_u8 tn_fmt_get_mask_fmt_reg(const tn_color_format fmt);

tn_bool tn_fmt_is_compressed(const tn_color_format fmt);

tn_bool tn_fmt_is_ETC2_compressed(const tn_color_format fmt);

tn_bool tn_fmt_is_IBC32x1_compressed(const tn_color_format fmt);

tn_bool tn_fmt_is_IBC8x8_compressed(const tn_color_format fmt);

tn_bool tn_fmt_is_yuv(const tn_color_format fmt);

tn_bool tn_fmt_is_yuv_alpha(const tn_color_format fmt);

tn_color_format tn_fmt_get_YV(const tn_color_format fmt);

tn_color_format tn_fmt_get_YU(const tn_color_format fmt);

tn_bool tn_fmt_is_rgb_a_planar(const tn_color_format fmt);

tn_u8 tn_fmt_is_gamma(const tn_color colorI);

tn_bool tn_fmt_is_indexed(const tn_color_format fmt);

tn_bool tn_fmt_is_vglite(tn_color_format fmt);

tn_bool tn_fmt_is_supported_on_dst(const tn_color colorI);

tn_u32 tn_fmt_get_supported_src_chan(const tn_color_format fmt);

tn_bool tn_fmt_not_supported_on_s1(const tn_color_format fmt);

tn_bool tn_fmt_is_supported_on_s2(const tn_color_format fmt);

tn_u32 tn_fmt_get_yuv_type(const tn_color_format fmt);

tn_u32 tn_fmt_get_compressed_type(const tn_color_format fmt);

tn_u8 tn_fmt_is_premul(const tn_color colorI);

tn_u8 tn_fmt_get_lut_width(const tn_color_format fmt);

tn_reg_32 tn_fmt_quality_to_reg_value(tn_reg_32 q);


#endif // __TN_FMT_H__
