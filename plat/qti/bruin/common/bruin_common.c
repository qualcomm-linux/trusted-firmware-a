/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <lib/mmio.h>

#include <platform_def.h>

extern const uint32_t bruin_spi_cfg[APSS_SPI_NUM_WORDS];

/*
 * configure_irq_arr() - Program SPI edge/level type via direct write.
 * cfg_arr[] convention: bit=1 → level-triggered, bit=0 → edge-triggered.
 * Values are written directly to APSS_SHARED_SPI_CONFIG_n registers.
 * A readback follows each write to ensure the configuration is visible
 * before the GIC distributor is enabled.
 */
static void configure_irq_arr(uintptr_t base, const uint32_t *cfg_arr,
		       unsigned int num_words)
{
	unsigned int i;

	for (i = 0U; i < num_words; i++) {
		mmio_write_32(base + 4U * i, cfg_arr[i]);
		mmio_read_32(base + 4U * i);
	}
}

/* Configures platform CPUSS specific configurations */
void plat_cpuss_early_config(void)
{
        configure_irq_arr(APSS_SPI_CONFIG_BASE, bruin_spi_cfg, APSS_SPI_NUM_WORDS);
}
