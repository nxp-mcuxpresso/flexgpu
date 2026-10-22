//
// Copyright 2025-2026 NXP
//
// SPDX-License-Identifier: MIT
//

#ifndef __TN_DYNARR_H__
#define __TN_DYNARR_H__

/// @file tn_dynarr.h
/// DYNARR: Dynamic Array.

#include "tn_sort.h"
#include "tn_aver.h"
#include "tn_sysheap.h"
#include "tn_mbuf.h"
#include "tn_mspace.h"
#include "tn_trace.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    tn_size   reserved_size;
    tn_size   reserved_size_max;
    tn_size   element_size;
} tn_dynarr_base;


typedef struct
{
    tn_dynarr_base        base;
    tn_range              range;
    mspace_realloc_fptr   realloc;
    TN_MSPACE_ID          mspace;
} tn_dynarr;


#define typedef_dynarr(element_type, ident, ...)                                                             \
    typedef union {                                                                                          \
        tn_dynarr   dynarr;                                                                                  \
        struct {                                                                                             \
            tn_dynarr_base   base;                                                                           \
            element_type    *arr;                                                                            \
            tn_size          size;                                                                           \
        };                                                                                                   \
    } ident;                                                                                                 \
                                                                                                             \
    typedef union {                                                                                          \
        tn_range   range;                                                                                    \
        struct {                                                                                             \
            element_type  *arr;                                                                              \
            tn_size        size;                                                                             \
        };                                                                                                   \
    } ident ## _slice;                                                                                       \
                                                                                                             \
    __VA_ARGS__ ident ident ## _create(void);                                                                \
    __VA_ARGS__ ident ident ## _create(void) {                                                               \
        tnIN();                                                                                              \
        tn_dynarr tmp = tn_dynarr_create(7, sizeof(element_type), tn_mspace_realloc, TN_MSPACE_SYSHEAP);     \
        tnOUT();                                                                                             \
        return (ident){.dynarr = tmp};                                                                       \
    }                                                                                                        \
    __VA_ARGS__ ident ident ## _create_init_size(tn_size s);                                                 \
    __VA_ARGS__ ident ident ## _create_init_size(tn_size s) {                                                \
        tnIN();                                                                                              \
        tn_dynarr tmp = tn_dynarr_create(s, sizeof(element_type), tn_mspace_realloc, TN_MSPACE_SYSHEAP);     \
        tnOUT();                                                                                             \
        return (ident){.dynarr = tmp};                                                                       \
    }                                                                                                        \
    __VA_ARGS__ ident ident ## _create_a(mspace_realloc_fptr r_fptr, TN_MSPACE_ID mspace);                   \
    __VA_ARGS__ ident ident ## _create_a(mspace_realloc_fptr r_fptr, TN_MSPACE_ID mspace) {                  \
        tnIN();                                                                                              \
        tn_dynarr tmp = tn_dynarr_create(7, sizeof(element_type), r_fptr, mspace);                           \
        tnOUT();                                                                                             \
        return (ident){.dynarr = tmp};                                                                       \
    }                                                                                                        \
    __VA_ARGS__ ident ident ## _create_on_pool(tn_mbuf pool);                                                \
    __VA_ARGS__ ident ident ## _create_on_pool(tn_mbuf pool) {                                               \
        tnIN();                                                                                              \
        tn_dynarr tmp = tn_dynarr_create_on_pool(sizeof(element_type), pool);                                \
        tnOUT();                                                                                             \
        return (ident){.dynarr = tmp};                                                                       \
    }                                                                                                        \
    __VA_ARGS__ ident ## _slice ident ## _get_slice(ident list, tn_size start,                               \
                                                    tn_size end);                                            \
    __VA_ARGS__ ident ## _slice ident ## _get_slice(ident list, tn_size start,                               \
                                                    tn_size end) {                                           \
        tnIN();                                                                                              \
        tn_range tmp = tn_dynarr_get_slice(list.dynarr.range.ptr,                                            \
                                           start, end, sizeof(element_type));                                \
        tnOUT();                                                                                             \
        return (ident ## _slice){.range = tmp};                                                              \
    }                                                                                                        \
    __VA_ARGS__ ident ident ## _wrap(void *adr, tn_size sz);                                                 \
    __VA_ARGS__ ident ident ## _wrap(void *adr, tn_size sz) {                                                \
        tnIN();                                                                                              \
        tn_dynarr tmp = tn_dynarr_wrap(adr, sz, sizeof(element_type));                                       \
        tnOUT();                                                                                             \
        return (ident){.dynarr = tmp};                                                                       \
    }

