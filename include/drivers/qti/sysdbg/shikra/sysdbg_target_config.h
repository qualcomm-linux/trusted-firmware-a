/*
 * Copyright (c) 2026, Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SYSDBG_TARGET_CONFIG_H
#define SYSDBG_TARGET_CONFIG_H

#ifndef __ASSEMBLER__
#include <stdint.h>
#endif

/*
 * Shikra CPU topology
 *
 * Shikra has 4 CPUs across 2 clusters:
 * - Cluster 0: 3 cores
 * - Cluster 1: 1 core
 */
#define SYSDBG_NUM_CPUS		4
#define SYSDBG_NUM_CLUSTERS	2
#define SYSDBG_CLUSTER_0_SIZE	3
#define SYSDBG_CLUSTER_1_SIZE	1
#define TARGET_APPS_CPUS	SYSDBG_NUM_CPUS

/* Clock frequencies used for the reset debug timers */
#define GCC_XO_CLOCK_HZ		19200000
#define GCC_SLEEP_CLOCK_HZ	32768

/* Shared IMEM base used for the SDI cookies and dump table */
#define SYSTEM_SHAREDIMEM_BASE	0x0C11E000

/*
 * DDR self-refresh recovery entry points, provided by the boot (QCLib)
 * image and resident in IMEM. They must match the boot image flashed with
 * this BL31; override them at build time, for example
 *   make ... SDI_DDR_INIT_ADDR=0x0C11418C SDI_DDR_EXIT_ADDR=0x0C1143EC
 *
 *   ddr_sdi_init - bring DDR out of self-refresh
 *   ddr_sdi_exit - put DDR back into self-refresh
 */
#ifndef SDI_DDR_INIT_ADDR
#define SDI_DDR_INIT_ADDR	0x0C11418C
#endif
#ifndef SDI_DDR_EXIT_ADDR
#define SDI_DDR_EXIT_ADDR	0x0C1143EC
#endif

#ifndef __ASSEMBLER__
#define SYSDBG_DDR_SDI_INIT_ADDR	((uintptr_t)SDI_DDR_INIT_ADDR)
#define SYSDBG_DDR_SDI_EXIT_ADDR	((uintptr_t)SDI_DDR_EXIT_ADDR)
#endif

#endif /* SYSDBG_TARGET_CONFIG_H */
