//
// Copyright 2025-2026 NXP
//
// SPDX-License-Identifier: MIT
//
#include "vg_lite.h"

// --- Buffer allocation ---

vg_lite_error_t
vg_lite_map(
    vg_lite_buffer_t  *buffer,
    vg_lite_map_flag_t flag,
    vg_lite_int32_t    fd)
{
    return VG_LITE_NOT_SUPPORT;
}



vg_lite_error_t
vg_lite_unmap(vg_lite_buffer_t *buffer)
{
    return VG_LITE_NOT_SUPPORT;
}



vg_lite_error_t
vg_lite_flush_mapped_buffer(vg_lite_buffer_t *buffer)
{
    return VG_LITE_NOT_SUPPORT;
}



// --- Alpha blending ---

vg_lite_error_t
vg_lite_blend_func(
    vg_lite_uint32_t factor_src_alpha,
    vg_lite_uint32_t factor_src_color,
    vg_lite_uint32_t factor_dst_alpha,
    vg_lite_uint32_t factor_dst_color,
    vg_lite_uint32_t final_equation_opcode,
    vg_lite_uint32_t srcchannelmode,
    vg_lite_uint32_t dstchannelmode)
{
    return VG_LITE_NOT_SUPPORT;
}



// --- Color / dither ---

vg_lite_error_t
vg_lite_set_mirror(vg_lite_orientation_t orientation)
{
    return VG_LITE_NOT_SUPPORT;
}



vg_lite_error_t
vg_lite_set_pixel_matrix(
    vg_lite_pixel_matrix_t          matrix,
    vg_lite_pixel_channel_enable_t *channel)
{
    return VG_LITE_NOT_SUPPORT;
}



// --- Rendering - draw ---

vg_lite_error_t
vg_lite_draw(
    vg_lite_buffer_t *target,
    vg_lite_path_t   *path,
    vg_lite_fill_t    fill_rule,
    vg_lite_matrix_t *matrix,
    vg_lite_blend_t   blend,
    vg_lite_color_t   color)
{
    return VG_LITE_NOT_SUPPORT;
}



vg_lite_error_t
vg_lite_draw_pattern(
    vg_lite_buffer_t      *target,
    vg_lite_path_t        *path,
    vg_lite_fill_t         fill_rule,
    vg_lite_matrix_t      *path_matrix,
    vg_lite_buffer_t      *pattern_image,
    vg_lite_matrix_t      *pattern_matrix,
    vg_lite_blend_t        blend,
    vg_lite_pattern_mode_t pattern_mode,
    vg_lite_color_t        pattern_color,
    vg_lite_color_t        color,
    vg_lite_filter_t       filter)
{
    return VG_LITE_NOT_SUPPORT;
}



// --- Path ---

vg_lite_uint32_t
vg_lite_get_path_length(
    vg_lite_uint8_t *opcode,
    vg_lite_uint32_t count,
    vg_lite_format_t format)
{
    return 0;
}



vg_lite_error_t
vg_lite_init_path(
    vg_lite_path_t   *path,
    vg_lite_format_t  format,
    vg_lite_quality_t quality,
    vg_lite_uint32_t  length,
    vg_lite_pointer   data,
    vg_lite_float_t   min_x,
    vg_lite_float_t   min_y,
    vg_lite_float_t   max_x,
    vg_lite_float_t   max_y)
{
    return VG_LITE_NOT_SUPPORT;
}



vg_lite_error_t
vg_lite_init_arc_path(
    vg_lite_path_t   *path,
    vg_lite_format_t  format,
    vg_lite_quality_t quality,
    vg_lite_uint32_t  length,
    vg_lite_pointer   data,
    vg_lite_float_t   min_x,
    vg_lite_float_t   min_y,
    vg_lite_float_t   max_x,
    vg_lite_float_t   max_y)
{
    return VG_LITE_NOT_SUPPORT;
}



vg_lite_error_t
vg_lite_clear_path(vg_lite_path_t *path)
{
    return VG_LITE_NOT_SUPPORT;
}



vg_lite_error_t
vg_lite_upload_path(vg_lite_path_t *path)
{
    return VG_LITE_NOT_SUPPORT;
}



vg_lite_error_t
vg_lite_append_path(
    vg_lite_path_t  *path,
    vg_lite_uint8_t *opcode,
    vg_lite_pointer  data,
    vg_lite_uint32_t seg_count)
{
    return VG_LITE_NOT_SUPPORT;
}



