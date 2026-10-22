//
// Copyright 2026 NXP
//
// SPDX-License-Identifier: MIT
//

#ifndef   __TN_DEFS_H__
#define   __TN_DEFS_H__

#define TN_IMAGE_QUALITY_NONANTIALIASED   (1 << 0)
#define TN_IMAGE_QUALITY_FASTER           (1 << 1)
#define TN_IMAGE_QUALITY_BETTER           (1 << 2)

#define TN_BLEND_SRC                      0x00
#define TN_BLEND_SRC_OVER                 0x01
#define TN_BLEND_DST_OVER                 0x02
#define TN_BLEND_SRC_IN                   0x03
#define TN_BLEND_DST_IN                   0x04
#define TN_BLEND_MULTIPLY                 0x05
#define TN_BLEND_SCREEN                   0x06
#define TN_BLEND_DARKEN                   0x07
#define TN_BLEND_LIGHTEN                  0x08
#define TN_BLEND_ADDITIVE                 0x09
#define TN_BLEND_SRC_OUT                  0x0A
#define TN_BLEND_DST_OUT                  0x0B
#define TN_BLEND_SUBTRACT                 0x0C
#define TN_BLEND_XOR                      0x0D
#define TN_BLEND_SPECIAL                  0x0E

// The blend modes enumerated above are the same for both OpenVG and VGLite
// If TN_BLEND_VGLITE is enabled, it means that the blending is done in
// VG_LITE mode (non-premultiplied)
#define TN_BLEND_VGLITE                   0x10

// Masks that are used to extract blend mode and VG_LITE flag
#define TN_BLEND_MODE_MASK                0x0F
#define TN_BLEND_VGLITE_MASK              0xF0

#define TN_FILLRULE_EVEN_ODD              0x1900
#define TN_FILLRULE_NON_ZERO              0x1901

// Pattern paint parameters
#define TN_PAINT_PATTERN_TILING_MODE      0x1A06
#define TN_PAINT_TYPE_COLOR               0x1B00
#define TN_PAINT_TYPE_LINEAR_GRADIENT     0x1B01
#define TN_PAINT_TYPE_RADIAL_GRADIENT     0x1B02
#define TN_PAINT_TYPE_PATTERN             0x1B03
#define TN_COLOR_RAMP_SPREAD_PAD          0x1C00
#define TN_COLOR_RAMP_SPREAD_REPEAT       0x1C01
#define TN_COLOR_RAMP_SPREAD_REFLECT      0x1C02
#define TN_TILE_FILL                      0x1D00
#define TN_TILE_PAD                       0x1D01
#define TN_TILE_REPEAT                    0x1D02
#define TN_TILE_REFLECT                   0x1D03

/// ????
#define TN_MAX_EDGES 262144
// OpenVG   VGLite
#define TN_RENDERING_QUALITY_NONANTIALIASED 0x00 //  no AA    no AA
#define TN_RENDERING_QUALITY_FASTER         0x01 //  8x AA    4x AA
#define TN_RENDERING_QUALITY_BETTER         0x02 // 32x AA    8x AA
#define TN_RENDERING_QUALITY_BEST           0x03 // 32x AA   16x AA
#define TN_RENDERING_QUALITY_MAX            0x03 //          MUST have be equal to the one above

#define TN_DRAW_IMAGE_NORMAL              0x1F00
#define TN_DRAW_IMAGE_MULTIPLY            0x1F01
#define TN_DRAW_IMAGE_STENCIL             0x1F02

// Mask that is used to extract the image drawing mode
#define TN_DRAW_IMAGE_MODE_MASK           0x0003

#define TN_RED                            (1 << 3)
#define TN_GREEN                          (1 << 2)
#define TN_BLUE                           (1 << 1)
#define TN_ALPHA                          (1 << 0)

#endif // __TN_DEFS_H__
