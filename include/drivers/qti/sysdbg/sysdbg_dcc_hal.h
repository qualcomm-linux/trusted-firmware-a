/*
 * Copyright (c) 2026, Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SYSDBG_DCC_HAL_H
#define SYSDBG_DCC_HAL_H

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

/**
 * @file sysdbg_dcc_hal.h
 * @brief DCC Hardware Abstraction Layer API
 */

/*===========================================================================
 * HAL Macros
 *===========================================================================
 */

/* Macro to align 'addr' to 'align' number of bytes */
#define ALIGN_ADDR(addr, align)    ((addr + align - 1) & (~(align-1)))

/* Get the word offset for a byte address. DCC requires word index/offset everywhere */
#define WORD_INDEX(x)               (x >> 2)
#define BYTE_INDEX(x)               (x << 2)

/*===========================================================================
 * HAL Enums
 *===========================================================================
 */

typedef enum {
	HAL_DCC_SINK_SRAM,
	HAL_DCC_SINK_ETB,
	HAL_DCC_SINK_MAX,
	HAL_DCC_SINK_INVALID = 0x7FFFFFFF
} hal_dcc_sink_type;

typedef enum {
	HAL_DCC_FUNC_REG_CAPTURE,
	HAL_DCC_FUNC_CRC_CAPTURE,
	HAL_DCC_FUNC_MAX,
	HAL_DCC_FUNC_INVALID = 0x7FFFFFFF,
} hal_dcc_func_type;

typedef enum {
	HAL_DCC_HW_TRIGGER,
	HAL_DCC_SW_TRIGGER,
	HAL_DCC_CTI_TRIGGER,
	HAL_DCC_TRIGGER_MAX,
} hal_dcc_trig_type;

/*===========================================================================
 * HAL Structures
 *===========================================================================
 */

typedef struct {
	uint32_t ntimeout;
	uint32_t ntranstimeout;
	uint32_t noffset;
	hal_dcc_sink_type esink;
	hal_dcc_func_type efunc;
	hal_dcc_trig_type etrigger;
} hal_dcc_list_params_type;

/*===========================================================================
 * HAL Function Declarations
 *===========================================================================
 */

uint32_t hal_dcc_get_max_lists(void);
uint64_t hal_dcc_get_sinkbase(hal_dcc_sink_type sink);
uint32_t hal_dcc_get_database(uint32_t list);
bool hal_dcc_init_sink(hal_dcc_sink_type sink);
hal_dcc_sink_type hal_dcc_get_sink(uint32_t list);
void hal_dcc_config_list_base(uint32_t list, uint32_t base);
void hal_dcc_config_data_base(uint32_t list, uint32_t base);
void hal_dcc_config_timeout(uint32_t list, uint32_t timeout);
void hal_dcc_config_trans_timeout(uint32_t list, uint32_t timeout);
void hal_dcc_config_sink(uint32_t list, uint32_t sink);
void hal_dcc_config_func(uint32_t list, uint32_t func);
void hal_dcc_config_trigger(uint32_t list, hal_dcc_trig_type trigger);
bool hal_dcc_config_list(uint32_t idx, hal_dcc_list_params_type *params);
bool hal_dcc_program_list(void *list, uint32_t list_idx, uint32_t offset, uint32_t size);
void hal_dcc_lock_list(uint32_t list);
void hal_dcc_lock_list_ns(uint32_t list);
void hal_dcc_unlock_list(uint32_t list);
bool hal_dcc_is_list_locked(uint32_t list);

#endif /* SYSDBG_DCC_HAL_H */
