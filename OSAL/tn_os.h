//
// Copyright 2025-2026 NXP
//
// SPDX-License-Identifier: MIT
//

#pragma once

#include "tn_types.h"
#include "tn_os_pltf.h"


typedef enum
{
    TN_THREAD_PRIO_LOWEST,
    TN_THREAD_PRIO_LOW,
    TN_THREAD_PRIO_MID,
    TN_THREAD_PRIO_HIGH,
    TN_THREAD_PRIO_HIGHEST,
} tn_thread_prio_t;

/// Block the calling thread for at least the given number of milliseconds.
/// \param msec time to sleep, in milliseconds
void tn_os_msleep(tn_u32 msec);

/// Start the OS scheduler, for ports where the application must start it
/// explicitly. Does nothing on OSes that are already running when the driver
/// is entered.
void tn_os_sched(void);

/// Create and start a thread that runs the given callback.
/// \param prio scheduling priority, mapped onto the priorities of the host OS
/// \param callback entry point of the new thread
/// \param stack_size stack size for the new thread, in bytes
/// \param user_data value passed unmodified to the callback
/// \return the new thread, or NULL on failure
tn_thread_t *tn_os_thread_init(
    tn_thread_prio_t prio,
    int           ( *callback )(void*),
    tn_size          stack_size,
    void            *user_data);

/// Wait for a thread created with tn_os_thread_init() to finish and release its
/// resources.
/// \param thread the thread to delete
void tn_os_thread_delete(tn_thread_t *thread);

/// Get an identifier of the calling thread, unique among the live threads.
/// \return the identifier of the current thread
tn_u32 tn_os_current_thread_id(void);

/// Create a synchronization object that lets a thread wait for signals raised
/// by another thread or by an interrupt handler.
/// \return the new synchronization object, or NULL on failure
tn_thread_sync_t *tn_os_thread_sync_init(void);

/// Wait for a signal posted with tn_os_thread_sync_signal() or
/// tn_os_thread_sync_signal_from_isr().
/// \param sync the synchronization object to wait on
/// \param msec_to_wait time to wait, in milliseconds, 0 to wait indefinitely
/// \return tnTRUE if the wait timed out, tnFALSE if a signal was received
tn_bool tn_os_thread_sync_wait_timeout(
    tn_thread_sync_t *sync,
    tn_u32            msec_to_wait // set to 0 to wait indefinitely
);

/// Wait for one specific event posted with
/// tn_os_thread_sync_signal_event_from_isr().
/// \param sync the synchronization object to wait on
/// \param event index of the event to wait for, below TN_OS_EVENT_COUNT
/// \param msec_to_wait time to wait, in milliseconds, 0 to wait indefinitely
/// \return tnTRUE if the event was received, tnFALSE if the wait timed out
tn_bool tn_os_thread_sync_wait_event(
    tn_thread_sync_t *sync,
    tn_u8             event,
    tn_u32            msec_to_wait // set to 0 to wait indefinitely
);

/// Signal a synchronization object from thread context, waking up a thread
/// blocked in tn_os_thread_sync_wait_timeout().
/// \param sync the synchronization object to signal
void tn_os_thread_sync_signal(tn_thread_sync_t *sync);

/// Interrupt safe version of tn_os_thread_sync_signal(), to be called from an
/// interrupt handler.
/// \param sync the synchronization object to signal
void tn_os_thread_sync_signal_from_isr(tn_thread_sync_t *sync);

/// Signal one specific event of a synchronization object from an interrupt
/// handler, waking up a thread blocked in tn_os_thread_sync_wait_event().
/// \param sync the synchronization object to signal
/// \param event index of the event to signal, below TN_OS_EVENT_COUNT
void tn_os_thread_sync_signal_event_from_isr(
    tn_thread_sync_t *sync,
    tn_u8             event);

/// Release a synchronization object created with tn_os_thread_sync_init().
/// No thread may be waiting on it at this point.
/// \param sync the synchronization object to delete
void tn_os_thread_sync_delete(tn_thread_sync_t *sync);

/// Initialize a mutex that serializes threads only, it must not be used from
/// interrupt context.
/// \param mutex the mutex to initialize
void tn_os_mutex_init(tn_mutex_t *mutex);

/// Acquire a mutex, blocking until it becomes available.
/// \param mutex the mutex to acquire
void tn_os_mutex_acquire(tn_mutex_t *mutex);

/// Release a mutex taken with tn_os_mutex_acquire().
/// \param mutex the mutex to release
void tn_os_mutex_release(tn_mutex_t *mutex);

/// Release the resources of a mutex initialized with tn_os_mutex_init().
/// \param mutex the mutex to delete
void tn_os_mutex_delete(tn_mutex_t *mutex);

/// Acquire a lock that also excludes the interrupt handler, for state shared
/// between thread and ISR context. tn_os_lock_t objects are zero-initialized.
/// \param lock the lock to acquire
void tn_os_lock_acquire(tn_os_lock_t *lock);

/// Release a lock taken with tn_os_lock_acquire().
/// \param lock the lock to release
void tn_os_lock_release(tn_os_lock_t *lock);
