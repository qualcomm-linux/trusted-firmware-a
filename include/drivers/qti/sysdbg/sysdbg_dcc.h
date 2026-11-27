/*
 * Copyright (c) 2026, Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SYSDBG_DCC_H
#define SYSDBG_DCC_H

#include <stdint.h>
#include <stdbool.h>

/**
 * @file sysdbg_dcc.h
 * @brief DCC (Data Capture and Compare) Driver API
 */

/*===========================================================================
 * DCC Sink Types
 *===========================================================================
 */

typedef enum {
	DCC_SINK_SRAM,
	DCC_SINK_ETB,
	DCC_SINK_MAX,
	DCC_SINK_INVALID = 0x7FFFFFFF
} dcc_sink_type;

/*===========================================================================
 * DCC Trigger Types
 *===========================================================================
 */

typedef enum {
	DCC_TRIGGER_SW,
	DCC_TRIGGER_SCANDUMP,
	DCC_TRIGGER_IHM_MHM_SCANDUMP,
	DCC_TRIGGER_DDRAUX,
	DCC_TRIGGER_APSSWDOGBITE,
	DCC_TRIGGER_WINDDOWN,
	DCC_TRIGGER_CTI,
	DCC_TRIGGER_MAX,
	DCC_TRIGGER_INVALID = 0x7FFFFFFF
} dcc_trigger_type;

/*===========================================================================
 * DCC Linked List Configuration
 *===========================================================================
 */

typedef struct {
	const uint32_t ll_idx;
	const uint32_t ll_timeout;
	const bool ll_timeout_en;
	const uint32_t llt_timeout;
	const bool llt_timeout_en;
	const uint32_t ll_offset;
	const uint32_t ll_fd_offset;
	const uint32_t ll_fd_size;
	const dcc_trigger_type ll_trigger;
	const void *ll;
	const uint32_t ll_size;
} dcc_ll_config_type;

/*===========================================================================
 * DCC Global Configuration
 *===========================================================================
 */

typedef struct {
	const bool dcc_en;
	const uint32_t dcc_mem_size;
	dcc_sink_type sink;
	dcc_trigger_type pc_trigger;
	uint32_t pc_offset;
	uint32_t pc_size;
	uint32_t pc_count;
} dcc_config_type;

/* Linked List entry type */
typedef uint32_t dcc_ll_entry_type;

/*===========================================================================
 * Function Declarations
 *===========================================================================
 */

/**
 * sysdbg_dcc_init() - Top level DCC initialization
 */
bool sysdbg_dcc_init(void);

/**
 * dcc_get_config() - Get DCC global configuration
 */
const dcc_config_type *dcc_get_config(void);

/**
 * dcc_get_ll_config() - Get linked list configuration
 */
const dcc_ll_config_type *dcc_get_ll_config(uint32_t idx);

/**
 * dcc_get_ll_cnt() - Get linked list count
 */
uint32_t dcc_get_ll_cnt(void);

#endif /* SYSDBG_DCC_H */
