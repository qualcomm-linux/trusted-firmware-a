/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>

#include <lib/mmio.h>

#include <platform_def.h>

static void core_hang_detection_init(void)
{
#if CORE_HANG_DETECTION_EN
	/* Read CPU present mask from hardware register */
	uint32_t cpu_mask = mmio_read_32(APSS_SHARED_CPU_PRESENT) & 0xFF;

	for (unsigned int cpu = 0; cpu < PLATFORM_CORE_COUNT; cpu++) {
		if (!(cpu_mask & (1U << cpu)))
			continue;

		uintptr_t cfg_addr = APSS_ALIAS_n_CORE_HANG_CONFIG(cpu);
		uintptr_t thr_addr = APSS_ALIAS_n_CORE_HANG_THRESHOLD(cpu);

		/* The reset bit is sticky: pulse it 1 then 0 before enabling. */
		mmio_write_32(cfg_addr, (1U << CORE_HANG_CONFIG_CORE_HANG_RESET_SHIFT));
		mmio_write_32(cfg_addr, 0);
		mmio_write_32(thr_addr, CORE_HANG_THRESHOLD_VALUE);
		mmio_write_32(cfg_addr, (1U << CORE_HANG_CONFIG_CORE_HANG_COUNTER_EN_SHIFT));
	}
#endif
}

static void ace_hang_detection_init(void)
{
#if CPUSS_ACE_HANG_DETECTION_EN
	mmio_write_32(APSS_SHARED_GNOC_HANG_CONFIG_ADDR, 0);
	mmio_write_32(APSS_SHARED_GNOC_HANG_THRESHOLD_ADDR, CPUSS_ACE_HANG_THRESHOLD_VALUE);
	mmio_write_32(APSS_SHARED_GNOC_HANG_CONFIG_ADDR,
		      (1U << GNOC_HANG_CONFIG_ACE_COUNTER_EN_SHIFT));
#endif
}

