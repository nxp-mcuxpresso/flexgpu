//
// Copyright 2025-2026 NXP
//
// SPDX-License-Identifier: MIT
//

#include "vg_lite.h"
#include "vg_lite_ext.h"
#include "vglite_tin_utils.h"
#include "tn_regs_image.h"
#include "tn_trace.h"
#include "tn_trace_api.h"
#include "tn_gbuf.h"
#include "tn_fmt.h"
#include "tn_ctx_img.h"
#include "tn_misc.h"

#include "tn_mspace.h"
#include "tn_obj.h"
#include "tn_mem.h"
#include "tn_hw.h"




vg_lite_error_t
vg_lite_enable_dither_ext(
    vg_lite_uint8_t matrix_type,
    vg_lite_uint8_t order_scale_factor_r,
    vg_lite_uint8_t order_scale_factor_g,
    vg_lite_uint8_t order_scale_factor_b,
    vg_lite_uint8_t order_scale_factor_a,
    vg_lite_uint8_t order_scale_factor_bw)
{
    tnIN();
    DUMP_API_CALL(vg_lite_enable_dither_ext,
                  "matrix_type:%d order_scale_factor_r:%d order_scale_factor_g:%d "
                  "order_scale_factor_b:%d order_scale_factor_a:%d order_scale_factor_bw:%d\n",
                  matrix_type, order_scale_factor_r, order_scale_factor_g,
                  order_scale_factor_b, order_scale_factor_a, order_scale_factor_bw);

    tn_regs_enable_dither_ext(matrix_type,
                              order_scale_factor_r,
                              order_scale_factor_g,
                              order_scale_factor_b,
                              order_scale_factor_a,
                              order_scale_factor_bw);

    tnOUT();
    return VG_LITE_SUCCESS;
}



vg_lite_error_t
vg_lite_enable_signed_fmt_ext(
    vg_lite_ext_signed_fmt_t fmt,
    vg_lite_uint32_t         scale,
    vg_lite_uint16_t         zero_point)
{
    tnIN();
    DUMP_API_CALL(vg_lite_enable_signed_fmt_ext, "fmt:%d scale:%d zero_point:%d\n",
                  fmt, scale, zero_point);

    tn_regs_enable_signed_fmt(fmt, scale, zero_point);

    tnOUT();
    return VG_LITE_SUCCESS;
}



vg_lite_error_t
vg_lite_disable_signed_fmt_ext()
{
    tnIN();
    DUMP_API_CALL(vg_lite_disable_signed_fmt_ext, "\n");

    tn_regs_disable_signed_fmt();

    tnOUT();
    return VG_LITE_SUCCESS;
}



vg_lite_error_t
vg_lite_wrap_buffer_ext(
    vg_lite_buffer_t *buffer,
    vg_lite_void     *user_buffer,
    vg_lite_uint32_t  user_buffer_size)
{
    tnIN();
    DUMP_API_CALL(vg_lite_wrap_buffer_ext, "buffer:%p user_buffer:%p user_buffer_size:%d\n",
                  buffer, user_buffer, user_buffer_size);

    tnRET_IF(tn_is_null(buffer), VG_LITE_INVALID_ARGUMENT);
    tnRET_IF(tn_is_null(user_buffer), VG_LITE_INVALID_ARGUMENT);
    tnRET_IF(user_buffer_size == 0, VG_LITE_INVALID_ARGUMENT);

    tn_color_format color_format = vglite_color_2_tn_color(buffer->format);
    tnRET_IF(color_format == TN_MAX_COLOR_FORMAT, VG_LITE_NOT_SUPPORT);

    tn_result res = vglite_buffer_gbuf_wrap(&buffer->gbuf,
                                            buffer,
                                            (tn_vptr)user_buffer,
                                            (tn_size)user_buffer_size);
    tnRET_IF(res == TN_ILLEGAL_ARGUMENT_ERROR, VG_LITE_INVALID_ARGUMENT);
    tnRET_IF(res != TN_NO_ERROR, VG_LITE_OUT_OF_RESOURCES);

    buffer->memory  = buffer->gbuf.vaddr;
    buffer->address = (tn_u32)buffer->gbuf.paddr;
    buffer->handle = buffer->memory;

    if (buffer->gbuf.uv_offset != 0)
    {
        buffer->yuv.uv_memory = (tn_vptr)((tn_u8*)buffer->gbuf.vaddr + buffer->gbuf.uv_offset);
    }

    if (buffer->gbuf.v_offset != 0)
    {
        buffer->yuv.v_memory = (tn_vptr)((tn_u8*)buffer->gbuf.vaddr + buffer->gbuf.v_offset);
    }

    // Mark the buffer as external so that vg_lite_free() does not release the
    // caller-owned memory.
    buffer->external = tnTRUE;

    tnOUT();
    return VG_LITE_SUCCESS;
}



