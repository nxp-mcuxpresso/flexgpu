//
// Copyright 2025-2026 NXP
//
// SPDX-License-Identifier: MIT
//

#ifndef __TN_STUB_H__
#define __TN_STUB_H__

#include "tn_aver.h"
#include "tn_trace.h"
#include "tn_log.h"

#define tnSTUB() tnABORT(TN_FAULT_UNSUPPORTED, "STUB-FUNCTION: Implement me!")

#define tnINCOMPLETE() tnLOG_INFO("INCOMPLETE")

#endif // __TN_STUB_H__
