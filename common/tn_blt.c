//
// Copyright 2025-2026 NXP
//
// SPDX-License-Identifier: MIT
//

#include "tin.h"
#include "tn_blt.h"
#include "tn_gbuf.h"
#include "tn_color.h"
#include "tn_mem.h"
#include "tn_fmt.h"
#include "tn_hw.h"
#include "tn_regs_image.h"
#include "tn_gpu_regs.h"
#include "tn_ctx_img.h"
#include "tn_cmd_buf_ccdma.h"



#define FLOAT_THRESHOLD     0.000001f

tn_result
tn_blt_fill(
    tn_blt_id       id_blt,
    tn_color        color,
    tn_blt_xform_t *arg)
{
    tn_u32 packed_color;
    tn_bool dst_fill_en = tnTRUE;
    tn_paddr tgt_addr;

    tnEXPECT(!tn_is_null(arg));

    tnIN_ARG("sx:%d sy:%d dx:%d dy:%d width:%d height:%d",
             arg->sx, arg->sy, arg->dx, arg->dy, arg->width, arg->height);

    const tn_gbuf *dstI = arg->dst_img;

    tn_map_coord start_x = tnMAX(arg->dx, 0);
    tn_map_coord start_y = tnMAX(arg->dy, 0);
    /* End coordinates are first pixel outside the fill region */
    tn_map_coord end_x = tnMIN(dstI->width, arg->dx + arg->width);
    tn_map_coord end_y = tnMIN(dstI->height, arg->dy + arg->height);

    if (start_x >= dstI->width || start_y >= dstI->height || end_x <= 0 || end_y <= 0)
    {
        /* No work for rectangle out of the dst dimensions. */

        tnOUT();
        return TN_NO_ERROR;
    }

    /* Get width and height corrected in respect to dst image dimensions */
    tn_map_coord w = end_x - start_x;
    tn_map_coord h = end_y - start_y;
    if (w <= 0 || h <= 0)
    {
        /* No need to fill an empty rectangle. */

        tnOUT();
        return TN_NO_ERROR;
    }

    tn_map_coord img_stride = dstI->stride;
    tn_color colI;
    tn_color_copy(dstI->color_format, &colI);
    tn_color_csc_imm(&color, colI.colorspace);
    packed_color = tn_color_pack(&color);
    // vg_lite_clear sets the format to TN_sABGR_8888.
    // TINGPU_S1_BKGND needs it in RGBA8888 format, therefore convert it
    packed_color = tn_color_u32_bswap(packed_color);

    if (tn_fmt_is_supported_on_dst(colI))
    {
        /* Frame buffer will be filled with the value from DST_FILL,
           color transform will be performed in software, not hardware */
        if (tn_fmt_is_vglite(dstI->color_format) && g_img_ctx.gamma_en)
        {
            colI.colorspace = (colI.colorspace & ~(tn_color_space)TN_CS_NONLINEAR) | g_img_ctx.gamma_cs;
        }
        packed_color = tn_color_rgba8888_to(packed_color, &colI);
        tn_regs_enq(TINGPU_DST_FILL_DATA, packed_color);
        tn_regs_disable_s2();
    }
    else
    {
        tn_regs_set_s2_bkgnd_color(packed_color);
        tn_regs_set_size(0, 0, S2);
        dst_fill_en = tnFALSE;
        tn_reg_32 s2_ctrl = TINGPU_S2_CTRL_EN(1) |
                            tn_regs_set_premul(colI, S2) |
                            tn_regs_set_gamma(colI, S2);
        tn_regs_enq(TINGPU_S2_CTRL, s2_ctrl);
    }

    tn_regs_set_s2_bkgnd_color(packed_color);
    tn_regs_set_size(w, h, DST);
    tn_regs_enq(TINGPU_DST_STRIDE0, img_stride);
    tn_regs_set_dst_color_format(dstI, dst_fill_en, tn_fmt_is_yuv(dstI->color_format));
    tgt_addr = arg->dst_img->paddr;

    tn_regs_enq(TINGPU_S1_CTRL, TINGPU_S1_CTRL_EN(0));
    if (start_x != 0 || start_y != 0)
    {
        tn_regs_set_base_addr(tgt_addr + start_y * img_stride + (start_x * colI.bpp >> 3), DST);
    }
    else
    {
        tn_regs_set_base_addr(tgt_addr, DST);
    }

    /* ensure masking is disabled during clear */
    tn_u8 mask_en = 0;
    tn_u8 mask_fmt = 0;
    tn_u8 mask_num_tiles = 0;

    mask_en = tn_regs_get_mask_enabled();
    if (mask_en)
    {
        mask_fmt = tn_regs_get_mask_fmt();
        mask_num_tiles = tn_regs_get_mask_num_tiles();
        tn_regs_disable_mask();
    }

    tn_regs_enable_gpu_ctrl();

    /* Restore cached masking state for subsequent operations. */
    if (mask_en)
    {
        tn_regs_enable_mask(mask_fmt, mask_num_tiles);
    }

    tnOUT();
    return TN_NO_ERROR;
}



static void
blend_area_align_8x8(tn_rect_t *blend_area)
{
    tnEXPECT(!tn_is_null(blend_area));

    tn_u8 align;
    if ((align = (blend_area->x & 0x7)) != 0)
    {
        blend_area->x -= align;
        blend_area->width += align;
    }

    if ((align = (blend_area->y & 0x7)) != 0)
    {
        blend_area->y -= align;
        blend_area->height += align;
    }

    //round up to multiple of 8 pixels
    blend_area->width = ((blend_area->x + blend_area->width + 0x7) & 0xFFFFFFF8) - blend_area->x;
    blend_area->height = ((blend_area->y + blend_area->height + 0x7) & 0xFFFFFFF8) - blend_area->y;
}



static void
blend_area_align_to_byte(tn_rect_t *blend_area)
{
    tn_u8 align;
    if ((align = (blend_area->x & 0x7)) != 0)
    {
        blend_area->x -= align;
        blend_area->width += align;
    }
    blend_area->width = ((blend_area->x + blend_area->width + 0x7) & 0xFFFFFFF8) - blend_area->x;
}



static tn_bool
is_blend_patch_required(tn_m3f32 *matrix)
{
    tnEXPECT(!tn_is_null(matrix));

    float m00 = matrix->m[0][0];
    float m10 = matrix->m[1][0];

    // squared x scale factor
    float sx = m00 * m00 + m10 * m10;

    // cos2(theta) = m00^2 / sx. The check below tests whether the value is
    // near 0 or near 1, which catches all axis-aligned angles (0, 90, 180, 270 deg).
    // sin2 + cos2 = 1, so both give identical results for this threshold test.
    float sine = (m00 * m00) / sx;
    // if cos2(theta) is close to zero or 1 this is 90, 180, 270 or no rotation
    // no patching required for
    if (fabs(sine) < FLOAT_THRESHOLD || fabs(sine - 1.0f) < FLOAT_THRESHOLD)
        return tnFALSE;
    else
        return tnTRUE;
}



