/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SYSDBG_H
#define SYSDBG_H

#include <stdbool.h>
#include <stdint.h>

#define SYSDBG_MAGIC_NUMBER		0x42445953

#define SCAN_DUMP_FORMAT_VERSION	0x10
#define DUMP_MAGIC_NUMBER		0x42445953U
#define FCM_DUMP_FORMAT_VERSION	0x10

typedef enum {
	MSM_DUMP_TYPE_DATA = 0x0,
	MSM_DUMP_TYPE_TABLE = 0x1,
} msm_dump_entry_type;

typedef enum {
	MSM_CPU_REGS_DUMP = 0x00,
	MSM_CPU_REGS_DUMP2 = 0x10,
	MSM_PMIC_DUMP = 0xE4,
	MSM_LPASSETB_DUMP = 0xF4,
	MSM_DCP_IMG_BACKUP = 0x290,
	ALL_CPU_CACHES = 0x230,
	ALL_CLUSTER_CACHES = 0x240,
	MSM_MAX_DUMP = 0x800,
	MAX_DUMP_TYPES = 0x7FFFFFFF,
} dump_type;

typedef enum {
	MSM_DUMP_CLIENT_APPS = 0,
	MSM_DUMP_CLIENT_RPM = 1,
	MSM_DUMP_CLIENT_MPSS = 2,
	MSM_DUMP_CLIENT_ADSP = 3,
	MSM_DUMP_CLIENT_WCSS = 4,
	MSM_DUMP_CLIENT_MAX = 5,
} msm_dump_clients;

typedef struct {
	uint32_t version;
	uint32_t magic;
	char name[32];
} dump_header_type;

typedef struct {
	uint32_t version;
	uint32_t magic;
	char name[32];
	uint64_t start_addr;
	uint64_t len;
} dump_data_type;

typedef struct {
	dump_type id;
	char name[32];
	uint32_t type;
	uint64_t start_addr;
} dump_entry_type;

typedef struct {
	uint32_t version;
	uint32_t num_entries;
	dump_entry_type entries[MSM_MAX_DUMP >> 4];
} dump_table_type;

/* Shared structure for monitor-QSEE handoff in pass 1 (see sysdbg_target.c
 * for why this is ported unchanged despite having no active caller on nord).
 */
typedef struct {
	uint64_t enter_ddr_self_refresh_addr;
	uint64_t mon_imem_stack_ptr;
} sysdbg_shared_data;

unsigned int sysdbg_reset_check(void);
void sysdbg_entry_handler(void);
dump_data_type *sysdbg_get_buffer(dump_type name, uint32_t block);

#endif /* SYSDBG_H */
