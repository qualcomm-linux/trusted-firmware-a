/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SYSDBG_SYSTEM_H
#define SYSDBG_SYSTEM_H

#include <stdbool.h>
#include <stdint.h>

#include <lib/spinlock.h>

#include <sysdbg_target_config.h>

typedef enum {
	ARMv8_CPU,
	HYDRA_CPU,
	ARMv7_CPU,
	MAX_CPUS,
	UNKNOWN_CPU = 0x7FFFFFFF,
} cpu_type;

typedef enum {
	SUBSYS_APPS = 0,
	SUBSYS_RPM = 1,
	SUBSYS_MSS = 2,
	SUBSYS_LPASS = 3,
	SUBSYS_SSC = 4,
	SUBSYS_SP = 5,
	SUBSYS_TURING = 6,
	SUBSYS_TURING1 = 7,
	SUBSYS_NPU = 8,
	SUBSYS_SMSS = 9,
	SUBSYS_WPSS = 0xA,
	SUBSYS_TURINGGDSP = 0xB,
	SUBSYS_TURINGGDSP1 = 0xC,
	SUBSYS_MVM = 0xD,
	SUBSYS_HPASS = 0xE,
	SUBSYS_HPASS1 = 0xF,
	SUBSYS_HPASS2 = 0x10,
	SUBSYS_TURING2 = 0x11,
	SUBSYS_TURING3 = 0x12,
	SUBSYS_DCP = 0x13,
	SUBSYS_AWM = 0x14,
	SUBSYS_SWM = 0x15,
	SUBSYS_MAX = 0x16,
	SUBSYS_UNKNOWN = 0x7FFFFFFF,
} subsys_type;

typedef struct {
	volatile uint32_t cores;
	volatile cpu_type cpuname;
} cluster_info_type;

typedef struct {
	cluster_info_type clusters[MAX_CLUSTERS];
	uint32_t cpumask;
} subsys_info_type;

typedef struct {
	uint32_t target_version;
	subsys_info_type subsys[SUBSYS_MAX];
} target_info_type;

bool SysdbgTargetInfoUpdate(void);
subsys_type get_current_subsys(void);
uint32_t sysdbg_get_corecount(subsys_type subsystem);
uint32_t get_current_cluster(subsys_type subsystem);
uint32_t get_current_cpu_seq(subsys_type subsystem);
uint32_t sysdbg_get_clusternum(subsys_type subsystem, uint32_t cpu);
uint32_t xlate_cluscpu_seqcpu(subsys_type subsystem, uint32_t cpu, uint32_t cluster);

extern target_info_type target_info;

spinlock_t *get_mem_lock(void);
spinlock_t *get_q6_lock(void);

#endif /* SYSDBG_SYSTEM_H */
