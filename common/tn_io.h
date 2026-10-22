//
// Copyright 2025 NXP
//
// SPDX-License-Identifier: MIT
//

#ifndef __TN_IO_H__
#define __TN_IO_H__

#include "tn_types.h"
#include <stdarg.h>

int tn_printf(
    const char *format,
    ...);

int tn_sprintf(
    char       *buffer,
    const char *format,
    ...);

int tn_snprintf(
    char       *buffer,
    tn_size     count,
    const char *format,
    ...);

int tn_vprintf(
    const char *format,
    va_list     va);

int tn_vsnprintf(
    char       *buffer,
    tn_size     count,
    const char *format,
    va_list     va);

#endif // __TN_IO_H__
