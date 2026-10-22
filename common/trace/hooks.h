//
// Copyright 2025-2026 NXP
//
// SPDX-License-Identifier: MIT
//


#ifndef __HOOK_H__
#define __HOOK_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define HOOK_PORT_MEM      1
#define HOOK_PORT_LOCK     2
#define HOOK_PORT_SIGNAL   3
#define HOOK_PORT_VIDMEM   4
#define HOOK_PORT_HOOK     5
#define HOOK_PORT_STRTAB   6


#define HOOK_TRACE_TASK_CREATE                  0x01
#define HOOK_TRACE_TASK_DELETE                  0x02
#define HOOK_TRACE_TASK_SWITCHED_OUT            0x03
#define HOOK_TRACE_TASK_SWITCHED_IN             0x04
#define HOOK_TRACE_TASK_SUSPEND                 0x05
#define HOOK_TRACE_TASK_RESUME                  0x06
#define HOOK_TRACE_MOVED_TASK_TO_READY_STATE    0x07
#define HOOK_TRACE_BLOCKING_ON_QUEUE_RECEIVE    0x08
#define HOOK_TRACE_BLOCKING_ON_QUEUE_SEND       0x09

typedef struct hook_signal_args
{
    int   signaling;
    int   manualReset;
} hook_signal_args_t;

// must be externally provided.
void hook_send_u32v(
    int             port,
    const uint32_t *v,
    int             vsz);

#if HOOK_MEM

void hook_mem_new(
    void    *addr,
    unsigned size);

void hook_mem_del(void *addr);

#else
    #define hook_mem_new(...)
    #define hook_mem_del(...)
#endif

#if HOOK_LOCK

void hook_lock_new(void *addr);

void hook_lock_del(void *addr);

void hook_lock_acquire(void *addr);

void hook_lock_release(void *addr);

#else
    #define hook_lock_new(...)
    #define hook_lock_del(...)
    #define hook_lock_acquire(...)
    #define hook_lock_release(...)
#endif

#if HOOK_SIGNAL

void hook_signal_new(void *addr);

void hook_signal_del(void *addr);

void hook_signal_release(void *addr, hook_signal_args_t);
void hook_signal_acquire(void *addr, hook_signal_args_t);
#else
    #define hook_signal_new(...)
    #define hook_signal_del(...)
    #define hook_signal_bcast(...)
    #define hook_signal_release(...)
    #define hook_signal_acquire(...)
#endif

#if HOOK_VIDMEM

void hook_vidmem_new(void *addr);

void hook_vidmem_inc(void *addr);

void hook_vidmem_dec(void *addr);

void hook_vidmem_lock(void *addr);

void hook_vidmem_unlock(void *addr);

#else
    #define hook_vidmem_new(...)
    #define hook_vidmem_inc(...)
    #define hook_vidmem_dec(...)
    #define hook_vidmem_lock(...)
    #define hook_vidmem_unlock(...)
#endif


#if HOOK_HOOK

void hook_hook(
    void    *addr,
    unsigned tag);

#else
    #define hook_trace_hook(addr, tag)
#endif

#if HOOK_STRTAB

void hook_strtab(
    void    *addr,
    unsigned tag);

#else
    #define hook_strtab(addr, tag)
#endif

#ifdef __cplusplus
}
#endif

#endif /* __HOOK_H__ */
