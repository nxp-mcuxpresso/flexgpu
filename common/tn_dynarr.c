//
// Copyright 2025-2026 NXP
//
// SPDX-License-Identifier: MIT
//

#include "tn_aver.h"
#include "tn_result.h"
#include "tn_types.h"
#include "tn_dynarr.h"
#include "tn_mem.h"
#include "tn_trace.h"
#include "tn_log.h"

tn_dynarr
tn_dynarr_create(
    tn_size             reserved_size,
    tn_size             element_size,
    mspace_realloc_fptr r_fptr,
    TN_MSPACE_ID        mspace)
{
    tnIN();

    void *addr;
    tn_dynarr res = {0};

    tnEXPECT(reserved_size != 0);
    tnEXPECT(element_size != 0);
    tnEXPECT(r_fptr != tnNULL);

    tn_size reserved_size_max =  TN_SIZE_MAX / element_size;
    tnEXPECT(reserved_size <= reserved_size_max);

    addr = r_fptr(mspace, tnNULL, reserved_size * element_size);
    if (tn_is_null(addr))
    {
        tnLOG_ERR("allocation failed");

        tnOUT();
        return res;
    }

    res.base.reserved_size_max = reserved_size_max;
    res.base.element_size = element_size;
    res.base.reserved_size = reserved_size;
    res.range.ptr = addr;
    res.realloc = r_fptr;
    res.mspace = mspace;

    tnOUT_ARG("addr:%p reserved_size_max:%u element_size:%u", addr, reserved_size_max, element_size);
    return res;
}



tn_dynarr
tn_dynarr_create_on_pool(
    tn_size element_size,
    tn_mbuf pool)
{
    tnIN();

    tn_dynarr res = {0};

    tnEXPECT(element_size != 0);
    tnEXPECT(!tn_is_null(pool.vaddr));

    tn_size reserved_num_elem =  pool.size / element_size;
    tnEXPECT(reserved_num_elem != 0);

    res.base.reserved_size_max = reserved_num_elem;
    res.base.element_size = element_size;
    res.base.reserved_size = reserved_num_elem;
    res.range.ptr = pool.vaddr;
    res.realloc = tnNULL;

    tnOUT_ARG("addr:%p reserved_size_max:%u element_size:%u", pool.vaddr, reserved_num_elem, element_size);
    return res;
}



tn_range
tn_dynarr_get_slice(
    void   *array,
    tn_size start,
    tn_size end,
    tn_size element_size)
{
    tnIN();

    tn_range res = {0};

    tnEXPECT(!tn_is_null(array));
    tnEXPECT(element_size != 0);
    tnEXPECT(start <= end);

    res.ptr = (uint8_t*)array + start * element_size;
    res.size = end - start;

    tnOUT_ARG("addr:%p size:%u element_size:%u", res.ptr, res.size, element_size);
    return res;
}



tn_dynarr
tn_dynarr_wrap(
    void   *adr,
    tn_size sz,
    tn_size element_size)
{
    tnIN();
    tn_dynarr res = {0};

    tnEXPECT(element_size != 0);
    tnEXPECT(!tn_is_null(adr) || sz == 0);

    tn_size reserved_size_max = TN_SIZE_MAX / element_size;
    res.base.reserved_size_max = reserved_size_max;
    res.base.element_size = element_size;
    res.base.reserved_size = sz;
    res.range.size = sz;
    res.range.ptr = adr;
    res.realloc = tnNULL;

    tnOUT_ARG("adr:%p sz:%d reserved_size_max:%u element_size:%u", adr, sz, reserved_size_max, element_size);
    return res;
}



void
tn_dynarr_destroy(tn_dynarr *dst)
{
    tnIN();

    tnEXPECT(!tn_is_null(dst));
    if (dst->realloc != tnNULL)
    {
        (void)dst->realloc(dst->mspace, dst->range.ptr, 0);
    }
    *dst = (tn_dynarr){0};

    tnOUT();
}



