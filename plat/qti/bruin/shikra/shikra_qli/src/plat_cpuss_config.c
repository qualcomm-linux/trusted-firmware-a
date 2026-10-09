/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>

#include <qti_plat.h>

/*
 * APSS_SPI_CONFIG_BASE is defined in platform_def.h (APSS_INTU_BASE + 0x00F0).
 * Each bit represents one SPI: bit=1 → level-triggered (default),
 * 19 words cover SPI[0:607] (VECTOR[63:32] through VECTOR[639:608]).
 * bit=1 → edge-triggered; configure_irq_arr() writes ~cfg_arr[i] to each
 * APSS_SHARED_SPI_CONFIG_n register.
 */
static const uint32_t bruin_spi_cfg[APSS_SPI_NUM_WORDS] = {
	0x00018000U,	/* [32*00+32+31:32*00+32+0] = VECTOR[063:032] : SPI[031:000] */
	0x00001000U,	/* [32*01+32+31:32*01+32+0] = VECTOR[095:064] : SPI[063:032] */
	0x000004FFU,	/* [32*02+32+31:32*02+32+0] = VECTOR[127:096] : SPI[095:064] */
	0x00000000U,	/* [32*03+32+31:32*03+32+0] = VECTOR[159:128] : SPI[127:096] */
	0xE0000000U,	/* [32*04+32+31:32*04+32+0] = VECTOR[191:160] : SPI[159:128] */
	0x00000000U,	/* [32*05+32+31:32*05+32+0] = VECTOR[223:192] : SPI[191:160] */
	0x3C3FFFFCU,	/* [32*06+32+31:32*06+32+0] = VECTOR[255:224] : SPI[223:192] */
	0x00600000U,	/* [32*07+32+31:32*07+32+0] = VECTOR[287:256] : SPI[255:224] */
	0x000009E0U,	/* [32*08+32+31:32*08+32+0] = VECTOR[319:288] : SPI[287:256] */
	0x007F8000U,	/* [32*09+32+31:32*09+32+0] = VECTOR[351:320] : SPI[319:288] */
	0x00000000U,	/* [32*10+32+31:32*10+32+0] = VECTOR[383:352] : SPI[351:320] */
	0x00000010U,	/* [32*11+32+31:32*11+32+0] = VECTOR[415:384] : SPI[383:352] */
	0x00000000U,	/* [32*12+32+31:32*12+32+0] = VECTOR[447:416] : SPI[415:384] */
	0x3FC03F00U,	/* [32*13+32+31:32*13+32+0] = VECTOR[479:448] : SPI[447:416] */
	0x00000000U,	/* [32*14+32+31:32*14+32+0] = VECTOR[511:480] : SPI[479:448] */
	0x00000000U,	/* [32*15+32+31:32*15+32+0] = VECTOR[543:512] : SPI[511:480] */
	0x00000000U,	/* [32*16+32+31:32*16+32+0] = VECTOR[575:544] : SPI[543:512] */
	0x00000000U,	/* [32*17+32+31:32*17+32+0] = VECTOR[607:576] : SPI[575:544] */
	0x00000780U,	/* [32*18+32+31:32*18+32+0] = VECTOR[639:608] : SPI[607:576] */
};

