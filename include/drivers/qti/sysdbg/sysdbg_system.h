/*
 * Copyright (c) 2026, Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * System Debug System Level Functions
 *
 * Ported from TZ.XF.5.36-00138-SHIKRA-1:
 * trustzone_images/core/systemdebug/sysdbg/inc/sysdbg_system.h
 *
 * This module handles the system level functions performed by the driver.
 */

#ifndef SYSDBG_SYSTEM_H
#define SYSDBG_SYSTEM_H

#include <stdint.h>
#include <stdbool.h>

/* Maximum clusters the driver supports */
#define MAX_CLUSTERS  2

/* CPU specific enums */
typedef enum {
	ARMV8_CPU,
	HYDRA_CPU,
	ARMv7_CPU,
	MAX_CPUS,
	UNKNOWN_CPU = 0x7FFFFFFF,
} cpu_type;

/* Name for the various sub-systems */
typedef enum {
	SUBSYS_APPS   = 0,
	SUBSYS_RPM    = 1,
	SUBSYS_MSS    = 2,
	SUBSYS_LPASS  = 3,
	SUBSYS_SSC    = 4,
	SUBSYS_SP     = 5,
	SUBSYS_TURING = 6,
	SUBSYS_TURING1 = 7,
	SUBSYS_NPU    = 8,
	SUBSYS_SMSS   = 9,
	SUBSYS_WPSS   = 0xA,
	SUBSYS_TURINGGDSP = 0xB,
	SUBSYS_TURINGGDSP1 = 0xC,
	SUBSYS_MVM    = 0xD,
	SUBSYS_HPASS  = 0xE,
	SUBSYS_HPASS1 = 0xF,
	SUBSYS_HPASS2 = 0x10,
	SUBSYS_TURING2 = 0x11,
	SUBSYS_TURING3 = 0x12,
	SUBSYS_DCP    = 0x13,
	SUBSYS_AWM    = 0x14,
	SUBSYS_SWM    = 0x15,
	SUBSYS_LMCU   = 0x16,
	SUBSYS_MAX    = 0x17,
	SUBSYS_UNKNOWN = 0x7FFFFFFF,
} subsys_type;

/* Cluster data type */
typedef struct {
	/* Number of cores present */
	volatile uint32_t cores;
	/* Type of the core */
	volatile cpu_type cpuname;
} cluster_info_type;

/* Sub-system data type */
typedef struct {
	cluster_info_type clusters[MAX_CLUSTERS];
	/* Bit mask indicating valid cores */
	uint32_t cpumask;
} subsys_info_type;

/* Device specific data */
typedef struct {
	subsys_info_type subsys[SUBSYS_MAX];
	uint32_t chip_version;
} target_info_type;

/* Function declarations */
subsys_type get_current_subsys(void);
uint32_t sysdbg_get_corecount(subsys_type subsystem);
uint32_t get_current_cluster(subsys_type subsystem);
uint32_t get_current_cpu_seq(subsys_type subsystem);
uint32_t sysdbg_get_clusternum(subsys_type subsystem, uint32_t cpu);

#endif /* SYSDBG_SYSTEM_H */
