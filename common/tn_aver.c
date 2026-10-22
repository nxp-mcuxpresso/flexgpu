//
// Copyright 2025-2026 NXP
//
// SPDX-License-Identifier: MIT
//

#include "tn_aver.h"
#include "tn_trace.h"
#include "tn_io.h"

#include <stdarg.h>
#include <stdlib.h>

#define TN_AVER_REPORT_BUF_SIZE 160

void
tn_aver_fault(tn_fault code)
{
    tn_trace_fmt("tn fault %d\n", (int)code);
}



// Mirrors the declaration guard in tn_aver.h: tnABORT / tnEXPECT pull this in
// under TN_AVER_DEBUG, tnRET_IF / tnRET_VOID_IF under TN_RET_REPORT.
#if defined(TN_AVER_DEBUG) || defined(TN_RET_REPORT)
void
tn_aver_report(
    tn_fault    code,
    const char *func,
    int         line,
    const char *cond,
    const char *fmt,
    ...)
{
    char buf[TN_AVER_REPORT_BUF_SIZE];
    int rv;

    rv = tn_snprintf(buf, TN_AVER_REPORT_BUF_SIZE - 1,
                     "aver fault %d: %s: ", (int)code, cond);
    if (rv < 0)
    {
        rv = 0;
    }
    if ((tn_size)rv >= TN_AVER_REPORT_BUF_SIZE - 1)
    {
        rv = TN_AVER_REPORT_BUF_SIZE - 2;
    }

    if (fmt != tnNULL && fmt[0] != '\0')
    {
        va_list ap;

        va_start(ap, fmt);
        (void)tn_vsnprintf(buf + rv, TN_AVER_REPORT_BUF_SIZE - 1 - (tn_size)rv,
                           fmt, ap);
        va_end(ap);
    }
    buf[TN_AVER_REPORT_BUF_SIZE - 1] = '\0';

    tn_trace_funln(func, line, "%s\n", buf);
}



#endif



// Default panic policy: abort().
//
// The declaration in tn_aver.h is TN_NORETURN, and callers rely on it: tnABORT
// sits in the default: arm of value-returning functions (see tn_fmt.c,
// path/tn_datatype.c) with no fallback return, so the compiler emits no return
// path there. Any implementation of this function must therefore actually not
// return. abort() is itself noreturn, which keeps that contract.
//
// There is deliberately no compile-time switch to turn the trap into a no-op.
// A body that just falls off the end would still carry TN_NORETURN, which is
// undefined behaviour: the compiler drops the return machinery in both callee
// and caller, so control runs off into whatever code follows in .text instead
// of stopping. A build that needs a different policy overrides this symbol
// instead - see the weak attribute below.
//
// weak, not static: a build can supply its own strong tn_aver_panic() to
// replace this policy. src/unittest/tn_aver_cmocka.c does exactly that, so a
// failing tnEXPECT / tnABORT / tnRET_IF routes into cmocka's mock_assert() and
// can be caught by expect_assert_failure() rather than killing the test
// process. Any such override must also never return.
TN_NORETURN void
__attribute__((weak))
tn_aver_panic(tn_fault code)
{
    tnUNUSED(code);
    abort();
}
