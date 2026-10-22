//
// Copyright 2025-2026 NXP
//
// SPDX-License-Identifier: MIT
//

#include "vg_lite.h"
#include "tn_trace_api.h"
#include "vglite_tin_utils.h"
#include "tn_hw.h"
#include "tn_sync.h"
#include "tn_regs_image.h"
#include "tn_mbuf.h"
#include "tn_blt.h"
#include "tn_mspace.h"
#include "tn_ctx.h"
#include "tn_obj.h"
#include "tn_stub.h"
#include "tn_os.h"
#include "tn_cmd_buf_ccdma.h"
#include "tn_cmdq_ccdma.h"
#include "tn_io.h"
#include "tn_ctx_img.h"
#include "tn_ctx_api.h"
#include "tn_fmt.h"
#include "tn_cyc_cnt.h"
#include "vg_lite_ext.h"
#include "tn_mem.h"
#include "tn_misc.h"


// vg_lite_upload_buffer only supports 3 planes formats.
// Use vg_lite_upload_buffer_ext if 4 planes formats (AYUV) are needed.
vg_lite_error_t
vg_lite_upload_buffer(
    vg_lite_buffer_t *buffer,
    vg_lite_uint8_t  *data[3],
    vg_lite_uint32_t  stride[3])
{
    tnIN();
    DUMP_API_CALL(vg_lite_upload_buffer, "buffer:%p data:%p stride:%p\n", buffer, data, stride);

    tnRET_IF(tn_is_null(buffer) || tn_is_null(data), VG_LITE_INVALID_ARGUMENT);

    tn_u8 num_planes;
    tn_u32 plane_height[4];
    tn_u32 plane_stride[4];

    get_planes_info(buffer, &num_planes, plane_height, plane_stride);
    tnRET_IF(!(0 < num_planes && num_planes < 4), VG_LITE_INVALID_ARGUMENT);

    tn_size required = 0;
    tnRET_IF(!vglite_planes_total_bytes(num_planes, plane_height, plane_stride, &required),
             VG_LITE_INVALID_ARGUMENT);
    tnRET_IF(!tn_gbuf_is_valid(&buffer->gbuf), VG_LITE_INVALID_ARGUMENT);
    tnRET_IF(!tn_span_fits(0, required, buffer->gbuf.size), VG_LITE_INVALID_ARGUMENT);

    vg_lite_uint8_t *pdata, *pbuffer;
    pbuffer = (vg_lite_uint8_t*)buffer->memory;
    for (tn_u8 i = 0; i < num_planes; i++)
    {
        // The destination planes in video memory are tightly packed, so a full
        // plane_stride[i] bytes are copied per row and the destination advances
        // by plane_stride[i]. The source may carry padding described by the
        // caller-provided stride[i]; a source stride of 0 means the source plane
        // is also tightly packed, in which case fall back to plane_stride[i] so
        // the source pointer still advances one row per iteration.
        tn_u32 src_stride = (stride != tnNULL && stride[i] != 0) ? stride[i] : plane_stride[i];

        pdata = data[i];
        tnRET_IF(tn_is_null(pdata), VG_LITE_INVALID_ARGUMENT);

        for (tn_u32 j = 0; j < plane_height[i]; j++)
        {
            tn_memcpy(pbuffer, pdata, plane_stride[i]);
            pbuffer += plane_stride[i];
            pdata   += src_stride;
        }
    }


    tnOUT();
    return VG_LITE_SUCCESS;
}



vg_lite_error_t
vg_lite_finish(void)
{
    tnIN();
    DUMP_API_CALL(vg_lite_finish, "\n");

    tn_result result = tn_sync_on_finish();

    tnOUT();
    return tn_result_to_vg_lite_error(result);
}



vg_lite_error_t
vg_lite_flush(void)
{
    tnIN();
    DUMP_API_CALL(vg_lite_flush, "\n");

    tn_result result = tn_sync_on_flush();

    tnOUT();
    return tn_result_to_vg_lite_error(result);
}



vg_lite_error_t
vg_lite_dump_command_buffer(vg_lite_void)
{
    tnIN();
    DUMP_API_CALL(vg_lite_dump_command_buffer, "\n");

    tn_vptr base_vaddr = tn_cmdq_ccdma_get_base();
    tn_paddr base_paddr = tn_cmdq_ccdma_paddr();
    tn_size start_idx  = tn_cmdq_ccdma_get_start();
    tn_size end_idx    = tn_cmdq_ccdma_get_end();
    tn_size ring_size  = tn_cmdq_ccdma_get_size();

    tn_printf("@[command paddr=0x%08x start=%u end=%u ring_size=%u]\n",
              (unsigned)base_paddr,
              (unsigned)start_idx, (unsigned)end_idx, (unsigned)ring_size);

    if (tn_cmdq_ccdma_is_full() || start_idx > end_idx)
    {
        // Wrapping case (including full ring): segment 1 from start_idx to end
        // of ring, then segment 2 from 0 to end_idx. When full, end_idx ==
        // start_idx so the two segments together cover the entire ring.
        tn_u8 *seg1    = (tn_u8*)base_vaddr + start_idx * 8U;
        tn_size seg1_sz = (ring_size - start_idx) * 8U;
        for (tn_size i = 0; i < seg1_sz; i++)
        {
            if (i > 0 && (i % 16) == 0)
                tn_printf("\n");
            tn_printf("0x%02x, ", seg1[i]);
        }
        // Ensure seg2 starts on a new line
        tn_printf("\n");
        // Segment 2: wrapped portion from 0 to end_idx
        tn_u8 *seg2    = (tn_u8*)base_vaddr;
        tn_size seg2_sz = end_idx * 8U;
        for (tn_size i = 0; i < seg2_sz; i++)
        {
            if (i > 0 && (i % 16) == 0)
                tn_printf("\n");
            tn_printf("0x%02x, ", seg2[i]);
        }
    }
    else
    {
        // Non-wrapping case: contiguous segment from start_idx to end_idx
        tn_u8 *seg    = (tn_u8*)base_vaddr + start_idx * 8U;
        tn_size nbytes = (end_idx - start_idx) * 8U;
        for (tn_size i = 0; i < nbytes; i++)
        {
            if (i > 0 && (i % 16) == 0)
                tn_printf("\n");
            tn_printf("0x%02x, ", seg[i]);
        }
    }
    tn_printf("\n@[commit]\n");

    tnOUT();
    return VG_LITE_SUCCESS;
}



vg_lite_error_t
vg_lite_frame_delimiter(
    vg_lite_frame_flag_t flag,
    vg_lite_bool_t       stall_flag)
{
    tnIN();
    DUMP_API_CALL(vg_lite_frame_delimiter, "flag:%d stall_flag:%d\n", flag, stall_flag);

    // flag selects the HW event token (FRAME_END vs END); tin does not expose
    // that distinction at this level, so it is accepted but not acted upon.
    tnUNUSED(flag);

    // Submit a frame-delimiter CCDMA event (TN_CCDMA_EVENT_FRAME_FLUSH/FINISH)
    // instead of the plain IMG FLUSH/FINISH event used by vg_lite_flush()/
    // vg_lite_finish(). The ISR treats both pairs identically; the
    // distinct event token exists so a frame boundary can be told apart from
    // a regular flush/finish on the HW event stream, in case that needs to be
    // forwarded further up the stack in the future.
    tn_result result;
    if (stall_flag == VGL_FALSE)
    {
        result = tn_cmd_buf_ccdma_frame_flush();    // non-blocking: submit and return
    }
    else
    {
        result = tn_cmd_buf_ccdma_frame_finish();   // blocking: submit and wait for GPU idle
    }


    tnOUT();
    return tn_result_to_vg_lite_error(result);
}



vg_lite_error_t
vg_lite_init(
    vg_lite_uint32_t tessellation_width,
    vg_lite_uint32_t tessellation_height)
{
    tnIN();
    DUMP_API_CALL(vg_lite_init, "tessellation_width:%d tessellation_height:%d\n",
                  tessellation_width, tessellation_height);

    tn_result rv = tn_init();
    tnRET_IF(rv != TN_NO_ERROR, tn_result_to_vg_lite_error(rv));

    tnOUT();
    return VG_LITE_SUCCESS;
}



