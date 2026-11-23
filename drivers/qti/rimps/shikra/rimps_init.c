/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * RIMPS CPU PLL bring-up for shikra.
 *
 * The vendor TZ reference enables each CPU PLL and parks its domain's mux on
 * it at a safe frequency before handing off to the full OSM/DCVS runtime
 * ("The CPU PLL needs to be configured/enabled prior to enabling OSM, so set
 * an initial safe frequency to run from it."). RIMPS's power-collapse
 * opcode sequences carry that same assumption forward: they expect the PLLs
 * already locked and outputting when a core first idles. This file ports
 * only that static bring-up slice (Clock_EnableCPUPLL/Clock_SetCPUFrequency
 * for the cold-boot safe frequency); the LUT-driven DCVS runtime itself,
 * the CPUCP firmware that implements it, and ACD/AVG/BCL/APM are not
 * ported.
 *
 * PLL_CONFIG_CTL/PLL_TEST_CTL/PLL_USER_CTL are left at their power-on-reset
 * values, same as the vendor reference (its own comment marks them
 * "not urgent for RUMI"); this is carried forward as an open caveat rather
 * than silently resolved.
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <lib/mmio.h>
#include <lib/utils_def.h>

#include <drivers/qti/rimps/rimps.h>

#include "rimps_hwio.h"

/* Cold-boot safe CPU PLL frequency: 652.8 MHz / 19.2 MHz XO = L-val 34. */
#define RIMPS_COLD_BOOT_L_VAL		34U

/* CFG_GFMUX source select to run the domain from its CPU PLL. */
#define RIMPS_GFMUX_SRC_CPUPLL_EARLY	2U

#define RIMPS_PLL_LOCK_POLL_RETRIES		1000U

struct rimps_pll_domain {
	uintptr_t base;
	bool is_gold;
};

static const struct rimps_pll_domain rimps_domains[] = {
	{ .base = L3_APSS_KRYO_CLK_CTL_BASE,     .is_gold = false },
	{ .base = SILVER_APSS_KRYO_CLK_CTL_BASE, .is_gold = false },
	{ .base = GOLD_APSS_KRYO_CLK_CTL_BASE,   .is_gold = true  },
};

static void rimps_enable_cpu_pll(const struct rimps_pll_domain *domain)
{
	uint32_t retry = RIMPS_PLL_LOCK_POLL_RETRIES;

	if (domain->is_gold) {
		mmio_write_32(GOLD_PLL_APC_PDN, 0U);
	}

	mmio_setbits_32(PLL_MODE(domain->base), PLL_MODE_BYPASSNL_BMSK);
	udelay(5U);
	mmio_setbits_32(PLL_MODE(domain->base), PLL_MODE_RESET_N_BMSK);

	while (((mmio_read_32(PLL_MODE(domain->base)) &
		 PLL_MODE_PLL_LOCK_DET_BMSK) == 0U) &&
	       (--retry > 0U)) {
		udelay(1U);
	}

	if (retry == 0U) {
		WARN("rimps: CPU PLL @ 0x%lx lock timed out\n", domain->base);
	}

	mmio_setbits_32(PLL_MODE(domain->base), PLL_MODE_OUTCTRL_BMSK);
}

void rimps_cpu_pll_init(void)
{
	size_t i;

	for (i = 0U; i < ARRAY_SIZE(rimps_domains); i++) {
		const struct rimps_pll_domain *domain = &rimps_domains[i];

		mmio_write_32(PLL_L_VAL(domain->base), RIMPS_COLD_BOOT_L_VAL);
		rimps_enable_cpu_pll(domain);
		mmio_clrsetbits_32(CFG_GFMUX(domain->base),
				   CFG_GFMUX_GFMUX_SRC_SEL_LF_BMSK,
				   RIMPS_GFMUX_SRC_CPUPLL_EARLY <<
				   CFG_GFMUX_GFMUX_SRC_SEL_LF_SHFT);
	}
}
