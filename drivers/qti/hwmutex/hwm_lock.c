/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Ticket-lock and LPM-sync lock implementation using the TF-A hwmutex driver.
 *
 * The hardware mutex (hwmutex) is used only to make the read-modify-write of
 * the ticket counter atomic across CPUs.  The actual mutual exclusion for the
 * critical section is provided by the ticket-lock spin loop.
 *
 * Ported from CPUCP hwm_lock.c:
 *   hw_mutex_acquire(id) / hw_mutex_release(id)
 *     → hwmutex_acquire(&handle) / hwmutex_release(&handle)
 *   __asm__ volatile ("dsb ish" ::: "memory")
 *     → dsbish()  (from <arch_helpers.h>)
 *   busywait(us)
 *     → udelay(us)  (from <drivers/delay_timer.h>)
 */

#include <stdint.h>

#include <arch_helpers.h>
#include <drivers/delay_timer.h>
#include <drivers/qti/hwmutex/hwmutex.h>

/*
 * Chipset-specific CPUCP hardware register addresses.
 * Provides CPU_LPM_SYNC_DATA_BASE (shared SRAM base for per-core LPM status).
 * The include path -Iinclude/drivers/qti/cpucp/<chipset> is set by platform.mk.
 */
#include <cpucp_hwio.h>
#include <cpucp_shared.h>

#define CPUCP_LPMSYNC_DELAY	100U	/* microseconds */

volatile uint32_t *lpm_status = (uint32_t *)CPU_LPM_SYNC_DATA_BASE;

/*
 * Static HW mutex handles.
 *
 * Statically initialised to avoid a separate init call: hwmutex_init() only
 * assigns mutex_id, so direct struct initialisation is equivalent.
 */
static hwmutex_t hwm_atomics = { .mutex_id = HW_MUTEX_S_ATOMICS };
static hwmutex_t hwm_lpm     = { .mutex_id = HW_MUTEX_S_LPM };

struct ticket_lock {
	volatile unsigned int next_ticket;
	volatile unsigned int current_ticket;
};

/*
 * hwm_lock - acquire the ticket lock.
 *
 * Uses the HW mutex to atomically read-and-increment next_ticket, then spins
 * until current_ticket reaches our ticket number.  Issues DSB ISH after the
 * spin to order subsequent critical-section memory accesses.
 */
void hwm_lock(void *lock)
{
	struct ticket_lock *l1 = (struct ticket_lock *)lock;
	unsigned int my_ticket;

	/*
	 * Acquire HW mutex to make the next_ticket read-modify-write atomic.
	 * hwmutex_acquire() issues DSB ISH after the hardware grants ownership.
	 */
	hwmutex_acquire(&hwm_atomics);

	my_ticket = l1->next_ticket;
	l1->next_ticket++;

	/*
	 * Release HW mutex.
	 * hwmutex_release() issues DSB ISH before writing 0 to the register.
	 */
	hwmutex_release(&hwm_atomics);

	/* Spin until our ticket becomes the current ticket */
	while (l1->current_ticket != my_ticket)
		;

	/*
	 * Barrier: ensure all critical-section memory accesses happen
	 * strictly after the ticket lock is acquired.
	 */
	dsbish();
}

/*
 * hwm_unlock - release the ticket lock.
 *
 * Issues DSB ISH to complete all critical-section memory accesses, then
 * advances current_ticket to wake the next waiter.
 */
void hwm_unlock(void *lock)
{
	struct ticket_lock *l1 = (struct ticket_lock *)lock;

	/*
	 * Barrier: ensure all critical-section memory accesses complete
	 * before releasing the ticket lock.
	 */
	dsbish();

	/* Give the next waiter a turn */
	l1->current_ticket++;
}

/*
 * acquire_lpm_lock - acquire the per-core LPM synchronisation lock.
 *
 * Spins under the HW mutex until the core's LPM status slot is free
 * (CORE_NOT_IN_LPM), then marks it as CORE_IN_LPM and returns.
 * Backs off with a short delay between retries to reduce bus contention.
 */
void acquire_lpm_lock(uint8_t core_id)
{
	do {
		hwmutex_acquire(&hwm_lpm);

		if (lpm_status[core_id] == CORE_NOT_IN_LPM) {
			lpm_status[core_id] = CORE_IN_LPM;
			hwmutex_release(&hwm_lpm);
			break;
		}

		hwmutex_release(&hwm_lpm);

		udelay(CPUCP_LPMSYNC_DELAY);

	} while (1);
}

/*
 * release_lpm_lock - release the per-core LPM synchronisation lock.
 *
 * Clears the core's LPM status slot back to CORE_NOT_IN_LPM under the
 * HW mutex so that other cores see a consistent update.
 */
void release_lpm_lock(uint8_t core_id)
{
	hwmutex_acquire(&hwm_lpm);
	lpm_status[core_id] = CORE_NOT_IN_LPM;
	hwmutex_release(&hwm_lpm);
}
