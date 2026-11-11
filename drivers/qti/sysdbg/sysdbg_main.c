/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdbool.h>
#include <stdint.h>

#include <arch_helpers.h>
#include <lib/utils_def.h>

#include <sysdbg_main.h>
#include <sysdbg_system.h>
#include <sysdbg_target.h>
#include <sysdbg_target_config.h>

extern sysdbg_core_status_type sysdbg_core_status;

static void sysdbg_execute_pass(const sysdbg_pass_data_type *sysdbg_pass_ptr)
{
	bool pass = true;
	uint32_t i;
	subsys_type subsys = get_current_subsys();
	uint32_t current_cpu = get_current_cpu_seq(subsys);
	const sysdbg_pass_sequence_type *sysdbg_seq_entry;

	if (sysdbg_pass_ptr == NULL) {
		return;
	}

	sysdbg_seq_entry = &sysdbg_pass_ptr->pass_sequence;

	for (i = 0; (i < ARRAY_SIZE(sysdbg_seq_entry->fcn_info)) && pass; i++) {
		const sysdbg_info_type *entry = &sysdbg_seq_entry->fcn_info[i];

		if (!entry->cpu_affinity[current_cpu]) {
			continue;
		}
		if (entry->sysdbg_fcn == NULL) {
			break;
		}
		pass = entry->sysdbg_fcn();
	}
}

void system_debug_exit(uint32_t cpu)
{
	uint32_t i;

	sysdbg_core_status.pass1_done[cpu] = true;
	dsbsy();
	isb();

	for (i = 0; i < TARGET_APPS_CPUS; i++) {
		if (sysdbg_core_status.pwr_sts[i] != sysdbg_core_status.pass1_done[i]) {
			SysdbgWaitForReset();
		}
	}

	/* Write the pass-1-complete cookie, then park (as hawi/TZ). */
	SysdbgResetSoC();
}

void system_debug_reset_entry(void)
{
	static bool spinlock_initialized;
	uint32_t this_cpu = get_current_cpu_seq(SUBSYS_APPS);
	const sysdbg_pass_data_type *sysdbg_pass;

	sysdbg_core_status.pass1_started[this_cpu] = true;

	if (!spinlock_initialized) {
		/*
		 * spinlock_t is unlocked at zero-init; get_mem_lock()/
		 * get_q6_lock() already return zero-initialised static
		 * storage, so no explicit init call is needed (Rule I1).
		 */
		spinlock_initialized = true;
	}

	sysdbg_pass = Sysdbg_PassConfig();
	if (sysdbg_pass != NULL) {
		sysdbg_execute_pass(sysdbg_pass);
	}

	system_debug_exit(this_cpu);
}