// Release the internal driver objects created by vg_lite_wrap_buffer_ext.
// The caller-owned backing memory is left intact and must be freed by the
// caller.
vg_lite_error_t
vg_lite_unwrap_buffer_ext(vg_lite_buffer_t *buffer)
{
    tnIN();
    DUMP_API_CALL(vg_lite_unwrap_buffer_ext, "buffer:%p\n", buffer);

    tnRET_IF(tn_is_null(buffer), VG_LITE_INVALID_ARGUMENT);

    if (tn_gbuf_is_valid(&buffer->gbuf))
    {
        const tn_bool was_yuv = tn_fmt_is_yuv((tn_color_format)buffer->gbuf.color_format);

        if (g_img_ctx.mask_color_gbuf == &buffer->gbuf)
        {
            g_img_ctx.mask_color_gbuf = tnNULL;
            tn_regs_disable_mask();
        }

        tnIGNORE_RESULT(tn_gbuf_destroy(&buffer->gbuf));


        buffer->address     = 0;
        buffer->memory      = tnNULL;
        buffer->handle      = tnNULL;

        if (was_yuv)
        {
            // uv planes are part of the caller-owned memory; just clear the pointers.
            buffer->yuv.uv_memory = 0;
            buffer->yuv.v_memory  = 0;
        }
    }

    buffer->external = tnFALSE;

    tnOUT();
    return VG_LITE_SUCCESS;
}



vg_lite_error_t
vg_lite_upload_buffer_ext(
    vg_lite_buffer_t *buffer,
    vg_lite_uint8_t  *data[4],
    vg_lite_uint32_t  stride[4])
{
    tnIN();
    DUMP_API_CALL(vg_lite_upload_buffer_ext, "buffer:%p data:%p stride:%p\n", buffer, data, stride);

    tnRET_IF(tn_is_null(buffer), VG_LITE_INVALID_ARGUMENT);
    tnRET_IF(tn_is_null(data), VG_LITE_INVALID_ARGUMENT);

    tn_u8 num_planes;
    tn_u32 dst_height[4];
    tn_u32 dst_stride[4];

    get_planes_info(buffer, &num_planes, dst_height, dst_stride);
    tnRET_IF(num_planes == 0, VG_LITE_INVALID_ARGUMENT);

    tn_size required = 0;
    tnRET_IF(!vglite_planes_total_bytes(num_planes, dst_height, dst_stride, &required),
             VG_LITE_INVALID_ARGUMENT);
    tnRET_IF(!tn_gbuf_is_valid(&buffer->gbuf), VG_LITE_INVALID_ARGUMENT);
    tnRET_IF(!tn_span_fits(0, required, buffer->gbuf.size), VG_LITE_INVALID_ARGUMENT);

    vg_lite_uint8_t *pdata, *pbuffer;
    pbuffer = (vg_lite_uint8_t*)buffer->memory;
    for (tn_u8 i = 0; i < num_planes; i++)
    {
        // A source stride of 0 means the plane is tightly packed; fall back
        // to the destination plane stride in that case.
        vg_lite_uint32_t src_stride = (stride != tnNULL && stride[i] != 0) ? stride[i] : dst_stride[i];

        pdata = data[i];
        if (pdata == tnNULL)
        {
            tnOUT();
            return VG_LITE_INVALID_ARGUMENT;
        }

        for (tn_u16 j = 0; j < dst_height[i]; j++)
        {
            tn_memcpy(pbuffer, pdata, dst_stride[i]);
            pbuffer += dst_stride[i];
            pdata += src_stride;
        }
    }

    tnOUT();
    return VG_LITE_SUCCESS;
}



void
vg_lite_irq_handler_ext(void)
{
    tnIN();
    DUMP_API_CALL(vg_lite_irq_handler_ext, "\n");

    tn_hw_irq_handler();

    tnOUT();
}
