//
// Copyright 2025-2026 NXP
//
// SPDX-License-Identifier: MIT
//

#ifndef __TN_RESULT_H__
#define __TN_RESULT_H__

typedef enum
{
    TN_NO_ERROR                       =0x0000, //VG_LITE_SUCCESS
    TN_ALREADY_INITIALIZED            =0x1000,
    TN_INVALID_ENTRY                  =0x1001,
    TN_BAD_HANDLE_ERROR               =0x1002,
    TN_PATH_CAPABILITY_ERROR          =0x1003,
    TN_UNSUPPORTED_IMAGE_FORMAT_ERROR =0x1004,
    TN_UNSUPPORTED_PATH_FORMAT_ERROR  =0x1005,
    TN_IMAGE_IN_USE_ERROR             =0x1006,
    TN_BUFFER_FULL_ERROR              =0x1007,
    TN_NOT_SUPPORTED                  =0x2000, //VG_LITE_NOT_SUPPORT
    TN_GENERIC_IO_ERROR               =0x2001, //VG_LITE_GENERIC_IO
    TN_TIMEOUT_ERROR                  =0x2002, //VG_LITE_TIMEOUT
    TN_OUT_OF_MEMORY_ERROR            =0x2003, //VG_LITE_OUT_OF_MEMORY
    TN_OUT_OF_RESOURCES_ERROR         =0x2004, //VG_LITE_OUT_OF_RESOURCES
    TN_ILLEGAL_ARGUMENT_ERROR         =0x2005, //VG_LITE_INVALID_ARGUMENT
    TN_NO_CONTEXT_ERROR               =0x2006, //VG_LITE_NO_CONTEXT
    TN_ERROR_MAX
} tn_result;

// ---------------------------------------------------------------------------
// Error-propagation enforcement.
//
// tnNODISCARD marks a fallible function so the compiler rejects callers that
// discard the status. Applied to a declaration it turns a silently swallowed
// TN_OUT_OF_MEMORY_ERROR into a -Wunused-result diagnostic. The name mirrors
// the standard [[nodiscard]] spelling of C23 / C++17.
//
// Note that gcc deliberately does NOT accept a (void) cast as an
// acknowledgement, so tnIGNORE_RESULT is provided for the rare call site where
// the status is genuinely uninteresting. It stores the value in a temporary,
// which satisfies the attribute while keeping the intent greppable.
// ---------------------------------------------------------------------------

#if !defined(tnNODISCARD)
    #if defined(__GNUC__)
        #define tnNODISCARD __attribute__(( warn_unused_result ))
    #else
        #define tnNODISCARD
    #endif
#endif

#define tnIGNORE_RESULT(expr)                                                  \
    do {                                                                       \
        __typeof__(expr) tn_ignored_ = (expr);                                 \
        (void)tn_ignored_;                                                     \
    } while (0)

#endif // __TN_RESULT_H__
