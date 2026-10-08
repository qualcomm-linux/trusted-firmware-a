/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>

/*
 * APSS_SHARED_SPI_CONFIG_n register format (APSS_SPI_CONFIG_BASE + 4*n):
 *   bit=1 → level-triggered (hardware default for all SPIs)
 *   bit=0 → edge-triggered  (exception, explicitly cleared)
 * Values below are written directly to the hardware registers.
 * 19 words cover SPI[0:607] (VECTOR[63:32] through VECTOR[639:608]).
 * Edge-triggered SPIs (bit=0) per word.
 */
const uint32_t bruin_spi_cfg[APSS_SPI_NUM_WORDS] = {
	0xFFFE7FFFU,	/* [32*00+32+31:32*00+32+0] = VECTOR[063:032] : SPI[031:000] */
	0xFFFFEFFFU,	/* [32*01+32+31:32*01+32+0] = VECTOR[095:064] : SPI[063:032] */
	0xFFFFFB00U,	/* [32*02+32+31:32*02+32+0] = VECTOR[127:096] : SPI[095:064] */
	0xFFFFFFFFU,	/* [32*03+32+31:32*03+32+0] = VECTOR[159:128] : SPI[127:096] */
	0x1FFFFFFFU,	/* [32*04+32+31:32*04+32+0] = VECTOR[191:160] : SPI[159:128] */
	0xFFFFFFFFU,	/* [32*05+32+31:32*05+32+0] = VECTOR[223:192] : SPI[191:160] */
	0xC3C00003U,	/* [32*06+32+31:32*06+32+0] = VECTOR[255:224] : SPI[223:192] */
	0xFF9FFFFFU,	/* [32*07+32+31:32*07+32+0] = VECTOR[287:256] : SPI[255:224] */
	0xFFFFF61FU,	/* [32*08+32+31:32*08+32+0] = VECTOR[319:288] : SPI[287:256] */
	0xFF807FFFU,	/* [32*09+32+31:32*09+32+0] = VECTOR[351:320] : SPI[319:288] */
	0xFFFFFFFFU,	/* [32*10+32+31:32*10+32+0] = VECTOR[383:352] : SPI[351:320] */
	0xFFFFFFEFU,	/* [32*11+32+31:32*11+32+0] = VECTOR[415:384] : SPI[383:352] */
	0xFFFFFFFFU,	/* [32*12+32+31:32*12+32+0] = VECTOR[447:416] : SPI[415:384] */
	0xC03FC0FFU,	/* [32*13+32+31:32*13+32+0] = VECTOR[479:448] : SPI[447:416] */
	0xFFFFFFFFU,	/* [32*14+32+31:32*14+32+0] = VECTOR[511:480] : SPI[479:448] */
	0xFFFFFFFFU,	/* [32*15+32+31:32*15+32+0] = VECTOR[543:512] : SPI[511:480] */
	0xFFFFFFFFU,	/* [32*16+32+31:32*16+32+0] = VECTOR[575:544] : SPI[543:512] */
	0xFFFFFFFFU,	/* [32*17+32+31:32*17+32+0] = VECTOR[607:576] : SPI[575:544] */
	0xFFFFF87FU,	/* [32*18+32+31:32*18+32+0] = VECTOR[639:608] : SPI[607:576] */
};

