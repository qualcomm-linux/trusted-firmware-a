/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <stdint.h>

#include <arch_helpers.h>
#include <common/debug.h>
#include <drivers/arm/gicv3.h>
#include <lib/cassert.h>
#include <lib/spinlock.h>
#include <lib/utils_def.h>
#include <platform.h>

#include <drivers/qti/qgic/qgic.h>
#include <drivers/qti/watchdog/watchdog.h>
#include <drivers/qti/watchdog/watchdog_generic.h>
#include <qti_plat.h>
#include <watchdog_defs.h>

#define MPIDR_INVALID		0xDEAD
#define WDOG_CPU_NONE		UINT32_MAX

CASSERT(PLATFORM_CORE_COUNT <= 32U, watchdog_cpu_state_mask_too_small);

static u_register_t mpidr[PLATFORM_CORE_COUNT] = {
	[0 ... PLATFORM_CORE_COUNT - 1] = MPIDR_INVALID
};

static void *bark_handler(void *ctx)
{
	qti_watchdog_pet();

	return ctx;
}

void qti_watchdog_set_target(qti_watchdog_cpu_state_t state)
{
	static struct {
		struct {
			uint32_t interruptible;
			uint32_t on;
		} state;
		uint32_t watchdog;
		spinlock_t lock;
	} cpu = { .watchdog = WDOG_CPU_NONE };
	unsigned int core = plat_my_core_pos();
	unsigned int tgt = UINT32_MAX;

	if (mpidr[core] == MPIDR_INVALID) {
		mpidr[core] = read_mpidr_el1();
	}

	spin_lock(&cpu.lock);
	switch (state) {
	case QTI_WATCHDOG_CPU_WAKEUP:
		cpu.state.interruptible |= BIT_32(core);
		cpu.state.on |= BIT_32(core);

		/* unassigned, or wdog's owner is asleep -> move it to this core */
		if (cpu.watchdog == WDOG_CPU_NONE ||
		    (cpu.state.on & BIT_32(cpu.watchdog)) == 0U) {
			tgt = core;
		}
		break;
	case QTI_WATCHDOG_CPU_HOTPLUG:
		cpu.state.interruptible &= ~BIT_32(core);
		if (cpu.watchdog != core) {
			cpu.state.on &= ~BIT_32(core);
			break;
		}

		/* if more than one cpu online */
		if (cpu.state.on & (cpu.state.on - 1)) {
			cpu.state.on &= ~BIT_32(core);
			tgt = __builtin_ctz((uintptr_t)cpu.state.on);
		} else {
			assert(cpu.state.interruptible != 0);
			cpu.state.on &= ~BIT_32(core);
			tgt = __builtin_ctz((uintptr_t)cpu.state.interruptible);
		}
		break;
	case QTI_WATCHDOG_CPU_SUSPEND:
		if (cpu.watchdog != core) {
			cpu.state.on &= ~BIT_32(core);
			break;
		}

		/* if more than one cpu online */
		if (cpu.state.on & (cpu.state.on - 1)) {
			cpu.state.on &= ~BIT_32(core);
			tgt = __builtin_ctz((uintptr_t)cpu.state.on);
		} else {
			assert(core == cpu.watchdog);
			cpu.state.on &= ~BIT_32(core);
		}
		break;
	default:
		break;

	};

	if (tgt != UINT32_MAX) {
		assert(mpidr[tgt] != MPIDR_INVALID);
		cpu.watchdog = tgt;
		gic_set_spi_routing(WDOG_BARK_INT_ID, GICV3_IRM_PE, mpidr[tgt]);
	}
	spin_unlock(&cpu.lock);
}

bool qti_watchdog_handle_stop_on_fail(void)
{
	return false;
}

void qti_watchdog_set_stop_on_fail(uint32_t err)
{
	(void)err;
}

int qti_watchdog_platform_init(void)
{
	int ret;

	ret = qti_gic_register_isr(WDOG_BARK_INT_ID, bark_handler, NULL);
	if (ret != 0) {
		ERROR("Failure registering watchdog: %d\n", ret);
		return ret;
	}

	qti_watchdog_set_target(QTI_WATCHDOG_CPU_WAKEUP);
	return 0;
}
