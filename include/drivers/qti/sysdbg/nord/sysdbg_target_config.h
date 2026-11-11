/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SYSDBG_TARGET_CONFIG_H
#define SYSDBG_TARGET_CONFIG_H

#include <stdbool.h>

#include <platform_def.h>

/*
 * Nord topology used by SDI: 3 clusters of 6 cores each (18 total).
 * PLAT_CLUSTER_COUNT and PLATFORM_CORE_COUNT come from qti_platform_pm.h via
 * platform_def.h; the per-cluster core counts are only needed by SDI, so they
 * are defined here behind #ifndef guards.
 */
#ifndef PLAT_CLUSTER0_CORE_COUNT
#define PLAT_CLUSTER0_CORE_COUNT	6
#endif

#ifndef PLAT_CLUSTER1_CORE_COUNT
#define PLAT_CLUSTER1_CORE_COUNT	6
#endif

#ifndef PLAT_CLUSTER2_CORE_COUNT
#define PLAT_CLUSTER2_CORE_COUNT	6
#endif

#define TARGET_APPS_CPUS		PLATFORM_CORE_COUNT
#define TARGET_CLUSTERS			PLAT_CLUSTER_COUNT
#define TARGET_APPS_CLUSTERS		PLAT_CLUSTER_COUNT
#define TARGET_CLUSTER_0_SIZE		PLAT_CLUSTER0_CORE_COUNT
#define TARGET_CLUSTER_1_SIZE		PLAT_CLUSTER1_CORE_COUNT
#define TARGET_CLUSTER_2_SIZE		PLAT_CLUSTER2_CORE_COUNT
#define MAX_CLUSTERS			PLAT_CLUSTER_COUNT

#define SYSDBG_DEBUG_ENABLE_FLAG

/*
 * On-chip SYSTEM_IMEM base (IP Catalog module id 150303855, verified against
 * nordschleife_2.0): base 0x14680000, size 0x59000. Identical to hawi's
 * hardcoded SHARED_IMEM_BASE - this on-chip IMEM base is architecturally
 * fixed across this SoC lineage. Named SYSDBG_SHARED_IMEM_BASE so it does not
 * clash with platform_def.h's SHARED_IMEM_BASE (0x146AA000), which is the
 * separate 4KB BL31 shared-IMEM window. No collision with BL2 (platform_def.h:
 * BL2_BASE=0x14694000, BL2_SIZE=0x40000): sysdbg's dump-table/log-buffer
 * window only reaches offset 0xB14 from this base, ~77KB before BL2 starts.
 */
#define SYSDBG_SHARED_IMEM_BASE		0x14680000

/*
 * DCC SRAM: QDSS_DCC_DCC_SRAM_SIZE_INFO.RAM_KB_SIZE reads 32 (KB) on
 * nordschleife_2.0 (IP Catalog, read-only reset value).
 */
#define DCC_SRAM_SIZE			(0x8000)
#define DCC_SRAM_ADDRMASK		(0x7FFF)

/* Minimum buffer sizes for DumpTable IDs. */
#define CPU_BUF_MIN_SIZE		(0x800)
#define BUF_MIN_SIZE			(0x200)

typedef struct {
	bool pwr_sts[TARGET_APPS_CPUS];
	bool cache_flushed[TARGET_APPS_CPUS];
	bool pass1_started[TARGET_APPS_CPUS];
	bool pass1_done[TARGET_APPS_CPUS];
} sysdbg_core_status_type;

#endif /* SYSDBG_TARGET_CONFIG_H */
