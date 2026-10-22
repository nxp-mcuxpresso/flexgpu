//
// Copyright 2026 NXP
//
// SPDX-License-Identifier: MIT
//

#ifndef   __TN_BITVEC32__
#define   __TN_BITVEC32__


#if !defined(TN_BITVEC_SIZEOF)
    #define TN_BITVEC_SIZEOF sizeof(tn_u32)
#endif

#if !defined(TN_BITVEC_NB_BITS)
    #define TN_BITVEC_NB_BITS 32
#endif


#define _TYPESHIFT(arr, word, shift) \
    ((__typeof(*(arr)))((__typeof(*(arr)))(word) << (shift)))

#define first_trailing_zero(x)       ((x) ? ((~x) ? (__typeof(x))__builtin_ctz(~x) : (__typeof(x)) TN_BITVEC_SIZEOF * 8) : (__typeof(x)) 0)
#define trailing_zeroes(x)           ((x) ? (__typeof(x))__builtin_ctz(x) : (__typeof(x)) TN_BITVEC_SIZEOF * 8)
#define leading_zeroes(x)            ((x) ? (__typeof(x))__builtin_clz(x) : (__typeof(x)) TN_BITVEC_SIZEOF * 8)
#define roundup2pow(x)               (1U << (TN_BITVEC_NB_BITS - leading_zeroes(x)))


// Bit functions on arrays
#define bitvec2_get(arr, wrd, idx)     (((arr)[wrd] >> (idx)) & 0x1)
#define bitvec2_set(arr, wrd, idx)     ((arr)[wrd] |=  _TYPESHIFT(arr, 1, idx))
#define bitvec2_del(arr, wrd, idx)     ((arr)[wrd] &= ~_TYPESHIFT(arr, 1, idx))


typedef struct tn_bitvec
{
    tn_u32  *words;
    tn_u32   num_of_bits;
    tn_u32   num_of_words;
    tn_u32   capacity_in_words;
} tn_bitvec;

static inline
tn_u32
bitvecX_wrd(
    tn_u32 bits_in_type,
    tn_u32 pos)
{
    return (pos) / (bits_in_type);
}



static inline
tn_u32
bitvecX_idx(
    tn_u32  bits_in_type,
    tn_size pos)
{
    // pos % bits_in_type
    return (pos) & (bits_in_type - 1);
}



static inline
tn_u32
bitvec_wrd(
    tn_u32 *arr,
    tn_size pos)
{
    tnEXPECT(!tn_is_null(arr));
    return bitvecX_wrd(TN_BITVEC_NB_BITS, pos);
}



static inline
tn_u32
bitvec_idx(
    tn_u32 *arr,
    tn_size pos)
{
    tnEXPECT(!tn_is_null(arr));
    return bitvecX_idx(TN_BITVEC_NB_BITS, pos);
}



#define bitvec_bits2words(num_of_bits)   (((num_of_bits) + TN_BITVEC_NB_BITS - 1) / TN_BITVEC_NB_BITS)
#define bitvec_words2pow(num_of_words)   (tnMAX(4U, roundup2pow(num_of_words)))


#endif // __TN_BITVEC32__
