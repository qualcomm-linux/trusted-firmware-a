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
#include <lib/spinlock.h>
#include <platform.h>

#include <drivers/qti/qgic/qgic.h>
#include <drivers/qti/watchdog/watchdog.h>
#include <drivers/qti/watchdog/watchdog_generic.h>
#include <qti_plat.h>
#include <watchdog_defs.h>

#define MPIDR_INVALID		0xDEAD

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
			uint8_t interruptible;
			uint8_t on;
		} state;
		uint8_t watchdog;
		spinlock_t lock;
	} cpu = { .watchdog = 0xFFU };	/* 0xFF = no core owns the wdog yet */
	unsigned int core = plat_my_core_pos();
	unsigned int tgt = UINT32_MAX;

	if (mpidr[core] == MPIDR_INVALID) {
		mpidr[core] = read_mpidr_el1();
	}

	spin_lock(&cpu.lock);
	switch (state) {
	case QTI_WATCHDOG_CPU_WAKEUP:
		cpu.state.interruptible |= (1 << core);
		cpu.state.on |= (1 << core);

		/* unassigned, or wdog's owner is asleep -> move it to this core */
		if (cpu.watchdog == 0xFFU ||
		    !(cpu.state.on & (1 << cpu.watchdog))) {
			tgt = core;
		}
		break;
	case QTI_WATCHDOG_CPU_HOTPLUG:
		cpu.state.interruptible &= ~(1 << core);
		if (cpu.watchdog != core) {
			cpu.state.on &= ~(1 << core);
			break;
		}

		/* if more than one cpu online */
		if (cpu.state.on & (cpu.state.on - 1)) {
			cpu.state.on &= ~(1 << core);
			tgt = __builtin_ctz((uintptr_t)cpu.state.on);
		} else {
			assert(cpu.state.interruptible != 0);
			cpu.state.on &= ~(1 << core);
			tgt = __builtin_ctz((uintptr_t)cpu.state.interruptible);
		}
		break;
	case QTI_WATCHDOG_CPU_SUSPEND:
		if (cpu.watchdog != core) {
			cpu.state.on &= ~(1 << core);
			break;
		}

		/* if more than one cpu online */
		if (cpu.state.on & (cpu.state.on - 1)) {
			cpu.state.on &= ~(1 << core);
			tgt = __builtin_ctz((uintptr_t)cpu.state.on);
		} else {
			assert(core == cpu.watchdog);
			cpu.state.on &= ~(1 << core);
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

void qti_watchdog_platform_init(void)
{
	int ret;

	ret = qti_gic_register_isr(WDOG_BARK_INT_ID, bark_handler, NULL);
	if (ret) {
		ERROR("Failure registering watchdog\n");
		return;
	}

	qti_watchdog_set_target(QTI_WATCHDOG_CPU_WAKEUP);
}
