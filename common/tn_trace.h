//
// Copyright 2025-2026 NXP
//
// SPDX-License-Identifier: MIT
//

#ifndef __TN_TRACE_H__
#define __TN_TRACE_H__

/// @file tn_trace.h
/// Trace transport layer.
///
/// Provides the low-level sink primitives (tn_trace_funln, tn_trace_fmt) that
/// format a message and push it to tn_printf, plus two families of trace
/// macros, both gated as a whole by TN_TRACE (no severity concept):
///   - tnTRACE / tnTRACELN / tnBINARY_TRACE: free-form detail trace.
///   - tnIN / tnIN_ARG / tnOUT / tnOUT_ARG: function enter/exit call-flow
///     trace (prints "v {" on entry and "^}" on exit).
///
/// This is tracing, not logging. For severity-filtered logging
/// (ERROR/WARN/INFO gated by TN_LOG_LEVEL) use tn_log.h, which is built on
/// the sink primitives declared here.
///
/// The call-flow macros (tnIN/tnOUT and their _ARG variants) are additionally
/// filtered by call-nesting depth via the compile-time constant TN_TRACE_DEPTH:
///   - TN_TRACE_DEPTH < 0 : unlimited depth (all frames traced).
///   - TN_TRACE_DEPTH == 0 : no call-flow frames traced.
///   - TN_TRACE_DEPTH == N : only frames at nesting levels 0..N-1 are traced.
/// The threshold is set at build time (CMake cache variable TN_TRACE_DEPTH,
/// mirroring TN_LOG_LEVEL); the nesting level itself is counted at run time.

#include "tn_config.h"

