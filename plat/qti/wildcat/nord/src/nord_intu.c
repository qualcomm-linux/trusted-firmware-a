/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * plat_intu_init() - APSS Interrupt Unit (INTU) bring-up for SA8797P "Nord" V2.
 *
 * The INTU (@0x17840000, secure-only block: SHR_SECURE rst 0x1FFFF all-NSEC,
 * CLK_GATE_OVRD rst 0x3F all-clocks-on) conditions peripheral SPIs (per-SPI
 * edge/level latching + merger) before they reach GIC-700. Reset values
 * already enable forwarding, so NO global-enable / reset-release write is
 * needed. Our open BL31 was missing ONLY this per-SPI type programming - the
 * full Hoya CPUSS init was stubbed to no-ops because parts hang on Oryon - so
 * level SPIs (UFS 265/INTID297, geni 615, RSC 61) were left mis-typed at reset
 * and never delivered/cleared to the GIC.
 *
 * The HW registers are 64-bit (stride 0x8) but are written as 32-bit stores at
 * +4 stride, filling consecutive low/high halves.
 *
 * Call from bl31_platform_setup() after plat_qti_gic_init(), BEFORE the GICD
 * group-enable, so SPIs latch with the correct type. Does not touch the GIC
 * distributor.
 */

#include <stdint.h>

#include <arch_helpers.h>
#include <lib/mmio.h>

#include <qti_plat.h>

/* INTU base = HWIO_APSS_HM_BASE(0x17000000) + 0x840000 */
#define APSS_INTU_BASE			0x17840000U

/* Per-SPI type arrays (+4*n stride) */
#define APSS_SET_LEVEL_BASE		(APSS_INTU_BASE + 0x3000U)	/* 0x17843000 */
#define APSS_CLR_EDGE_BASE		(APSS_INTU_BASE + 0x3400U)	/* 0x17843400 */
#define APSS_MRG_SET_LEVEL_BASE		(APSS_INTU_BASE + 0x4000U)	/* 0x17844000 */
#define APSS_MRG_CLR_EDGE_BASE		(APSS_INTU_BASE + 0x4400U)	/* 0x17844400 */
#define APSS_MRG_DIRECT_SPI_CFG		(APSS_INTU_BASE + 0x5C00U)	/* 0x17845C00 */

/*
 * SPI_CONFIGURATION_DATA_NORDV2 (57 words), Revision
 * nordschleife_v2.0_p3q3r85_MTO (matches IPcat IRQ map id 3711).
 * Word 8 bit 9 = 0 -> UFS SPI 265 = LEVEL.
 */
static const uint32_t nordv2_spi_type[57] = {
	0x001F8000U, 0x0000A000U, 0x00000000U, 0x0C000220U, 0x88000000U, 0x1004300CU, 0x00000890U, 0x00140000U,
	0x20608000U, 0x00FC080AU, 0x02000000U, 0x03000000U, 0x20180000U, 0x00009C13U, 0xE000801FU, 0xE0060000U,
	0x00010000U, 0x00000000U, 0xEA000000U, 0x00000001U, 0x01823000U, 0x0C000001U, 0x00000100U, 0x00000006U,
	0x08000000U, 0x0C000000U, 0x00048001U, 0x00000000U, 0x000007F6U, 0x8FC80000U, 0xFFFFFFEFU, 0x00000000U,
	0x00000000U, 0x00000000U, 0x00000000U, 0x00000000U, 0x00000000U, 0x00000000U, 0x00000000U, 0x00000000U,
	0x00000000U, 0x00000000U, 0x00000000U, 0x00000000U, 0x00000000U, 0x00000000U, 0x00000000U, 0x00000000U,
	0x00000000U, 0x00000000U, 0x00000000U, 0x00000000U, 0x00000080U, 0x00000000U, 0x00000000U, 0xDA000500U,
	0x00000002U,
};

/* SPI_MERGER_LOGIC_CONFIGURATION_DATA_NORD_SECA (6 words) */
static const uint32_t nord_seca_merger_type[6] = {
	0x00000024U, 0x7F000000U, 0x0FFF0000U, 0x00000000U, 0x1A000010U, 0xE025B400U,
};

/* DIRECT_SPI_CONFIGURATION_DATA_NORD_SECA (6 words) */
static const uint32_t nord_seca_direct_cfg[6] = {
	0xFFFFFFFFU, 0xFFFFFFFFU, 0xFFFFFFFFU, 0xFFFFFFFFU, 0xFFFFFFFFU, 0xFFFFFFFFU,
};

void plat_intu_init(void)
{
	unsigned int i;

	/* Main SPI edge/level type (covers UFS 265, geni 615, RSC 61).
	 * BIS CLR_EDGE = data ; BIC SET_LEVEL = ~data
	 */
	for (i = 0U; i < 57U; i++) {
		mmio_write_32(APSS_CLR_EDGE_BASE  + 4U * i,  nordv2_spi_type[i]);
		mmio_write_32(APSS_SET_LEVEL_BASE + 4U * i, ~nordv2_spi_type[i]);
	}

	/* Merger-logic SPI type (NORD_SECA). */
	for (i = 0U; i < 6U; i++) {
		mmio_write_32(APSS_MRG_CLR_EDGE_BASE  + 4U * i,  nord_seca_merger_type[i]);
		mmio_write_32(APSS_MRG_SET_LEVEL_BASE + 4U * i, ~nord_seca_merger_type[i]);
	}

	/* Merger direct-SPI passthrough config (all 0xFFFFFFFF = passthrough). */
	for (i = 0U; i < 6U; i++) {
		mmio_write_32(APSS_MRG_DIRECT_SPI_CFG + 4U * i, nord_seca_direct_cfg[i]);
	}

	/* Ensure all INTU config is visible before the GIC distributor enable. */
	dsbsy();
	isb();
}
