//
// Copyright 2025-2026 NXP
//
// SPDX-License-Identifier: MIT
//

#ifndef __TN_AVER_H__
#define __TN_AVER_H__

/// @file tn_aver.h
/// aver: verb, to state or assert to be the case.
///
/// tnABORT                     - unconditional trap, noreturn, release-active.
/// tnRET_IF                    - reject bad input, return a code; active in both builds. It
///                               always keeps the branch, the tnOUT_ARG trace and the return.
///                               It additionally reports func/line/condition when
///                               TN_RET_REPORT is set, and panics when TN_RET_PANIC is set.
///                               Condition only, no call.
/// tnRET_VOID_IF               - tnRET_IF for void-returning functions: same contract, but
///                               returns early instead of propagating a retval.
/// tnEXPECT                    - debug only, absent in release.
///

#include "tn_compiler.h"
#include "tn_config.h"
#include "tn_trace.h"

#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

/// Fault classification emitted in release diagnostics.
typedef enum tn_fault
{
    TN_FAULT_NULL_PTR     = 1,
    TN_FAULT_BOUNDS       = 2,
    TN_FAULT_RANGE        = 3,
    TN_FAULT_OVERFLOW     = 4,
    TN_FAULT_STATE        = 5,
    TN_FAULT_UNREACHABLE  = 6,
    TN_FAULT_UNSUPPORTED  = 7,
    TN_FAULT_INTERNAL     = 8,
} tn_fault;

void tn_aver_fault(tn_fault code);

TN_NORETURN void tn_aver_panic(tn_fault code);

// ---------------------------------------------------------------------------
// TN_RET_REPORT - per-check reporting for tnRET_IF / tnRET_VOID_IF.
//
// TN_RET_PANIC implies it: a build that traps at the first failing check wants
// to know which check it was, and it stops at the first one so the cost of the
// report does not matter. Derived here, before the declaration below tests it.
// ---------------------------------------------------------------------------

#if defined(TN_RET_PANIC) && !defined(TN_RET_REPORT)
    #define TN_RET_REPORT
#endif

// tn_aver_report() is compiled whenever any of its callers can be active.
// tnABORT / tnEXPECT use it under TN_AVER_DEBUG; tnRET_IF / tnRET_VOID_IF use
// it under the narrower TN_RET_REPORT, which can be enabled on its own.
#if defined(TN_AVER_DEBUG) || defined(TN_RET_REPORT)

void tn_aver_report(
    tn_fault    code,
    const char *func,
    int         line,
    const char *cond,
    const char *fmt,
    ...);

#endif

#define TN_AVER_XSTR_(x) TN_AVER_STR_(x)
#define TN_AVER_STR_(x)  #x

// ---------------------------------------------------------------------------
// tnABORT - unconditional, noreturn, always present.
// ---------------------------------------------------------------------------

