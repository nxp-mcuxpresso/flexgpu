//
// Copyright 2025-2026 NXP
//
// SPDX-License-Identifier: MIT
//

#ifndef __TN_OBJ_H__
#define __TN_OBJ_H__

#include "tn_types.h"
#include "tn_aver.h"

#ifdef __cplusplus
extern "C" {
#endif

// A 32 bits Entity IDentifier guarantees:
// 12 bits for the Entity Index.
// 8 bit for the Entity Type.
// 5 bit for the Entity Version
//
// Use the functions tn_entity_type_get and tn_entity_index_get to
// retrieve each part of an Entity.


#ifndef OBJ_INDEX_MASK
    #define OBJ_INDEX_MASK  0x00000FFFU
#endif

#ifndef OBJ_TYPE_MASK
    #define OBJ_TYPE_MASK  0x000FF000U
#endif



/// Entity type for the Entity-Component-System.
typedef enum
{
    TN_OBJ_TYPE_IMAGE      = 0x01000,
    TN_OBJ_TYPE_PATH       = 0x02000,
    TN_OBJ_TYPE_PAINT      = 0x03000,
    TN_OBJ_TYPE_MASK       = 0x04000,
    TN_OBJ_TYPE_FONT       = 0x05000,
    TN_OBJ_TYPE_PIXMAP     = 0x06000,
    TN_OBJ_TYPE_RAST       = 0x07000,
    TN_OBJ_TYPE_SURFACE    = 0x08000,
    TN_OBJ_TYPE_DRAWABLE   = 0x09000,
    TN_OBJ_TYPE_EGLSURF    = 0x0A000,
    TN_OBJ_TYPE_OVGCTX     = 0x0B000,
    TN_OBJ_TYPE_EGLCTX     = 0x0C000,
    TN_OBJ_TYPE_BUFFER     = 0x0D000,
    TN_OBJ_TYPE_WINDOW     = 0x0E000,
    TN_OBJ_TYPE_DISPLAY    = 0x0F000,
    TN_OBJ_TYPE_CMDQ       = 0x10000,
    TN_OBJ_TYPE_ALLOC      = 0x11000,
    TN_OBJ_TYPE_THREAD     = 0x12000,
    TN_OBJ_TYPE_MUTEX      = 0x13000,
    TN_OBJ_TYPE_EGLTHREAD  = 0x14000,
    TN_OBJ_TYPE_SEGMENTS   = 0x15000,
    TN_OBJ_TYPE_POLYLINE   = 0x16000,
    TN_OBJ_TYPE_PATH_ATTR  = 0x17000,
    TN_OBJ_TYPE_SPANBUFFER = 0x18000,
    TN_OBJ_TYPE_CMDQ_CCDMA = 0x19000,
    TN_OBJ_TYPE_CMDQ_FW    = 0xA0000,
    TN_OBJ_TYPE_MAX        = 0xFFFFF
} tn_obj_type;

#define TN_INVALID_ID     ((tn_id)TN_OBJ_TYPE_MAX)




/// extract type from id
static inline tn_obj_type
tn_obj_type_get(tn_id in_id)
{
    return (tn_obj_type)(in_id & OBJ_TYPE_MASK);
}



/// extract index from id
static inline tn_u32
tn_obj_index_get(tn_id in_id)
{
    tn_u32 obj_index = in_id & OBJ_INDEX_MASK;
    return obj_index;
}



#ifdef __cplusplus
}
#endif


#endif // __TN_OBJ_H__