static void
blend_area_align_2x2(tn_rect_t *blend_area)
{
    tnEXPECT(!tn_is_null(blend_area));

    if (blend_area->x & 0x1)
    {
        blend_area->x -= 1;
        blend_area->width += 1;
    }
    if ((blend_area->x + blend_area->width) & 0x1)
    {
        blend_area->width += 1;
    }

    if (blend_area->y & 0x1)
    {
        blend_area->y -= 1;
        blend_area->height += 1;
    }
    if ((blend_area->y + blend_area->height) & 0x1)
    {
        blend_area->height += 1;
    }
}



static void
blend_src_patch(
    tn_i32 *blend_mode,
    tn_u32 *background_color)
{
    tnEXPECT(!tn_is_null(blend_mode));
    tnEXPECT(!tn_is_null(background_color));

    tn_i32 mode = *blend_mode & TN_BLEND_MODE_MASK;
    tn_i32 flags = *blend_mode & TN_BLEND_VGLITE_MASK;

    switch (mode)
    {
    case TN_BLEND_SRC:
    case TN_BLEND_SRC_IN:
        /* Replace with BLEND_SRC_OVER, keeping the original flags */
        *blend_mode = flags | TN_BLEND_SRC_OVER;
        break;
    case TN_BLEND_DST_IN:
        /* A background color for S2 source with alpha set to 1 will fix issue
           RGB: D*Sa
           A:   Da*Sa */
        *background_color = 0x000000FF;
        break;
    }
}



