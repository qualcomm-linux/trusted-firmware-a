/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SYSDBG_TARGET_H
#define SYSDBG_TARGET_H

#include <stdbool.h>
#include <stdint.h>

#include <sysdbg.h>
#include <sysdbg_err.h>
#include <sysdbg_main.h>
#include <sysdbg_mem_dump.h>
#include <sysdbg_system.h>

/* Target-specific constants. */
#define GCC_XO_CLOCK_HZ			19200000
#define GCC_SLEEP_CLOCK_HZ			32768
#define MAX_WAIT_TIME_FOR_SAIL			2500

bool SysdbgTargetHWInit(void);
void SysdbgTargetResetSoc(void);
bool sysdbg_buffer_len_is_valid(dump_type name, uint64_t len);
bool SysdbgTargetResetCore(void);
uint8_t *SysdbgGetSharedImemBase(void);
const mem_config_type *Sysdbg_MemConfig(void);
const sysdbg_config_type *Sysdbg_Config(void);
const sysdbg_pass_data_type *Sysdbg_PassConfig(void);
void *Sysdbg_RegConfig(void);
bool SysdbgTargetResetStatus(void);
bool SysdbgCacheEnable(void);
bool SysdbgTargetEnable(void);
void sysdbg_save_shared_data(sysdbg_shared_data *shared_data);
uint32_t sysdbg_xlate_seqcpu_physicalcpu(uint32_t cpu);

#endif /* SYSDBG_TARGET_H */
