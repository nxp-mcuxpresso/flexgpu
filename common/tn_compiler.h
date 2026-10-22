//
// Copyright 2025-2026 NXP
//
// SPDX-License-Identifier: MIT
//

#ifndef __TN_COMPILER_H__
#define __TN_COMPILER_H__

/// @file tn_compiler.h
/// Compiler dependant definitions (pragmas, builtins)

#include "tn_types.h"

#define tnLIKELY(x)    __builtin_expect((x), 1)
#define tnUNLIKELY(x)  __builtin_expect((x), 0)

/// Portable noreturn: C23 keyword when available, otherwise the compiler
/// attribute.
#if defined(__cplusplus)
    #define TN_NORETURN [[noreturn]]
#elif defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 202311L)
    #define TN_NORETURN [[noreturn]]
#else
    #define TN_NORETURN __attribute__(( noreturn ))
#endif


/// Thread-local qualifier, only used where the target has a TLS runtime.
/// The bare-metal toolchains accept the qualifier but lower each access to
/// __aeabi_read_tp plus a TLS relocation that their C library and link
/// script do not provide, so the link fails with "dangerous relocation".
/// There TN_THREAD_LOCAL expands to nothing and the object is a plain
/// static. TN_NO_THREAD_LOCAL forces that fallback on any target.
#if defined(TN_NO_THREAD_LOCAL) || defined(__ICCARM__)
    #define TN_HAS_THREAD_LOCAL 0
#elif defined(__linux__)
    #define TN_HAS_THREAD_LOCAL 1
#else
    #define TN_HAS_THREAD_LOCAL 0
#endif

#if !TN_HAS_THREAD_LOCAL
    #define TN_THREAD_LOCAL
#elif defined(__cplusplus)
    #define TN_THREAD_LOCAL thread_local
#elif defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 202311L)
    #define TN_THREAD_LOCAL thread_local
#elif defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L)
    #define TN_THREAD_LOCAL _Thread_local
#else
    #define TN_THREAD_LOCAL __thread
#endif


#define TN_ATTR_PACKED   __attribute__(( packed ))
#define TN_ATTR_ALIGN(a) __attribute__(( aligned(a) ))

#define TN_ALWAYS_INLINE inline __attribute__(( always_inline ))

/// Suppress the unused warning for objects that are only referenced by some
/// translation units, typically debug tables defined in a header.
#define TN_MAYBE_UNUSED  __attribute__(( unused ))


#if defined(__ICCARM__)
    #include <intrinsics.h>

    #define PRAGMA_SUPRESS_ON  _Pragma("diag_suppress=Pe236")
    #define PRAGMA_SUPRESS_OFF _Pragma("diag_default=Pe236")



static inline tn_i32
tn_ffs(unsigned int word)
{
    const unsigned int reverse = word & (~word + 1);
    const int bit = 32 - __CLZ(reverse);
    return bit - 1;
}



static inline tn_i32
tn_fls(unsigned int word)
{
    const int bit = word ? 32 - __CLZ(word) : 0;
    return bit - 1;
}



#else

    #define PRAGMA_SUPRESS_ON
    #define PRAGMA_SUPRESS_OFF



/// ffs/fls returning value 0..31.
static inline tn_i32
tn_ffs(tn_u32 word)
{
    return __builtin_ffs((tn_i32)word) - 1;
}



/// ffs/fls returning value 0..31.
static inline tn_i32
tn_fls(tn_u32 word)
{
    const int bit = word ? 32 - __builtin_clz(word) : 0;
    return bit - 1;
}



#endif

#endif // __TN_COMPILER_H__
