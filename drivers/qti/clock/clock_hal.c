/*
 * Copyright (c) 2026 Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Low-level clock register accessors for the QTI clock driver.
 *
 * These routines used to live behind a separate HAL abstraction (HALclk.c).
 * They are now folded directly into the driver and operate on the plain
 * descriptor structures declared in <drivers/qti/clock/clock_types.h>. The
 * address of the shared PLL vote register is chipset specific and is supplied
 * by the back-end through clock_hal_get_pll_vote_addr().
 */

#include <stdbool.h>
#include <stdint.h>

#include <drivers/delay_timer.h>
#include <drivers/qti/clock/clock_bsp.h>
#include <drivers/qti/clock/clock_driver.h>
#include <drivers/qti/clock/clock_types.h>
#include <lib/mmio.h>

void clock_hal_enable_clock(struct clock_clk_desc *clock)
{
	uintptr_t addr;
	uint32_t val, mask;

	if (clock->vote_reg.addr != 0U) {
		addr = clock->vote_reg.addr;
		mask = clock->vote_reg.mask;
	} else {
		addr = clock->cbcr_addr;
		mask = HAL_CLK_BRANCH_CTRL_REG_CLK_ENABLE_FMSK;
	}

	if (addr != 0U) {
		val = mmio_read_32(addr);
		/* Note if TZ enabled this clock. */
		clock->tz_enabled = ((val & mask) == 0U);
		mmio_write_32(addr, val | mask);
	}
}

void clock_hal_disable_clock(struct clock_clk_desc *clock)
{
	uintptr_t addr;
	uint32_t mask;

	if (clock->vote_reg.addr != 0U) {
		addr = clock->vote_reg.addr;
		mask = clock->vote_reg.mask;
	} else {
		addr = clock->cbcr_addr;
		mask = HAL_CLK_BRANCH_CTRL_REG_CLK_ENABLE_FMSK;
	}

	if (addr != 0U) {
		mmio_write_32(addr, mmio_read_32(addr) & ~mask);
	}

	clock->tz_enabled = false;
}

void clock_hal_enable_source(struct clock_source_desc *source)
{
	uintptr_t addr;

	/* Only votable PLLs are supported. */
	if (source->vote_mask == 0U) {
		return;
	}

	addr = clock_hal_get_pll_vote_addr();
	mmio_write_32(addr, mmio_read_32(addr) | source->vote_mask);

	addr = source->mode_addr;
	if (addr != 0U) {
		while ((mmio_read_32(addr) &
			HAL_CLK_PLL_MODE_PLL_ACTIVE_FLAG_BMSK) == 0U) {
		}
	}
}

void clock_hal_disable_source(struct clock_source_desc *source)
{
	uintptr_t addr;

	/* Only votable PLLs are supported. */
	if (source->vote_mask == 0U) {
		return;
	}

	addr = clock_hal_get_pll_vote_addr();
	if (addr != 0U) {
		mmio_write_32(addr, mmio_read_32(addr) & ~source->vote_mask);
	}
}

int clock_hal_is_clock_on(struct clock_clk_desc *clock)
{
	if (clock->cbcr_addr == 0U) {
		return 0;
	}

	if ((mmio_read_32(clock->cbcr_addr) &
	     HAL_CLK_BRANCH_CTRL_REG_CLK_OFF_FMSK) != 0U) {
		return -1;
	}

	return 0;
}

int clock_hal_wait_for_clock_on(struct clock_clk_desc *clock)
{
	uint32_t retry = 100U;

	if (clock->cbcr_addr == 0U) {
		return -1;
	}

	/*
	 * Skip polling if HW_CTL is set, since the clock will only enable
	 * based on traffic and/or usage.
	 */
	if ((mmio_read_32(clock->cbcr_addr) &
	     HAL_CLK_BRANCH_CTRL_REG_CLK_HW_CTL_FMSK) != 0U) {
		return 0;
	}

	while ((clock_hal_is_clock_on(clock) != 0) && (--retry > 0U)) {
		udelay(1U);
	}

	if (retry == 0U) {
		return -1;
	}

	return 0;
}

void clock_hal_enable_power_domain(struct clock_power_domain_desc *power_domain)
{
	uintptr_t addr;
	uint32_t val, mask;

	addr = power_domain->gdscr_addr;
	mask = HAL_CLK_GDSCR_SW_COLLAPSE_FMSK;
	if (addr != 0U) {
		val = mmio_read_32(addr);
		power_domain->tz_enabled = ((val & mask) != 0U);
		mmio_write_32(addr, val & ~mask);
	}
}

void clock_hal_disable_power_domain(struct clock_power_domain_desc *power_domain)
{
	uintptr_t addr;

	addr = power_domain->gdscr_addr;
	if (addr != 0U) {
		mmio_write_32(addr, mmio_read_32(addr) |
				    HAL_CLK_GDSCR_SW_COLLAPSE_FMSK);
	}

	power_domain->tz_enabled = false;
}

int clock_hal_is_power_domain_on(struct clock_power_domain_desc *power_domain)
{
	if (power_domain->gdscr_addr == 0U) {
		return -1;
	}

	if ((mmio_read_32(power_domain->gdscr_addr) &
	     HAL_CLK_GDSCR_PWR_ON_FMSK) == 0U) {
		return -1;
	}

	return 0;
}

int clock_hal_wait_for_power_domain_on(struct clock_power_domain_desc *power_domain)
{
	uint32_t retry = 10000U;

	/*
	 * Wait at least 8 XO cycles before checking the status, since it is
	 * not reliable immediately after powering on the domain.
	 */
	udelay(1U);

	while ((clock_hal_is_power_domain_on(power_domain) == 0) &&
	       (--retry > 0U)) {
		udelay(1U);
	}

	if (retry == 0U) {
		return -1;
	}

	return 0;
}