tn_result
tn_blt_image(
    const tn_gbuf *tgt_image,
    const tn_gbuf *src_image,
    tn_m3f32      *user_matrix,
    tn_i32         blend_mode,
    tn_color       color,
    tn_u32         image_mode,
    tn_u32         quality)
{
    tnIN();

    tnEXPECT(!tn_is_null(tgt_image));
    tnEXPECT(!tn_is_null(src_image));

    tnUNUSED(color);
    tnUNUSED(image_mode);

    const tn_gbuf *src_imgI, *tgt_imgI;
    tn_color src_colI, tgt_colI;
    tn_paddr src_addr, tgt_addr;
    tn_bool s1_opt_active = tnFALSE;

    // Setup DST, S1 & S2.
    // Some image formats are not supported on S2,
    // so they have to be loaded into S1 (transformations are not applicable).

    // Get color format and color pub
    tgt_imgI = tgt_image;
    tn_color_copy(tgt_imgI->color_format, &tgt_colI);
    src_imgI = src_image;
    tn_color_copy(src_imgI->color_format, &src_colI);

    src_addr = src_image->paddr;
    tgt_addr = tgt_image->paddr;

    tn_rect_t blend_area = {0, 0, tgt_imgI->width, tgt_imgI->height};

    //set transform regs
    tn_reg_32 xform_mode = TN_REGS_S2_XFORM_MODE_AFFINE;
    tn_m3f32 surface_to_image;
    tn_bool translation_only = tnTRUE;
    tn_u32 src_background_color = 0x00000000;
    tn_u32 s1_background_color = 0x00000000, s2_background_color = 0x00000000;

    if (user_matrix == tnNULL)
    {
        tn_m3f32_identity(&surface_to_image);
    }
    else
    {
        translation_only = tn_m3f32_is_translation_only(user_matrix);
        surface_to_image = *user_matrix;
        if (tn_m3f32_is_affine(&surface_to_image))
        {
            // Get the visible blend area and reset the translations of the xform matrix.
            // 32x1 formats do not support blending and also require alignment,
            //    therefore the optimization does not apply because the bkgnd color will overwrite
            //    the OOB pixels from the aligned region
            if (!tn_fmt_is_IBC32x1_compressed(tgt_imgI->color_format))
            {
                tn_m3f32_get_blend_area(&blend_area, &surface_to_image, src_imgI->width,
                                        src_imgI->height, translation_only);
            }
        }
        else     //projective
        {
            xform_mode = TN_REGS_S2_XFORM_MODE_PROJECTIVE;

            // Compute the projective bounding box at blit time by transforming
            // the source corners with the forward (source->destination) matrix.
            // The box is derived here rather than kept as sticky context
            // state precomputed by vg_lite_get_transform_matrix.
            tn_m3f32_get_blend_area(&blend_area, &surface_to_image, src_imgI->width, src_imgI->height, translation_only);
        }
    }

    if (blend_area.width == 0 || blend_area.height == 0)
    {
        tnOUT();
        return TN_NO_ERROR; // nothing visible to blit
    }

    // S1 has no transform unit: it can only blit axis-aligned (translation-only)
    // regions.  If the source format is not supported on S2 and the transform is
    // not translation-only there is no hardware path available.  Return an error
    // early before any scissor or register writes are made.
    if (tn_fmt_get_supported_src_chan(src_imgI->color_format) != S2 && !translation_only)
    {
        tnOUT();
        return TN_NOT_SUPPORTED;
    }

    tn_bool needs_alignment = tnFALSE;
    if (xform_mode == TN_REGS_S2_XFORM_MODE_AFFINE)
    {
        // some formats require the start of the blend area
        // to be aligned
        if (tn_fmt_is_yuv(tgt_imgI->color_format))
        {
            needs_alignment = tnTRUE;
            blend_area_align_2x2(&blend_area);
        }
        else if (tn_fmt_is_compressed(tgt_imgI->color_format))
        {
            needs_alignment = tnTRUE;
            if (tn_fmt_is_IBC8x8_compressed(tgt_imgI->color_format))
            {
                blend_area_align_8x8(&blend_area);
            }
            // 32x1 needs alignment, but does not support the optimization
        }
        else if (tgt_colI.bpp % 8)
        {
            blend_area_align_to_byte(&blend_area);
        }

        // shift the translation components of the matrix
        surface_to_image.m[0][2] -= blend_area.x;
        surface_to_image.m[1][2] -= blend_area.y;

        if (is_blend_patch_required(&surface_to_image))
        {
            blend_src_patch(&blend_mode, &src_background_color);
        }

        // check if the affine matrix is singular and invert it
        if (!tn_m3f32_invert_imm(&surface_to_image))
        {
            // cannot invert the matrix
            tnOUT();
            return TN_ILLEGAL_ARGUMENT_ERROR;
        }
    }
    else // (xform_mode == TN_REGS_S2_XFORM_MODE_PROJECTIVE)
    {
        // for projective xform, enable temporary scissor
        if (g_img_ctx.scissor_num >= SCISSOR_NUM)
        {
            tnOUT();
            return TN_OUT_OF_RESOURCES_ERROR;
        }

        tn_regs_set_scissor((tn_scissor_id)g_img_ctx.scissor_num, 0, 0, blend_area.width, blend_area.height);
        g_img_ctx.scissor_num++;

        // The HW processes tile-local pixels (0..width) x (0..height), but
        // the visible blend tile starts at (blend_area.x, blend_area.y) in
        // framebuffer coordinates. Pre-multiply the forward matrix by
        // translate(-blend_area.x, -blend_area.y) to account for this origin
        // shift before inverting. For a general projective matrix M, left-
        // multiplying by T = [[1,0,-bx],[0,1,-by],[0,0,1]] gives:
        //   T*M row0[j] = M.row0[j] - bx * M.row2[j]
        //   T*M row1[j] = M.row1[j] - by * M.row2[j]
        //   T*M row2[j] = M.row2[j]  (unchanged)
        // The affine path does the same adjustment, simplified to just
        // subtracting from the translation column because row2 = [0,0,1]
        // there. For projective the full row update is required.
        tn_f32 bx = (tn_f32)blend_area.x;
        tn_f32 by_f = (tn_f32)blend_area.y;
        for (int j = 0; j < 3; j++)
        {
            surface_to_image.m[0][j] -= bx * surface_to_image.m[2][j];
            surface_to_image.m[1][j] -= by_f * surface_to_image.m[2][j];
        }

        // Invert the adjusted forward matrix so the HW can map each
        // tile-local destination pixel back to its source texel.
        if (!tn_m3f32_invert_imm(&surface_to_image))
        {
            g_img_ctx.scissor_num -= 1;
            tn_regs_disable_crt_scissor(g_img_ctx.scissor_num);
            tnOUT();
            return TN_ILLEGAL_ARGUMENT_ERROR;
        }
    }

    // Record the blit geometry so a later cache EXECUTE with a replacement
    // matrix can re-derive the inverse transform and the matrix-dependent
    // DST/S1 SIZE/ADDR registers.  Only the simple affine, byte-aligned,
    // non-YUV / non-compressed target case is injectable; anything else marks
    // injectable = tnFALSE so EXECUTE falls back to raw TF patching.  The call
    // is a no-op when no cache recording session is active.
    {
        tn_cmd_buf_cache_blit_geo_t geo;
        geo.injectable = (xform_mode == TN_REGS_S2_XFORM_MODE_AFFINE)
                         && !tn_fmt_is_yuv(tgt_imgI->color_format)
                         && !tn_fmt_is_compressed(tgt_imgI->color_format)
                         && !tn_fmt_is_rgb_a_planar(tgt_imgI->color_format)
                         && ((tgt_colI.bpp % 8) == 0);
        geo.src_w      = src_imgI->width;
        geo.src_h      = src_imgI->height;
        geo.tgt_w      = tgt_imgI->width;
        geo.tgt_h      = tgt_imgI->height;
        geo.tgt_stride = tgt_imgI->stride;
        geo.tgt_bpp    = tgt_colI.bpp;
        geo.tgt_addr   = tgt_addr;
        tn_cmd_buf_ccdma_cache_note_blit_geometry(&geo);
    }

    // set xform registers
    tn_regs_set_xform(&surface_to_image);
    tn_regs_set_xform_mode(xform_mode);

    // setup DST
    tn_regs_set_dst_color_format(tgt_imgI, 0, tn_fmt_is_yuv(src_imgI->color_format));
    (void)tn_regs_set_stride_and_addr(tgt_addr, blend_area.x, blend_area.y, tgt_imgI, tgt_colI, DST);
    tn_regs_set_size(blend_area.width, blend_area.height, DST);

    // Optimization: S1 supports more color formats than S2 and is much faster,
    // therefore, if there is ONLY TRANSLATION xform and no blending requested by the user,
    // the driver can use S1 over DST blit operation
    if ((tn_fmt_get_supported_src_chan(src_imgI->color_format) == S2)
        && translation_only
        && ((blend_mode & TN_BLEND_MODE_MASK) == TN_BLEND_SRC) && !needs_alignment
        && (src_colI.bpp % 8 == 0)
        && (tgt_colI.bpp % 8 == 0))
    {
        // disable S2 because image is copied directly from S1
        s1_opt_active = tnTRUE;
        tn_regs_disable_s2();

        // setup S1
        tn_regs_set_s1_color_format(src_imgI);
        (void)tn_regs_set_stride_and_addr(src_addr, 0, 0, src_imgI, src_colI, S1);
        tn_regs_set_size(src_imgI->width, src_imgI->height, S1);

        // if color key is enabled, move it to S1
        if (g_img_ctx.color_key_on_s2 && !g_img_ctx.color_key_on_s1)
        {
            //transfer the cached color keys to S1
            tn_regs_transfer_color_key(S1);
            g_img_ctx.color_key_on_s1 = tnTRUE;
            //reset color_key on S2
            tn_regs_reset_color_key(S2);
            g_img_ctx.color_key_on_s2 = tnFALSE;
        }

    }
    else if (tn_fmt_get_supported_src_chan(src_imgI->color_format) == S2)
    {
        // Source will be loaded on S2

        // if color key is enabled on S1, transfer it to S2
        if (g_img_ctx.color_key_on_s1 && !g_img_ctx.color_key_on_s2)
        {
            //transfer the cached color keys to S2
            tn_regs_transfer_color_key(S2);
            g_img_ctx.color_key_on_s2 = tnTRUE;
            //reset color_key on S1
            tn_regs_reset_color_key(S1);
            g_img_ctx.color_key_on_s1 = tnFALSE;
        }

        if ((blend_mode & TN_BLEND_MODE_MASK) == TN_BLEND_SRC)    //no blending, just the source
        // disable S1 because image is copied directly from S2
        {
            tn_regs_disable_s1();
        }
        else if (tn_fmt_not_supported_on_s1(tgt_imgI->color_format))
        {
            tnOUT();
            return TN_NOT_SUPPORTED;
        }
        else
        {
            // setup S1 with tgt's properties (including base address)
            tn_regs_set_s1_color_format(tgt_imgI);
            (void)tn_regs_set_stride_and_addr(tgt_addr, blend_area.x, blend_area.y, tgt_imgI, tgt_colI, S1);
            tn_regs_set_size(blend_area.width, blend_area.height, S1);
        }

        // setup S2
        tn_regs_set_s2_color_format(src_imgI, quality);
        (void)tn_regs_set_stride_and_addr(src_addr, 0, 0, src_imgI, src_colI, S2);
        tn_regs_set_size(src_imgI->width, src_imgI->height, S2);
        s2_background_color = src_background_color;
    }
    // S2 does not support the source format, therefore use S1 for source.
    // Since S1 does not support xforms, the driver can only handle translations.
    else if (translation_only)
    {
        // if color key is enabled, move it to S1
        if (g_img_ctx.color_key_on_s2 && !g_img_ctx.color_key_on_s1)
        {
            //transfer the cached color keys from S2 to S1
            tn_regs_transfer_color_key(S1);
            g_img_ctx.color_key_on_s1 = tnTRUE;
            //reset color_key on S2
            tn_regs_reset_color_key(S2);
            g_img_ctx.color_key_on_s2 = tnFALSE;
        }

        // setup S1
        tn_regs_set_s1_color_format(src_imgI);
        (void)tn_regs_set_stride_and_addr(src_addr, 0, 0, src_imgI, src_colI, S1);
        tn_regs_set_size(src_imgI->width, src_imgI->height, S1);
        s1_background_color = src_background_color;

        if ((blend_mode & TN_BLEND_MODE_MASK) == TN_BLEND_SRC)
        {
            // disable S2, image is copied directly from S1
            tn_regs_disable_s2();
        }
        else
        {
            if (tn_fmt_get_supported_src_chan(tgt_imgI->color_format) == S2)
            {
                // setup S2
                tn_regs_set_s2_color_format(tgt_imgI, quality);
                (void)tn_regs_set_stride_and_addr(tgt_addr, blend_area.x, blend_area.y, tgt_imgI, tgt_colI, S2);
                tn_regs_set_size(blend_area.width, blend_area.height, S2);
            }
            else
            {
                tn_regs_disable_s2();
            }
        }
    }
    else
    {
        tnOUT();
        return TN_NOT_SUPPORTED;
    }

    tn_regs_set_s1_bkgnd_color(s1_background_color);
    tn_regs_set_s2_bkgnd_color(s2_background_color);

    // Query the supported source channel once and compare it against S2.
    // tn_regs_set_blending_mode() takes a bool meaning "the source is
    // carried on the S2 channel", so the value must be tested, not merely
    // stored. Comparing here consumes the return value (satisfies Coverity
    // CHECKED_RETURN) and matches the "== S2" idiom used by the setup
    // branches above. A plain (tn_bool) cast would be wrong: S1 has enum
    // value 0 (would read as false) while DST/MASK are non-zero.
    tn_u32 src_chan = tn_fmt_get_supported_src_chan(src_imgI->color_format);
    tn_regs_set_blending_mode(blend_mode,
                              !s1_opt_active && (src_chan == S2));

    tn_regs_enable_gpu_ctrl();

    if (xform_mode == TN_REGS_S2_XFORM_MODE_PROJECTIVE)
    {
        // Disable scissor that was used in projective transform
        g_img_ctx.scissor_num -= 1;
        tn_regs_disable_crt_scissor(g_img_ctx.scissor_num);
    }

    tnOUT();
    return TN_NO_ERROR;
}



