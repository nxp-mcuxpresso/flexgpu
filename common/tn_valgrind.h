//
// Copyright 2025 NXP
//
// SPDX-License-Identifier: MIT
//

#ifndef   __TN_VALGRIND_H__
#define   __TN_VALGRIND_H__


#ifdef TN_VALGRIND
    #include <valgrind/valgrind.h>
    #include <valgrind/memcheck.h>
    #define VG(x) x
#else
    #define VG(x)
#endif

#define TN_VALGRIND_MALLOCLIKE_BLOCK(p, s) VG(VALGRIND_MALLOCLIKE_BLOCK ((p), (s), 0, 0))
#define TN_VALGRIND_RESIZE_BLOCK(p, o, n)  VG(VALGRIND_RESIZEINPLACE_BLOCK ((p), (o), (n), 0))
#define TN_VALGRIND_FREELIKE_BLOCK(p)      VG(VALGRIND_FREELIKE_BLOCK ((p), 0))


/// Memory profile:
///   valgrind --tool=cachegrind ./program
///   cg_annotate --show=Ir,Dr,Dw cachegrind.out.<PID>
///
/// Memory tracking:
///   valgrind --tool=massif --stacks=yes --log-file=valgrind.log ./program
///   ms_print massif.out.<PID>
///   massif-visualizer massif.out.<PID>

#endif // __TN_VALGRIND_H__