static void acg_init(void)
{
#if BRUIN_ACG_EN
	/* Hysteresis settings are the same across all SoCs. */
	mmio_write_32(APSS_CL_SCLK_QCHANNEL_FSM_HYST_CTL_ADDR,
		(SCLK_QCHANNEL_ACG_HYST_CNT << QCHANNEL_FSM_HYST_CTL_ACG_HYST_CNT_SHFT) |
		(SCLK_QCHANNEL_CBC_HYST_CNT << QCHANNEL_FSM_HYST_CTL_CBC_HYST_CNT_SHFT));
	mmio_write_32(APSS_CL_GNOC_QCHANNEL_FSM_HYST_CTL_ADDR,
		(GNOC_QCHANNEL_ACG_HYST_CNT << QCHANNEL_FSM_HYST_CTL_ACG_HYST_CNT_SHFT) |
		(GNOC_QCHANNEL_CBC_HYST_CNT << QCHANNEL_FSM_HYST_CTL_CBC_HYST_CNT_SHFT));
	mmio_write_32(APSS_CL_GICCLK_QCHANNEL_FSM_HYST_CTL_ADDR,
		(GICCLK_QCHANNEL_ACG_HYST_CNT << QCHANNEL_FSM_HYST_CTL_ACG_HYST_CNT_SHFT) |
		(GICCLK_QCHANNEL_CBC_HYST_CNT << QCHANNEL_FSM_HYST_CTL_CBC_HYST_CNT_SHFT));
	mmio_write_32(APSS_CL_PCLK_QCHANNEL_FSM_HYST_CTL_ADDR,
		(PCLK_QCHANNEL_ACG_HYST_CNT << QCHANNEL_FSM_HYST_CTL_ACG_HYST_CNT_SHFT) |
		(PCLK_QCHANNEL_CBC_HYST_CNT << QCHANNEL_FSM_HYST_CTL_CBC_HYST_CNT_SHFT));
	mmio_write_32(APSS_CL_ATCLK_QCHANNEL_FSM_HYST_CTL_ADDR,
		(ATCLK_QCHANNEL_ACG_HYST_CNT << QCHANNEL_FSM_HYST_CTL_ACG_HYST_CNT_SHFT) |
		(ATCLK_QCHANNEL_CBC_HYST_CNT << QCHANNEL_FSM_HYST_CTL_CBC_HYST_CNT_SHFT));

	/*
	 * ACG enable bits: GNOC/SCLK/ATCLK are active-high, GICCLK/PCLK are
	 * active-low (CLK_ON mirrors the inverse of ACG_EN in all cases).
	 * GNOC_ACG_DISABLE/SCLK_ACG_DISABLE replace the original's runtime
	 * SOC_ID override that force-disabled GNOC/SCLK ACG on this family.
	 */
	unsigned int gnoc_acg_en = BRUIN_GNOC_ACG_EN && !GNOC_ACG_DISABLE;
	unsigned int sclk_acg_en = BRUIN_SCLK_ACG_EN && !SCLK_ACG_DISABLE;

	uint32_t val =
		((gnoc_acg_en ? 1U : 0U)         << QCHANNEL_FSM_EN_CTL_GNOC_ACG_EN_SHFT) |
		((gnoc_acg_en ? 0U : 1U)         << QCHANNEL_FSM_EN_CTL_GNOC_CLK_ON_SHFT) |
		((BRUIN_PCLK_ACG_EN ? 0U : 1U)   << QCHANNEL_FSM_EN_CTL_PCLK_ACG_EN_SHFT) |
		((BRUIN_PCLK_ACG_EN ? 0U : 1U)   << QCHANNEL_FSM_EN_CTL_PCLK_CLK_ON_SHFT) |
		((BRUIN_GICCLK_ACG_EN ? 0U : 1U) << QCHANNEL_FSM_EN_CTL_GICCLK_ACG_EN_SHFT) |
		((BRUIN_GICCLK_ACG_EN ? 0U : 1U) << QCHANNEL_FSM_EN_CTL_GICCLK_CLK_ON_SHFT) |
		((BRUIN_ATCLK_ACG_EN ? 1U : 0U)  << QCHANNEL_FSM_EN_CTL_ATCLK_ACG_EN_SHFT) |
		((BRUIN_ATCLK_ACG_EN ? 0U : 1U)  << QCHANNEL_FSM_EN_CTL_ATCLK_CLK_ON_SHFT) |
		((sclk_acg_en ? 1U : 0U)         << QCHANNEL_FSM_EN_CTL_SCLK_ACG_EN_SHFT) |
		((sclk_acg_en ? 0U : 1U)         << QCHANNEL_FSM_EN_CTL_SCLK_CLK_ON_SHFT);

	mmio_write_32(APSS_CL_QCHANNEL_FSM_EN_CTL_ADDR, val);
#endif
}

static void errors_as_fatal_init(void)
{
#if CPUSS_ERRORS_AS_FATAL_EN
	uint32_t val =
		(1U << WDOG_BITE_INT0_CONFIG_GIC_FAULT_INT_SEL_SHFT) |
		(1U << WDOG_BITE_INT0_CONFIG_GIC_ERR_INT_SEL_SHFT) |
		(1U << WDOG_BITE_INT0_CONFIG_GNOC_HANG_STS_SEL_SHFT) |
		(1U << WDOG_BITE_INT0_CONFIG_CORE_HANG_STS_SEL_SHFT) |
		(1U << WDOG_BITE_INT0_CONFIG_WDOG_BITE_INT_SEL_SHFT);

	mmio_write_32(APSS_SHARED_WDOG_BITE_INT0_CONFIG_ADDR, val);
#endif
}

static void gic_fault_err_intr_init(void)
{
#if GIC_FAULT_ERR_INTR_EN
	/* ERR Record 1: correctable SPI RAM errors. */
	mmio_write_32(APSS_GICT_ERR1CTLR_ADDR, (1U << GICT_ERR1CTLR_CFI_SHFT));

	/* ERR Record 2: uncorrectable SPI RAM errors. */
	mmio_write_32(APSS_GICT_ERR2CTLR_ADDR,
		      (1U << GICT_ERR2CTLR_FI_SHFT) | (1U << GICT_ERR2CTLR_UI_SHFT));
#endif
}

void plat_cpuss_late_config(void)
{
	core_hang_detection_init();
	ace_hang_detection_init();
	acg_init();
	errors_as_fatal_init();
	gic_fault_err_intr_init();
}
