/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stddef.h>
#include <stdint.h>

#include <drivers/qti/sec_core/sec_core.h>

#include <sec_core_defs.h>

/*
 * Nord does not require any ACPS security register initialisation at boot.
 * The qti_sec_core_init() loop is a no-op when qti_sec_core_cfg_count is 0.
 */
const struct sec_core_cfg_s qti_sec_core_cfg[0];
const size_t qti_sec_core_cfg_count = 0U;

/*
 * NCC RVBAR register addresses for Nord.
 * Taken from the NCC architecture register map.
 */
const uintptr_t qti_sec_core_rvbaraddr_lo = NCC0_NCC_NCC_ARCH_0_NCC_RVBAR_ADDR_LO;
const uintptr_t qti_sec_core_rvbaraddr_hi = NCC0_NCC_NCC_ARCH_0_NCC_RVBAR_ADDR_HI;

/*
 * RVBAR address shift offsets for Nord.
 * The NCC RVBAR registers on Nord store the full physical address split at
 * the 32-bit boundary:
 *   LO register: bits [31:0]  of the entrypoint address (addr >> 0)
 *   HI register: bits [63:32] of the entrypoint address (addr >> 32)
 */
#define QTI_SEC_CORE_RVBARADDR_LO_SHIFT		(0U)
#define QTI_SEC_CORE_RVBARADDR_HI_SHIFT		(32U)

const unsigned int qti_sec_core_rvbaraddr_lo_shift = QTI_SEC_CORE_RVBARADDR_LO_SHIFT;
const unsigned int qti_sec_core_rvbaraddr_hi_shift = QTI_SEC_CORE_RVBARADDR_HI_SHIFT;