vg_lite_error_t
vg_lite_set_path_type(
    vg_lite_path_t     *path,
    vg_lite_path_type_t path_type)
{
    return VG_LITE_NOT_SUPPORT;
}



vg_lite_error_t
vg_lite_split_path(vg_lite_uint32_t endis)
{
    return VG_LITE_NOT_SUPPORT;
}



// --- Stroke ---

vg_lite_error_t
vg_lite_set_stroke(
    vg_lite_path_t      *path,
    vg_lite_cap_style_t  cap_style,
    vg_lite_join_style_t join_style,
    vg_lite_float_t      line_width,
    vg_lite_float_t      miter_limit,
    vg_lite_float_t     *dash_pattern,
    vg_lite_uint32_t     pattern_count,
    vg_lite_float_t      dash_phase,
    vg_lite_color_t      color)
{
    return VG_LITE_NOT_SUPPORT;
}



vg_lite_error_t
vg_lite_update_stroke(vg_lite_path_t *path)
{
    return VG_LITE_NOT_SUPPORT;
}



// --- Gradient - linear (legacy) ---

vg_lite_matrix_t*
vg_lite_get_grad_matrix(vg_lite_linear_gradient_t *grad)
{
    return tnNULL;
}



vg_lite_error_t
vg_lite_init_grad(vg_lite_linear_gradient_t *grad)
{
    return VG_LITE_NOT_SUPPORT;
}



vg_lite_error_t
vg_lite_clear_grad(vg_lite_linear_gradient_t *grad)
{
    return VG_LITE_NOT_SUPPORT;
}



vg_lite_error_t
vg_lite_update_grad(vg_lite_linear_gradient_t *grad)
{
    return VG_LITE_NOT_SUPPORT;
}



vg_lite_error_t
vg_lite_set_grad(
    vg_lite_linear_gradient_t *grad,
    vg_lite_uint32_t           count,
    vg_lite_uint32_t          *colors,
    vg_lite_uint32_t          *stops)
{
    return VG_LITE_NOT_SUPPORT;
}



vg_lite_error_t
vg_lite_draw_grad(
    vg_lite_buffer_t          *target,
    vg_lite_path_t            *path,
    vg_lite_fill_t             fill_rule,
    vg_lite_matrix_t          *matrix,
    vg_lite_linear_gradient_t *grad,
    vg_lite_blend_t            blend)
{
    return VG_LITE_NOT_SUPPORT;
}



// --- Gradient - linear (extended) ---

vg_lite_matrix_t*
vg_lite_get_linear_grad_matrix(vg_lite_ext_linear_gradient_t *grad)
{
    return tnNULL;
}



vg_lite_error_t
vg_lite_clear_linear_grad(vg_lite_ext_linear_gradient_t *grad)
{
    return VG_LITE_NOT_SUPPORT;
}



vg_lite_error_t
vg_lite_update_linear_grad(vg_lite_ext_linear_gradient_t *grad)
{
    return VG_LITE_NOT_SUPPORT;
}



vg_lite_error_t
vg_lite_set_linear_grad(
    vg_lite_ext_linear_gradient_t      *grad,
    vg_lite_uint32_t                    count,
    vg_lite_color_ramp_t               *color_ramp,
    vg_lite_linear_gradient_parameter_t grad_param,
    vg_lite_gradient_spreadmode_t       spread_mode,
    vg_lite_uint8_t                     pre_mult)
{
    return VG_LITE_NOT_SUPPORT;
}



vg_lite_error_t
vg_lite_draw_linear_grad(
    vg_lite_buffer_t              *target,
    vg_lite_path_t                *path,
    vg_lite_fill_t                 fill_rule,
    vg_lite_matrix_t              *path_matrix,
    vg_lite_ext_linear_gradient_t *grad,
    vg_lite_color_t                paint_color,
    vg_lite_blend_t                blend,
    vg_lite_filter_t               filter)
{
    return VG_LITE_NOT_SUPPORT;
}



// --- Gradient - radial ---

vg_lite_matrix_t*
vg_lite_get_radial_grad_matrix(vg_lite_radial_gradient_t *grad)
{
    return tnNULL;
}



vg_lite_error_t
vg_lite_clear_radial_grad(vg_lite_radial_gradient_t *grad)
{
    return VG_LITE_NOT_SUPPORT;
}



vg_lite_error_t
vg_lite_update_radial_grad(vg_lite_radial_gradient_t *grad)
{
    return VG_LITE_NOT_SUPPORT;
}



