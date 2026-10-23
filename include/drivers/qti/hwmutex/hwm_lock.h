/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Public API for the ticket-lock and LPM-sync lock built on top of the
 * TF-A hwmutex driver (hwm_lock.c).
 */

#ifndef HWM_LOCK_H
#define HWM_LOCK_H

#include <stdint.h>

/*
 * hwm_lock - acquire the ticket lock.
 *
 * Uses the HW mutex to atomically read-and-increment next_ticket, then spins
 * until current_ticket reaches our ticket number.  Issues DSB ISH after the
 * spin to order subsequent critical-section memory accesses.
 *
 * @lock: pointer to a ticket_lock object (opaque to callers)
 */
void hwm_lock(void *lock);

/*
 * hwm_unlock - release the ticket lock.
 *
 * Issues DSB ISH to complete all critical-section memory accesses, then
 * advances current_ticket to wake the next waiter.
 *
 * @lock: pointer to a ticket_lock object (opaque to callers)
 */
void hwm_unlock(void *lock);

/*
 * acquire_lpm_lock - acquire the per-core LPM synchronisation lock.
 *
 * Spins under the HW mutex until the core's LPM status slot is free
 * (CORE_NOT_IN_LPM), then marks it as CORE_IN_LPM and returns.
 * Backs off with a short delay between retries to reduce bus contention.
 *
 * @core_id: zero-based index of the CPU core
 */
void acquire_lpm_lock(uint8_t core_id);

/*
 * release_lpm_lock - release the per-core LPM synchronisation lock.
 *
 * Clears the core's LPM status slot back to CORE_NOT_IN_LPM under the
 * HW mutex so that other cores see a consistent update.
 *
 * @core_id: zero-based index of the CPU core
 */
void release_lpm_lock(uint8_t core_id);

#endif /* HWM_LOCK_H */