// Both forms of the conditional comma are spelled out here, selected on
// TN_HAS_VA_OPT (tn_trace.h), because __VA_OPT__ is C23 / C++20 only and IAR
// rejects it in the toolchain's default C mode. The comma cannot be factored
// into a shared helper - see the comment next to TN_HAS_VA_OPT for why.
#if defined(TN_AVER_DEBUG)
    #if TN_HAS_VA_OPT
        #define tnABORT(code, fmt, ...)                                      \
            do {                                                             \
                tn_aver_report((code), __func__, __LINE__, "",               \
                               fmt __VA_OPT__( , ) __VA_ARGS__);             \
                tn_aver_panic(code);                                         \
            } while (0)
    #else
        #define tnABORT(code, fmt, ...)                                      \
            do {                                                             \
                tn_aver_report((code), __func__, __LINE__, "",               \
                               fmt, ## __VA_ARGS__);                         \
                tn_aver_panic(code);                                         \
            } while (0)
    #endif
#else
    #define tnABORT(code, fmt, ...)                                        \
        do {                                                               \
            tn_aver_fault(code);                                           \
            tn_aver_panic(code);                                           \
        } while (0)
#endif


// ---------------------------------------------------------------------------
// tnRET_IF - reject bad input; returns retval when the condition holds. No trap.
// Active in both debug and release, so it is safe to guard recoverable
// conditions whose retval the caller relies on (bad handle, out of memory,
// out-of-range index) as well as pure invariants.
// The condition describes the failure; when it is true return retval to the
// caller. Argument must be side-effect free: hoist any call into a local first.
//
// TN_RET_REPORT adds the diagnostic: func/line/condition are reported before
// the return. Without it the macro keeps only the branch, the tnOUT_ARG trace
// and the return, which is what a plain TN_ASSERT build now gets.
//
// When TN_RET_PANIC is defined, a taken tnRET_IF behaves like tnABORT:
// it reports the failure and then panics (noreturn) instead of returning
// retval, so the driver traps at the first check that fails.
// ---------------------------------------------------------------------------

#if defined(TN_RET_REPORT)
    #if defined(TN_RET_PANIC)
        #define tnRET_IF(cond, retval)                                     \
            do {                                                           \
                if (tnUNLIKELY(cond))                                      \
                {                                                          \
                    tn_aver_report(TN_FAULT_INTERNAL, __func__, __LINE__,  \
                                   TN_AVER_XSTR_(cond), "tnRET_IF");       \
                    tnOUT_ARG("rv:x%x", (retval));                         \
                    tn_aver_panic(TN_FAULT_INTERNAL);                      \
                }                                                          \
            } while (0)
    #else
        #define tnRET_IF(cond, retval)                                     \
            do {                                                           \
                if (tnUNLIKELY(cond))                                      \
                {                                                          \
                    tn_aver_report(TN_FAULT_INTERNAL, __func__, __LINE__,  \
                                   TN_AVER_XSTR_(cond), "tnRET_IF");       \
                    tnOUT_ARG("rv:x%x", (retval));                         \
                    return (retval);                                       \
                }                                                          \
            } while (0)
    #endif
#else
    #define tnRET_IF(cond, retval)                                         \
        do {                                                               \
            if (tnUNLIKELY(cond))                                          \
            {                                                              \
                tnOUT_ARG("rv:x%x", (retval));                             \
                return (retval);                                           \
            }                                                              \
        } while (0)
#endif




// ---------------------------------------------------------------------------
// tnRET_VOID_IF - tnRET_IF for functions that return void.
//
// Same contract and the same gating as tnRET_IF: active in both builds,
// condition must be side-effect free, reports func/line/condition under
// TN_RET_REPORT (and panics when TN_RET_PANIC is set). The only difference is
// that there is no status to hand back, so a taken check returns early
// instead of propagating a retval.
//
// Use this where a fallible call must still be acknowledged but the enclosing
// function has no return channel, e.g. a tnNODISCARD tn_slab_id_ref() in a
// void accessor. Prefer changing the signature to tn_result when the caller
// could actually act on the failure; this macro is for the cases where it
// cannot.
// ---------------------------------------------------------------------------

#if defined(TN_RET_REPORT)
    #if defined(TN_RET_PANIC)
        #define tnRET_VOID_IF(cond)                                        \
            do {                                                           \
                if (tnUNLIKELY(cond))                                      \
                {                                                          \
                    tn_aver_report(TN_FAULT_INTERNAL, __func__, __LINE__,  \
                                   TN_AVER_XSTR_(cond), "tnRET_VOID_IF");  \
                    tnOUT();                                               \
                    tn_aver_panic(TN_FAULT_INTERNAL);                      \
                }                                                          \
            } while (0)

    #else
        #define tnRET_VOID_IF(cond)                                        \
            do {                                                           \
                if (tnUNLIKELY(cond))                                      \
                {                                                          \
                    tn_aver_report(TN_FAULT_INTERNAL, __func__, __LINE__,  \
                                   TN_AVER_XSTR_(cond), "tnRET_VOID_IF");  \
                    tnOUT();                                               \
                    return;                                                \
                }                                                          \
            } while (0)
    #endif
#else
    #define tnRET_VOID_IF(cond)                                            \
        do {                                                               \
            if (tnUNLIKELY(cond))                                          \
            {                                                              \
                tnOUT();                                                   \
                return;                                                    \
            }                                                              \
        } while (0)
#endif



// ---------------------------------------------------------------------------
// tnEXPECT - debug-only invariant; traps in debug, absent in release.
// Argument must be side-effect free.
// ---------------------------------------------------------------------------

#if defined(TN_AVER_DEBUG)
    #define tnEXPECT(cond, ...)                                             \
        do {                                                                \
            if (tnUNLIKELY(!(cond)))                                        \
            {                                                               \
                tn_aver_report(TN_FAULT_INTERNAL, __func__, __LINE__,       \
                               TN_AVER_XSTR_(cond),                         \
                               "" __VA_ARGS__);                             \
                tn_aver_panic(TN_FAULT_INTERNAL);                           \
            }                                                               \
        } while (0)
#else
    #define tnEXPECT(cond, ...)                                            \
        do { (void)sizeof(!!(cond)); } while (0)
#endif

// ---------------------------------------------------------------------------

#include <assert.h>

#ifdef TN_UNIT_TESTING
    #include <stdarg.h>
    #include <stddef.h>
    #include <stdint.h>
    #include <setjmp.h>
    #include <cmocka.h>

    #undef assert
    #define assert(expression) mock_assert((uintptr_t)(expression), #expression, __FILE__, __LINE__)
#endif

// ---------------------------------------------------------------------------
// Predicates - static inline, side-effect free, used as the condition of
// tnRET_IF / tnEXPECT.
// ---------------------------------------------------------------------------

/// True when p is NULL.
static inline tn_bool
tn_is_null(const void *p)
{
    return p == tnNULL;
}



/// True when i is a valid index into an array of n elements.
static inline tn_bool
tn_in_bounds(
    tn_i32 i,
    tn_i32 n)
{
    return (tn_size)i < (tn_size)n;
}



/// True when lo <= v <= hi.
static inline tn_bool
tn_in_range(
    tn_i32 v,
    tn_i32 lo,
    tn_i32 hi)
{
    return (v >= lo) && (v <= hi);
}



/// True when the span [off, off+len) fits inside [0, cap).
static inline tn_bool
tn_span_fits(
    tn_size off,
    tn_size len,
    tn_size cap)
{
    return (len <= cap) && (off <= (cap - len));
}



/// Narrowing checks: true when v survives a round trip through the
/// destination type unchanged.
static inline tn_bool
tn_fits_u8(tn_i32 v)
{
    return (tn_u8)v == v;
}



static inline tn_bool
tn_fits_u16(tn_i32 v)
{
    return (tn_u16)v == v;
}



static inline tn_bool
tn_fits_u32(tn_i64 v)
{
    return (tn_u32)v == v;
}



static inline tn_bool
tn_fits_i32(tn_i64 v)
{
    return (tn_i32)v == v;
}



/// True when p is aligned to a (a must be a power of two).
static inline tn_bool
tn_is_aligned(
    const void *p,
    tn_size     a)
{
    return ((tn_uptr)p & (a - 1u)) == 0u;
}



#if defined(__cplusplus)
    #define tnSTATIC_ASSERT(cond, msg) static_assert(cond, msg)
#else
    #define tnSTATIC_ASSERT(cond, msg) _Static_assert(cond, msg)
#endif

// tnNODISCARD / tnIGNORE_RESULT live in tn_result.h, next to the tn_result enum
// they guard, so headers that only pull in tn_types.h can use them too.
#include "tn_result.h"


#define tnUNUSED(x) ((void)(x))

#ifdef __cplusplus
}
#endif

#endif // __TN_AVER_H__
