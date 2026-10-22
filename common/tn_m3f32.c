//
// Copyright 2025-2026 NXP
//
// SPDX-License-Identifier: MIT
//

#include "tin.h"
#include "tn_log.h"
#include "tn_m3f32.h"
#include "tn_v2f32.h"

#define PI  3.141592654f

/* Determinant magnitude below which a matrix is treated as singular. An exact
   "== 0.0f" test only catches determinants that round to precisely zero; a
   determinant that is merely tiny (e.g. a face rotated almost edge-on) would
   otherwise pass and produce a blown-up 1/det inverse. Rejecting near-zero
   determinants keeps the inverse well conditioned. */
#define TN_DET_EPS  1.0e-6f

static inline tn_f32
deg_to_rad(tn_f32 a)
{
    return a * PI / 180.0f;
}



/** set m1 to be the identity matrix.
 *
 * @param  m1:  the matrix to be checked.
 */
void
tn_m3f32_identity(tn_m3f32 *m1)
{
    *m1 = (tn_m3f32){
        .m =
        {
            [0] = {1, 0, 0},
            [1] = {0, 1, 0},
            [2] = {0, 0, 1}
        }



    };
}



/** matrix multiply.
 *
 * @param  m1:  the matrix to be checked.
 * @param  m2:  the matrix to be checked.
 * @return m1*m2
 */
tn_m3f32
tn_m3f32_mul_m3f32(
    const tn_m3f32 *m1,
    const tn_m3f32 *m2)
{
    tn_m3f32 t;
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            t.m[i][j] = m1->m[i][0] * m2->m[0][j] + m1->m[i][1] * m2->m[1][j] + m1->m[i][2] * m2->m[2][j];
    return t;
}



/** matrix multiply inplace.
 *
 * @param  m1:  the matrix to be checked.
 * @param  m2:  the matrix to be checked.
 */
void
tn_m3f32_mul_m3f32_imm(
    tn_m3f32       *m1,
    const tn_m3f32 *m2)
{
    tn_m3f32 t;
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            t.m[i][j] = m1->m[i][0] * m2->m[0][j] + m1->m[i][1] * m2->m[1][j] + m1->m[i][2] * m2->m[2][j];
    *m1 = t;
}



/** check whether m1 is affine.
 *
 * @param  m1:  the matrix to be checked.
 */
tn_bool
tn_m3f32_is_affine(const tn_m3f32 *m1)
{
    if (m1->m[2][0] == 0.0f && m1->m[2][1] == 0.0f && m1->m[2][2] == 1.0f)
        return 1;
    return 0;
}



/** check whether m1 is performing only translation.
 *
 * @param  m1:  the matrix to be checked.
 */
tn_bool
tn_m3f32_is_translation_only(const tn_m3f32 *m1)
{
    if (tn_m3f32_is_affine(m1) &&
        (m1->m[0][0] == 1 && m1->m[0][1] == 0
         && m1->m[1][0] == 0 && m1->m[1][1] == 1))
        return 1;
    return 0;
}



/** invert m1.
 *
 * @param  m1:  the matrix to be inverted.
 * @return  1:  the matrix is invertible.
 * @return  0:  the matrix in not invertible.
 */
tn_bool
tn_m3f32_invert_imm(tn_m3f32 *m1)
{
    tn_bool affine = tn_m3f32_is_affine(m1);
    tn_f32 det00 = m1->m[1][1] * m1->m[2][2] - m1->m[2][1] * m1->m[1][2];
    tn_f32 det01 = m1->m[2][0] * m1->m[1][2] - m1->m[1][0] * m1->m[2][2];
    tn_f32 det02 = m1->m[1][0] * m1->m[2][1] - m1->m[2][0] * m1->m[1][1];

    tn_f32 d = m1->m[0][0] * det00 + m1->m[0][1] * det01 + m1->m[0][2] * det02;
    if (tn_fabsf32(d) < TN_DET_EPS)
        return 0;               //singular or near-singular, leave m1 unmodified and return false

    d = 1.0f / d;

    tn_m3f32 t;
    t.m[0][0] = d * det00;
    t.m[1][0] = d * det01;
    t.m[2][0] = d * det02;
    t.m[0][1] = d * (m1->m[2][1] * m1->m[0][2] - m1->m[0][1] * m1->m[2][2]);
    t.m[1][1] = d * (m1->m[0][0] * m1->m[2][2] - m1->m[2][0] * m1->m[0][2]);
    t.m[2][1] = d * (m1->m[2][0] * m1->m[0][1] - m1->m[0][0] * m1->m[2][1]);
    t.m[0][2] = d * (m1->m[0][1] * m1->m[1][2] - m1->m[1][1] * m1->m[0][2]);
    t.m[1][2] = d * (m1->m[1][0] * m1->m[0][2] - m1->m[0][0] * m1->m[1][2]);
    t.m[2][2] = d * (m1->m[0][0] * m1->m[1][1] - m1->m[1][0] * m1->m[0][1]);
    if (affine)
        t.v[2] = (tn_v3f32){{0, 0, 1}}
        ;                               //preserve affine
    *m1 = t;
    return 1;
}