tn_dynarr tn_dynarr_create(
    tn_size             reserved_size,
    tn_size             element_size,
    mspace_realloc_fptr fptr,
    TN_MSPACE_ID        mspace);

/// Create a dynamic array on a preallocated memory pool.
/// \param[in]  element_size     the size of an element
/// \param[in]  pool             Size to allocate.
/// \param[out] tn_dynarr        a fixed capacity dynamic array
///  Because the memory pool is managed externally,
///  the methods tn_dynarr_reserve & tn_dynarr_shrink_to_fit
///  DO NOT APPLY to the newly created dynamic array
tn_dynarr tn_dynarr_create_on_pool(
    tn_size element_size,
    tn_mbuf pool);

// Get a slice (tn_range) of the dynarr in the interval [start, end)
tn_range tn_dynarr_get_slice(
    void   *array,
    tn_size start,
    tn_size end,
    tn_size element_size);

tn_dynarr tn_dynarr_wrap(
    void   *adr,
    tn_size sz,
    tn_size element_size);

void tn_dynarr_destroy(tn_dynarr*);

tn_bool tn_dynarr_reserve(
    tn_dynarr *dst,
    tn_size    sz);

tn_bool tn_dynarr_insert(
    tn_dynarr  *dst,
    tn_size     pos,
    const void *src,
    tn_size     element_count);

tn_bool tn_dynarr_selfinsert(
    tn_dynarr *foo,
    tn_size    idst,
    tn_size    isrc,
    tn_size    element_count);

tn_bool tn_dynarr_remove(
    tn_dynarr *dst,
    tn_size    i,
    tn_size    element_count);

void tn_dynarr_shrink_to_fit(tn_dynarr *d);


/// Append a value by directly assigning to the array element.
/// This avoids the tn_memcpy done by tn_dynarr_push_back.
#define tn_dynarr_push_val(dst, val)                                                                                 \
    do {                                                                                                             \
        tn_size new_size = (dst)->size + 1;                                                                          \
        if (new_size >= (dst)->dynarr.base.reserved_size && !tn_dynarr_reserve(&(dst)->dynarr, new_size)) assert(0); \
        (dst)->arr[ (dst)->size++ ] = val;                                                                           \
    } while (0)


static inline tn_bool
tn_dynarr_push_back(
    tn_dynarr  *dst,
    const void *val)
{
    return tn_dynarr_insert(dst, dst->range.size, val, 1);
}



static inline tn_bool
tn_dynarr_concat(
    tn_dynarr       *dst,
    const tn_dynarr *src)
{
    tnEXPECT(dst->base.element_size != 0);
    tnEXPECT(src->base.element_size != 0);
    return tn_dynarr_insert(dst,
                            dst->range.size,
                            src->range.ptr,
                            src->range.size);
}



static inline tn_size
tn_dynarr_size(const tn_dynarr *dst)
{
    tnEXPECT(dst->base.element_size != 0);
    return dst->range.size;
}



static inline void
tn_dynarr_clear(tn_dynarr *dst)
{
    tnEXPECT(dst->base.element_size != 0);
    dst->range.size = 0;
}



static inline void
tn_dynarr_sort(
    tn_dynarr *dst,
    int      (*cmpfunc)(
        const void*,
        const void*))
{
    tnEXPECT(dst->base.element_size != 0);
    tn_sort(dst->range.ptr, dst->range.size, dst->base.element_size, cmpfunc);
}



static inline void
tn_dynarr_sort_r(
    tn_dynarr  *dst,
    int       (*cmpfunc)(
        const void*,
        const void*,
        void*),
    const void *arg)
{
    tnEXPECT(dst->base.element_size != 0);
    tn_sort_r(dst->range.ptr, dst->range.size, dst->base.element_size, cmpfunc, arg);
}



#define tn_dynarr_last(dst)  ( &((dst)->arr[(dst)->size - 1]) )

typedef_dynarr(tn_id, tn_id_list, static inline)
typedef_dynarr(tn_u32, tn_u32_list, static inline)
typedef_dynarr(tn_u16, tn_u16_list, static inline)


#ifdef __cplusplus
}
#endif

#endif // __TN_DYNARR_H__