vg_lite_error_t
vg_lite_close(void)
{
    tnIN();
    DUMP_API_CALL(vg_lite_close, "\n");

    vg_lite_error_t err = vg_lite_finish();
    tnRET_IF(err != VG_LITE_SUCCESS, err);

    tn_deinit();

    tnOUT();
    return VG_LITE_SUCCESS;
}



vg_lite_error_t
vg_lite_get_info(vg_lite_info_t *info)
{
    tnIN();
    DUMP_API_CALL(vg_lite_get_info, "info:%p\n", info);

    tnRET_IF(tn_is_null(info), VG_LITE_INVALID_ARGUMENT);

    info->api_version     = VGLITE_API_VERSION_3_0;
    info->header_version  = VGLITE_HEADER_VERSION;
    info->release_version = VGLITE_RELEASE_VERSION;
    info->reserved = 0;

    tnOUT();
    return VG_LITE_SUCCESS;
}



vg_lite_uint32_t
vg_lite_get_product_info(
    char   *name,
    tn_u32 *chip_id,
    tn_u32 *chip_rev)
{
    tnIN();
    DUMP_API_CALL(vg_lite_get_product_info, "name:%p chip_id:%p chip_rev:%p\n", name, chip_id, chip_rev);

    tnRET_IF(tn_is_null(name) ||
             tn_is_null(chip_id) ||
             tn_is_null(chip_rev), VG_LITE_INVALID_ARGUMENT);

    tn_regs_get_product_info(name, chip_id, chip_rev);

    tnOUT();
    return 0;
}



vg_lite_uint32_t
vg_lite_query_feature(vg_lite_feature_t feature)
{
    DUMP_API_CALL(vg_lite_query_feature, "feature:%d\n", feature);
    return tn_ctx_api_has_feature(feature);
}



vg_lite_error_t
vg_lite_get_register(
    tn_u32  address,
    tn_u32 *result)
{
    tnIN();

    DUMP_API_CALL(vg_lite_get_register, "address:%d result:%p\n", address, result);

    tnRET_IF(tn_is_null(result), VG_LITE_INVALID_ARGUMENT);
    *result = tn_hw_reg_get(address);

    tnOUT();
    return VG_LITE_SUCCESS;
}



vg_lite_error_t
vg_lite_source_global_alpha(
    vg_lite_global_alpha_t alpha_mode,
    vg_lite_uint8_t        alpha_value)
{
    tnIN();
    DUMP_API_CALL(vg_lite_source_global_alpha, "alpha_mode:%d alpha_value:%d\n", alpha_mode, alpha_value);

    switch (alpha_mode)
    {
    case VG_LITE_NORMAL:
        g_img_ctx.src_alpha_mode = TN_LOCAL_ALPHA;
        break;
    case VG_LITE_GLOBAL:
        g_img_ctx.src_alpha_mode = TN_GLOBAL_ALPHA;
        break;
    case VG_LITE_SCALED:
        g_img_ctx.src_alpha_mode = TN_SCALED_ALPHA;
        break;
    default:
        tnOUT();
        return VG_LITE_INVALID_ARGUMENT;
    }

    g_img_ctx.src_alpha_value =  alpha_value;

    tnOUT();
    return VG_LITE_SUCCESS;
}



vg_lite_error_t
vg_lite_allocate(vg_lite_buffer_t *buffer)
{
    tnIN();
    DUMP_API_CALL(vg_lite_allocate, "buffer:%p\n", buffer);

    tnRET_IF(tn_is_null(buffer), VG_LITE_INVALID_ARGUMENT);

    tn_color_format color_format = vglite_color_2_tn_color(buffer->format);
    tnRET_IF(color_format == TN_MAX_COLOR_FORMAT, VG_LITE_NOT_SUPPORT);

    // Memory is owned by the driver; mark buffer as non-external so that
    // vg_lite_free() releases the backing memory.
    buffer->external = tnFALSE;

    tn_result res = vglite_buffer_gbuf_create(&buffer->gbuf, buffer);
    tnRET_IF(res != TN_NO_ERROR, VG_LITE_OUT_OF_MEMORY);

    buffer->memory = buffer->gbuf.vaddr;
    buffer->address = (tn_u32)(tn_uptr)buffer->gbuf.paddr;
    // handle is not used internally, but upper layer might use it for checks,
    // so non-zero value is assigned to indicate successful allocation
    buffer->handle = buffer->memory;

    // The chroma planes live inside the same allocation, at the offsets the
    // descriptor carries. An offset of 0 means the format has no such plane.
    if (buffer->gbuf.uv_offset != 0)
    {
        buffer->yuv.uv_memory = (tn_vptr)((tn_u8*)buffer->gbuf.vaddr + buffer->gbuf.uv_offset);
    }

    if (buffer->gbuf.v_offset != 0)
    {
        buffer->yuv.v_memory = (tn_vptr)((tn_u8*)buffer->gbuf.vaddr + buffer->gbuf.v_offset);
    }

    tnOUT();
    return VG_LITE_SUCCESS;
}



vg_lite_error_t
vg_lite_free(vg_lite_buffer_t *buffer)
{
    tnIN();
    DUMP_API_CALL(vg_lite_free, "buffer:%p\n", buffer);

    tnRET_IF(tn_is_null(buffer), VG_LITE_INVALID_ARGUMENT);

    // Buffers wrapped via vg_lite_wrap_buffer_ext own their memory externally.
    // Their internal data structures are released with vg_lite_unwrap_buffer_ext,
    // so vg_lite_free must not touch them.
    if (buffer->external)
    {
        tnOUT();
        return VG_LITE_SUCCESS;
    }

    if (buffer->handle != tnNULL)
    {
        const tn_bool was_yuv = tn_fmt_is_yuv((tn_color_format)buffer->gbuf.color_format);

        if (g_img_ctx.mask_color_gbuf == &buffer->gbuf)
        {
            g_img_ctx.mask_color_gbuf = tnNULL;
            tn_regs_disable_mask();
        }

        tnIGNORE_RESULT(tn_gbuf_destroy(&buffer->gbuf));


        buffer->address = 0;
        buffer->memory  = tnNULL;
        buffer->handle = tnNULL;

        if (was_yuv)
        {
            //uv planes are part of the buffer->memory, so they are already deallocated
            buffer->yuv.uv_memory = 0;
            buffer->yuv.v_memory = 0;
        }
    }

    tnOUT();
    return VG_LITE_SUCCESS;
}



vg_lite_error_t
vg_lite_clear(
    vg_lite_buffer_t    *target,
    vg_lite_rectangle_t *rectangle,
    vg_lite_color_t      color)
{
    tnIN();
    DUMP_API_CALL(vg_lite_clear, "target:%p rectangle:%p color:%x\n", target, rectangle, color);

    tnRET_IF(tn_is_null(target), VG_LITE_INVALID_ARGUMENT);

    tn_result res = tn_sync_on_img_operation();
    tnRET_IF(res != TN_NO_ERROR, tn_result_to_vg_lite_error(res));

    tn_color clr_color = {0};
    clr_color.format = TN_sABGR_8888;
    clr_color.colorspace = TN_CS_sRGBA;
    tn_color_unpack(color, &clr_color);

    res = tn_blt_fill(
        g_img_ctx.blit,
        clr_color,
        &(tn_blt_xform_t) {
        .src_img = tnNULL,
        .dst_img = &target->gbuf,
        .sx = 0,
        .sy = 0,
        .dx = rectangle != tnNULL? (tn_map_coord)rectangle->x : 0,
        .dy = rectangle != tnNULL? (tn_map_coord)rectangle->y : 0,
        .width = rectangle != tnNULL? (tn_map_coord)rectangle->width : target->width,
        .height = rectangle != tnNULL? (tn_map_coord)rectangle->height : target->height
    });
    tnRET_IF(res != TN_NO_ERROR, tn_result_to_vg_lite_error(res));

    tnOUT();
    return VG_LITE_SUCCESS;
}



