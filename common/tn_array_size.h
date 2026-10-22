//
// Copyright 2025 NXP
//
// SPDX-License-Identifier: MIT
//

#pragma once

#ifndef tnARRAY_SIZE
    #define tnARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]) )
#endif
