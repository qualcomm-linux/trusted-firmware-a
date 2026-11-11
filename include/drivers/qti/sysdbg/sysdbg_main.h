/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SYSDBG_MAIN_H
#define SYSDBG_MAIN_H

#include <stdbool.h>
#include <stdint.h>

#include <sysdbg_target_config.h>

typedef bool (*sysdbg_fcn_type)(void);
typedef uint64_t (*sysdbg_timestamp_type)(void);

typedef struct {
	bool cpu_affinity[TARGET_APPS_CPUS];
	sysdbg_fcn_type sysdbg_fcn;
} sysdbg_info_type;

typedef struct {
	sysdbg_info_type fcn_info[20];
} sysdbg_pass_sequence_type;

typedef struct {
	sysdbg_timestamp_type function_profiler;
	sysdbg_pass_sequence_type pass_sequence;
} sysdbg_pass_data_type;

/*
 * Device-specific configuration. Only the fields the TF-A SDI path reads are
 * kept; on Nord the reset-debug enables (pmic_resin/tsense/wdog, timeouts,
 * per-subsystem dbgen) are programmed by SOCCP, not by TF-A.
 */
typedef struct {
	const uint32_t reset_dbgen;
	const uint32_t dcache_en;
	const uint32_t icache_en;
} sysdbg_config_type;

typedef enum {
	SHARED_IMEM_REGION1,
	SHARED_IMEM_REGION2,
	SHARED_IMEM_REGION3,
	SHARED_IMEM_REGION4,
	DUMP_TABLE_REGION,
	MAX_REGIONS,
} mem_region_type;

typedef struct {
	uint32_t base;
	uint32_t offset : 16;
	uint32_t size : 16;
	bool zinit;
} mem_slice_type;

typedef struct {
	mem_slice_type slices[8];
} mem_config_type;

typedef struct {
	const uint64_t register_addr;
	const char *register_name;
	uint64_t register_val;
} register_type;

#endif /* SYSDBG_MAIN_H */
