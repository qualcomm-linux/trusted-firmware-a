/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * NCC (Next-generation CPU Cluster) hardware register definitions for Nord.
 *
 * Defines the NCC0 architecture register addresses, bit-field masks/shifts,
 * and LPM state identifiers used by qcom_ncc_psci.c to program the NCC LPM
 * permission bits and RVBAR-force control.
 *
 * TODO: Verify all register addresses and bit-field positions against the
 *       Nord NCC register specification / SoC memory map.
 */

#ifndef NCC_HWIO_H
#define NCC_HWIO_H

#include <stdint.h>

/* --------------------------------------------------------------------------
 * NCC cluster topology
 * -------------------------------------------------------------------------- */

/*
 * Physical stride between consecutive NCC cluster register banks.
 * Each cluster occupies a contiguous 0x10000-byte window.
 * TODO: Verify against Nord NCC register map.
 */
#define NCC_CLUSTER_STRIDE		(0x10000ULL)

/* --------------------------------------------------------------------------
 * NCC0 architecture register base addresses (cluster 0).
 * Cluster N base = NCC0_NCC_NCC_ARCH_0_BASE + N * NCC_CLUSTER_STRIDE
 *
 * TODO: Verify base address against Nord SoC memory map.
 * -------------------------------------------------------------------------- */
#define NCC0_NCC_NCC_ARCH_0_BASE	(0x18200000ULL)

/*
 * NCC_PWR_CFG1 – core and cluster LPM permission register.
 * Controls OK2ENTERC3/C4 (per-core) and OK2ENTERCL3/CL4 (per-cluster) bits.
 */
#define NCC0_NCC_NCC_ARCH_0_NCC_PWR_CFG1 \
	(NCC0_NCC_NCC_ARCH_0_BASE + 0x0008ULL)

/*
 * NCC_BOOT_CFG1 – cluster boot/power configuration register.
 * Controls OK2ENTERCL5 (cluster CL5 permission) bit.
 */
#define NCC0_NCC_NCC_ARCH_0_NCC_BOOT_CFG1 \
	(NCC0_NCC_NCC_ARCH_0_BASE + 0x0010ULL)

/*
 * NCC_PWR_CTL3 – core power control register 3.
 * Controls CORFRCCPURVBAR (force RVBAR fetch on next power-on) per-core bits.
 */
#define NCC0_NCC_NCC_ARCH_0_NCC_PWR_CTL3 \
	(NCC0_NCC_NCC_ARCH_0_BASE + 0x0020ULL)

/* --------------------------------------------------------------------------
 * NCC_PWR_CFG1 bit-field definitions
 *
 * Per-core C3/C4 LPM permission bits (bits[7:0] = core mask):
 *   OK2ENTERC3: allow core to enter C3 (clock-gated)
 *   OK2ENTERC4: allow core to enter C4 (power-collapsed)
 *
 * Per-cluster CL3/CL4 LPM permission bits:
 *   OK2ENTERCL3: allow cluster to enter CL3
 *   OK2ENTERCL4: allow cluster to enter CL4 (L2 power-collapsed)
 *
 * TODO: Verify bit positions against Nord NCC register specification.
 * -------------------------------------------------------------------------- */
#define OK2ENTERC3_SHIFT	0U
#define OK2ENTERC3_MASK		0xFFU
#define OK2ENTERC4_SHIFT	8U
#define OK2ENTERC4_MASK		0xFFU
#define OK2ENTERCL3_SHIFT	16U
#define OK2ENTERCL3_MASK	0x1U
#define OK2ENTERCL4_SHIFT	17U
#define OK2ENTERCL4_MASK	0x1U

/* --------------------------------------------------------------------------
 * NCC_BOOT_CFG1 bit-field definitions
 *
 * OK2ENTERCL5: allow cluster to enter CL5 (fully powered off).
 *
 * TODO: Verify bit position against Nord NCC register specification.
 * -------------------------------------------------------------------------- */
#define OK2ENTERCL5_SHIFT	0U
#define OK2ENTERCL5_MASK	0x1U

/* --------------------------------------------------------------------------
 * NCC_PWR_CTL3 bit-field definitions
 *
 * CORFRCCPURVBAR: force RVBAR fetch on next power-on (per-core bits).
 * When set, the core fetches its reset vector from RVBAR_EL3 on the next
 * power-on, ensuring it re-enters TF-A warm-boot after a power-collapse.
 *
 * TODO: Verify bit positions against Nord NCC register specification.
 * -------------------------------------------------------------------------- */
#define CORFRCCPURVBAR_SHIFT	0U
#define CORFRCCPURVBAR_MASK	0xFFU

/* --------------------------------------------------------------------------
 * NCC LPM state identifiers.
 *
 * These are plat_local_state_t values used in the PSCI power-state arrays
 * and as switch-case labels in ncc_config_core_lpm / ncc_config_cluster_lpm.
 * They must match the QTI_LOCAL_STATE_* values from qti_platform_pm.h so
 * that qti_pm.c's is_cpu_off() check works correctly.
 *
 * Mapping:
 *   STATE_C3  = QTI_LOCAL_STATE_RET     (5) – core clock-gated, context retained
 *   STATE_C4  = QTI_LOCAL_STATE_OFF     (6) – core power-collapsed
 *   STATE_CL4 = QTI_LOCAL_STATE_OFF     (6) – cluster L2 power-collapsed
 *   STATE_CL5 = QTI_LOCAL_STATE_DEEPOFF (7) – cluster fully off
 *
 * Note: STATE_C4 and STATE_CL4 share the same numeric value (6) because
 * they are used in separate switch statements (core vs cluster context).
 * -------------------------------------------------------------------------- */
#define STATE_C3	(5U)	/* = QTI_LOCAL_STATE_RET:     core clock-gated */
#define STATE_C4	(6U)	/* = QTI_LOCAL_STATE_OFF:     core power-collapsed */
#define STATE_CL4	(6U)	/* = QTI_LOCAL_STATE_OFF:     cluster L2 collapsed */
#define STATE_CL5	(7U)	/* = QTI_LOCAL_STATE_DEEPOFF: cluster fully off */

#endif /* NCC_HWIO_H */