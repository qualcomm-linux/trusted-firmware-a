/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Low-level clock register descriptor types for the QTI clock driver. These
 * used to live behind a HAL abstraction; the register accessors are now folded
 * directly into the driver, so only the plain data descriptors remain here.
 */

#ifndef CLOCK_TYPES_H
#define CLOCK_TYPES_H

#include <stdbool.h>
#include <stdint.h>

/*
 * Register field definitions, folded in from the former HAL layer.
 */

/* Branch (CBCR) register fields. */
#define HAL_CLK_BRANCH_CTRL_REG_CLK_OFF_FMSK	0x80000000U
#define HAL_CLK_BRANCH_CTRL_REG_CLK_HW_CTL_FMSK	0x00000002U
#define HAL_CLK_BRANCH_CTRL_REG_CLK_ENABLE_FMSK	0x00000001U

/* GDSCR (power domain) register fields. */
#define HAL_CLK_GDSCR_PWR_ON_FMSK		0x80000000U
#define HAL_CLK_GDSCR_SW_COLLAPSE_FMSK		0x00000001U

/* PLL mode register fields. */
#define HAL_CLK_PLL_MODE_PLL_ACTIVE_FLAG_BMSK	0x40000000U

/* Clock source enumeration. */
enum clock_source_id {
	CLOCK_SOURCE_XO			= 0,
	CLOCK_SOURCE_GPLL0		= 1,
	CLOCK_SOURCE_GPLL0_DIV2		= 2,
	CLOCK_SOURCE_SCCPLL		= 3,
	CLOCK_SOURCE_SCCPLL_EVEN	= 4,
	CLOCK_SOURCE_GPLL6		= 5,
	CLOCK_SOURCE_GPLL4		= 6,
	CLOCK_SOURCE_TOTAL
};

/* PLL source descriptor. */
struct clock_source_desc {
	uintptr_t	mode_addr;
	uint32_t	vote_mask;
	uint32_t	active_mask;
};

/* Register address/mask pair (vote register). */
struct clock_register_mask {
	uint32_t	addr;
	uint32_t	mask;
};

/* Clock (CBCR) descriptor. */
struct clock_clk_desc {
	uintptr_t			cbcr_addr;
	struct clock_register_mask	vote_reg;
	bool				tz_enabled;
};

/* Power domain (GDSCR) descriptor. */
struct clock_power_domain_desc {
	uintptr_t	gdscr_addr;
	bool		tz_enabled;
};

#endif /* CLOCK_TYPES_H */
