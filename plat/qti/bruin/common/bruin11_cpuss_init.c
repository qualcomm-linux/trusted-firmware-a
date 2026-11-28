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
#if defined(CORE_HANG_DETECTION_EN) && (CORE_HANG_DETECTION_EN)
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
#if defined(CPUSS_ACE_HANG_DETECTION_EN) && (CPUSS_ACE_HANG_DETECTION_EN)
	mmio_write_32(APSS_SHARED_GNOC_HANG_CONFIG_ADDR, 0);
	mmio_write_32(APSS_SHARED_GNOC_HANG_THRESHOLD_ADDR, CPUSS_ACE_HANG_THRESHOLD_VALUE);
	mmio_write_32(APSS_SHARED_GNOC_HANG_CONFIG_ADDR,
		      (1U << GNOC_HANG_CONFIG_ACE_COUNTER_EN_SHIFT));
#endif
}

static void errors_as_fatal_init(void)
{
#if defined(CPUSS_ERRORS_AS_FATAL_EN) && (CPUSS_ERRORS_AS_FATAL_EN)
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
#if defined(GIC_FAULT_ERR_INTR_EN) && (GIC_FAULT_ERR_INTR_EN)
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
	/* acg_init() removed - dead code */
	errors_as_fatal_init();
	gic_fault_err_intr_init();
}
