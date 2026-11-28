/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>

#include <platform_def.h>

/*
 * APSS_SHARED_SPI_CONFIG_n register format (APSS_SPI_CONFIG_BASE + 4*n):
 *   bit=1 → level-triggered (hardware default for all SPIs)
 *   bit=0 → edge-triggered  (exception, explicitly cleared)
 * Values below are written directly to the hardware registers.
 * 19 words cover SPI[0:607] (VECTOR[63:32] through VECTOR[639:608]).
 * Edge-triggered SPIs (bit=0) per word.
 */
const uint32_t bruin_spi_cfg[APSS_SPI_NUM_WORDS] = {
	0xFFFE7FFFU,
	0xFFFFEFFFU,
	0xFFFFFB00U,
	0xFFFFFFFFU,
	0x1FFFFFFFU,
	0xFFFFFFFFU,
	0xC3C00003U,
	0xFF9FFFFFU,
	0xFFFFF61FU,
	0xFF807FFFU,
	0xFFFFFFFFU,
	0xFFFFFFEFU,
	0xFFFFFFFFU,
	0xC03FC0FFU,
	0xFFFFFFFFU,
	0xFFFFFFFFU,
	0xFFFFFFFFU,
	0xFFFFFFFFU,
	0xFFFFF87FU,
};