vg_lite_error_t
vg_lite_blit(
    vg_lite_buffer_t *target,
    vg_lite_buffer_t *source,
    vg_lite_matrix_t *matrix,
    vg_lite_blend_t   blend,
    vg_lite_color_t   color,
    vg_lite_filter_t  filter)
{
    tnIN();
    DUMP_API_CALL(vg_lite_blit, "target:%p source:%p matrix:%p blend:%d color:%x filter:%d\n",
                  target, source, matrix, blend, color, filter);

    tnRET_IF(tn_is_null(target) || tn_is_null(source), VG_LITE_INVALID_ARGUMENT);

    tn_u32 quality;

    tn_result res = tn_sync_on_img_operation();
    tnRET_IF(res != TN_NO_ERROR, tn_result_to_vg_lite_error(res));

    if (filter == VG_LITE_FILTER_BI_LINEAR)
    {
        quality = TN_RENDERING_QUALITY_BETTER;
    }
    else if (filter == VG_LITE_FILTER_LINEAR)
    {
        quality = TN_RENDERING_QUALITY_FASTER;
    }
    else
    {
        quality = TN_RENDERING_QUALITY_NONANTIALIASED;
    }

    tn_m3f32 *user_matrix = (tn_m3f32*)matrix;

    tn_color src_color = {0};
    src_color.format = TN_sABGR_8888;
    src_color.colorspace = TN_CS_sRGBA;
    tn_color_unpack(color, &src_color);

    res = tn_blt_image(
        &target->gbuf,
        &source->gbuf,
        user_matrix,
        vglite_blend_to_tn_blend(blend),
        src_color,
        vglite_image_mode_to_tn_image_mode(source->image_mode),
        quality);

    tnRET_IF(res != TN_NO_ERROR, tn_result_to_vg_lite_error(res));

    tnOUT();
    return VG_LITE_SUCCESS;
}



vg_lite_error_t
vg_lite_blit_rect(
    vg_lite_buffer_t    *target,
    vg_lite_buffer_t    *source,
    vg_lite_rectangle_t *rect,
    vg_lite_matrix_t    *matrix,
    vg_lite_blend_t      blend,
    vg_lite_color_t      color,
    vg_lite_filter_t     filter)
{
    tnIN();
    DUMP_API_CALL(vg_lite_blit_rect, "target:%p source:%p rect:%p matrix:%p blend:%d color:%x filter:%d\n",
                  target, source, rect, matrix, blend, color, filter);

    tnRET_IF(tn_is_null(target) || tn_is_null(source), VG_LITE_INVALID_ARGUMENT);

    tn_u32 quality;

    tn_result res = tn_sync_on_img_operation();
    tnRET_IF(res != TN_NO_ERROR, tn_result_to_vg_lite_error(res));

    if (filter == VG_LITE_FILTER_BI_LINEAR)
    {
        quality = TN_RENDERING_QUALITY_BETTER;
    }
    else if (filter == VG_LITE_FILTER_LINEAR)
    {
        quality = TN_RENDERING_QUALITY_FASTER;
    }
    else
    {
        quality = TN_RENDERING_QUALITY_NONANTIALIASED;
    }

    tn_color src_color = {0};
    src_color.format = TN_sABGR_8888;
    src_color.colorspace = TN_CS_sRGBA;
    tn_color_unpack(color, &src_color);

    res = tn_blt_image_rect(
        &target->gbuf,
        &source->gbuf,
        (tn_m3f32*)matrix,
        vglite_blend_to_tn_blend(blend),
        src_color,
        vglite_image_mode_to_tn_image_mode(source->image_mode),
        quality,
        &(tn_blt_xform_t){
        .src_img = 0,
        .dst_img = 0,
        .sx = 0,
        .sy = 0,
        .dx = rect != tnNULL ? (tn_map_coord)rect->x : 0,
        .dy = rect != tnNULL ? (tn_map_coord)rect->y : 0,
        .width = rect != tnNULL ? (tn_map_coord)rect->width : target->width,
        .height = rect != tnNULL ? (tn_map_coord)rect->height : target->height
    });
    tnRET_IF(res != TN_NO_ERROR, tn_result_to_vg_lite_error(res));

    tnOUT();
    return VG_LITE_SUCCESS;
}



vg_lite_error_t
vg_lite_blit2(
    vg_lite_buffer_t *target,
    vg_lite_buffer_t *source0,
    vg_lite_buffer_t *source1,
    vg_lite_matrix_t *matrix0,
    vg_lite_matrix_t *matrix1,
    vg_lite_blend_t   blend,
    vg_lite_filter_t  filter)
{
    tnIN();
    DUMP_API_CALL(vg_lite_blit2, "target:%p source0:%p source1:%p matrix0:%p matrix1:%p blend:%d filter:%d\n",
                  target, source0, source1, matrix0, matrix1, blend, filter);

    tn_u32 quality;

    tnRET_IF(tn_is_null(target) ||
             tn_is_null(source0) ||
             tn_is_null(source1), VG_LITE_INVALID_ARGUMENT);

    tn_result res = tn_sync_on_img_operation();
    tnRET_IF(res != TN_NO_ERROR, tn_result_to_vg_lite_error(res));

    if (filter == VG_LITE_FILTER_BI_LINEAR)
    {
        quality = TN_RENDERING_QUALITY_BETTER;
    }
    else if (filter == VG_LITE_FILTER_LINEAR)
    {
        quality = TN_RENDERING_QUALITY_FASTER;
    }
    else
    {
        quality = TN_RENDERING_QUALITY_NONANTIALIASED;
    }

    // blit2 does not take a color argument, so use a zero (transparent) color
    // to disable any color mixing.
    tn_color src_color = {0};
    src_color.format = TN_sABGR_8888;
    src_color.colorspace = TN_CS_sRGBA;
    tn_color_unpack(0, &src_color);

    tn_color_format src0_color_format = vglite_color_2_tn_color(source0->format);
    tnRET_IF(src0_color_format == TN_MAX_COLOR_FORMAT, VG_LITE_NOT_SUPPORT);
    tn_color_format src1_color_format = vglite_color_2_tn_color(source1->format);
    tnRET_IF(src1_color_format == TN_MAX_COLOR_FORMAT, VG_LITE_NOT_SUPPORT);

    // FlexGPU has a native two-source datapath (S1, S2 and DST channels).
    //   - source0 rides on S2 (the transform channel) with matrix0 and is
    //     blended as the foreground image using the requested blend mode.
    //   - source1 rides on S1 (background) with matrix1. S1 has no transform
    //     unit, so matrix1 may only contain a translation.
    // When matrix1 is a translation (or absent) AND matrix0 is affine (or
    // absent), blit2 maps to a single hardware pass via tn_blt_image2. This
    // matches vg_lite_blit2 where source0 is the "image" blended over
    // source1 the "paint" background, and avoids the intermediate write/read of
    // the target that a two-pass emulation would incur.
    //
    // If matrix0 is projective, tn_blt_image2 cannot handle it (S2 projective
    // with a live S1 background requires extra scissor bookkeeping that is not
    // implemented there). In that case fall through to the two-pass path below,
    // which calls tn_blt_image for each source separately -- tn_blt_image does
    // support projective on S2.
    //
    // If only one of the two sources has a YUV format, two passes are also
    // required. This is due to a limitation of the YUV to RGBA conversion
    // which would cause inconsistencies between the two images.
    tn_m3f32 *user_matrix1 = (tn_m3f32*)matrix1;
    tn_m3f32 *user_matrix0 = (tn_m3f32*)matrix0;
    tn_bool use_two_pass = tnTRUE;
    if ((user_matrix1 == tnNULL || tn_m3f32_is_translation_only(user_matrix1))
        && (user_matrix0 == tnNULL || tn_m3f32_is_affine(user_matrix0))
        && (tn_fmt_is_yuv(src0_color_format) == tn_fmt_is_yuv(src1_color_format)))
    {
        res = tn_blt_image2(
            &target->gbuf,
            &source0->gbuf,
            &source1->gbuf,
            (tn_m3f32*)matrix0,
            user_matrix1,
            vglite_blend_to_tn_blend(blend),
            src_color,
            vglite_image_mode_to_tn_image_mode(source0->image_mode),
            quality);
        if (res == TN_NO_ERROR)
        {
            use_two_pass = tnFALSE;
        }
        else if (res != TN_NOT_SUPPORTED)
        {
            // A hard error (not a "please use the fallback" signal).
            tnOUT();
            return VG_LITE_NOT_SUPPORT;
        }
    }

    if (use_two_pass)
    {
        // matrix1 carries a transformation other than a pure translation, which
        // S1 cannot apply. Fall back to two consecutive single-source blits
        // sharing the same render target, serialized through the GPU command
        // stream.
        //
        // First pass: render source1 (background) over the existing target
        // content. This must use SRC_OVER, not BLEND_NONE: BLEND_NONE maps to
        // TN_BLEND_SRC (a straight copy of the source, ignoring the target), so
        // any transparent pixels in source1 would overwrite the target with
        // source1's transparent-black texels instead of leaving the target
        // visible. SRC_OVER composites source1 over the target so its
        // transparent regions keep the target background intact. The result
        // becomes the destination content for the second pass.
        // TN_DRAW_IMAGE_NORMAL is used unconditionally for source1: image_mode
        // only applies to the interaction between the source image and the paint
        // colour (src_color). In blit2, src_color is always transparent black,
        // so MULTIPLY or STENCIL image modes would corrupt source1's pixels.
        // The single-pass path (tn_blt_image2) never applies source1->image_mode
        // either, so NORMAL keeps both paths consistent.
        res = tn_blt_image(
            &target->gbuf,
            &source1->gbuf,
            user_matrix1,
            vglite_blend_to_tn_blend(VG_LITE_BLEND_SRC_OVER),
            src_color,
            TN_DRAW_IMAGE_NORMAL,
            quality);
        tnRET_IF(res != TN_NO_ERROR, tn_result_to_vg_lite_error(res));

        // Second pass: render source0 (foreground image) over the target using
        // the requested blend mode and matrix0.
        res = tn_blt_image(
            &target->gbuf,
            &source0->gbuf,
            (tn_m3f32*)matrix0,
            vglite_blend_to_tn_blend(blend),
            src_color,
            vglite_image_mode_to_tn_image_mode(source0->image_mode),
            quality);
        tnRET_IF(res != TN_NO_ERROR, tn_result_to_vg_lite_error(res));
    }

    tnOUT();
    return VG_LITE_SUCCESS;
}



