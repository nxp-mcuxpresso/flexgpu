//
// Copyright 2025-2026 NXP
//
// SPDX-License-Identifier: MIT
//

#ifndef __TN_LOG_H__
#define __TN_LOG_H__

/// @file tn_log.h
/// Compile-time leveled logging.
///
/// Logging conveys intentional, classifiable diagnostics (events/state)
/// filtered by severity. This is distinct from tracing (tn_trace.h, gated by
/// TN_TRACE, which also covers the tnIN/tnOUT call-flow macros). Logging is
/// layered on top of the trace sink and does not modify it.
///
/// A message emitted at level L is compiled in only when L <= TN_LOG_LEVEL.
/// Disabled macros expand to a no-op that still compile-checks their
/// arguments, so there are no unused-variable warnings.
///
/// tnLOG_ERR   - error    [E], present when TN_LOG_LEVEL >= 1
/// tnLOG_WARN  - warning  [W], present when TN_LOG_LEVEL >= 2
/// tnLOG_INFO  - info     [I], present when TN_LOG_LEVEL >= 3
///
/// Each macro below is spelled out twice and selected on TN_HAS_VA_OPT
/// (tn_trace.h), because __VA_OPT__ is C23 / C++20 only and IAR rejects it in
/// the toolchain's default C mode. The conditional comma cannot be factored
/// into a shared helper - see the comment next to TN_HAS_VA_OPT for why.

#include "tn_config.h"
#include "tn_trace.h"

#ifdef __cplusplus
extern "C" {
#endif

// Severity ladder - numeric and monotonic so it can be used in #if arithmetic.
#define TN_LOG_NONE  0
#define TN_LOG_ERR   1
#define TN_LOG_WARN  2
#define TN_LOG_INFO  3

// Fallback threshold in case tn_config.h did not define one.
#ifndef TN_LOG_LEVEL
    #define TN_LOG_LEVEL TN_LOG_ERR
#endif

// ---------------------------------------------------------------------------
// tnLOG_ERR - error severity.
// ---------------------------------------------------------------------------

#if TN_LOG_LEVEL >= TN_LOG_ERR
    #if TN_HAS_VA_OPT
        #define tnLOG_ERR(fmt, ...)                                           \
            tn_trace_funln(__func__, __LINE__,                                \
                           "[E] " fmt "\n" __VA_OPT__( , ) __VA_ARGS__)
    #else
        #define tnLOG_ERR(fmt, ...)                                           \
            tn_trace_funln(__func__, __LINE__,                                \
                           "[E] " fmt "\n", ## __VA_ARGS__)
    #endif
#else
// The disabled form keeps the whole call inside an if (0) rather than just
// touching fmt: that way every argument is still mentioned, so a variable
// used only by this log does not become unused, and the format string is
// still type-checked against its arguments. The branch is folded away.
    #if TN_HAS_VA_OPT
        #define tnLOG_ERR(fmt, ...)                                              \
            do {                                                                 \
                if (0)                                                           \
                {                                                                \
                    tn_trace_funln(__func__, __LINE__,                           \
                                   "[E] " fmt "\n" __VA_OPT__( , ) __VA_ARGS__); \
                }                                                                \
            } while (0)
    #else
        #define tnLOG_ERR(fmt, ...)                                           \
            do {                                                              \
                if (0)                                                        \
                {                                                             \
                    tn_trace_funln(__func__, __LINE__,                        \
                                   "[E] " fmt "\n", ## __VA_ARGS__);          \
                }                                                             \
            } while (0)
    #endif
#endif

// ---------------------------------------------------------------------------
// tnLOG_WARN - warning severity.
// ---------------------------------------------------------------------------

#if TN_LOG_LEVEL >= TN_LOG_WARN
    #if TN_HAS_VA_OPT
        #define tnLOG_WARN(fmt, ...)                                          \
            tn_trace_funln(__func__, __LINE__,                                \
                           "[W] " fmt "\n" __VA_OPT__( , ) __VA_ARGS__)
    #else
        #define tnLOG_WARN(fmt, ...)                                          \
            tn_trace_funln(__func__, __LINE__,                                \
                           "[W] " fmt "\n", ## __VA_ARGS__)
    #endif
#else
// See tnLOG_ERR above for why the disabled form keeps the full call.
    #if TN_HAS_VA_OPT
        #define tnLOG_WARN(fmt, ...)                                             \
            do {                                                                 \
                if (0)                                                           \
                {                                                                \
                    tn_trace_funln(__func__, __LINE__,                           \
                                   "[W] " fmt "\n" __VA_OPT__( , ) __VA_ARGS__); \
                }                                                                \
            } while (0)
    #else
        #define tnLOG_WARN(fmt, ...)                                          \
            do {                                                              \
                if (0)                                                        \
                {                                                             \
                    tn_trace_funln(__func__, __LINE__,                        \
                                   "[W] " fmt "\n", ## __VA_ARGS__);          \
                }                                                             \
            } while (0)
    #endif
#endif

// ---------------------------------------------------------------------------
// tnLOG_INFO - informational severity.
// ---------------------------------------------------------------------------

#if TN_LOG_LEVEL >= TN_LOG_INFO
    #if TN_HAS_VA_OPT
        #define tnLOG_INFO(fmt, ...)                                          \
            tn_trace_funln(__func__, __LINE__,                                \
                           "[I] " fmt "\n" __VA_OPT__( , ) __VA_ARGS__)
    #else
        #define tnLOG_INFO(fmt, ...)                                          \
            tn_trace_funln(__func__, __LINE__,                                \
                           "[I] " fmt "\n", ## __VA_ARGS__)
    #endif
#else
// See tnLOG_ERR above for why the disabled form keeps the full call.
    #if TN_HAS_VA_OPT
        #define tnLOG_INFO(fmt, ...)                                             \
            do {                                                                 \
                if (0)                                                           \
                {                                                                \
                    tn_trace_funln(__func__, __LINE__,                           \
                                   "[I] " fmt "\n" __VA_OPT__( , ) __VA_ARGS__); \
                }                                                                \
            } while (0)
    #else
        #define tnLOG_INFO(fmt, ...)                                          \
            do {                                                              \
                if (0)                                                        \
                {                                                             \
                    tn_trace_funln(__func__, __LINE__,                        \
                                   "[I] " fmt "\n", ## __VA_ARGS__);          \
                }                                                             \
            } while (0)
    #endif
#endif

#ifdef __cplusplus
}
#endif

#endif // __TN_LOG_H__
