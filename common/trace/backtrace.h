//
// Copyright 2015 Stephen Street <stephen@redrocketcomputing.com>
// Copyright 2025-2026 NXP
//
// SPDX-License-Identifier: MPL-2.0
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//


#ifndef __BACKTRACE_H__
#define __BACKTRACE_H__

#include <stdint.h>

int btexidx_unwind(
    uint32_t functions[],
    uint32_t addresses[],
    unsigned nb_elem);

const char *btexidx_function_name(uint32_t pc);

// PC - r15 : program counter
// LR - r14 : the return address
// SP - r13 : stack pointer
// FP - r7 : frame pointer
#define getPC(regval) __asm volatile ("mov %0, r15" : "=r" (regval))
#define getLR(regval) __asm volatile ("mov %0, r14" : "=r" (regval))
#define getSP(regval) __asm volatile ("mov %0, r13" : "=r" (regval))
#define getFP(regval) __asm volatile ("mov %0, r7" : "=r" (regval))

#endif /* __BACKTRACE_H__ */