/** multiply m1 with a translation matrix.
 *
 * @param  m1:  the matrix object that will be translated.
 * @param  tx: adjustment along the X axis.
 * @param  ty: adjustment along the Y axis.
 */
void
tn_m3f32_translate_imm(
    tn_m3f32 *m1,
    tn_f32    tx,
    tn_f32    ty)
{
    tn_m3f32 n =
    {
        .m =
        {
            [0] = {1, 0, tx},
            [1] = {0, 1, ty},
            [2] = {0, 0, 1 }
        }



    };
    tn_m3f32_mul_m3f32_imm(m1, &n);
}



/** multiply m1, inplace, with a scaling matrix.
 *
 *  @param      m1:  the matrix object that will be scaled.
 *  @param[in]  sx:  scaling factor along the X axis.
 *  @param[in]  sy:  scaling factor along the Y axis.
 */
void
tn_m3f32_scale_imm(
    tn_m3f32 *m1,
    tn_f32    sx,
    tn_f32    sy)
{
    tn_m3f32 n =
    {
        .m =
        {
            [0] = {sx, 0,  0},
            [1] = {0,  sy, 0},
            [2] = {0,  0,  1}
        }



    };
    tn_m3f32_mul_m3f32_imm(m1, &n);
}



/** multiply m1, inplace, with a shearing matrix.
 *
 *  @param      m1:  the matrix object that will be sheared.
 *  @param[in]  shx: shearing factor along the X axis.
 *  @param[in]  shy: shearing factor along the Y axis.
 */
void
tn_m3f32_shear_imm(
    tn_m3f32 *m1,
    tn_f32    shx,
    tn_f32    shy)
{
    tn_m3f32 n =
    {
        .m =
        {
            [0] = {1,   shx, 0},
            [1] = {shy, 1,   0},
            [2] = {0,   0,   1}
        }



    };
    tn_m3f32_mul_m3f32_imm(m1, &n);
}



/** multiply m1, inplace, with a rotation matrix around Z axis (normal to the screen)
 *
 * @param  m1:  the matrix object that will be rotate.
 * @param  degrees:   number of degrees to rotate the matrix; positive numbers rotate counter clockwise.
 */
