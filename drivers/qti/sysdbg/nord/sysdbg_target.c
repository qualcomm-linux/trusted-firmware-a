/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <cdefs.h>
#include <stdbool.h>
#include <stdint.h>

#include <arch_helpers.h>

#include <sysdbg.h>
#include <sysdbg_main.h>
#include <sysdbg_mem_dump.h>
#include <sysdbg_system.h>
#include <sysdbg_target.h>
#include <sysdbg_target_config.h>

/* Cache-enable routines - drivers/qti/sysdbg/asm/sysdbg_cache_armv8.S. */
extern void dcache_enable(uint64_t sctlr);
extern void icache_enable(uint64_t sctlr);

/*
 * Alternate stack in IMEM, and DDR-self-refresh-entry callback, both
 * populated only by sysdbg_save_shared_data() below. Ported as-is from
 * source: sysdbg_save_shared_data() has zero callers in src_repo (its only
 * caller, mon_get_sysdbg_shared_data(), belongs to the Section-0 monitor
 * handoff this port drops) - preserved unchanged per explicit direction to
 * introduce no behavioral change versus the reference, not omitted despite
 * being presently uncalled on nord either.
 */
static void (*ddr_self_refresh_enter)(void);
static uint64_t alt_imem_stack_ptr;

target_info_type target_info;
sysdbg_core_status_type sysdbg_core_status;
volatile bool sysdbg_real_pwr_sts[TARGET_APPS_CPUS];

void SysdbgSaveCoreStatus(void)
{
	sysdbg_core_status.pwr_sts[0] = true;
}

bool SysdbgTargetHWInit(void)
{
	SysdbgSaveCoreStatus();

	return true;
}

__dead2 void SysdbgWaitForReset(void)
{
	while (true) {
	}
}

/*
 * SDI pass-1-complete: indicate completion and park. This does not trigger
 * a reset itself - the AOSS_CC_RST_CTL_REG reset FSM's own hardware state
 * machine asserts the reset once its independently-armed first-pass timer
 * expires. Ported unchanged from source (no behavioral change).
 */
static __dead2 void first_pass_complete(void)
{
	__asm volatile ("mov sp, %0" : : "r" (alt_imem_stack_ptr) :);

	while (true) {
	}
}

void SysdbgTargetResetSoc(void)
{
	first_pass_complete();
}

bool sysdbg_buffer_len_is_valid(dump_type name, uint64_t len)
{
	uint64_t min_required_size;

	switch (name) {
	case MSM_CPU_REGS_DUMP:
	case MSM_CPU_REGS_DUMP2:
		min_required_size = CPU_BUF_MIN_SIZE;
		break;
	default:
		min_required_size = BUF_MIN_SIZE;
		break;
	}

	return (len >= min_required_size);
}

bool SysdbgTargetResetCore(void)
{
	return true;
}

uint8_t *SysdbgGetSharedImemBase(void)
{
	return (uint8_t *)(uintptr_t)SYSDBG_SHARED_IMEM_BASE;
}

bool SysdbgTargetEnable(void)
{
	const sysdbg_config_type *sysdbg_config = Sysdbg_Config();

	if (!sysdbg_config->reset_dbgen) {
		return true;
	}

	return true;
}

bool SysdbgCacheEnable(void)
{
	const sysdbg_config_type *sysdbg_config = Sysdbg_Config();
	uint64_t sctlr_el3 = read_sctlr_el3();

	if (sysdbg_config->icache_en) {
		icache_enable(sctlr_el3);
	}

	if (sysdbg_config->dcache_en) {
		dcache_enable(sctlr_el3);
	}

	return true;
}

bool SysdbgTargetResetStatus(void)
{
	return false;
}

bool SysdbgTargetInfoUpdate(void)
{
	static bool init;

	if (!init) {
		target_info.subsys[SUBSYS_APPS].clusters[0].cores = TARGET_CLUSTER_0_SIZE;
		target_info.subsys[SUBSYS_APPS].clusters[1].cores = TARGET_CLUSTER_1_SIZE;
		target_info.subsys[SUBSYS_APPS].clusters[2].cores = TARGET_CLUSTER_2_SIZE;
		target_info.subsys[SUBSYS_APPS].clusters[0].cpuname = ARMv8_CPU;
		target_info.subsys[SUBSYS_APPS].clusters[1].cpuname = ARMv8_CPU;
		target_info.subsys[SUBSYS_APPS].clusters[2].cpuname = ARMv8_CPU;
		init = true;
	}

	return init;
}

void sysdbg_save_shared_data(sysdbg_shared_data *shared_data)
{
	ddr_self_refresh_enter =
		(void (*)(void))(uintptr_t)shared_data->enter_ddr_self_refresh_addr;
	alt_imem_stack_ptr = shared_data->mon_imem_stack_ptr;
}

uint32_t sysdbg_xlate_seqcpu_physicalcpu(uint32_t cpu)
{
	return cpu;
}
