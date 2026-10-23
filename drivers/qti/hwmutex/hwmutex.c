/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Hardware Mutex (HWMUTEX) driver for TF-A.
 *
 * Ported from CPUCP modules_v2/hwmutex/src/hwmutex.c with TF-A adaptations:
 *   - ioread32 / iowrite32  →  mmio_read_32 / mmio_write_32
 *   - cpu_get_logical_cpu_num()  →  plat_my_core_pos()
 *   - inline "dsb ish"  →  dsbish() from <arch_helpers.h>
 *   - APSS_CPUCP_S_MUTEX_BASE from platform_def.h / cpucp_hwio.h
 *
 * Only the secure HW mutex bank (region 0) is used by TF-A.  The mutex
 * used for NCC LPM vote-counter protection is HW_MUTEX_S_LPM (0x0004).
 *
 * APSS_CPUCP_S_MUTEX_BASE must be defined before including this file,
 * either in platform_def.h or include/drivers/qti/cpucp/<chipset>/cpucp_hwio.h.
 */

#include <arch_helpers.h>
#include <lib/mmio.h>
#include <platform.h>

/*
 * Chipset-specific CPUCP hardware register addresses.
 * Provides APSS_CPUCP_S_MUTEX_BASE (secure HW mutex bank base address).
 * The include path -Iinclude/drivers/qti/cpucp/<chipset> is set by platform.mk.
 */
#include <cpucp_hwio.h>

#include <drivers/qti/hwmutex/hwmutex.h>

#ifndef APSS_CPUCP_S_MUTEX_BASE
#error "APSS_CPUCP_S_MUTEX_BASE is not defined in cpucp_hwio.h. " \
       "Add it to include/drivers/qti/cpucp/<chipset>/cpucp_hwio.h."
#endif

/*
 * hwmutex_init - initialise a hardware mutex handle.
 *
 * Records the HW_MUTEX_* ID for use by acquire/release.
 */
void hwmutex_init(hwmutex_t *mutex, uint32_t mutex_id)
{
	mutex->mutex_id = mutex_id;
}

/*
 * hwmutex_acquire - acquire the hardware mutex (APPS_TZ side).
 *
 * Algorithm (from CPUCP hwmutex.c):
 *   1. Build a non-zero key:
 *        bits[31:24] = HWMUTEX_SUBSYS_APPS_TZ (0x2)
 *        bits[23:8]  = mutex_id
 *        bits[7:0]   = logical CPU number
 *   2. Write key to the mutex register.
 *   3. Read back; if the value matches the key, ownership is granted.
 *      Otherwise another subsystem holds the mutex – retry from step 2.
 *   4. Issue DSB ISH to order subsequent memory accesses after the grant.
 *
 * Note: only the secure bank (region 0, APSS_CPUCP_S_MUTEX_BASE) is used.
 */
void hwmutex_acquire(hwmutex_t *mutex)
{
	uint32_t mutex_id  = mutex->mutex_id;
	uint32_t mutex_num = (mutex_id >> HWMUTEX_NUM_SHIFT) & HWMUTEX_NUM_MASK;
	uint32_t cpu_num   = (uint32_t)plat_my_core_pos();
	uint32_t key =
		((HWMUTEX_SUBSYS_APPS_TZ & HWMUTEX_SUBSYS_MASK) << HWMUTEX_SUBSYS_SHIFT) |
		((mutex_id & HWMUTEX_ID_MASK) << HWMUTEX_ID_SHIFT) |
		((cpu_num  & HWMUTEX_CPU_MASK) << HWMUTEX_CPU_SHIFT);
	uintptr_t reg_addr = APSS_CPUCP_S_MUTEX_BASE +
			     (mutex_num * HWMUTEX_REG_SIZE);

	do {
		mmio_write_32(reg_addr, key);
	} while (key != mmio_read_32(reg_addr));

	/* Barrier: all critical-section memory ops happen after acquire */
	dsbish();
}

/*
 * hwmutex_release - release the hardware mutex (APPS_TZ side).
 *
 * Algorithm (from CPUCP hwmutex.c):
 *   1. Issue DSB ISH to complete all critical-section memory accesses.
 *   2. Write 0 to the mutex register to release ownership.
 *   3. Read back to ensure the write reaches device memory.
 */
void hwmutex_release(hwmutex_t *mutex)
{
	uint32_t mutex_id  = mutex->mutex_id;
	uint32_t mutex_num = (mutex_id >> HWMUTEX_NUM_SHIFT) & HWMUTEX_NUM_MASK;
	uintptr_t reg_addr = APSS_CPUCP_S_MUTEX_BASE +
			     (mutex_num * HWMUTEX_REG_SIZE);

	/* Barrier: all critical-section memory ops happen before release */
	dsbish();

	mmio_write_32(reg_addr, 0U);

	/* Read back to ensure write completion to device memory */
	(void)mmio_read_32(reg_addr);
}