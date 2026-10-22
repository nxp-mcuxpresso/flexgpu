//
// Copyright 2025-2026 NXP
//
// SPDX-License-Identifier: MIT
//

#include "hooks.h"
#include "backtrace.h"

#include <stddef.h>
#include <stdio.h>

#define HDR(nb_of_uint32, a, b) (a << 24) | (b << 16) | (nb_of_uint32 & 0xFF)

#if HOOK_MEM
void
hook_mem_new(
    void    *maddr,
    unsigned msize)
{
    uint32_t addresses[20] = {HDR(20, 'm', 'n'), (uint32_t)maddr, msize};
    btexidx_unwind(NULL, &addresses[3], 17);
    hook_send_u32v(HOOK_PORT_MEM, addresses, 20);
}



void
hook_mem_del(void *maddr)
{
    uint32_t addresses[19] = {HDR(19, 'm', 'd'), (uint32_t)maddr};
    btexidx_unwind(NULL, &addresses[2], 17);
    hook_send_u32v(HOOK_PORT_MEM, addresses, 19);
}



#endif


#if HOOK_LOCK
void
hook_lock_new(void *maddr)
{
    uint32_t addresses[12] = {HDR(12, 'l', 'n'), (uint32_t)maddr};
    btexidx_unwind(NULL, &addresses[2], 10);
    hook_send_u32v(HOOK_PORT_LOCK, addresses, 12);
}



void
hook_lock_del(void *maddr)
{
    uint32_t addresses[12] = {HDR(12, 'l', 'd'), (uint32_t)maddr};
    btexidx_unwind(NULL, &addresses[2], 10);
    hook_send_u32v(HOOK_PORT_LOCK, addresses, 12);
}



void
hook_lock_acquire(void *maddr)
{
    uint32_t addresses[7] = {HDR(7, 'l', 'a'), (uint32_t)maddr};
    btexidx_unwind(NULL, &addresses[2], 5);
    hook_send_u32v(HOOK_PORT_LOCK, addresses, 7);
}



void
hook_lock_release(void *maddr)
{
    uint32_t addresses[7] = {HDR(7, 'l', 'r'), (uint32_t)maddr};
    btexidx_unwind(NULL, &addresses[2], 5);
    hook_send_u32v(HOOK_PORT_LOCK, addresses, 7);
}



#endif

#if HOOK_SIGNAL
void
hook_signal_new(void *maddr)
{
    uint32_t addresses[12] = {HDR(12, 's', 'n'), (uint32_t)maddr};
    btexidx_unwind(NULL, &addresses[2], 10);
    hook_send_u32v(HOOK_PORT_SIGNAL, addresses, 12);
}



void
hook_signal_del(void *maddr)
{
    uint32_t addresses[12] = {HDR(12, 's', 'd'), (uint32_t)maddr};
    btexidx_unwind(NULL, &addresses[2], 10);
    hook_send_u32v(HOOK_PORT_SIGNAL, addresses, 12);
}



void
hook_signal_release(
    void              *maddr,
    hook_signal_args_t args)
{
    uint32_t addresses[21] = {HDR(21, 's', 'r'), (uint32_t)maddr, args.signaling, args.manualReset};
    btexidx_unwind(NULL, &addresses[4], 17);
    hook_send_u32v(HOOK_PORT_SIGNAL, addresses, 21);
}



void
hook_signal_acquire(
    void              *maddr,
    hook_signal_args_t args)
{
    uint32_t addresses[21] = {HDR(21, 's', 'a'), (uint32_t)maddr, args.signaling, args.manualReset};
    btexidx_unwind(NULL, &addresses[4], 17);
    hook_send_u32v(HOOK_PORT_SIGNAL, addresses, 21);
}



#endif


#if HOOK_VIDMEM
void
hook_vidmem_new(void *maddr)
{
    uint32_t addresses[12] = {HDR(12, 'v', 'n'), (uint32_t)maddr};
    btexidx_unwind(NULL, &addresses[2], 10);
    hook_send_u32v(HOOK_PORT_VIDMEM, addresses, 12);
}



void
hook_vidmem_del(void *maddr)
{
    uint32_t addresses[12] = {HDR(12, 'v', 'd'), (uint32_t)maddr};
    btexidx_unwind(NULL, &addresses[2], 10);
    hook_send_u32v(HOOK_PORT_VIDMEM, addresses, 12);
}



void
hook_vidmem_inc(void *maddr)
{
    uint32_t addresses[7] = {HDR(7, 'v', 'i'), (uint32_t)maddr};
    btexidx_unwind(NULL, &addresses[2], 5);
    hook_send_u32v(HOOK_PORT_VIDMEM, addresses, 7);
}



void
hook_vidmem_dec(void *maddr)
{
    uint32_t addresses[7] = {HDR(7, 'v', 'd'), (uint32_t)maddr};
    btexidx_unwind(NULL, &addresses[2], 5);
    hook_send_u32v(HOOK_PORT_VIDMEM, addresses, 7);
}



void
hook_vidmem_lock(void *maddr)
{
    uint32_t addresses[7] = {HDR(7, 'v', 'l'), (uint32_t)maddr};
    btexidx_unwind(NULL, &addresses[2], 5);
    hook_send_u32v(HOOK_PORT_VIDMEM, addresses, 7);
}



void
hook_vidmem_unlock(void *maddr)
{
    uint32_t addresses[7] = {HDR(7, 'v', 'u'), (uint32_t)maddr};
    btexidx_unwind(NULL, &addresses[2], 5);
    hook_send_u32v(HOOK_PORT_VIDMEM, addresses, 7);
}



#endif

#if HOOK_HOOK
void
hook_trace_hook(
    void    *addr,
    unsigned tag)
{
    uint32_t data = (( tag & 0x0F) << 28) | (((uint32_t)addr) & 0x0FFFFFFF);
    hook_send_u32(HOOK_PORT_HOOK, data);
}



#endif

#if HOOK_STRTAB
void
hook_trace_strtab(
    void    *addr,
    unsigned tag)
{
    uint32_t data[2] = {(uint32_t)addr, (uint32_t)tag};
    hook_send_u32v(HOOK_PORT_STRTAB, data, 2);
}



#endif

#if 0
void
hook_trace_ring(
    void    *addr,
    unsigned tag)
{
    extern uint32_t __TRACE_RING;
    static uint32_t *trace_ring = (uint32_t*)&__TRACE_RING;
    typedef _Atomic uint8_t atomic_uint8_t;
    #define HOOK_MAX  (0xFF + 1) //0x8000 //0x7Fu
    static atomic_uint8_t idx_ma = 0;
    uint32_t current_idx = atomic_fetch_add(&idx_ma, 1);
    current_idx %= HOOK_MAX;
    uint32_t cv = (~trace_ring[current_idx]) & 0x80000000;
    trace_ring[current_idx] = cv | ((tag & 0xFF) << 20) | ((uint32_t)addr) & 0x000FFFFF;
}



#endif

void
hook_send_u32v(
    int             port,
    const uint32_t *v,
    int             vsz)
{
    printf("\n");
    for (int i = 0; i < vsz; ++i)
    {
        printf("0x%lx,", v[i]);
    }
    printf("\n");
}
