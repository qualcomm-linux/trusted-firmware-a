/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <lib/mmio.h>

#include <qti_plat.h>

/*
 * configure_irq_arr() - Program SPI edge/level type via direct write.
 * cfg_arr[] convention: bit=1 → edge-triggered, bit=0 → level-triggered.
 */
void configure_irq_arr(uintptr_t base, const uint32_t *cfg_arr,
		       unsigned int num_words)
{
	unsigned int i;

	for (i = 0U; i < num_words; i++) {
		mmio_write_32(base + 4U * i, cfg_arr[i] ^ 0xFFFFFFFFU);
	}
}

/* Configures platform CPUSS specific configurations */
void plat_cpuss_config(void)
{
        configure_irq_arr(APSS_SPI_CONFIG_BASE, bruin_spi_cfg, APSS_SPI_NUM_WORDS);
}