vg_lite_error_t
vg_lite_identity(vg_lite_matrix_t *matrix)
{
    tnIN();
    DUMP_API_CALL(vg_lite_identity, "matrix:%p\n", matrix);

    tnRET_IF(tn_is_null(matrix), VG_LITE_INVALID_ARGUMENT);

    tn_m3f32_identity((tn_m3f32*)matrix);

    tnOUT();
    return VG_LITE_SUCCESS;
}



vg_lite_error_t
vg_lite_translate(
    vg_lite_float_t   x,
    vg_lite_float_t   y,
    vg_lite_matrix_t *matrix)
{
    tnIN();
    DUMP_API_CALL(vg_lite_translate, "x:%f y:%f matrix:%p\n", x, y, matrix);

    tnRET_IF(tn_is_null(matrix), VG_LITE_INVALID_ARGUMENT);

    tn_m3f32_translate_imm((tn_m3f32*)matrix, x, y);

    tnOUT();
    return VG_LITE_SUCCESS;
}



vg_lite_error_t
vg_lite_scale(
    vg_lite_float_t   scale_x,
    vg_lite_float_t   scale_y,
    vg_lite_matrix_t *matrix)
{
    tnIN();
    DUMP_API_CALL(vg_lite_scale, "scale_x:%f scale_y:%f matrix:%p\n", scale_x, scale_y, matrix);

    tnRET_IF(tn_is_null(matrix), VG_LITE_INVALID_ARGUMENT);

    tn_m3f32_scale_imm((tn_m3f32*)matrix, scale_x, scale_y);

    tnOUT();
    return VG_LITE_SUCCESS;
}



vg_lite_error_t
vg_lite_rotate(
    vg_lite_float_t   degrees,
    vg_lite_matrix_t *matrix)
{
    tnIN();
    DUMP_API_CALL(vg_lite_rotate, "degrees:%f matrix:%p\n", degrees, matrix);

    tnRET_IF(tn_is_null(matrix), VG_LITE_INVALID_ARGUMENT);

    tn_m3f32_rotate_deg_imm((tn_m3f32*)matrix, (tn_f32)degrees);

    tnOUT();
    return VG_LITE_SUCCESS;
}



vg_lite_error_t
vg_lite_set_scissor(
    vg_lite_int32_t x,
    vg_lite_int32_t y,
    vg_lite_int32_t right,
    vg_lite_int32_t bottom)
{
    tnIN();
    DUMP_API_CALL(vg_lite_set_scissor, "x:%d y:%d right:%d bottom:%d\n", x, y, right, bottom);

    if ((x == -1) && (y == -1) && (right == -1) && (bottom == -1))
    {
        // User wants to disable scissoring: drop every scissor rectangle
        // set so far, not only the most recently added one.
        if (g_img_ctx.scissor_num > 0)
        {
            tn_regs_disable_scissors();
            g_img_ctx.scissor_num = 0;
            // Clear all cached rects so a later
            // vg_lite_get_parameter(VG_LITE_SCISSOR_RECT) does not report stale
            // data for scissors that are no longer active.
            tn_memset(&g_img_ctx.scissor_rects[0], 0, sizeof(g_img_ctx.scissor_rects));
        }

        tnOUT();
        return VG_LITE_SUCCESS;
    }

    tnRET_IF(right < x || bottom < y, VG_LITE_INVALID_ARGUMENT);
    tnRET_IF(g_img_ctx.scissor_num >= SCISSOR_NUM, VG_LITE_OUT_OF_RESOURCES);

    if (x < 0)
    {
        x = 0;
    }
    if (y < 0)
    {
        y = 0;
    }

    vg_lite_int32_t w = right - x + 1;
    vg_lite_int32_t h = bottom - y + 1;
    if (w <= 0 || h <= 0)
    {
        w = 0;
        h = 0;
    }

    // Cache the rect (as sx/sy/width/height, matching what is programmed to
    // the scissor register) so vg_lite_get_parameter(VG_LITE_SCISSOR_RECT)
    // can report the active scissors back to the caller.
    g_img_ctx.scissor_rects[g_img_ctx.scissor_num].sx = x;
    g_img_ctx.scissor_rects[g_img_ctx.scissor_num].sy = y;
    g_img_ctx.scissor_rects[g_img_ctx.scissor_num].w  = w;
    g_img_ctx.scissor_rects[g_img_ctx.scissor_num].h  = h;

    tn_regs_set_scissor((tn_scissor_id)g_img_ctx.scissor_num, (tn_u16)x, (tn_u16)y, (tn_u16)w, (tn_u16)h);
    g_img_ctx.scissor_num++;

    tnOUT();
    return VG_LITE_SUCCESS;
}



vg_lite_error_t
vg_lite_enable_scissor(void)
{
    tnIN();
    DUMP_API_CALL(vg_lite_enable_scissor, "\n");

    tnOUT();
    return vg_lite_enable_masklayer();
}



vg_lite_error_t
vg_lite_disable_scissor(void)
{
    tnIN();
    DUMP_API_CALL(vg_lite_disable_scissor, "\n");

    tnOUT();
    return vg_lite_disable_masklayer();
}



vg_lite_error_t
vg_lite_scissor_rects(
    vg_lite_buffer_t   *target,
    vg_lite_uint32_t    nums,
    vg_lite_rectangle_t rect[])
{
    tnIN();
    DUMP_API_CALL(vg_lite_scissor_rects, "target:%p nums:%d rect:%p\n", target, nums, rect);

    vg_lite_error_t vg_err;

    for (int i = 0; i < nums; i++) {
        // vg_lite_scissor_rects uses mask layer as scissor
        vg_err = vg_lite_fill_masklayer(target, &rect[i], 0);
        tnRET_IF(vg_err != VG_LITE_SUCCESS, vg_err);
    }

    tnOUT();
    return VG_LITE_SUCCESS;
}



