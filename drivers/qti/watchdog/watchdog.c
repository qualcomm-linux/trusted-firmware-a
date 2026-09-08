/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <inttypes.h>

#include <arch_helpers.h>
#include <common/debug.h>
#include <drivers/arm/gicv3.h>
#include <drivers/qti/qtimer/qtimer.h>
#include <drivers/qti/watchdog/qcom_wdt_layout.h>
#include <drivers/qti/watchdog/watchdog.h>
#include <lib/mmio.h>
#include <lib/spinlock.h>
#include <lib/utils_def.h>
#include <platform.h>
#include <watchdog_defs.h>

#include <qti_interrupt_svc.h>
#include <qti_plat.h>

#define WDOG_BITE_TIME_MS	22000U
#define WDOG_BARK_TIME_MS	6000U
#define MPIDR_INVALID		0xDEAD

uint64_t qti_watchdog_pet_ticks __section("tzfw_coherent_mem");

static u_register_t mpidr[PLATFORM_CORE_COUNT] = {
	[0 ... PLATFORM_CORE_COUNT - 1] = MPIDR_INVALID
};

/*
 * Per-target register-offset table (indexed by enum wdt_reg), supplied by the
 * target's watchdog_defs.h as WDOG_REG_OFFSETS. Modelled on the upstream Linux
 * qcom-wdt.c reg_offset_data_* tables.
 */
static const uint32_t qcom_wdt_reg_offset[WDT_REG_COUNT] = WDOG_REG_OFFSETS;

static uintptr_t wdt_addr(enum wdt_reg reg)
{
	return WDOG_REG_BASE + qcom_wdt_reg_offset[reg];
}

static uint32_t ms_to_wdt_ticks(uint32_t ms)
{
	return (uint32_t)((WDOG_FREQ_IN_HZ * (uint64_t)ms) / 1000ULL);
}

#if WDOG_HAS_SYNC
/*
 * Classic AOSS windowed watchdog. The enable bit lives in a control
 * register (WDT_EN) alongside a clock-enable bit; the bark/bite time
 * registers latch asynchronously and raise a sync bit (bit 31) once the
 * write has taken effect. Keep the original program-and-poll sequence.
 */
static void wdog_program_start(uint32_t bark, uint32_t bite)
{
	mmio_clrbits_32(wdt_addr(WDT_EN), QCOM_WDT_ENABLE);
	mmio_clrsetbits_32(wdt_addr(WDT_BARK_TIME), WDOG_TIME_MASK, bark);
	while ((mmio_read_32(wdt_addr(WDT_BARK_TIME)) & WDOG_SYNC_BIT) == 0U) {
	}

	mmio_clrsetbits_32(wdt_addr(WDT_BITE_TIME), WDOG_TIME_MASK, bite);
	while ((mmio_read_32(wdt_addr(WDT_BITE_TIME)) & WDOG_SYNC_BIT) == 0U) {
	}

	mmio_setbits_32(wdt_addr(WDT_EN), QCOM_WDT_ENABLE);
	mmio_setbits_32(wdt_addr(WDT_EN), WDOG_CLK_ENABLE_BIT);
	mmio_write_32(wdt_addr(WDT_RST), 1U);
}

static void wdog_disable(void)
{
	mmio_clrbits_32(wdt_addr(WDT_EN), QCOM_WDT_ENABLE);
}
#else
/*
 * Simple enable/pet/timeout block (e.g. Nord APSS_WDT_SEC_WWDOG). Sequence
 * mirrors Linux qcom_wdt_start(): disable, pet, program the bark/bite
 * timeouts, then enable. The time registers are plain counters (no async
 * sync bit), and there is no separate clock-enable.
 */
static void wdog_program_start(uint32_t bark, uint32_t bite)
{
	mmio_write_32(wdt_addr(WDT_EN), 0U);
	mmio_write_32(wdt_addr(WDT_RST), 1U);
	mmio_write_32(wdt_addr(WDT_BARK_TIME), bark);
	mmio_write_32(wdt_addr(WDT_BITE_TIME), bite);
	mmio_write_32(wdt_addr(WDT_EN), QCOM_WDT_ENABLE);
}

static void wdog_disable(void)
{
	mmio_write_32(wdt_addr(WDT_EN), 0U);
}
#endif

void qti_watchdog_start(uint32_t bark_ms, uint32_t bite_ms)
{
	uint32_t bark, bite;

	bark = ms_to_wdt_ticks(MAX(bark_ms, 0x1U));
	bite = ms_to_wdt_ticks(MAX(bite_ms, 0x1U));
	bark = MIN(bark, (uint32_t)WDOG_MAX_TICK_COUNT);
	bite = MIN(bite, (uint32_t)WDOG_MAX_TICK_COUNT);

	wdog_program_start(bark, bite);
	dsb();
}

void qti_watchdog_stop(void)
{
	mmio_write_32(wdt_addr(WDT_RST), 1U);
	wdog_disable();
	dsb();
}

void qti_watchdog_pet(void)
{
	qti_watchdog_pet_ticks = qti_qtimer_get_raw();
	mmio_write_32(wdt_addr(WDT_RST), 1U);
	dsb();
}

static void *bark_handler(uint32_t id, void *ctx)
{
	if (id != WDOG_BARK_INT_ID)
		return NULL;

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
	} cpu;
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

		/* wdog currently in sleep, move it to this core */
		if (!(cpu.state.on & (1 << cpu.watchdog))) {
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

#if WDOG_HAS_CTL_INIT
/*
 * Classic block: one-time control programming (chip auto-pet + HW
 * sleep/wakeup enable) before the watchdog is started.
 */
static void wdog_ctl_init(void)
{
	mmio_write_32(wdt_addr(WDT_EN), WDOG_CTL_INIT_VAL);
}
#else
static void wdog_ctl_init(void)
{
}
#endif

int qti_watchdog_init(void)
{
	int ret;

	wdog_ctl_init();
	ret = qti_interrupt_svc_register(WDOG_BARK_INT_ID, bark_handler, NULL);
	if (ret) {
		ERROR("Failure registering watchdog\n");
		return ret;
	}

	qti_watchdog_set_target(QTI_WATCHDOG_CPU_WAKEUP);
	qti_watchdog_start(WDOG_BARK_TIME_MS, WDOG_BITE_TIME_MS);

	return 0;
}