// Blit and blend two source images onto a target in a single hardware pass.
// This emulates vg_lite_blit2, where source0 is the foreground "image"
// blended over source1, the background "paint".
//
// FlexGPU has three channels: S1, S2 and DST. A single blend op reads S1 and S2
// and writes DST. Here:
//   - src0 (foreground) goes in S2, the full transform channel. matrix0 may be
//     affine (projective is not supported by this path).
//   - src1 (background) goes in S1. S1 has no transform unit, so matrix1 may
//     only contain a translation.
//   - src0 is blended over src1 using blend_mode and the result is written to
//     the target on DST.
tn_result
tn_blt_image2(
    const tn_gbuf *tgt_image,
    const tn_gbuf *src0_image,
    const tn_gbuf *src1_image,
    tn_m3f32      *matrix0,
    tn_m3f32      *matrix1,
    tn_i32         blend_mode,
    tn_color       color,
    tn_u32         image_mode,
    tn_u32         quality)
{
    tnIN();

    tnUNUSED(color);
    tnUNUSED(image_mode);

    const tn_gbuf *src0_imgI, *src1_imgI, *tgt_imgI;
    tn_color src0_colI, src1_colI, tgt_colI;
    tn_paddr src0_addr, src1_addr, tgt_addr;

    // Get color format and color for all three surfaces.
    tgt_imgI = tgt_image;
    tn_color_copy(tgt_imgI->color_format, &tgt_colI);
    src0_imgI = src0_image;
    tn_color_copy(src0_imgI->color_format, &src0_colI);
    src1_imgI = src1_image;
    tn_color_copy(src1_imgI->color_format, &src1_colI);

    src0_addr = src0_image->paddr;
    src1_addr = src1_image->paddr;
    tgt_addr = tgt_image->paddr;

    // src0 rides on S2 (transform channel); it must be an S2-capable format.
    if (tn_fmt_get_supported_src_chan(src0_imgI->color_format) != S2)
    {
        tnOUT();
        return TN_NOT_SUPPORTED;
    }

    // src1 rides on S1; it must be loadable on S1.
    if (tn_fmt_not_supported_on_s1(src1_imgI->color_format))
    {
        tnOUT();
        return TN_NOT_SUPPORTED;
    }

    tn_rect_t blend_area = {0, 0, tgt_imgI->width, tgt_imgI->height};

    // Compute the blend area from source0 (S2) transformed bounding box.
    // Only affine transforms are supported on this path.
    tn_m3f32 surface_to_image;
    tn_bool translation_only0 = tnTRUE;
    tn_u32 src_background_color = 0x00000000;
    tn_u32 s1_background_color = 0x00000000, s2_background_color = 0x00000000;

    if (matrix0 == tnNULL)
    {
        tn_m3f32_identity(&surface_to_image);
    }
    else
    {
        translation_only0 = tn_m3f32_is_translation_only(matrix0);
        surface_to_image = *matrix0;
        if (tn_m3f32_is_affine(&surface_to_image))
        {
            if (!tn_fmt_is_IBC32x1_compressed(tgt_imgI->color_format))
            {
                tn_m3f32_get_blend_area(&blend_area, &surface_to_image, src0_imgI->width, src0_imgI->height, translation_only0);
            }
        }
        else //projective not supported here
        {
            tnOUT();
            return TN_NOT_SUPPORTED;
        }
    }

    if (blend_area.width == 0 || blend_area.height == 0)
    {
        tnOUT();
        return TN_NO_ERROR; // nothing visible to blit
    }

    // src1 uses S1 which has no transform unit, so matrix1 may only translate.
    tn_f32 tx1 = 0.0f, ty1 = 0.0f;
    if (matrix1 != tnNULL)
    {
        if (!tn_m3f32_is_translation_only(matrix1))
        {
            tnOUT();
            return TN_NOT_SUPPORTED;
        }
        tx1 = matrix1->m[0][2];
        ty1 = matrix1->m[1][2];
    }

    // Some formats require the start of the blend area to be aligned.
    if (tn_fmt_is_yuv(tgt_imgI->color_format))
    {
        blend_area_align_2x2(&blend_area);
    }
    else if (tn_fmt_is_compressed(tgt_imgI->color_format))
    {
        if (tn_fmt_is_IBC8x8_compressed(tgt_imgI->color_format))
        {
            blend_area_align_8x8(&blend_area);
        }
    }
    else if (tgt_colI.bpp % 8)
    {
        blend_area_align_to_byte(&blend_area);
    }

    // Shift the translation components of the S2 matrix and invert it.
    surface_to_image.m[0][2] -= blend_area.x;
    surface_to_image.m[1][2] -= blend_area.y;

    if (is_blend_patch_required(&surface_to_image))
    {
        blend_src_patch(&blend_mode, &src_background_color);
    }

    if (!tn_m3f32_invert_imm(&surface_to_image))
    {
        // cannot invert the matrix
        tnOUT();
        return TN_ILLEGAL_ARGUMENT_ERROR;
    }

    // Program the S2 transform (affine).
    tn_regs_set_xform(&surface_to_image);
    tn_regs_set_xform_mode(TN_REGS_S2_XFORM_MODE_AFFINE);

    // Setup DST.
    tn_regs_set_dst_color_format(tgt_imgI, 0, tn_fmt_is_yuv(src0_imgI->color_format));
    (void)tn_regs_set_stride_and_addr(tgt_addr, blend_area.x, blend_area.y, tgt_imgI, tgt_colI, DST);
    tn_regs_set_size(blend_area.width, blend_area.height, DST);

    // Setup S2 with source0 (foreground image, transformed).
    tn_regs_set_s2_color_format(src0_imgI, quality);
    (void)tn_regs_set_stride_and_addr(src0_addr, 0, 0, src0_imgI, src0_colI, S2);
    tn_regs_set_size(src0_imgI->width, src0_imgI->height, S2);
    s2_background_color = src_background_color;

    // Setup S1 with source1 (background paint, translation only).
    // The blend area is expressed in target coordinates; offset src1 by the
    // translation from matrix1 so the correct region of src1 is sampled.
    tn_i32 s1_off_x = (tn_i32)blend_area.x - (tn_i32)tx1;
    tn_i32 s1_off_y = (tn_i32)blend_area.y - (tn_i32)ty1;
    if (s1_off_x < 0)
    {
        s1_off_x = 0;
    }
    if (s1_off_y < 0)
    {
        s1_off_y = 0;
    }
    tn_regs_set_s1_color_format(src1_imgI);
    (void)tn_regs_set_stride_and_addr(src1_addr, (tn_u32)s1_off_x, (tn_u32)s1_off_y, src1_imgI, src1_colI, S1);

    // S1 has no transform unit: it is read linearly from the (s1_off_x,
    // s1_off_y) origin for the programmed S1 size. If the size were set to the
    // full blend_area, the HW would read past the end of src1 wherever src1
    // (after its translation) does not cover the whole blend tile. Clamp the S1
    // size to the portion of src1 that actually overlaps the blend area; the
    // uncovered part of the tile then falls back to the S1 background color.
    tn_u32 s1_w = blend_area.width;
    tn_u32 s1_h = blend_area.height;
    if ((tn_u32)s1_off_x < src1_imgI->width)
    {
        tn_u32 avail_w = src1_imgI->width - (tn_u32)s1_off_x;
        if (avail_w < s1_w)
        {
            s1_w = avail_w;
        }
    }
    else
    {
        s1_w = 0;
    }
    if ((tn_u32)s1_off_y < src1_imgI->height)
    {
        tn_u32 avail_h = src1_imgI->height - (tn_u32)s1_off_y;
        if (avail_h < s1_h)
        {
            s1_h = avail_h;
        }
    }
    else
    {
        s1_h = 0;
    }
    tn_regs_set_size(s1_w, s1_h, S1);


    tn_regs_set_s1_bkgnd_color(s1_background_color);
    tn_regs_set_s2_bkgnd_color(s2_background_color);

    tn_u32 src_chan = tn_fmt_get_supported_src_chan(src0_imgI->color_format);
    tn_regs_set_blending_mode(blend_mode, (src_chan == S2));

    tn_regs_enable_gpu_ctrl();

    tnOUT();
    return TN_NO_ERROR;
}