vg_lite_error_t
vg_lite_dest_global_alpha(
    vg_lite_global_alpha_t alpha_mode,
    vg_lite_uint8_t        alpha_value)
{
    tnIN();
    DUMP_API_CALL(vg_lite_dest_global_alpha, "alpha_mode:%d alpha_value:%d\n", alpha_mode, alpha_value);

    switch (alpha_mode)
    {
    case VG_LITE_NORMAL:
        g_img_ctx.dst_alpha_mode = TN_LOCAL_ALPHA;
        break;
    case VG_LITE_GLOBAL:
        g_img_ctx.dst_alpha_mode = TN_GLOBAL_ALPHA;
        break;
    case VG_LITE_SCALED:
        g_img_ctx.dst_alpha_mode = TN_SCALED_ALPHA;
        break;
    default:
        tnABORT(TN_FAULT_UNREACHABLE, "Unknown destination alpha mode");
    }

    g_img_ctx.dst_alpha_value =  alpha_value;

    tnOUT();
    return VG_LITE_SUCCESS;
}



vg_lite_error_t
vg_lite_set_color_key(vg_lite_color_key4_t colorkey)
{
    tnIN();
    DUMP_API_CALL(vg_lite_set_color_key, "colorkey:%p\n", colorkey);

    tn_u32 low, high;
    g_img_ctx.color_key_on_s2 = tnFALSE;

    for (tn_i8 i = 0; i < 4; i++) {
        if (colorkey[i].enable)
        {
            g_img_ctx.color_key_on_s2 = tnTRUE; //by default, color key is enabled on S2
        }
        low  = (colorkey[i].low_r << 16) | (colorkey[i].low_g << 8) | colorkey[i].low_b;
        high = (colorkey[i].high_r << 16) | (colorkey[i].high_g << 8) | colorkey[i].high_b;
        tn_regs_set_color_key((tn_col_key_id)i, colorkey[i].enable, low, high, colorkey[i].alpha);
    }

    if (!g_img_ctx.color_key_on_s2)
    {
        g_img_ctx.color_key_on_s1 = tnFALSE;
        if (g_img_ctx.color_key_on_s1)
        {
            tn_regs_reset_color_key(S1);
        }
    }

    tnOUT();
    return VG_LITE_SUCCESS;
}



vg_lite_error_t
vg_lite_set_CLUT(
    tn_u32  count,
    tn_u32 *colors)
{
    tnIN();
    DUMP_API_CALL(vg_lite_set_CLUT, "count:%d colors:%p\n", count, colors);

    tnRET_IF(count > 256, VG_LITE_INVALID_ARGUMENT);
    tnRET_IF(tn_is_null(colors), VG_LITE_INVALID_ARGUMENT);

    tn_regs_set_lut(count, colors);

    tnOUT();
    return VG_LITE_SUCCESS;
}



vg_lite_error_t
vg_lite_enable_dither(void)
{
    tnIN();
    DUMP_API_CALL(vg_lite_enable_dither, "\n");

    tn_regs_enable_dither();

    tnOUT();
    return VG_LITE_SUCCESS;
}



vg_lite_error_t
vg_lite_disable_dither(void)
{
    tnIN();
    DUMP_API_CALL(vg_lite_disable_dither, "\n");

    tn_regs_disable_dither();

    tnOUT();
    return VG_LITE_SUCCESS;
}



vg_lite_error_t
vg_lite_set_command_buffer_size(vg_lite_uint32_t size)
{
    tnIN();
    DUMP_API_CALL(vg_lite_set_command_buffer_size, "size:%d\n", size);

    tn_result result = tn_cmd_buf_ccdma_set_buffer_size(size);

    tnOUT();
    return result == TN_NO_ERROR ? VG_LITE_SUCCESS : VG_LITE_INVALID_ARGUMENT;
}



vg_lite_error_t
vg_lite_get_mem_size(vg_lite_uint32_t *size)
{
    tnIN();
    DUMP_API_CALL(vg_lite_get_mem_size, "size:%p\n", size);

    tnRET_IF(tn_is_null(size), VG_LITE_INVALID_ARGUMENT);

    *size = (vg_lite_uint32_t)tn_mspace_get_free_space(TN_MSPACE_SHMEM_0);

    tnOUT();
    return VG_LITE_SUCCESS;
}



vg_lite_error_t
vg_lite_set_command_buffer(
    tn_u32 physical,
    tn_u32 size)
{
    tnIN();
    DUMP_API_CALL(vg_lite_set_command_buffer, "physical:%d size:%d\n", physical, size);

    tn_cmd_buf_ccdma_deinit();
    tn_result rv = tn_cmd_buf_ccdma_init(physical, size);
    tnRET_IF(rv != TN_NO_ERROR, tn_result_to_vg_lite_error(rv));

    tnOUT();
    return VG_LITE_SUCCESS;
}



// Generate a 3x3 homogenous matrix to transform 4 source coordinates to 4 target coordinates.
vg_lite_error_t
vg_lite_get_transform_matrix(
    vg_lite_float_point4_t src,
    vg_lite_float_point4_t dst,
    vg_lite_matrix_t      *trans)
{
    tnIN();
    DUMP_API_CALL(vg_lite_get_transform_matrix, "src:%p dst:%p trans:%p\n", src, dst, trans);

    tn_f32 A[3][3], B[3][3];
    tn_f32 gmts[3], gmtd[3];
    tn_m3f32 invA, invB, result;

    tnRET_IF(tn_is_null(trans), VG_LITE_INVALID_ARGUMENT);

    A[0][0] = src[0].x; A[0][1] = src[1].x; A[0][2] = src[2].x;
    A[1][0] = src[0].y; A[1][1] = src[1].y; A[1][2] = src[2].y;
    A[2][0] = 1.0f; A[2][1] = 1.0f; A[2][2] = 1.0f;

    B[0][0] = dst[0].x; B[0][1] = dst[1].x; B[0][2] = dst[2].x;
    B[1][0] = dst[0].y; B[1][1] = dst[1].y; B[1][2] = dst[2].y;
    B[2][0] = 1.0f; B[2][1] = 1.0f; B[2][2] = 1.0f;

    tn_memcpy((void*)invA.m, (void*)A, sizeof(invA));
    tn_memcpy((void*)invB.m, (void*)B, sizeof(invB));
    if (!tn_m3f32_invert_imm(&invA) || !tn_m3f32_invert_imm(&invB))
    {
        tnOUT();
        return VG_LITE_INVALID_ARGUMENT;
    }
    for (int i = 0; i < 3; i++) {
        gmts[i] = invA.m[i][0] * src[3].x + invA.m[i][1] * src[3].y + invA.m[i][2] * 1.0f;
        gmtd[i] = invB.m[i][0] * dst[3].x + invB.m[i][1] * dst[3].y + invB.m[i][2] * 1.0f;
    }
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            A[i][j] *= gmts[j];
            B[i][j] *= gmtd[j];
        }
    }
    tn_memcpy((void*)invA.m, (void*)A, sizeof(invA));
    if (!tn_m3f32_invert_imm(&invA))
    {
        tnOUT();
        return VG_LITE_INVALID_ARGUMENT;
    }
    tn_memcpy((void*)result.m, (void*)B, sizeof(result.m));
    tn_m3f32_mul_m3f32_imm(&result, &invA);

    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            trans->m[i][j] = result.m[i][j];
        }
    }

    tnOUT();
    return VG_LITE_SUCCESS;
}



vg_lite_error_t
vg_lite_enable_color_transform(void)
{
    tnIN();
    DUMP_API_CALL(vg_lite_enable_color_transform, "\n");

    tn_regs_enable_color_transform();

    tnOUT();
    return VG_LITE_SUCCESS;
}



