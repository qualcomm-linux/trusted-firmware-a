/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Nord (SA8797P / Oryon "NCC") secure-core warm-boot vector programming.
 *
 * Unlike legacy Kryo (which uses the split APSS_SHARED_KRYO_RVBARADDR_LO/HI
 * registers, see sec_core.c), Oryon programs a per-CLUSTER reset vector register
 * NCC_RVBAR at (cluster_arch_base + 0x328), written as a single 64-bit store
 * (BASEADDR field [51:2], 4-byte aligned). The nord build previously linked
 * sec_core_stub.c, leaving this UNPROGRAMMED -> when CPUCP/RSC power-collapses an
 * Oryon core on WFI it resumes at the CPUCP PBL reset vector (not BL31) and is
 * effectively lost, which is the root cause of the intermittent boot and the
 * ~2s post-shell CPU shutdown. Program every cluster's NCC_RVBAR with the BL31
 * warm-boot entrypoint so collapsed cores re-enter BL31.
 *
 * Per-cluster NCC ARCH register block:
 *   NCC_ARCH_REG_BASE      = 0x19880000
 *   NCC_CLUSTER_REGS_WIDTH = 0x1000000   (16 MB stride: clus0/1/2 -> 0x19/0x1A/0x1B 880000)
 *   NCC_RVBAR offset       = 0x328
 * Nord = 3 clusters x 6 Oryon cores; one RVBAR per cluster covers its cores.
 */

#include <arch_helpers.h>
#include <common/debug.h>
#include <drivers/qti/sec_core/sec_core.h>
#include <lib/mmio.h>

#define NORD_NCC_ARCH_BASE		U(0x19880000)
#define NORD_NCC_CLUSTER_STRIDE		U(0x1000000)
#define NORD_NCC_RVBAR_OFFSET		U(0x328)
#define NORD_NCC_NUM_CLUSTERS		U(3)

void qti_sec_core_remap(uintptr_t entrypoint)
{
	unsigned int c;

	for (c = 0U; c < NORD_NCC_NUM_CLUSTERS; c++) {
		uintptr_t rvbar = (uintptr_t)NORD_NCC_ARCH_BASE +
				  ((uintptr_t)c * (uintptr_t)NORD_NCC_CLUSTER_STRIDE) +
				  (uintptr_t)NORD_NCC_RVBAR_OFFSET;

		mmio_write_64(rvbar, (uint64_t)entrypoint);
	}

	dsbsy();
	isb();
}

void qti_sec_core_init(void)
{
	/*
	 * Nord APC/SAW/PLL secure configuration is owned by CPUCP/XBL, not BL31;
	 * nothing to do here (legacy Kryo APC-secure writes in sec_core.c do not
	 * apply to Oryon).
	 */
}
