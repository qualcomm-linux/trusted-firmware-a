/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <arch_helpers.h>
#include <common/debug.h>
#include <drivers/arm/gicv3.h>
#include <lib/mmio.h>
#include <lib/spinlock.h>
#include <lib/utils_def.h>
#include <platform.h>

#include <drivers/qti/qgic/qgic.h>
#include <drivers/qti/watchdog/watchdog.h>
#include <drivers/qti/watchdog/watchdog_generic.h>
#include <bl31qtilib_defs.h>
#include <bl31qtilib_interface.h>
#include <qti_plat.h>
#include <watchdog_defs.h>

#define MPIDR_INVALID		0xDEAD

#ifdef WDOG_NSEC_BITE_INT_ID
#define WDOG_NSEC_RESET_DELAY_MS	500U
#endif

/*
 * Secure debug-options subsystem ID for the AP secure world
 * (dbgil_subsys_t).
 */
#define DBGIL_SUBSYS_APPS_SEC				5U

/* SSDBO bit indicating secure watchdog debug access is permitted. */
#define SSDBO_APPS_SEC_INDICATE_SECURE_WATCHDOG_ACCESS	0x40000000U

#ifndef WDOG_STOP_ON_FAIL_ERROR_0
#define WDOG_STOP_ON_FAIL_ERROR_0	0U
#endif
#ifndef WDOG_STOP_ON_FAIL_ERROR_1
#define WDOG_STOP_ON_FAIL_ERROR_1	0U
#endif
#ifndef WDOG_STOP_ON_FAIL_ERROR_2
#define WDOG_STOP_ON_FAIL_ERROR_2	0U
#endif
#ifndef WDOG_STOP_ON_FAIL_ERROR_3
#define WDOG_STOP_ON_FAIL_ERROR_3	0U
#endif

static const uint32_t wdog_stop_on_fail_error_list[] = {
	WDOG_STOP_ON_FAIL_ERROR_0,
	WDOG_STOP_ON_FAIL_ERROR_1,
	WDOG_STOP_ON_FAIL_ERROR_2,
	WDOG_STOP_ON_FAIL_ERROR_3,
};

static uint32_t wdog_stop_on_fail_err;
static bool wdog_stop_on_fail_armed;

static u_register_t mpidr[PLATFORM_CORE_COUNT] = {
	[0 ... PLATFORM_CORE_COUNT - 1] = MPIDR_INVALID
};

static void *bark_handler(void *ctx)
{
	qti_watchdog_pet();

	return ctx;
}

#ifdef WDOG_NSEC_BITE_INT_ID
static void *nsec_bite_handler(void *ctx)
{
	(void)ctx;

	ERROR("AP-NSEC watchdog bite\n");
	qti_watchdog_force_bite(WDOG_NSEC_RESET_DELAY_MS);

	while (true) {
		/* Wait for the secure watchdog to reset the device. */
	}

	return ctx;
}
#endif

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

static bool soc_wdog_debug_allowed(void)
{
	smc_rsp_t rsp = { { 0 } };

	if (bl31qtilib_get_subsystem_debug_options(
		DBGIL_SUBSYS_APPS_SEC, &rsp) != 0) {
		return false;
	}

	return (rsp.rsp[0] & SSDBO_APPS_SEC_INDICATE_SECURE_WATCHDOG_ACCESS) != 0U;
}

static bool wdog_error_in_list(uint32_t err)
{
	size_t i;

	for (i = 0U; i < ARRAY_SIZE(wdog_stop_on_fail_error_list); i++) {
		if (wdog_stop_on_fail_error_list[i] == err) {
			return true;
		}
	}

	return false;
}

static void system_watchdog_disable(void)
{
#if !defined(SYSTEM_WDOG_RESET_ADDR) || !defined(SYSTEM_WDOG_RESET) || \
    !defined(SYSTEM_WDOG_CTL_ADDR) || !defined(SYSTEM_WDOG_ENABLE_BIT)
	WARN("System watchdog constants not defined for this chipset\n");
#else
	mmio_write_32(SYSTEM_WDOG_RESET_ADDR, SYSTEM_WDOG_RESET);
	mmio_clrbits_32(SYSTEM_WDOG_CTL_ADDR, SYSTEM_WDOG_ENABLE_BIT);
#endif
}

void qti_watchdog_set_stop_on_fail(uint32_t err)
{
	wdog_stop_on_fail_err = err;
	wdog_stop_on_fail_armed = false;

	if (err == 0U) {
		return;
	}

	if (wdog_stop_on_fail_error_list[0] == 0U) {
		return;
	}

	if (!soc_wdog_debug_allowed()) {
		return;
	}

	if (!wdog_error_in_list(err)) {
		return;
	}

	wdog_stop_on_fail_armed = true;
}

bool qti_watchdog_handle_stop_on_fail(void)
{
	if (!wdog_stop_on_fail_armed) {
		return false;
	}

	ERROR("Watchdog start skipped: stop-on-fail armed for error 0x%x\n",
	      wdog_stop_on_fail_err);

	qti_watchdog_stop();
	system_watchdog_disable();

	return true;
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

	qti_gic_enable_intr(WDOG_BARK_INT_ID);

#ifdef WDOG_NSEC_BITE_INT_ID
	ret = qti_gic_register_isr(WDOG_NSEC_BITE_INT_ID,
				   nsec_bite_handler, NULL);
	if (ret) {
		ERROR("Failure registering AP-NSEC watchdog bite\n");
		return;
	}

	qti_gic_set_spi_routing(WDOG_NSEC_BITE_INT_ID, GICV3_IRM_ANY, 0U);
	qti_gic_enable_intr(WDOG_NSEC_BITE_INT_ID);
#endif
}