vg_lite_error_t
vg_lite_disable_color_transform(void)
{
    tnIN();
    DUMP_API_CALL(vg_lite_disable_color_transform, "\n");

    tn_regs_disable_color_transform();

    tnOUT();
    return VG_LITE_SUCCESS;
}



vg_lite_error_t
vg_lite_set_color_transform(vg_lite_color_transform_t *values)
{
    tnIN();
    DUMP_API_CALL(vg_lite_set_color_transform, "values:%p\n", values);

    tn_regs_set_color_transform((tn_color_transform*)values);

    tnOUT();
    return VG_LITE_SUCCESS;
}



//***********************************************************************************
//***********************************************************************************
//                         Mask handling functions
//***********************************************************************************
//***********************************************************************************

vg_lite_error_t
vg_lite_enable_masklayer(void)
{
    tnIN();
    DUMP_API_CALL(vg_lite_enable_masklayer, "\n");

    /* Using the last mask layer created with vg_lite_create_masklayer */
    g_img_ctx.mask_color_enabled = tnTRUE;
    if (g_img_ctx.mask_color_gbuf != tnNULL)
    {
        const tn_gbuf *msk = g_img_ctx.mask_color_gbuf;

        if (!tn_gbuf_is_valid(msk) ||
            !tn_fmt_is_mask_fmt((tn_color_format)msk->color_format))
        {
            g_img_ctx.mask_color_gbuf = tnNULL;
            tn_regs_disable_mask();

            tnOUT();
            return VG_LITE_NOT_SUPPORT;
        }

        tn_u8 mask_fmt = tn_fmt_get_mask_fmt_reg(
            (tn_color_format)msk->color_format);

        tn_regs_enable_mask(
            mask_fmt,
            1                             //num tiles
        );
        tn_mask_set(msk);
    }


    tnOUT();
    return VG_LITE_SUCCESS;
}



vg_lite_error_t
vg_lite_disable_masklayer(void)
{
    tnIN();
    DUMP_API_CALL(vg_lite_disable_masklayer, "\n");

    g_img_ctx.mask_color_enabled = tnFALSE;
    tn_regs_disable_mask();

    tnOUT();
    return VG_LITE_SUCCESS;
}



vg_lite_error_t
vg_lite_create_masklayer(
    vg_lite_buffer_t *masklayer,
    vg_lite_uint32_t  width,
    vg_lite_uint32_t  height)
{
    tnIN();
    DUMP_API_CALL(vg_lite_create_masklayer, "masklayer:%p width:%d height:%d\n", masklayer, width, height);

    tnRET_IF(tn_is_null(masklayer), VG_LITE_INVALID_ARGUMENT);
    tn_color_format color_format = vglite_color_2_tn_color(masklayer->format);
    tnRET_IF(color_format == TN_MAX_COLOR_FORMAT, VG_LITE_NOT_SUPPORT);

    tn_color colorI;
    tn_color_copy(color_format, &colorI);

    masklayer->stride = width * colorI.bpp >> 3;
    if (masklayer->stride == 0 && width > 0)
        masklayer->stride = 1; // at least one byte for A1 and A4
    tn_u16 trailing_byte = 0;
    if (color_format == TN_A_4)
    {
        if (width & 0x1)
            trailing_byte = 1;
    }
    else if (color_format == TN_A_1)
    {
        if (width & 0x7)
            trailing_byte = 1;
    }

    masklayer->stride = (width * colorI.bpp >> 3) + trailing_byte;

    masklayer->width  = width;
    masklayer->height = height;

    tn_result res = tn_gbuf_create(&masklayer->gbuf,
                                   &(tn_gbuf_layout) {
        .width = (tn_map_coord)masklayer->width,
        .height = (tn_map_coord)masklayer->height,
        .stride = (tn_map_coord)masklayer->stride,
        .color_format = (tn_u32)color_format,
        .yuv2rgb = 0,
        .size = (tn_size)masklayer->stride * height,
        .uv_offset = 0,
        .v_offset = 0
    });
    tnRET_IF(res != TN_NO_ERROR, VG_LITE_OUT_OF_MEMORY);

    masklayer->memory = masklayer->gbuf.vaddr;
    masklayer->address = (tn_u32)masklayer->gbuf.paddr;

    if (g_img_ctx.mask_color_gbuf == tnNULL && g_img_ctx.mask_color_enabled)
    {
        g_img_ctx.mask_color_gbuf = &masklayer->gbuf;
        /* masklayer has been previously enabled but there was no layer available. Enabling now. */
        vg_lite_enable_masklayer();
    }
    else
    {
        g_img_ctx.mask_color_gbuf = &masklayer->gbuf;
    }

    tnOUT();
    return VG_LITE_SUCCESS;
}



vg_lite_error_t
vg_lite_destroy_masklayer(vg_lite_buffer_t *masklayer)
{
    tnIN();
    DUMP_API_CALL(vg_lite_destroy_masklayer, "masklayer:%p\n", masklayer);

    tnRET_IF(tn_is_null(masklayer), VG_LITE_INVALID_ARGUMENT);

    if (tn_gbuf_is_valid(&masklayer->gbuf))
    {
        if (g_img_ctx.mask_color_gbuf == &masklayer->gbuf)
        {
            g_img_ctx.mask_color_gbuf = tnNULL;
        }

        tnIGNORE_RESULT(tn_gbuf_destroy(&masklayer->gbuf));

        masklayer->address = 0;
        masklayer->memory  = tnNULL;
    }

    tnOUT();
    return VG_LITE_SUCCESS;
}



vg_lite_error_t
vg_lite_fill_masklayer(
    vg_lite_buffer_t    *masklayer,
    vg_lite_rectangle_t *rect,
    vg_lite_uint8_t      value)
{
    tnIN();
    DUMP_API_CALL(vg_lite_fill_masklayer, "masklayer:%p rect:%p value:%d\n", masklayer, rect, value);

    tnRET_IF(tn_is_null(masklayer), VG_LITE_INVALID_ARGUMENT);

    vg_lite_error_t err = vg_lite_finish();
    tnRET_IF(err != VG_LITE_SUCCESS, err);

    tn_result res = tn_mask_fill(
        &masklayer->gbuf,
        &(tn_blt_xform_t){
        .src_img = tnNULL,
        .dst_img = tnNULL,
        .sx = 0,
        .sy = 0,
        .dx = rect != tnNULL ? (tn_map_coord)rect->x : 0,
        .dy = rect != tnNULL ? (tn_map_coord)rect->y : 0,
        .width = rect != tnNULL ? (tn_map_coord)rect->width : masklayer->width,
        .height = rect != tnNULL ? (tn_map_coord)rect->height : masklayer->height
    },
        value);

    tnRET_IF(res != TN_NO_ERROR, VG_LITE_INVALID_ARGUMENT);

    tnOUT();
    return VG_LITE_SUCCESS;
}