tn_bool
tn_dynarr_reserve(
    tn_dynarr *dst,
    tn_size    element_count)
{
    tnIN();

    tnEXPECT(!tn_is_null(dst));
    tnEXPECT(dst->base.element_size);
    tnEXPECT(dst->realloc != tnNULL);
    tnEXPECT(element_count <= dst->base.reserved_size_max);

    tn_size reserved_size = dst->base.reserved_size;
    if (reserved_size < element_count)
    {
        tn_size new_reserved_size = reserved_size + reserved_size / 2; /* Try growing 1.5x */
        /* Or grow to element_count elements if its bigger or overflow */
        if (new_reserved_size < element_count || new_reserved_size > dst->base.reserved_size_max)
            new_reserved_size = element_count;

        void *p = dst->realloc(dst->mspace, dst->range.ptr, new_reserved_size * dst->base.element_size);
        if (p)
        {
            dst->range.ptr = p;
            dst->base.reserved_size = new_reserved_size;
        }
        else
        {
            tnLOG_ERR("dynarr realloc failed");

            tnOUT();
            return tnFALSE;
        }
    }

    tnOUT();
    return tnTRUE;
}



tn_bool
tn_dynarr_insert(
    tn_dynarr  *dst,
    tn_size     idx,
    const void *src,
    tn_size     element_count)
{
    tnIN();

    // inserting nothing is a no-op, not a failure
    tnRET_IF(element_count == 0, tnTRUE);

    tnRET_IF(tn_is_null(dst) ||
             tn_is_null(src) ||
             !(idx + element_count <= dst->base.reserved_size_max), tnFALSE);

    tn_size new_size = idx + element_count;
    if (new_size >= dst->base.reserved_size && !tn_dynarr_reserve(dst, new_size))
    {
        tnOUT();
        return tnFALSE;
    }

    uint8_t *dst_arr = (uint8_t*)dst->range.ptr;
    tn_memcpy(dst_arr + idx * dst->base.element_size,
              src,
              element_count * dst->base.element_size);
    dst->range.size = new_size > dst->range.size ? new_size : dst->range.size;
    tnLOG_INFO("insert(%p): idx:%u element_size:%u element_count:%u size:%u new_size:%u",
               dst, idx, dst->base.element_size, element_count, dst->range.size, new_size);

    tnOUT();
    return tnTRUE;
}



tn_bool
tn_dynarr_selfinsert(
    tn_dynarr *dst,
    tn_size    idst,
    tn_size    isrc,
    tn_size    element_count)
{
    tnIN();

    // inserting nothing is a no-op, not a failure
    tnRET_IF(element_count == 0, tnTRUE);

    register tn_size size;
    if (dst && dst->base.reserved_size_max - element_count >= (size = dst->range.size) && idst <= size
        && isrc < size && tn_dynarr_reserve(dst, size + element_count))
    {
        uint8_t *arr = (uint8_t*)dst->range.ptr;
        tn_size esz = dst->base.element_size;

        tn_memmove(arr + (idst + element_count) * esz, arr + idst * esz, (size - idst) * esz);
        /* If idst < isrc, isrc has moved element_count ahead.
         * If idst == isrc, don't copy.
         */
        tn_memmove(arr + idst * esz,
                   arr + (isrc + (idst < isrc) * element_count) * esz,
                   element_count * esz * (idst != isrc));

        dst->range.size = size + element_count;

        tnOUT();
        return tnTRUE;
    }

    tnOUT();
    return tnFALSE;
}



tn_bool
tn_dynarr_remove(
    tn_dynarr *dst,
    tn_size    offset,
    tn_size    nb_elem)
{
    tnIN();

    register tn_dynarr m;
    if (dst && dst->base.element_size &&
        offset <= (m = *dst).range.size &&
        nb_elem <= m.range.size - offset)
    {

        /* Shift elements at index greater than offset one step back */
        tn_memmove((tn_u8*)m.range.ptr + offset * dst->base.element_size,
                   (tn_u8*)m.range.ptr + (offset + nb_elem) * dst->base.element_size,
                   (m.range.size - offset - nb_elem) * dst->base.element_size);

        dst->range.size = m.range.size - nb_elem;

        tnOUT();
        return tnTRUE;
    }

    tnOUT();
    return tnFALSE;
}



void
tn_dynarr_shrink_to_fit(tn_dynarr *dst)
{
    tnIN();

    tnEXPECT(!tn_is_null(dst));
    tnEXPECT(dst->realloc != tnNULL);

    if (dst->base.reserved_size > dst->range.size)
    {
        void *arr = dst->range.ptr;
        void *p = dst->realloc(dst->mspace, arr, dst->range.size * dst->base.element_size);
        if (dst->range.size == 0)
        {
            dst->range.ptr = p;
            dst->base.reserved_size = 0;
        }
        else if (p)
        {
            dst->range.ptr = p;
            dst->base.reserved_size = dst->range.size;
        }
    }

    tnOUT();
}