tn_result
tn_blt_image_rect(
    const tn_gbuf  *tgt_image,
    const tn_gbuf  *src_image,
    tn_m3f32       *user_matrix,
    tn_i32          blend_mode,
    tn_color        color,
    tn_u32          image_mode,
    tn_u32          quality,
    tn_blt_xform_t *rect)
{
    tnIN_ARG("tgt_image:%p src_image:%p blend_mode:%x quality:%x", tgt_image, src_image, blend_mode, quality);
    tnEXPECT(!tn_is_null(tgt_image));
    tnEXPECT(!tn_is_null(src_image));

    tnUNUSED(color);
    tnUNUSED(image_mode);

    //TODO: add support of PROJECTION transform mode
    //      add scissor setting for aligned mode, so we can suport correct clip
    const tn_gbuf *src_imgI, *tgt_imgI;
    tn_color src_colI, tgt_colI;
    tn_paddr src_addr, tgt_addr;

    // Setup DST, S1 & S2.
    // Some image formats are not supported on S2,
    // so they have to be loaded into S1 (transformations are not applicable).

    // Get color format and color pub
    tgt_imgI = tgt_image;
    tn_color_copy(tgt_imgI->color_format, &tgt_colI);
    src_imgI = src_image;
    tn_color_copy(src_imgI->color_format, &src_colI);

    src_addr = src_image->paddr;
    tgt_addr = tgt_image->paddr;

    /* blend_area initialization value is alter used in tn_m3f32_get_blend_area
     * to limit the size of blending region to target buffer size */
    tn_rect_t blend_area = {0, 0, tgt_imgI->width, tgt_imgI->height};
    tn_rect_t src_rectangle;
    tn_bool s1_opt_active = tnFALSE;

    if (rect == tnNULL)
    {
        src_rectangle = (tn_rect_t){0, 0, src_imgI->width, src_imgI->height};
    }
    else
    {
        src_rectangle = (tn_rect_t){rect->dx, rect->dy, rect->width, rect->height};
    }

    //set transform regs
    tn_reg_32 xform_mode = TN_REGS_S2_XFORM_MODE_AFFINE;
    tn_m3f32 surface_to_image;
    tn_bool translation_only = tnTRUE;
    tn_u32 src_background_color = 0x00000000;
    tn_u32 s1_background_color = 0x00000000, s2_background_color = 0x00000000;

    if (user_matrix == tnNULL)
    {
        tn_m3f32_identity(&surface_to_image);
    }
    else
    {
        translation_only = tn_m3f32_is_translation_only(user_matrix);
        surface_to_image = *user_matrix;
        if (tn_m3f32_is_affine(&surface_to_image))
        {
            // Get the visible blend area and reset the translations of the xform matrix.
            // 32x1 formats do not support blending and also require alignment,
            //    therefore the optimization does not apply because the bkgnd color will overwrite
            //    the OOB pixels from the aligned region
            if (!tn_fmt_is_IBC32x1_compressed(tgt_imgI->color_format))
            {
                tn_m3f32_get_blend_area(&blend_area, &surface_to_image, src_rectangle.width,
                                        src_rectangle.height, translation_only);
            }
        }
        else     //projective
        {
            xform_mode = TN_REGS_S2_XFORM_MODE_PROJECTIVE;
            tnEXPECT(0);  /* TODO to support projective transformations */

        }
    }

    if (blend_area.width == 0 || blend_area.height == 0)
    {
        tnOUT();
        return TN_NO_ERROR; // nothing visible to blit
    }

    tn_bool needs_alignment = tnFALSE;
    if (xform_mode == TN_REGS_S2_XFORM_MODE_AFFINE)
    {
        // some formats require the start of the blend area
        // to be aligned
        if (tn_fmt_is_yuv(tgt_imgI->color_format))
        {
            needs_alignment = tnTRUE;
            blend_area_align_2x2(&blend_area);
        }
        else if (tn_fmt_is_compressed(tgt_imgI->color_format))
        {
            needs_alignment = tnTRUE;
            if (tn_fmt_is_IBC8x8_compressed(tgt_imgI->color_format))
            {
                blend_area_align_8x8(&blend_area);
            }
            // 32x1 needs alignment, but does not support the optimization
        }
        else if (tgt_colI.bpp % 8)
        {
            blend_area_align_to_byte(&blend_area);
        }

        // shift the translation components of the matrix
        surface_to_image.m[0][2] -= blend_area.x;
        surface_to_image.m[1][2] -= blend_area.y;

        if (is_blend_patch_required(&surface_to_image))
        {
            blend_src_patch(&blend_mode, &src_background_color);
        }

        // check if the affine matrix is singular and invert it
        if (!tn_m3f32_invert_imm(&surface_to_image))
        {
            /* cannot invert the matrix */

            tnOUT();
            return TN_ILLEGAL_ARGUMENT_ERROR;
        }
    }
    else // (xform_mode == TN_REGS_S2_XFORM_MODE_PROJECTIVE)
    {
        // for projective xform, enable temporary scissor
        if (g_img_ctx.scissor_num >= SCISSOR_NUM)
        {
            tnOUT();
            return TN_OUT_OF_RESOURCES_ERROR;
        }

        tn_regs_set_scissor((tn_scissor_id)g_img_ctx.scissor_num, 0, 0, blend_area.width, blend_area.height);
        g_img_ctx.scissor_num++;
    }

    // set xform regiters
    tn_regs_set_xform(&surface_to_image);
    tn_regs_set_xform_mode(xform_mode);

    // setup DST
    tn_regs_set_dst_color_format(tgt_imgI, 0, tn_fmt_is_yuv(src_imgI->color_format));
    (void)tn_regs_set_stride_and_addr(tgt_addr, blend_area.x, blend_area.y, tgt_imgI, tgt_colI, DST);
    tn_regs_set_size(blend_area.width, blend_area.height, DST);

    // Optimization: S1 supports more color formats than S2 and is much faster,
    // therefore, if there is ONLY TRANSLATION xform and no blending requested by the user,
    // the driver can use S1 over DST blit operation
    if ((tn_fmt_get_supported_src_chan(src_imgI->color_format) == S2)
        && translation_only
        && ((blend_mode & TN_BLEND_MODE_MASK) == TN_BLEND_SRC) && !needs_alignment
        && (src_colI.bpp % 8 == 0)
        && (tgt_colI.bpp % 8 == 0))
    {
        // disable S2 because image is copied directly from S1
        s1_opt_active = tnTRUE;
        tn_regs_disable_s2();

        // setup S1
        tn_regs_set_s1_color_format(src_imgI);
        //(void)tn_regs_set_stride_and_addr(src_addr, 0, 0, src_imgI, src_colI, S1);
        //tn_regs_set_size(src_imgI->width, src_imgI->height, S1);
        (void)tn_regs_set_stride_and_addr(src_addr, src_rectangle.x, src_rectangle.y, src_imgI, src_colI, S1);
        tn_regs_set_size(src_rectangle.width, src_rectangle.height, S1);


        // if color key is enabled, move it to S1
        if (g_img_ctx.color_key_on_s2 && !g_img_ctx.color_key_on_s1)
        {
            //transfer the cached color keys to S1
            tn_regs_transfer_color_key(S1);
            g_img_ctx.color_key_on_s1 = tnTRUE;
            //reset color_key on S2
            tn_regs_reset_color_key(S2);
            g_img_ctx.color_key_on_s2 = tnFALSE;
        }

    }
    else if (tn_fmt_get_supported_src_chan(src_imgI->color_format) == S2)
    {
        // Source will be loaded on S2

        // if color key is enabled on S1, transfer it to S2
        if (g_img_ctx.color_key_on_s1 && !g_img_ctx.color_key_on_s2)
        {
            //transfer the cached color keys to S2
            tn_regs_transfer_color_key(S2);
            g_img_ctx.color_key_on_s2 = tnTRUE;
            //reset color_key on S1
            tn_regs_reset_color_key(S1);
            g_img_ctx.color_key_on_s1 = tnFALSE;
        }

        if ((blend_mode & TN_BLEND_MODE_MASK) == TN_BLEND_SRC)    //no blending, just the source
        // disable S1 because image is copied directly from S2
        {
            tn_regs_disable_s1();
        }
        else if (tn_fmt_not_supported_on_s1(tgt_imgI->color_format))
        {
            tnOUT();
            return TN_NOT_SUPPORTED;
        }
        else
        {
            // setup S1 with tgt's properties (including base address)
            tn_regs_set_s1_color_format(tgt_imgI);
            (void)tn_regs_set_stride_and_addr(tgt_addr, blend_area.x, blend_area.y, tgt_imgI, tgt_colI, S1);
            tn_regs_set_size(blend_area.width, blend_area.height, S1);
        }

        // setup S2
        tn_regs_set_s2_color_format(src_imgI, quality);
        //(void)tn_regs_set_stride_and_addr(src_addr, 0, 0, src_imgI, src_colI, S2);
        //tn_regs_set_size(src_imgI->width, src_imgI->height, S2);
        (void)tn_regs_set_stride_and_addr(src_addr, src_rectangle.x, src_rectangle.y, src_imgI, src_colI, S2);
        tn_regs_set_size(src_rectangle.width, src_rectangle.height, S2);
        s2_background_color = src_background_color;
    }
    // S2 does not support the source format, therefore use S1 for source.
    // Since S1 does not support xforms, the driver can only handle translations.
    else if (translation_only)
    {
        // if color key is enabled, move it to S1
        if (g_img_ctx.color_key_on_s2 && !g_img_ctx.color_key_on_s1)
        {
            //transfer the cached color keys from S2 to S1
            tn_regs_transfer_color_key(S1);
            g_img_ctx.color_key_on_s1 = tnTRUE;
            //reset color_key on S2
            tn_regs_reset_color_key(S2);
            g_img_ctx.color_key_on_s2 = tnFALSE;
        }

        // setup S1
        tn_regs_set_s1_color_format(src_imgI);
        //(void)tn_regs_set_stride_and_addr(src_addr, 0, 0, src_imgI, src_colI, S1);
        //tn_regs_set_size(src_imgI->width, src_imgI->height, S1);
        (void)tn_regs_set_stride_and_addr(src_addr, src_rectangle.x, src_rectangle.y, src_imgI, src_colI, S1);
        tn_regs_set_size(src_rectangle.width, src_rectangle.height, S1);
        s1_background_color = src_background_color;

        if ((blend_mode & TN_BLEND_MODE_MASK) == TN_BLEND_SRC)
        {
            // disable S2, image is copied directly from S1
            tn_regs_disable_s2();
        }
        else
        {
            if (tn_fmt_get_supported_src_chan(tgt_imgI->color_format) == S2)
            {
                // setup S2
                tn_regs_set_s2_color_format(tgt_imgI, quality);
                (void)tn_regs_set_stride_and_addr(tgt_addr, blend_area.x, blend_area.y, tgt_imgI, tgt_colI, S2);
                tn_regs_set_size(blend_area.width, blend_area.height, S2);
            }
            else
            {
                tn_regs_disable_s2();
            }
        }
    }
    else
    {
        tnOUT();
        return TN_NOT_SUPPORTED;
    }

    tn_regs_set_s1_bkgnd_color(s1_background_color);
    tn_regs_set_s2_bkgnd_color(s2_background_color);

    // Query the supported source channel once and compare it against S2.
    // tn_regs_set_blending_mode() takes a bool meaning "the source is
    // carried on the S2 channel", so the value must be tested, not merely
    // stored. Comparing here consumes the return value (satisfies Coverity
    // CHECKED_RETURN) and matches the "== S2" idiom used by the setup
    // branches above. A plain (tn_bool) cast would be wrong: S1 has enum
    // value 0 (would read as false) while DST/MASK are non-zero.
    tn_u32 src_chan = tn_fmt_get_supported_src_chan(src_imgI->color_format);
    tn_regs_set_blending_mode(blend_mode,
                              !s1_opt_active && (src_chan == S2));

    tn_regs_enable_gpu_ctrl();

    if (xform_mode == TN_REGS_S2_XFORM_MODE_PROJECTIVE)
    {
        // Disable scissor that was used in projective transform
        g_img_ctx.scissor_num -= 1;
        tn_regs_disable_crt_scissor(g_img_ctx.scissor_num);
    }

    tnOUT();
    return TN_NO_ERROR;

}