vg_lite_error_t
vg_lite_blend_masklayer(
    vg_lite_buffer_t        *dst,
    vg_lite_buffer_t        *src,
    vg_lite_mask_operation_t operation,
    vg_lite_rectangle_t     *rect)
{
    tnIN();
    DUMP_API_CALL(vg_lite_blend_masklayer, "dst:%p src:%p operation:%d rect:%p\n", dst, src, operation, rect);

    tnRET_IF(tn_is_null(dst), VG_LITE_INVALID_ARGUMENT);

    tn_result res;
    tn_u32 tmp_color_format;
    vg_lite_error_t err;

    tn_map_coord sx, sy, dx, dy, width, height;

    if (rect == NULL)
    {
        sx = 0;
        sy = 0;
        dx = 0;
        dy = 0;
        width = dst->width;
        height = dst->height;
    }
    else
    {
        if (rect->x < 0)
        {
            sx = -rect->x;
            dx = 0;
            width = rect->width + rect->x;
        }
        else
        {
            sx = 0;
            dx = rect->x;
            width = rect->width;
        }

        if (rect->y < 0)
        {
            sy = -rect->y;
            dy = 0;
            height = rect->height + rect->y;
        }
        else
        {
            sy = 0;
            dy = rect->y;
            height = rect->height;
        }
    }

    if (width <= 0 || height <= 0)
    {
        // nothing to do...
        return VG_LITE_SUCCESS;
    }

    switch (operation)
    {
    case VG_LITE_CLEAR_MASK:
        //This operation sets all mask values in the region of interest to 0,
        //ignoring the new mask layer.

        //update width and height to fit within dst bounds
        width = (width + dx > dst->width) ? (dst->width - dx) : width;
        height = (height + dy > dst->height) ? (dst->height - dy) : height;

        err = vg_lite_finish();
        tnRET_IF(err != VG_LITE_SUCCESS, err);

        res = tn_mask_fill(
            &dst->gbuf,
            &(tn_blt_xform_t){
            .src_img = tnNULL,
            .dst_img = tnNULL,
            .sx = sx,
            .sy = sy,
            .dx = dx,
            .dy = dy,
            .width = width,
            .height = height
        },
            0);
        break;
    case VG_LITE_FILL_MASK:
        // This operation sets all mask values in the region of interest to 1,
        // ignoring the new mask layer.

        //update width and height to fit within dst bounds
        width = (width + dx > dst->width) ? (dst->width - dx) : width;
        height = (height + dy > dst->height) ? (dst->height - dy) : height;

        err = vg_lite_finish();
        tnRET_IF(err != VG_LITE_SUCCESS, err);

        res = tn_mask_fill(
            &dst->gbuf,
            &(tn_blt_xform_t){
            .src_img = tnNULL,
            .dst_img = tnNULL,
            .sx = sx,
            .sy = sy,
            .dx = dx,
            .dy = dy,
            .width = width,
            .height = height
        },
            255);
        break;
    case VG_LITE_SET_MASK:
    case VG_LITE_UNION_MASK:
    case VG_LITE_INTERSECT_MASK:
    case VG_LITE_SUBTRACT_MASK:
    {
        tnRET_IF(tn_is_null(src), VG_LITE_INVALID_ARGUMENT);

        // before blending update the color format in source image
        // this is required because upper layers may change source color format
        // to be able to replace red channel info with alpha channel info without
        // change each pixel

        //update width and height to fit within src and dst bounds
        width = (width + sx > src->width) ? (src->width - sx) : width;
        width = (width + dx > dst->width) ? (dst->width - dx) : width;
        height = (height + sy > src->height) ? (src->height - sy) : height;
        height = (height + dy > dst->height) ? (dst->height - dy) : height;

        tn_color_format mask_color_format = vglite_color_2_tn_color(src->format);
        tnRET_IF(mask_color_format == TN_MAX_COLOR_FORMAT, VG_LITE_NOT_SUPPORT);
        tmp_color_format = src->gbuf.color_format;
        src->gbuf.color_format = (tn_u32)mask_color_format;
        res = tn_mask_blend(
            &dst->gbuf,
            &src->gbuf,
            vglite_mask_op_to_tn_blend(operation),
            &(tn_blt_xform_t){
                .src_img = tnNULL,
                .dst_img = tnNULL,
                .sx = sx,
                .sy = sy,
                .dx = dx,
                .dy = dy,
                .width = width,
                .height = height
            });
        src->gbuf.color_format = tmp_color_format;
        break;
    }
    default:
        res = TN_ILLEGAL_ARGUMENT_ERROR;
    }

    tnRET_IF(res != TN_NO_ERROR, tn_result_to_vg_lite_error(res));

    tnOUT();
    return VG_LITE_SUCCESS;
}



vg_lite_error_t
vg_lite_set_masklayer(
    vg_lite_buffer_t *masklayer,
    vg_lite_int32_t   x,
    vg_lite_int32_t   y)
{
    tnIN();
    DUMP_API_CALL(vg_lite_set_masklayer, "masklayer:%p x:%d y:%d\n", masklayer, x, y);

    tnRET_IF(tn_is_null(masklayer), VG_LITE_INVALID_ARGUMENT);

    tnUNUSED(x);
    tnUNUSED(y);

    g_img_ctx.mask_color_gbuf = &masklayer->gbuf;
    tn_result res = tn_mask_set(g_img_ctx.mask_color_gbuf);
    tnRET_IF(res != TN_NO_ERROR, VG_LITE_INVALID_ARGUMENT);

    tnOUT();
    return VG_LITE_SUCCESS;
}



vg_lite_error_t
vg_lite_get_parameter(
    vg_lite_param_type_t type,
    vg_lite_int32_t      count,
    vg_lite_pointer      params)
{
    tnIN();
    DUMP_API_CALL(vg_lite_get_parameter, "type:%d count:%d params:%p\n", type, count, params);

    tnRET_IF(tn_is_null(params), VG_LITE_INVALID_ARGUMENT);

#if defined(__GNUC__)
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wswitch"
#elif defined(__IAR_SYSTEMS_ICC__)
    #pragma diag_suppress=Pe047
#elif defined(__ARMCC_VERSION) && (__ARMCC_VERSION < 6000000)
    #pragma diag_suppress 1296
#endif

    switch ((tn_u32)type)
    {
    case VG_LITE_GPU_IDLE_STATE:                     /*! count must be 1 for GPU idle state TRUE or FALSE */
    {
        tnRET_IF(count != 1 || tn_is_null(params), VG_LITE_INVALID_ARGUMENT);

        // GPU is idle when the driver has no CCDMA command queues that were
        // submitted but not yet acknowledged (flushed) by the completion ISR.
        tn_u32 *idle = (tn_u32*)params;
        *idle = tn_hw_ccdma_has_pending_cmdqs() ? tnFALSE : tnTRUE;
        break;
    }
    case VG_LITE_SCISSOR_RECT:                       /*! count must be 4n for x, y, right, bottom */
    {
        tnRET_IF(count < 4 || (count % 4 ) != 0 || (count / 4) > SCISSOR_NUM || tn_is_null(params), VG_LITE_INVALID_ARGUMENT);

        // scissor_rects[] is cached as (sx, sy, w, h), but this API reports
        // rects as (x, y, right, bottom). Convert per rect using the inverse
        // of what vg_lite_set_scissor programmed (w = right - x + 1). Slots
        // beyond the number of active scissors (scissor_num) are zero-filled.
        tn_i32 *val = (tn_i32*)params;
        tn_i32 num_rects = count / 4;
        for (tn_i32 r = 0; r < num_rects; r++)
        {
            if (r < g_img_ctx.scissor_num)
            {
                tn_i32 sx = g_img_ctx.scissor_rects[r].sx;
                tn_i32 sy = g_img_ctx.scissor_rects[r].sy;
                tn_i32 w  = g_img_ctx.scissor_rects[r].w;
                tn_i32 h  = g_img_ctx.scissor_rects[r].h;

                val[r * 4 + 0] = sx;                          // x
                val[r * 4 + 1] = sy;                          // y
                val[r * 4 + 2] = sx + w - 1;                  // right
                val[r * 4 + 3] = sy + h - 1;                  // bottom
            }
            else
            {
                val[r * 4 + 0] = 0;
                val[r * 4 + 1] = 0;
                val[r * 4 + 2] = 0;
                val[r * 4 + 3] = 0;
            }
        }
        break;
    }
    case VG_LITE_HARDWARE_RUNNING_TIME:              /*! count must be 1 */
    {
        tnRET_IF(count != 1 || tn_is_null(params), VG_LITE_INVALID_ARGUMENT);

        // Report the time in milliseconds since the driver started running
        // (tn_hw_init). tn_hw_get_running_time_ms returns tnFALSE on targets
        // without a usable running-time source (non-Cortex-M).
        tn_u32 *run_time_ms = (tn_u32*)params;
        if (!tn_hw_get_running_time_ms(run_time_ms))
        {
            tnOUT();
            return VG_LITE_NOT_SUPPORT;
        }
        break;
    }
    case VG_LITE_SRC_BUF_ALIGNED_CHECK:              /*! count must be 1 for src buffer*/
    {
        if ((count != 1) || (params == NULL))
        {
            tnOUT();
            return VG_LITE_INVALID_ARGUMENT;
        }

        // Report whether the source buffer meets the GPU's address/stride/
        // format alignment constraints. The constraints are gated on the
        // runtime feature table (tn_ctx_api_has_feature), so only the checks
        // enabled for the current GPU generation are applied.
        vg_lite_buffer_t *src_buf = (vg_lite_buffer_t*)params;
        if (tn_ctx_api_has_feature(gcFEATURE_BIT_VG_16PIXELS_ALIGN)
            && vglite_source_stride_align_check(src_buf->format, (tn_u32)src_buf->stride) != VG_LITE_SUCCESS)
        {
            tnOUT();
            return VG_LITE_NOT_ALIGNED;
        }
        if (vglite_srcbuf_align_check(src_buf) != VG_LITE_SUCCESS)
        {
            tnOUT();
            return VG_LITE_NOT_ALIGNED;
        }
        // ETC2 formats require 4x4-aligned width and height.
        if ((src_buf->format == VG_LITE_RGBA8888_ETC2_EAC || src_buf->format == VG_LITE_RGB888_ETC2_EAC)
            && ((src_buf->width % 4) || (src_buf->height % 4)))
        {
            tnOUT();
            return VG_LITE_NOT_ALIGNED;
        }
        break;
    }
    case VG_LITE_DST_BUF_ALIGNED_CHECK:              /*! count must be 1 for dst buffer*/
    {
        if ((count != 1) || (params == NULL))
        {
            tnOUT();
            return VG_LITE_INVALID_ARGUMENT;
        }

        // Report whether the destination buffer meets the GPU's address/stride/
        // format alignment constraints. The constraints are gated on the
        // runtime feature table (tn_ctx_api_has_feature), so only the checks
        // enabled for the current GPU generation are applied.
        vg_lite_buffer_t *dst_buf = (vg_lite_buffer_t*)params;
        if (vglite_dstbuf_align_check(dst_buf) != VG_LITE_SUCCESS)
        {
            tnOUT();
            return VG_LITE_NOT_ALIGNED;
        }
        if (vglite_peclear_align_check(dst_buf, dst_buf->height) != VG_LITE_SUCCESS)
        {
            tnOUT();
            return VG_LITE_NOT_ALIGNED;
        }
        break;
    }
    case VG_LITE_HW_ID:                              /*! count must be 1 for GPU hw id */
    {
        if ((count != 1) || (params == NULL))
        {
            tnOUT();
            return VG_LITE_INVALID_ARGUMENT;
        }

        tn_u32 *reg_value = (tn_u32*)params;
        *reg_value = tn_hw_reg_get(TINGPU_VERID);
        break;
    }
    case VG_LITE_EXT_CYCLE_COUNTERS:                 /*! count must be 1 */
    {
        vg_lite_ext_prof_counters_t *counters = (vg_lite_ext_prof_counters_t*)params;

        if (counters == NULL)
        {
            tnOUT();
            return VG_LITE_INVALID_ARGUMENT;
        }

        tn_prof_get_counters((tn_prof_counters*)counters);
        break;
    }
    default:
        tnOUT();
        return VG_LITE_INVALID_ARGUMENT;
    }

#if defined(__GNUC__)
    #pragma GCC diagnostic pop
#elif defined(__IAR_SYSTEMS_ICC__)
    #pragma diag_default=Pe047
#elif defined(__ARMCC_VERSION) && (__ARMCC_VERSION < 6000000)
    #pragma diag_default 1296
#endif

    tnOUT();
    return VG_LITE_SUCCESS;
}