void
tn_m3f32_rotate_deg_imm(
    tn_m3f32 *m1,
    tn_f32    degrees)
{
    if (degrees < 0.0f)
    {
        degrees += 360.0f;
    }
    if (degrees == 90.0f)
    {
        /* Rotation matrix for 90 degree rotation is
                    [0] = {0, -1, 0},
                    [1] = {1,  0, 0},
                    [2] = {0,  0, 1}
                Matrix multiplication can be optimized.
        */
        tn_f32 element = m1->m[0][0];
        m1->m[0][0] = m1->m[0][1];
        m1->m[0][1] = -element;
        //m1->m[0][2] = m1->m[0][2]; - No change needed
        element = m1->m[1][0];
        m1->m[1][0] = m1->m[1][1];
        m1->m[1][1] = -element;
        //m1->m[1][2] = m1->m[1][2]; - No change needed
        element = m1->m[2][0];
        m1->m[2][0] = m1->m[2][1];
        m1->m[2][1] = -element;
        //m1->m[2][2] = m1->m[2][2]; - No change needed
        return;
    }
    else if (degrees == 180.0f)
    {
        /* Rotation matrix for 180 degree rotation is
                [0] = {-1,  0, 0},
                [1] = { 0, -1, 0},
                [2] = { 0,  0, 1}
            Matrix multiplication can be optimized.
        */
        m1->m[0][0] *= -1;
        m1->m[0][1] *= -1;
        //m1->m[0][2] *= 1; - No change needed
        m1->m[1][0] *= -1;
        m1->m[1][1] *= -1;
        //m1->m[1][2] *= 1; - No change needed
        m1->m[2][0] *= -1;
        m1->m[2][1] *= -1;
        //m1->m[2][2] *= 1; - No change needed
        return;
    }
    else if (degrees == 270.0f)
    {
        /* Rotation matrix for 90 degree rotation is
                    [0] = { 0, 1, 0},
                    [1] = {-1, 0, 0},
                    [2] = { 0, 0, 1}
                Matrix multiplication can be optimized.
        */
        tn_f32 element = m1->m[0][0];
        m1->m[0][0] = -m1->m[0][1];
        m1->m[0][1] = element;
        //m1->m[0][2] = m1->m[0][2]; - No change needed
        element = m1->m[1][0];
        m1->m[1][0] = -m1->m[1][1];
        m1->m[1][1] = element;
        //m1->m[1][2] = m1->m[1][2]; - No change needed
        element = m1->m[2][0];
        m1->m[2][0] = -m1->m[2][1];
        m1->m[2][1] = element;
        //m1->m[2][2] = m1->m[2][2]; - No change needed
        return;
    }

    tn_f32 a = deg_to_rad(degrees);
    tn_m3f32 n;
    n.m[0][0] = (tn_f32)tn_cosf32(a);
    n.m[1][0] = (tn_f32)tn_sinf32(a);
    n.m[0][1] = -n.m[1][0];
    n.m[1][1] = n.m[0][0];
    n.m[0][2] = n.m[1][2] = n.m[2][0] = n.m[2][1] = 0.0;
    n.m[2][2] = 1.0;

    tn_m3f32_mul_m3f32_imm(m1, &n);
}



/** multiply m1, inplace, with a perspective matrix.
 *
 * @param  m1:  the matrix object that will be rotate.
 * @param  px:  the p_x of the perspective transformation matrix.
 * @param  py:  the p_y of the perspective transformation matrix.
 */
void
tn_m3f32_perspective_imm(
    tn_m3f32 *m1,
    tn_f32    px,
    tn_f32    py)
{
    tn_m3f32 n =
    {
        .m =
        {
            [0] = {1,  0,  0},
            [1] = {0,  1,  0},
            [2] = {px, py, 1}
        }



    };
    tn_m3f32_mul_m3f32_imm(m1, &n);
}



void
tn_m3f32_print(const tn_m3f32 *m1)
{
    tnLOG_INFO("%f %f %f", m1->m[0][0], m1->m[0][1], m1->m[0][2]);
    tnLOG_INFO("%f %f %f", m1->m[1][0], m1->m[1][1], m1->m[1][2]);
    tnLOG_INFO("%f %f %f", m1->m[2][0], m1->m[2][1], m1->m[2][2]);
}



