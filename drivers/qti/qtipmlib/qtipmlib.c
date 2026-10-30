/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stddef.h>
#include <stdint.h>

#include <drivers/qti/qtipmlib/qtipmlib.h>
#include <drivers/qti/vpp/vpp.h>
#include <lib/mmio.h>
#include <lib/utils_def.h>

#define QTI_RIMPSSLOW_CLKDOM0_BASE	0x0fd90000UL
#define QTI_RIMPSSLOW_CLKDOM1_BASE	0x0fd91000UL
#define QTI_RIMPSSLOW_CLKDOM2_BASE	0x0fd92000UL
#define QTI_RIMPSTOP_SEC_ACCESS_OVERRIDE	0x0fd98000UL
#define QTI_RIMPSTOP_GLOBAL_ENABLE	0x0fd98004UL

#define QTI_CLKDOM_LUT_FREQ(n)		(0x100U + (4U * (n)))
#define QTI_CLKDOM_LUT_VOLT(n)		(0x200U + (4U * (n)))
#define QTI_OSM_LUT_ROWS		12U

#define QTI_OSM_LUT_FREQ_CLKSRC_SHIFT	30U
#define QTI_OSM_LUT_FREQ_CC_SHIFT	16U
#define QTI_OSM_LUT_FREQ_LVAL_SHIFT	0U
#define QTI_OSM_LUT_VOLT_VC_SHIFT	16U
#define QTI_OSM_LUT_VOLT_OLV_SHIFT	0U

typedef struct {
	uintptr_t base;
	const char *rail_name;
	const char *clk_name;
} qti_rimps_clkdom_t;

static const qti_rimps_clkdom_t qti_rimps_clkdoms[] = {
	{ QTI_RIMPSSLOW_CLKDOM0_BASE, "APC", "L3" },
	{ QTI_RIMPSSLOW_CLKDOM1_BASE, "APC", "Silver" },
	{ QTI_RIMPSSLOW_CLKDOM2_BASE, "APC", "Gold" },
};

static void qti_rimps_program_lut_row(uintptr_t base, uint32_t row,
				      const vpp_osm_t *osm,
				      const vpp_cprh_t *cprh)
{
	uint32_t freq;
	uint32_t volt;

	freq = ((uint32_t)osm->clk_src << QTI_OSM_LUT_FREQ_CLKSRC_SHIFT) |
	       ((uint32_t)osm->max_core_count << QTI_OSM_LUT_FREQ_CC_SHIFT) |
	       ((uint32_t)osm->lval << QTI_OSM_LUT_FREQ_LVAL_SHIFT);
	volt = ((uint32_t)osm->vc << QTI_OSM_LUT_VOLT_VC_SHIFT) |
	       ((uint32_t)cprh->ceil_mV << QTI_OSM_LUT_VOLT_OLV_SHIFT);

	mmio_write_32(base + QTI_CLKDOM_LUT_FREQ(row), freq);
	mmio_write_32(base + QTI_CLKDOM_LUT_VOLT(row), volt);
}

static void qti_rimps_program_lut(const qti_rimps_clkdom_t *clkdom)
{
	const vpp_t *plan = qti_vpp_get_plan(clkdom->rail_name,
					     clkdom->clk_name);
	uint32_t row;

	if ((plan == NULL) || (plan->cnt == 0U)) {
		return;
	}

	for (row = 0U; row < QTI_OSM_LUT_ROWS; row++) {
		uint32_t idx = MIN(row, (uint32_t)plan->cnt - 1U);
		const vpp_osm_t *osm = &plan->osm[idx];
		const vpp_cprh_t *cprh;

		cprh = &plan->cprh[idx];
		qti_rimps_program_lut_row(clkdom->base, row, osm, cprh);
	}
}

static void qti_rimps_program_luts(void)
{
	mmio_write_32(QTI_RIMPSTOP_SEC_ACCESS_OVERRIDE, 0x8U);
	mmio_write_32(QTI_RIMPSTOP_GLOBAL_ENABLE, 0x1U);

	for (uint32_t i = 0U; i < ARRAY_SIZE(qti_rimps_clkdoms); i++) {
		qti_rimps_program_lut(&qti_rimps_clkdoms[i]);
	}
}

void qti_qtipmlib_init(void)
{
	qti_rimps_program_luts();
	(void)qtipmlib_init();
}
