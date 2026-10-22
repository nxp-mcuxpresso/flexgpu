//
// Copyright 2025-2026 NXP
//
// SPDX-License-Identifier: MIT
//

/// @file tn_types.h
/// Tin datatype library.
/// This should hold types used across modules.
/// Add module specific types in the module's header.

#ifndef __TN_TYPES_H__
#define __TN_TYPES_H__

#include <stdint.h>
#include <stddef.h>

typedef int             tn_int;
typedef unsigned int    tn_uint;
typedef char            tn_char;
typedef unsigned char   tn_uchar;
typedef float           tn_f32;
typedef double          tn_f64;
typedef int8_t          tn_i8;
typedef uint8_t         tn_u8;
typedef int16_t         tn_i16;
typedef uint16_t        tn_u16;
typedef int32_t         tn_i32;
typedef uint32_t        tn_u32;
typedef int64_t         tn_i64;
typedef uint64_t        tn_u64;
typedef void           *tn_vptr;

/// When doing pointer arithmetic.
typedef uintptr_t       tn_uptr;
typedef intptr_t        tn_iptr;
typedef tn_f32          tn_scalar;
typedef tn_uptr         tn_vaddr;
typedef tn_uptr         tn_paddr;
typedef ptrdiff_t       tn_ptrdiff;

typedef uint32_t        tn_bool;
#define tnTRUE    1
#define tnFALSE   0

typedef uint32_t tn_size;
#if !defined(TN_SIZE_MAX)
    #define TN_SIZE_MAX ((tn_size) - 1)
#endif

typedef uint32_t tn_id;
typedef tn_id    tn_image_id;
typedef tn_id    tn_mbuf_id;
typedef tn_id    tn_path_id;
typedef tn_id    tn_path_attr_id;
typedef tn_id    tn_paint_id;
typedef tn_id    tn_font_id;
typedef tn_id    tn_mask_id;
typedef tn_id    tn_rast_id;
typedef tn_id    tn_drawable_id;
typedef tn_id    tn_pixmap_id;
typedef tn_id    tn_blt_id;
typedef tn_id    tn_ovgctx_id;
typedef tn_id    tn_surface_id;
typedef tn_id    tn_cmdq_id;
typedef tn_id    tn_mspace_id;
typedef tn_id    tn_thrd_id;
typedef tn_id    tn_mtx_id;
typedef tn_id    tn_tp_id;
typedef tn_id    tn_polyline_id;
typedef tn_id    tn_segments_id;
typedef tn_id    tn_spanbuffer_id;
typedef tn_id    tn_cmdq_ccdma_id;
typedef tn_id    tn_cmdq_ec_id;

/// Object types for the Windowing System.
typedef tn_id    tn_ws_display_id;
typedef tn_id    tn_ws_window_id;
typedef tn_id    tn_ws_egl_id;

/// Object types for EGL.
typedef tn_id    tn_egldpy_id;
typedef tn_id    tn_eglctx_id;
typedef tn_i32   tn_eglcfg_id;
typedef tn_id    tn_eglthrd_id;
typedef tn_id    tn_eglsrf_id;

/// Type to be used for path Coordinates.
typedef tn_i32   tn_path_args_type;
typedef tn_i32   tn_path_tag;
typedef tn_i32   tn_path_absrel;

typedef tn_i32   tn_image_mode;
typedef tn_u32   tn_paint_mode;
typedef tn_u32   tn_reg_32;

/// Used as integer coordinates for Image width/height, bounding boxes.
typedef tn_i32   tn_map_coord;
#define TN_MAP_MIN  (tn_map_coord)0
#define TN_MAP_MAX  (tn_map_coord)32000

typedef struct
{
    tn_map_coord   x, y;
    tn_map_coord   width, height;
} tn_rect_t;


#ifdef __cplusplus
    #define tnNULL nullptr
#else
    #define tnNULL             ((void*)0U)
#endif

#define TN_OFF_MAX UINT32_MAX
#define TN_OFF_MIN (-TN_OFF_MAX - 1)

#include "tn_result.h"
#include "tn_range.h"

#endif // __TN_TYPES_H__