// get visible blend area with the same size (or less)
// as the bounding box of the source after affine or projective xform
// | a b tx |
// | c d ty |
// | p q w  |
void
tn_m3f32_get_blend_area(
    tn_rect_t *rect,
    tn_m3f32  *mat,
    tn_u32     w,
    tn_u32     h,
    tn_bool    translation_only)
{
    tn_f32 tx = mat->m[0][2];
    tn_f32 ty = mat->m[1][2];

    // the coordinates of the corners after xform
    // The right and bottom coordinate marks first pixel outside the transformed rectangle.
    tn_v2f32 TL, TR, BL, BR;

    // Initialize via the x/y struct members so that the union is read and
    // written through the same members (silences INCONSISTENT_UNION_ACCESS).
    TL = (tn_v2f32){.x = tx, .y = ty};

    if (translation_only)
    {
        // For translation-only transformations, simplify calculations
        TR = (tn_v2f32){.x = tx + (tn_f32)w, .y = ty};
        BL = (tn_v2f32){.x = tx, .y = ty + (tn_f32)h};
        BR = (tn_v2f32){.x = TR.x, .y = BL.y};
    }
    else if (tn_m3f32_is_affine(mat))
    {
        // Affine transformation (p=0, q=0, w=1)
        // the coordinates of the top-right corner after xform
        // TR = {a * width + tx, c * width + ty}
        tn_f32 scaled_a = mat->m[0][0] * ((tn_f32)w);
        tn_f32 scaled_c = mat->m[1][0] * ((tn_f32)w);
        TR = (tn_v2f32){.x = scaled_a + tx, .y = scaled_c + ty};

        // the coordinates of the bottom-left corner after xform
        // BL = {b * height + tx, d * height + ty}
        tn_f32 scaled_b = mat->m[0][1] * ((tn_f32)h);
        tn_f32 scaled_d = mat->m[1][1] * ((tn_f32)h);
        BL = (tn_v2f32){.x = scaled_b + tx, .y = scaled_d + ty};

        // the coordinates of the botton-right corner after xform
        BR = (tn_v2f32){.x = scaled_a + scaled_b + tx, .y = scaled_c + scaled_d + ty};
    }
    else
    {
        // Projective transformation - need to apply full homogeneous transformation
        // and divide by w component
        tn_f32 a = mat->m[0][0];
        tn_f32 b = mat->m[0][1];
        tn_f32 c = mat->m[1][0];
        tn_f32 d = mat->m[1][1];
        tn_f32 p = mat->m[2][0];
        tn_f32 q = mat->m[2][1];
        tn_f32 w_coeff = mat->m[2][2];

        // Top-left corner (0, 0)
        tn_f32 tl_w = p * 0.0f + q * 0.0f + w_coeff;
        TL = (tn_v2f32){.x = tx / tl_w, .y = ty / tl_w};

        // Top-right corner (w, 0)
        tn_f32 tr_x = a * (tn_f32)w + tx;
        tn_f32 tr_y = c * (tn_f32)w + ty;
        tn_f32 tr_w = p * (tn_f32)w + w_coeff;
        TR = (tn_v2f32){.x = tr_x / tr_w, .y = tr_y / tr_w};

        // Bottom-left corner (0, h)
        tn_f32 bl_x = b * (tn_f32)h + tx;
        tn_f32 bl_y = d * (tn_f32)h + ty;
        tn_f32 bl_w = q * (tn_f32)h + w_coeff;
        BL = (tn_v2f32){.x = bl_x / bl_w, .y = bl_y / bl_w};

        // Bottom-right corner (w, h)
        tn_f32 br_x = a * (tn_f32)w + b * (tn_f32)h + tx;
        tn_f32 br_y = c * (tn_f32)w + d * (tn_f32)h + ty;
        tn_f32 br_w = p * (tn_f32)w + q * (tn_f32)h + w_coeff;
        BR = (tn_v2f32){.x = br_x / br_w, .y = br_y / br_w};
    }


    // find the blend area
    tn_map_coord minx = (tn_map_coord)roundf(minf(minf(TL.x, TR.x), minf(BL.x, BR.x)));
    if (minx < 0)   // left to the visible area
    {
        rect->x = 0;
    }
    else if (minx >= rect->width)     //right to the visible area
    {
        rect->x = rect->width;
    }
    else
    {
        rect->x = minx;
    }

    tn_map_coord miny = (tn_map_coord)roundf(minf(minf(TL.y, TR.y), minf(BL.y, BR.y)));
    if (miny < 0)   // above the visible area
    {
        rect->y = 0;
    }
    else if (miny >= rect->height)     //below the visible area
    {
        rect->y = rect->height;
    }
    else
    {
        rect->y = miny;
    }

    tn_f32 maxr = maxf(maxf(TL.x, TR.x), maxf(BL.x, BR.x));
    tn_map_coord maxx = (roundf(maxr) >= rect->width + 1) ?
                        rect->width + 1 : (tn_map_coord)roundf(maxr);

    if (maxx < 0)
    {
        rect->width = 0;
    }
    else if (maxx >= rect->width)
    {
        rect->width = rect->width - rect->x;
    }
    else
    {
        rect->width = maxx - rect->x;
    }

    maxr = maxf(maxf(TL.y, TR.y), maxf(BL.y, BR.y));
    tn_map_coord maxy = (roundf(maxr) >= rect->height + 1) ?
                        rect->height + 1 : (tn_map_coord)roundf(maxr);

    if (maxy < 0)   // above the visible area
    {
        rect->height = 0;
    }
    else if (maxy >= rect->height)     //below the visible area
    {
        rect->height = rect->height - rect->y;
    }
    else
    {
        rect->height = maxy - rect->y;
    }
}
