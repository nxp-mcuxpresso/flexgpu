//
// Copyright 2025-2026 NXP
//
// SPDX-License-Identifier: MIT
//

#include "tn_config.h"
#include "tn_trace.h"
#include "tn_io.h"
#include <stdarg.h>
#include "tn_types.h"
#include "tn_aver.h"
#include "tn_compiler.h"



// Number of spaces prepended per call-flow nesting level.
#define TN_TRACE_INDENT_WIDTH 2

/** Format and emit a single trace line from an already-started va_list.
* \param[in] func_name          Calling function name.
* \param[in] line               Calling Line number.
* \param[in] indent             Call-flow nesting level; the message body is
*                               prefixed with indent*TN_TRACE_INDENT_WIDTH
*                               spaces. Use 0 for un-indented (free-form) trace.
* \param[in] fmt                Optional pointer to a descriptive text.
* \param[in] ap                 Argument list matching fmt.
*
* \note The message is formatted into a function-local static buffer to keep
*       the stack footprint small on the embedded targets. As a consequence
*       this function is neither reentrant nor async-signal-safe: concurrent
*       calls from several threads, or a call from a signal handler that
*       interrupts an ongoing call, can interleave or truncate the output.
*       Trace output is diagnostic only, so no locking is used here.
*/
static void
tn_trace_vfunln(
    const char *func_name,
    int         line,
    int         indent,
    const char *fmt,
    va_list     ap)
{
    // Shared across all callers, see the reentrancy note above.
    static char buf[256];
    int rv = tn_snprintf(buf, 255, "%30s:%-5d\t", func_name, line);

    // Prepend one indentation unit per nesting level, clamped to the buffer.
    int spaces = indent * TN_TRACE_INDENT_WIDTH;
    for (int i = 0; i < spaces && rv < 255; i++)
    {
        buf[rv++] = ' ';
    }

    rv += tn_vsnprintf(buf + rv, 255 - rv, fmt, ap);
    tnUNUSED(rv);
    buf[255] = 0;
    tn_printf("%s", buf);
}



/** Output a binary trace message.
* \param[in] func_name          Calling function name.
* \param[in] line               Calling Line number.
* \param[in] fmt                Optional pointer to a descriptive text.
* \param[in] ...                Optional arguments to the descriptive text.
*
* \note See tn_trace_vfunln for the reentrancy/thread-safety note.
*/
void
tn_trace_funln(
    const char *func_name,
    int         line,
    const char *fmt,
    ...)
{
    va_list ap;
    va_start(ap, fmt);
    // Free-form trace is not part of the call-flow nesting, so it is not indented.
    tn_trace_vfunln(func_name, line, 0, fmt, ap);
    va_end(ap);
}



// Current call-flow nesting depth, maintained by tn_trace_in/tn_trace_out.

// A frame is traced only while its level (the depth *before* entering it) is
// below the TN_TRACE_DEPTH threshold; a negative threshold means unlimited.
static TN_THREAD_LOCAL int tn_trace_depth = 0;

/** Return whether a frame entered at nesting level `level` should be traced. */
static int
tn_trace_level_enabled(int level)
{
#if TN_TRACE_DEPTH < 0
    (void)level;
    return 1;
#else
    return level < TN_TRACE_DEPTH;
#endif
}



/** Emit the call-flow "enter" marker (subject to TN_TRACE_DEPTH) and descend. */
void
tn_trace_in(
    const char *func_name,
    int         line,
    const char *fmt,
    ...)
{
    if (tn_trace_level_enabled(tn_trace_depth))
    {
        va_list ap;
        va_start(ap, fmt);
        tn_trace_vfunln(func_name, line, tn_trace_depth, fmt, ap);
        va_end(ap);
    }
    // Always descend so nesting stays balanced regardless of the threshold.
    tn_trace_depth++;

}



/** Ascend and emit the call-flow "exit" marker (subject to TN_TRACE_DEPTH). */
void
tn_trace_out(
    const char *func_name,
    int         line,
    const char *fmt,
    ...)
{
    // Ascend first so the exit marker is filtered at the same level as the
    // matching enter marker. Guard against underflow from unbalanced calls.
    if (tn_trace_depth > 0)
    {
        tn_trace_depth--;
    }

    if (tn_trace_level_enabled(tn_trace_depth))
    {
        va_list ap;
        va_start(ap, fmt);
        tn_trace_vfunln(func_name, line, tn_trace_depth, fmt, ap);
        va_end(ap);
    }
}



/** Output a binary trace message.
* \param[in] func_name          Calling function name.
* \param[in] line               Calling Line number.
* \param[in] fmt                Optional pointer to a descriptive text.
* \param[in] ...                Optional arguments to the descriptive text.
*/
void
tn_trace_fmt(
    const char *fmt,
    ...)
{
    va_list ap;
    va_start(ap, fmt);
    tn_vprintf(fmt, ap);
    va_end(ap);
}



/** Output a vg_lite API-call dump trace line.
*
* Backs the runtime-toggled vg_lite DUMP_API_CALL()/vg_lite_set_dump_api()
* mechanism (see api/vglite/vg_lite_priv.h). Unlike every other sink in this
* file, emission here is not gated by the TN_TRACE compile-time switch: the
* caller is responsible for checking its own runtime flag before calling
* this function, so the dump works identically whether or not the build was
* configured with TN_TRACE.
*
* \param[in] func_name          Name of the vg_lite API function being dumped.
* \param[in] fmt                Descriptive text with the function's arguments.
* \param[in] ...                Arguments matching fmt.
*
* \note See tn_trace_vfunln for the reentrancy/thread-safety note.
*/
void
tn_trace_dump_api(
    const char *func_name,
    const char *fmt,
    ...)
{
    va_list ap;
    va_start(ap, fmt);
    // Free-form dump trace is not part of the call-flow nesting, so it is
    // not indented, matching tn_trace_funln's convention.
    tn_trace_vfunln(func_name, 0, 0, fmt, ap);
    va_end(ap);
}
