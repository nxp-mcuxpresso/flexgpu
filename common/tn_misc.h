//
// Copyright 2025 NXP
//
// SPDX-License-Identifier: MIT
//

#ifndef __TN_MISC_H__
#define __TN_MISC_H__

/// @file tn_misc.h
/// miscellaneous: a variety of objects that do not fit into any other specific category.
/// This should be seen as temporary landing place until
/// its contents get their own category.


#include "tn_types.h"


#define tnALIGN(number, alignment)  \
    (((number) + ((alignment) - 1)) & ~((alignment) - 1))



static inline void
RI_INT_SWAP(
    int *a,
    int *b)
{
    int tmp = *a; *a = *b; *b = tmp;
}



#endif // __TN_MISC_H__
