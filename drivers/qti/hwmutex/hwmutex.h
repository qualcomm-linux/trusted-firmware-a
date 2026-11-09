/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Hardware Mutex (HWMUTEX) driver interface for TF-A.
 *
 * Implements the AP-side (APPS_TZ) view of the CPUCP hardware mutex block.
 * The hardware mutex provides mutual exclusion between TF-A and the CPUCP
 * co-processor firmware using a shared register bank.
 *
 * Protocol (ported from CPUCP hwmutex.c):
 *   Acquire: write non-zero key, spin until readback matches, then DSB ISH.
 *   Release: DSB ISH, write 0, readback for device-memory write completion.
 *
 * Key encoding (32-bit register value):
 *   bits[31:24] = SUBSYSTEM_TYPE  (APPS_TZ = 0x2 for TF-A)
 *   bits[23:8]  = MUTEX_ID        (hw_mutex_id enum value)
 *   bits[7:0]   = CPU_NUM         (logical CPU number from plat_my_core_pos())
 *
 * Register address:
 *   APSS_CPUCP_S_MUTEX_BASE + mutex_num * HWMUTEX_REG_SIZE
 *   where mutex_num = mutex_id & 0xFF  (low byte of the mutex ID)
 *
 * Reference: CPUCP modules_v2/hwmutex/src/hwmutex.c
 */

#ifndef QTI_HWMUTEX_H
#define QTI_HWMUTEX_H

#include <stdint.h>

/* --------------------------------------------------------------------------
 * Subsystem type identifiers (CPUCP hwmutex.h SUBSYSTEM_TYPE values).
 * TF-A runs as APPS_TZ.
 * -------------------------------------------------------------------------- */
#define HWMUTEX_SUBSYS_CPUCP		0x1U
#define HWMUTEX_SUBSYS_APPS_TZ		0x2U	/* TF-A subsystem type */
#define HWMUTEX_SUBSYS_APPS_PDP		0x3U
#define HWMUTEX_SUBSYS_APPS_NS_HYP	0x4U
#define HWMUTEX_SUBSYS_APPS_NS_HLOS	0x5U

/* --------------------------------------------------------------------------
 * Bit-field shifts and masks (from CPUCP hwmutex.h).
 * -------------------------------------------------------------------------- */
#define HWMUTEX_SUBSYS_SHIFT		24U
#define HWMUTEX_SUBSYS_MASK		0xFFU
#define HWMUTEX_ID_SHIFT		8U
#define HWMUTEX_ID_MASK			0xFFFFU
#define HWMUTEX_CPU_SHIFT		0U
#define HWMUTEX_CPU_MASK		0xFFU

/* Region and number fields within the mutex_id */
#define HWMUTEX_REGION_ID_SHIFT		8U
#define HWMUTEX_REGION_ID_MASK		0xFFU
#define HWMUTEX_NUM_SHIFT		0U
#define HWMUTEX_NUM_MASK		0xFFU

/* Size of one HW mutex register in bytes */
#define HWMUTEX_REG_SIZE		4U

/* --------------------------------------------------------------------------
 * HW Mutex IDs (from CPUCP hwmutex.h enum hw_mutex_id).
 *
 * ID encoding:
 *   bits[15:8] = Region ID  (0 = secure bank, 1 = non-secure bank,
 *                             2 = multidie secure, 3 = multidie non-secure)
 *   bits[7:0]  = Mutex number within the bank
 * -------------------------------------------------------------------------- */

/* Secure bank (region 0) */
#define HW_MUTEX_S_CPUCP		0x0000U
#define HW_MUTEX_S_ATOMICS		0x0001U
#define HW_MUTEX_S_TZ_SYSINI		0x0002U
#define HW_MUTEX_S_PDP			0x0003U
#define HW_MUTEX_S_LPM			0x0004U	/* NCC LPM vote-counter mutex */

/* Non-secure bank (region 1) */
#define HW_MUTEX_NS_ATOMICS		0x0100U
#define HW_MUTEX_NS_HYP			0x0101U
#define HW_MUTEX_NS_HLOS		0x0102U

/* --------------------------------------------------------------------------
 * Hardware mutex handle.
 *
 * @mutex_id  One of the HW_MUTEX_* IDs above.  Encodes both the register
 *            bank (region) and the mutex number within that bank.
 * -------------------------------------------------------------------------- */
typedef struct {
	uint32_t mutex_id;
} hwmutex_t;

/*
 * hwmutex_init - initialise a hardware mutex handle.
 *
 * Must be called once before hwmutex_acquire() / hwmutex_release().
 *
 * @mutex     Pointer to the hwmutex_t to initialise.
 * @mutex_id  One of the HW_MUTEX_* IDs above (e.g. HW_MUTEX_S_LPM).
 */
void hwmutex_init(hwmutex_t *mutex, uint32_t mutex_id);

/*
 * hwmutex_acquire - acquire the hardware mutex.
 *
 * Writes the APPS_TZ key to the HW mutex register and spins until the
 * readback matches (i.e. the hardware has granted ownership to this
 * subsystem).  Issues DSB ISH after the grant to order subsequent memory
 * accesses within the critical section.
 *
 * @mutex  Pointer to an initialised hwmutex_t.
 */
void hwmutex_acquire(hwmutex_t *mutex);

/*
 * hwmutex_release - release the hardware mutex.
 *
 * Issues DSB ISH to complete all critical-section memory accesses, then
 * writes 0 to the HW mutex register to release ownership.  Reads back the
 * register to ensure the write reaches device memory before returning.
 *
 * @mutex  Pointer to an initialised hwmutex_t.
 */
void hwmutex_release(hwmutex_t *mutex);

#endif /* QTI_HWMUTEX_H */