tn_result
tn_mask_fill(
    const tn_gbuf  *mask_image,
    tn_blt_xform_t *rect,
    tn_u8           value)
{
    tnIN_ARG("mask_image:%p", mask_image);
    tnEXPECT(!tn_is_null(mask_image));
    tnEXPECT(!tn_is_null(rect));

    const tn_gbuf *msk_imgI = mask_image;

    tn_map_coord start_x = tnMAX(rect->dx, 0); // TN_A_8
    tn_map_coord start_y = tnMAX(rect->dy, 0);
    /* End coordinates are first pixel outside the fill region */
    tn_map_coord end_x = tnMIN(msk_imgI->width, rect->dx + rect->width);
    tn_map_coord end_y = tnMIN(msk_imgI->height, rect->dy + rect->height);

    if (start_x >= msk_imgI->width || start_y >= msk_imgI->height || end_x <= 0 || end_y <= 0)
    {
        /* No work for rectangle out of the mask dimensions. */

        tnOUT();
        return TN_NO_ERROR;
    }

    /* Get width and height corrected in respect to mask_image dimensions */
    tn_map_coord w = end_x - start_x;
    tn_map_coord h = end_y - start_y;
    if (w <= 0 || h <= 0)
    {
        /* No need to fill empty rectangle. */

        tnOUT();
        return TN_NO_ERROR;
    }

    /* The mask is filled by the CPU, so the write goes through the virtual
       address. The descriptor resolved both addresses at allocate or wrap
       time; matches blt/V3/tn_blt.c, which reads mask_image->vaddr here. */
    const tn_vptr mask_addr = mask_image->vaddr;
    tnRET_IF(tn_is_null(mask_addr), TN_ILLEGAL_ARGUMENT_ERROR);

    tn_color msk_colI;
    tn_color_copy(msk_imgI->color_format, &msk_colI);

    tn_u8 has_partial_leading_mask = 0;
    tn_u8 leading_byte_mask = 0x00;
    tn_u8 leading_byte_mask_length = 0;
    tn_u8 leading_mask_complement;
    if (msk_imgI->color_format == TN_A_4)
    {
        leading_byte_mask = (start_x & 0x01) ? has_partial_leading_mask = 1, 0xF0
                                             : 0x00;
        leading_mask_complement = 0x0F;
        start_x >>= 1;
    }
    else if (msk_imgI->color_format == TN_A_1)
    {
        tn_u8 remainder = start_x & 0x07;
        leading_byte_mask_length = 8 - remainder;
        leading_byte_mask = (remainder) ? has_partial_leading_mask = 1, ~((1 << remainder) - 1)
                                        : 0x00;
        leading_mask_complement = ~leading_byte_mask;
        start_x >>= 3;
    }

    // Determine the number of fully covered bytes that can be 'memset' in bulk.
    // A8: 1 byte alignment (all are full bytes)
    // A4: 4 bit alignment (possible partial bytes at the ends)
    // A1: 1 bit alignment (possible partial bytes at the ends)
    tn_size fully_used_bytes = w; // TN_A_8
    tn_u8 trailing_byte_mask = 0x00;
    tn_u8 has_partial_trailing_mask = 0;
    tn_u8 trailing_mask_complement;
    if (msk_imgI->color_format == TN_A_4)
    {
        tn_u8 odd_width = w & 0x01;
        // w > 0 is verified above, but "w / 2 - 1" is evaluated in signed int
        // and can reach -1 for a narrow mask whose only nibble pair is taken
        // by the leading partial byte. Storing -1 into the unsigned tn_size
        // fully_used_bytes would wrap to a huge length and overrun tn_memset.
        // Compute the full-byte count in signed int and clamp it to >= 0.
        tn_i32 full_bytes = (has_partial_leading_mask && !odd_width) ? (w / 2 - 1)
                                                                     : (w / 2);
        fully_used_bytes = (full_bytes > 0) ? (tn_size)full_bytes : 0;
        trailing_byte_mask = (has_partial_leading_mask && !odd_width) || (!has_partial_leading_mask && odd_width)
                             ? has_partial_trailing_mask = 1, 0x0F
                             : 0x00;
        trailing_mask_complement = 0xF0;
    }
    else if (msk_imgI->color_format == TN_A_1)
    {
        tn_u8 remainder = w & 0x07;
        // Same guard for the A1 path: "w / 8 - 1" is a signed expression that
        // can reach -1 for narrow masks and would wrap when stored in the
        // unsigned tn_size. Compute in signed int and clamp to >= 0 so
        // tn_memset never receives an underflowed length.
        tn_i32 full_bytes = (remainder >= leading_byte_mask_length) ? (w / 8)
                                                                    : (w / 8 - 1);
        fully_used_bytes = (full_bytes > 0) ? (tn_size)full_bytes : 0;
        trailing_byte_mask = (remainder >= leading_byte_mask_length) ? (1 << (remainder - leading_byte_mask_length)) - 1
                                                                     : (8 - leading_byte_mask_length);
        trailing_mask_complement = ~(trailing_byte_mask);
    }

    // start_y is clamped to >= 0 above and stride is non-negative; widen to
    // tn_size before multiplying so the byte offset is computed in 32-bit
    // unsigned arithmetic instead of overflowing signed int for tall masks.
    tn_u8 *start_line = (tn_u8*)mask_addr + (tn_size)start_y * (tn_size)msk_imgI->stride;
    tn_u8 masked_original_content;
    for (int i = 0; i < h; i++)
    {
        //handle partial mask not fully covering the leading byte
        if (has_partial_leading_mask)
        {
            masked_original_content = *(tn_u8*)(start_line + start_x) & leading_mask_complement;
            *(tn_u8*)(start_line + start_x) = masked_original_content | (value & leading_byte_mask);
        }

        //bulk transfer of "values"
        tn_memset((void*)(start_line + start_x + has_partial_leading_mask), value, fully_used_bytes);

        //handle partial mask not fully covering the trailing byte
        if (has_partial_trailing_mask)
        {
            masked_original_content = *(tn_u8*)(start_line + start_x +  has_partial_leading_mask + fully_used_bytes) & trailing_mask_complement;
            *(tn_u8*)(start_line + start_x +  has_partial_leading_mask + fully_used_bytes) = masked_original_content | (value & trailing_byte_mask);
        }

        start_line += msk_imgI->stride;
    }

    tnOUT();

    return TN_NO_ERROR;
}