vg_lite_error_t
vg_lite_set_radial_grad(
    vg_lite_radial_gradient_t          *grad,
    vg_lite_uint32_t                    count,
    vg_lite_color_ramp_t               *color_ramp,
    vg_lite_radial_gradient_parameter_t grad_param,
    vg_lite_gradient_spreadmode_t       spread_mode,
    vg_lite_uint8_t                     pre_mult)
{
    return VG_LITE_NOT_SUPPORT;
}



vg_lite_error_t
vg_lite_draw_radial_grad(
    vg_lite_buffer_t          *target,
    vg_lite_path_t            *path,
    vg_lite_fill_t             fill_rule,
    vg_lite_matrix_t          *path_matrix,
    vg_lite_radial_gradient_t *grad,
    vg_lite_color_t            paint_color,
    vg_lite_blend_t            blend,
    vg_lite_filter_t           filter)
{
    return VG_LITE_NOT_SUPPORT;
}



// --- Filter / post-process ---

vg_lite_error_t
vg_lite_gaussian_filter(
    vg_lite_float_t w0,
    vg_lite_float_t w1,
    vg_lite_float_t w2)
{
    return VG_LITE_NOT_SUPPORT;
}



// --- Mask layer ---

vg_lite_error_t
vg_lite_render_masklayer(
    vg_lite_buffer_t        *masklayer,
    vg_lite_mask_operation_t operation,
    vg_lite_path_t          *path,
    vg_lite_fill_t           fill_rule,
    vg_lite_color_t          color,
    vg_lite_matrix_t        *matrix)
{
    return VG_LITE_NOT_SUPPORT;
}



// --- Tessellation buffer ---

vg_lite_error_t
vg_lite_set_tess_buffer(
    vg_lite_uint32_t physical,
    vg_lite_uint32_t size)
{
    return VG_LITE_NOT_SUPPORT;
}



// --- Memory pool ---

vg_lite_error_t
vg_lite_set_memory_pool(
    vg_lite_buffer_type_t type,
    vg_lite_memory_pool_t pool)
{
    return VG_LITE_NOT_SUPPORT;
}



vg_lite_error_t
vg_lite_set_target_mesh(
    vg_lite_mesh_mode_t mesh_mode,
    vg_lite_uint32_t    mesh_height,
    vg_lite_uint32_t    mesh_count)
{
    return VG_LITE_NOT_SUPPORT;
}



// --- Debug / dump ---

vg_lite_error_t
vg_lite_dump_png(
    const vg_lite_char *filename,
    vg_lite_buffer_t   *buffer)
{
    return VG_LITE_NOT_SUPPORT;
}



// --- Flexa ---

vg_lite_error_t
vg_lite_flexa_enable()
{
    return VG_LITE_NOT_SUPPORT;
}



vg_lite_error_t
vg_lite_flexa_disable()
{
    return VG_LITE_NOT_SUPPORT;
}



vg_lite_error_t
vg_lite_flexa_set_stream(vg_lite_uint8_t stream_id)
{
    return VG_LITE_NOT_SUPPORT;
}



vg_lite_error_t
vg_lite_flexa_bg_buffer(
    vg_lite_uint8_t   stream_id,
    vg_lite_buffer_t *buffer,
    vg_lite_uint32_t  seg_count,
    vg_lite_uint32_t  seg_size)
{
    return VG_LITE_NOT_SUPPORT;
}



vg_lite_error_t
vg_lite_flexa_stop_frame(vg_lite_void)
{
    return VG_LITE_NOT_SUPPORT;
}



vg_lite_error_t
vg_lite_flexa_set_consumer(
    vg_lite_flexa_config_t *flexa_cfg,
    vg_lite_buffer_t       *buffer)
{
    return VG_LITE_NOT_SUPPORT;
}



vg_lite_error_t
vg_lite_flexa_set_producer(
    vg_lite_flexa_config_t *flexa_cfg,
    vg_lite_buffer_t       *buffer)
{
    return VG_LITE_NOT_SUPPORT;
}



vg_lite_error_t
vg_lite_flexa_stop_consumer(vg_lite_flexa_config_t *flexa_cfg)
{
    return VG_LITE_NOT_SUPPORT;
}



vg_lite_error_t
vg_lite_flexa_stop_producer(vg_lite_flexa_config_t *flexa_cfg)
{
    return VG_LITE_NOT_SUPPORT;
}