vg_lite_error_t
vg_lite_copy_image(
    vg_lite_buffer_t *target,
    vg_lite_buffer_t *source,
    vg_lite_int32_t   dx,
    vg_lite_int32_t   dy,
    vg_lite_int32_t   sx,
    vg_lite_int32_t   sy,
    vg_lite_uint32_t  width,
    vg_lite_uint32_t  height)
{
    tnIN();
    DUMP_API_CALL(vg_lite_copy_image, "target:%p source:%p dx:%d dy:%d sx:%d sy:%d width:%d height:%d\n",
                  target, source, dx, dy, sx, sy, width, height);

    tnRET_IF(tn_is_null(target) || tn_is_null(source), VG_LITE_INVALID_ARGUMENT);
    tnRET_IF(!(width > 0 && height > 0), VG_LITE_INVALID_ARGUMENT);

    vg_lite_matrix_t matrix;
    vg_lite_error_t err;

    tnRET_IF(!(dx >= 0 && dy >= 0 && sx >= 0 && sy >= 0), VG_LITE_INVALID_ARGUMENT);
    tnRET_IF(!(sx <= source->width && sy <= source->height), VG_LITE_INVALID_ARGUMENT);
    tnRET_IF(!(dx <= target->width || dy <= target->height), VG_LITE_INVALID_ARGUMENT);

    /* Define source rectangle. */
    vg_lite_rectangle_t rect =
    {
        .x      = sx,
        .y      = sy,
        .width  = width,
        .height = height
    };

    /* Align to the source dimension */
    if ((rect.x + rect.width) > source->width)
    {
        rect.width = source->width - rect.x;
    }
    if ((rect.y + rect.height) > source->height)
    {
        rect.height = source->height - rect.y;
    }
    /* Align to the target dimension */
    if ((dx + rect.width) > target->width)
    {
        rect.width = target->width - dx;
    }
    if ((dy + rect.height) > target->height)
    {
        rect.height = target->height - dy;
    }

    vg_lite_identity(&matrix);
    // vg_lite_translate only mutates the matrix and always returns
    // VG_LITE_SUCCESS, so the result is intentionally discarded.
    tnIGNORE_RESULT(vg_lite_translate((vg_lite_float_t)dx, (vg_lite_float_t)dy, &matrix));

    err = vg_lite_blit_rect(
        target,
        source,
        &rect,
        &matrix,
        VG_LITE_BLEND_NONE,
        0,
        VG_LITE_FILTER_POINT
    );

    tnOUT();
    return err;
}



vg_lite_error_t
vg_lite_cache_command(
    vg_lite_cmdcache_operation_t operation,
    vg_lite_int32_t             *buf_index,
    vg_lite_matrix_t            *matrix)
{
    tnIN();
    DUMP_API_CALL(vg_lite_cache_command, "operation:%d buf_index:%p matrix:%p\n", operation, buf_index, matrix);

    tnRET_IF(tn_is_null(buf_index), VG_LITE_INVALID_ARGUMENT);

    tn_result result = tn_cmd_buf_ccdma_cache_command(
        (tn_cmd_buf_cache_op_t)operation,
        buf_index,
        (const tn_m3f32*)matrix);

    tnOUT();
    return tn_result_to_vg_lite_error(result);
}



// --- Color / gamma ---

vg_lite_error_t
vg_lite_set_gamma(vg_lite_gamma_conversion_t gamma_value)
{
    tnIN();
    DUMP_API_CALL(vg_lite_set_gamma, "gamma_value:%d\n", gamma_value);

    switch (gamma_value)
    {
    case VG_LITE_GAMMA_NO_CONVERSION:
        g_img_ctx.gamma_en = tnFALSE;
        g_img_ctx.gamma_cs = TN_CS_LINEAR;
        break;
    case VG_LITE_GAMMA_LINEAR:
        g_img_ctx.gamma_en = tnTRUE;
        g_img_ctx.gamma_cs = TN_CS_LINEAR;
        break;
    case VG_LITE_GAMMA_NON_LINEAR:
        g_img_ctx.gamma_en = tnTRUE;
        g_img_ctx.gamma_cs = TN_CS_NONLINEAR;
        break;
    default:
        tnOUT();
        return VG_LITE_INVALID_ARGUMENT;
    }

    tnOUT();
    return VG_LITE_SUCCESS;
}



vg_lite_error_t
vg_lite_set_dump_api(vg_lite_char flag)
{
    tnIN_ARG("flag:%d", flag);

    g_img_ctx.dump_api_flag = flag ? tnTRUE : tnFALSE;

    tnOUT();
    return VG_LITE_SUCCESS;
}