tn_result
tn_mask_blend(
    const tn_gbuf  *dst_image,
    const tn_gbuf  *src_image,
    tn_i32          blend_mode,
    tn_blt_xform_t *rect)
{
    tnIN_ARG("dst_image:%p src_image:%p blend_mode:%x", dst_image, src_image, blend_mode);
    tnEXPECT(!tn_is_null(dst_image));
    tnEXPECT(!tn_is_null(src_image));

    const tn_gbuf *src_imgI, *dst_imgI;
    tn_color src_colI, dst_colI;
    tn_paddr src_addr, dst_addr;

    dst_addr = dst_image->paddr;
    src_addr = src_image->paddr;

    // Setup DST, S1 & S2
    dst_imgI = dst_image;
    tn_color_copy(dst_imgI->color_format, &dst_colI);
    src_imgI = src_image;
    tn_color_copy(src_imgI->color_format, &src_colI);

    // setup DST
    tn_regs_set_dst_color_format(dst_imgI, 0, tn_fmt_is_yuv(src_imgI->color_format));
    (void)tn_regs_set_stride_and_addr(dst_addr, rect->dx, rect->dy, dst_imgI, dst_colI, DST);
    tn_regs_set_size(rect->width, rect->height, DST);

    // setup S1 for dst
    tn_regs_set_s1_color_format(dst_imgI);
    (void)tn_regs_set_stride_and_addr(dst_addr, rect->dx, rect->dy, dst_imgI, dst_colI, S1);
    tn_regs_set_size(rect->width, rect->height, S1);
    tn_regs_set_s1_bkgnd_color(0x00000000);

    // setup S2 for src
    tn_regs_set_s2_color_format(src_imgI, TN_RENDERING_QUALITY_NONANTIALIASED);
    (void)tn_regs_set_stride_and_addr(src_addr, rect->sx, rect->sy, src_imgI, src_colI, S2);
    tn_regs_set_size(rect->width, rect->height, S2);
    tn_regs_set_s2_bkgnd_color(0x00000000);

    // Query the supported source channel once and compare it against S2.
    // tn_regs_set_blending_mode() takes a bool meaning "the source is
    // carried on the S2 channel", so the value must be tested, not merely
    // stored (satisfies Coverity CHECKED_RETURN). A plain (tn_bool) cast
    // would be wrong: S1 has enum value 0 (would read as false) while
    // DST/MASK are non-zero.
    tn_u32 src_chan = tn_fmt_get_supported_src_chan(src_imgI->color_format);
    tn_regs_set_blending_mode(blend_mode, (src_chan == S2));

    // translate the S2 rect area over the same position in S1 (and DST)
    // We always assume size(S2) <= size(S1)
    tn_m3f32 surface_to_image;
    tn_m3f32_identity(&surface_to_image);
    tn_regs_set_xform(&surface_to_image);
    tn_reg_32 xform_mode = TN_REGS_S2_XFORM_MODE_AFFINE;
    tn_regs_set_xform_mode(xform_mode);

    tn_regs_enable_gpu_ctrl();

    tnOUT();
    return TN_NO_ERROR;
}



tn_result
tn_mask_set(const tn_gbuf *mask)
{
    tnIN_ARG("mask:%p", mask);
    tnEXPECT(!tn_is_null(mask));


    tn_regs_set_mask_stride(mask->stride);
    tn_regs_set_base_addr(mask->paddr, MASK);

    tnOUT();
    return TN_NO_ERROR;
}