#ifdef __cplusplus
extern "C" {
#endif

// ---------------------------------------------------------------------------
// TN_HAS_VA_OPT - single feature switch for the "comma only if there are
// variadic arguments" idiom. __VA_OPT__ is C23 / C++20 only and IAR rejects
// it in the toolchain's default C mode, so every variadic macro in this tree
// spells out both forms and selects on this switch:
//
//     #if TN_HAS_VA_OPT
//         ... fmt __VA_OPT__( , ) __VA_ARGS__ ...
//     #else
//         ... fmt, ## __VA_ARGS__ ...
//     #endif
//
// The conditional comma has to stay inside the macro that actually owns the
// variadic arguments. It cannot be factored out into a shared helper: with no
// variadic arguments the caller would forward an empty-but-present argument
// into that helper, GNU comma swallowing would not trigger, and the expansion
// would keep a trailing comma - "sink(fmt,)" - which does not compile. The
// duplication below is therefore deliberate, not an oversight.
//
// IAR is pinned to the fallback explicitly rather than inferred from the
// language version, so the GNU form is used there whatever mode the SDK
// example builds with.
// ---------------------------------------------------------------------------

#if defined(__ICCARM__)
    #define TN_HAS_VA_OPT 0
#elif defined(__cplusplus)
    #define TN_HAS_VA_OPT (__cplusplus >= 202002L)
#elif defined(__STDC_VERSION__)
    #define TN_HAS_VA_OPT (__STDC_VERSION__ >= 202311L)
#else
    #define TN_HAS_VA_OPT 0
#endif

// Fallback threshold in case tn_config.h did not define one (unlimited).
#ifndef TN_TRACE_DEPTH
    #define TN_TRACE_DEPTH (-1)
#endif

void tn_trace_funln(
    const char *function_name,
    int         line_no,
    const char *fmt,
    ...);

void tn_trace_fmt(
    const char *fmt,
    ...);

// Call-flow enter/exit sinks. tn_trace_in emits the "enter" marker and
// increments the nesting counter; tn_trace_out decrements it and emits the
// "exit" marker. Both suppress output for frames deeper than TN_TRACE_DEPTH,
// while always keeping the counter balanced so nesting stays correct.
void tn_trace_in(
    const char *function_name,
    int         line_no,
    const char *fmt,
    ...);

void tn_trace_out(
    const char *function_name,
    int         line_no,
    const char *fmt,
    ...);

// Runtime-gated API-call dump sink. Unlike every other primitive in this
// file, this one is NOT gated by the TN_TRACE compile-time switch: it backs
// the vg_lite DUMP_API_CALL()/vg_lite_set_dump_api() runtime toggle, which must
// work the same whether or not the build was configured with TN_TRACE.
// Callers are expected to test their own  runtime flag before calling this;
// the function itself always emits.
void tn_trace_dump_api(
    const char *function_name,
    const char *fmt,
    ...);


#if !defined(TN_TRACE)

    #define tnTRACE(fmt, ...)
    #define tnTRACELN(fmt, ...)
    #define tnBINARY_TRACE(fmt, ...)

    #define tnIN()
    #define tnIN_ARG(fmt, ...)
    #define tnOUT()
    #define tnOUT_ARG(fmt, ...)

#else // TN_TRACE

    #ifndef tnTRACE
        #if TN_HAS_VA_OPT
            #define tnTRACE(fmt, ...)                                      \
                tn_trace_funln(__func__, __LINE__,                         \
                               fmt __VA_OPT__( , ) __VA_ARGS__)
        #else
            #define tnTRACE(fmt, ...)                                      \
                tn_trace_funln(__func__, __LINE__,                         \
                               fmt, ## __VA_ARGS__)
        #endif
    #endif

// Same sink as tnTRACE, with a newline appended to the format string.
// The sink is called directly rather than through tnTRACE so that the
// TN_HAS_VA_OPT selection stays in the macro that owns the variadic
// arguments, as explained in the TN_HAS_VA_OPT note above. A platform
// that overrides tnTRACE therefore has to override tnTRACELN as well.
    #ifndef tnTRACELN
        #if TN_HAS_VA_OPT
            #define tnTRACELN(fmt, ...)                                    \
                tn_trace_funln(__func__, __LINE__,                         \
                               fmt "\n" __VA_OPT__( , ) __VA_ARGS__)
        #else
            #define tnTRACELN(fmt, ...)                                    \
                tn_trace_funln(__func__, __LINE__,                         \
                               fmt "\n", ## __VA_ARGS__)
        #endif
    #endif

    #ifndef tnBINARY_TRACE
        #if TN_HAS_VA_OPT
            #define tnBINARY_TRACE(fmt, ...)                               \
                tn_trace_funln(__func__, __LINE__,                         \
                               fmt __VA_OPT__( , ) __VA_ARGS__)
        #else
            #define tnBINARY_TRACE(fmt, ...)                               \
                tn_trace_funln(__func__, __LINE__,                         \
                               fmt, ## __VA_ARGS__)
        #endif
    #endif

    #ifndef tnIN
        #define tnIN() \
            tn_trace_in(__func__, __LINE__, "v  {\n")
    #endif

    #ifndef tnIN_ARG
        #if TN_HAS_VA_OPT
            #define tnIN_ARG(fmt, ...)                                     \
                tn_trace_in(__func__, __LINE__,                            \
                            "v  {" fmt "\n" __VA_OPT__( , ) __VA_ARGS__)
        #else
            #define tnIN_ARG(fmt, ...)                                     \
                tn_trace_in(__func__, __LINE__,                            \
                            "v  {" fmt "\n", ## __VA_ARGS__)
        #endif
    #endif

    #ifndef tnOUT
        #define tnOUT() \
            tn_trace_out(__func__, __LINE__, "  ^}\n")
    #endif

    #ifndef tnOUT_ARG
        #if TN_HAS_VA_OPT
            #define tnOUT_ARG(fmt, ...)                                    \
                tn_trace_out(__func__, __LINE__,                           \
                             "  ^} " fmt "\n" __VA_OPT__( , ) __VA_ARGS__)
        #else
            #define tnOUT_ARG(fmt, ...)                                    \
                tn_trace_out(__func__, __LINE__,                           \
                             "  ^} " fmt "\n", ## __VA_ARGS__)
        #endif
    #endif

#endif // TN_TRACE

#ifdef __cplusplus
}
#endif

#endif // __TN_TRACE_H__
