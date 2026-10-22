//
// Copyright 2025-2026 NXP
//
// SPDX-License-Identifier: MIT
//
#ifndef   __TN_CONFIG_H__

// Debug diagnostics in tn_aver.h are gated on TN_AVER_DEBUG. The release
// default is off: tnEXPECT is compiled out, and tnRET_IF keeps only the branch
// and the return. Uncomment TN_AVER_DEBUG to get the func/line/condition
// reports, and TN_RET_PANIC with it to make a failing check panic instead of
// returning. tn_aver.c is always linked: tnABORT and the release tnRET_IF path
// need tn_aver_fault() and tn_aver_panic() regardless of this setting.
//#define TN_AVER_DEBUG
//#define TN_RET_PANIC



#define TN_CONFIG_SHM0_POOL_SIZE (0x400000)

// Compile-time log level (see src/tn_log.h): 0 none, 1 err, 2 warn, 3 info.
// Release default is 0 (TN_LOG_NONE): tn_log.h is included after this file,
// so the TN_LOG_NONE symbol is not yet available here - use the numeric value.
#define TN_LOG_LEVEL 0



#endif // __TN_CONFIG_H__
