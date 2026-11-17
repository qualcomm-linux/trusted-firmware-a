/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stddef.h>
#include <stdint.h>

#include <drivers/delay_timer.h>
#include <drivers/qti/qtipmlib/qtipmlib.h>
#include <drivers/qti/vpp/vpp.h>
#include <lib/mmio.h>
#include <lib/utils_def.h>

#define QTI_RIMPSSLOW_CLKDOM0_BASE	0x0fd90000UL
#define QTI_RIMPSSLOW_CLKDOM1_BASE	0x0fd91000UL
#define QTI_RIMPSSLOW_CLKDOM2_BASE	0x0fd92000UL
#define QTI_RIMPSTOP_SEC_ACCESS_OVERRIDE	0x0fd98000UL
#define QTI_RIMPSTOP_GLOBAL_ENABLE	0x0fd98004UL

#define QTI_ACD_L3_BASE			0x0fA94000UL
#define QTI_ACD_SILVER_BASE		0x0fA90000UL
#define QTI_ACD_GOLD_BASE		0x0fA92000UL
#define QTI_ACD_L3_CLK_CTL_BASE		0x0fA88000UL
#define QTI_ACD_SILVER_CLK_CTL_BASE	0x0fA80000UL
#define QTI_ACD_GOLD_CLK_CTL_BASE	0x0fA84000UL
#define QTI_ACD_ACDCR			0x004U
#define QTI_ACD_ACDTD			0x008U
#define QTI_ACD_ACDSSCR			0x028U
#define QTI_ACD_EXTINT_CFG		0x030U
#define QTI_ACD_DCVS_SW			0x034U
#define QTI_ACD_GFMUX_CFG		0x03cU
#define QTI_ACD_AVG_CFG_0		0x04cU
#define QTI_ACD_AVG_CFG_1		0x050U
#define QTI_ACD_AVG_CFG_2		0x054U
#define QTI_ACD_AUTOXFER_CFG		0x080U
#define QTI_ACD_AUTOXFER_CTL		0x088U
#define QTI_ACD_WRITE_CTL		0x090U
#define QTI_ACD_WRITE_STATUS		0x094U
#define QTI_ACD_CONFIG			0x0acU
#define QTI_ACD_THRTL_CTL		0x090U

#define QTI_CLKDOM_CYCLE_CNTR_VERSION_SEL	0x3e0U
#define QTI_CLKDOM_CORE_ACD_SW_OVERRIDE(n)	(0x3e4U + (4U * (n)))

#define QTI_ACD_ACDCR_INIT		0x002C5FFDU
#define QTI_ACD_ACDTD_INIT		0x0000B611U
#define QTI_ACD_ACDSSCR_INIT		0x00000961U
#define QTI_ACD_EXTINT_CFG_INIT		0x000C0080U
#define QTI_ACD_EXTINT_CFG_FINAL	0x024F9AE6U
#define QTI_ACD_DCVS_SW_INIT		0x00FFFF08U
#define QTI_ACD_AVG_CFG_0_INIT		0x208007A1U
#define QTI_ACD_AVG_CFG_1_INIT		0x006400C8U
#define QTI_ACD_AVG_CFG_2_INIT		0x20000427U
#define QTI_ACD_AUTOXFER_CFG_INIT	0x0000940EU
#define QTI_ACD_AUTOXFER_CFG_AVG	0x0038F40EU
#define QTI_ACD_CONFIG_INIT		0x00000711U

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
	uintptr_t acd_base;
	uintptr_t clk_ctl_base;
	enum qtipmlib_osm_domain_id domain_id;
	uint8_t cpu_count;
	uint8_t acd_init;
	const char *rail_name;
	const char *clk_name;
} qti_rimps_clkdom_t;

static const qti_rimps_clkdom_t qti_rimps_clkdoms[] = {
	{
		.base = QTI_RIMPSSLOW_CLKDOM0_BASE,
		.acd_base = QTI_ACD_L3_BASE,
		.clk_ctl_base = QTI_ACD_L3_CLK_CTL_BASE,
		.domain_id = QTIPMLIB_OSM_L3,
		.cpu_count = 1U,
		.acd_init = 1U,
		.rail_name = "APC",
		.clk_name = "L3",
	},
	{
		.base = QTI_RIMPSSLOW_CLKDOM1_BASE,
		.acd_base = QTI_ACD_SILVER_BASE,
		.clk_ctl_base = QTI_ACD_SILVER_CLK_CTL_BASE,
		.domain_id = QTIPMLIB_OSM_SILVER,
		.cpu_count = 3U,
		.acd_init = 1U,
		.rail_name = "APC",
		.clk_name = "Silver",
	},
	{
		.base = QTI_RIMPSSLOW_CLKDOM2_BASE,
		.acd_base = QTI_ACD_GOLD_BASE,
		.clk_ctl_base = QTI_ACD_GOLD_CLK_CTL_BASE,
		.domain_id = QTIPMLIB_OSM_GOLD,
		.cpu_count = 1U,
		/* Keep Gold's ACD block idle while the existing OSM config disables Gold. */
		.acd_init = 0U,
		.rail_name = "APC",
		.clk_name = "Gold",
	},
};

typedef uint32_t (*qti_acd_value_fn_t)(const vpp_osm_t *osm);

static uint32_t qti_acd_dvm_value(const vpp_osm_t *osm)
{
	return osm->acd_acdcr;
}

static uint32_t qti_acd_avg_value(const vpp_osm_t *osm)
{
	return osm->acd_avg_cfg_2;
}

static uint32_t qti_acd_acdsscr_value(const vpp_osm_t *osm)
{
	return osm->acd_acdsscr;
}

static int qti_rimps_build_acd_lut(const vpp_t *plan,
				  qtipmlib_acd_lut_t *lut,
				  qti_acd_value_fn_t get_value)
{
	uint32_t lut_idx = 0U;
	uint32_t row;

	if ((plan->cnt == 0U) || (plan->cnt > VPP_MAX_LUT_SIZE)) {
		return QTIPMLIB_E_INVAL;
	}

	for (row = 0U; row < plan->cnt; row++) {
		const vpp_osm_t *osm = &plan->osm[row];

		if ((row != 0U) &&
		    (get_value(osm) != get_value(&plan->osm[row - 1U]))) {
			if (lut_idx >= (QTIPMLIB_ACD_MAX_VALUES - 1U)) {
				return QTIPMLIB_E_INVAL;
			}
			lut->vc[lut_idx] = plan->osm[row - 1U].vc;
			lut_idx++;
		}
		lut->value[lut_idx] = get_value(osm);
	}

	for (; lut_idx < QTIPMLIB_ACD_MAX_VALUES; lut_idx++) {
		lut->vc[lut_idx] = plan->osm[plan->cnt - 1U].vc;
		lut->value[lut_idx] = get_value(&plan->osm[plan->cnt - 1U]);
	}

	return QTIPMLIB_SUCCESS;
}

static int qti_rimps_program_acd_tables(void)
{
	uint32_t i;

	for (i = 0U; i < ARRAY_SIZE(qti_rimps_clkdoms); i++) {
		const qti_rimps_clkdom_t *clkdom = &qti_rimps_clkdoms[i];
		const vpp_t *plan = qti_vpp_get_plan(clkdom->rail_name,
						    clkdom->clk_name);
		qtipmlib_acd_config_t config = { 0 };
		int ret;

		if ((plan == NULL) || (plan->acd_enabled == 0U)) {
			continue;
		}

		ret = qti_rimps_build_acd_lut(plan, &config.dvm,
					      qti_acd_dvm_value);
		if (ret != QTIPMLIB_SUCCESS) {
			return ret;
		}
		ret = qti_rimps_build_acd_lut(plan, &config.avg,
					      qti_acd_avg_value);
		if (ret != QTIPMLIB_SUCCESS) {
			return ret;
		}
		ret = qti_rimps_build_acd_lut(plan, &config.acdsscr,
					      qti_acd_acdsscr_value);
		if (ret != QTIPMLIB_SUCCESS) {
			return ret;
		}

		ret = qtipmlib_config_acd(clkdom->domain_id, &config);
		if (ret != QTIPMLIB_SUCCESS) {
			return ret;
		}
	}

	return QTIPMLIB_SUCCESS;
}

static void qti_rimps_acd_transfer(uintptr_t base, uint32_t offset)
{
	uint32_t index = offset / sizeof(uint32_t);
	uint32_t mask;

	if (index >= 32U) {
		return;
	}

	mask = 1U << index;
	mmio_write_32(base + QTI_ACD_WRITE_CTL, 0U);
	mmio_write_32(base + QTI_ACD_WRITE_CTL, (index << 1U) | 1U);
	while ((mmio_read_32(base + QTI_ACD_WRITE_STATUS) & mask) == 0U) {
	}
}

static void qti_rimps_acd_write(uintptr_t base, uint32_t offset,
				uint32_t value)
{
	mmio_write_32(base + offset, value);
	qti_rimps_acd_transfer(base, offset);
}

static void qti_rimps_program_acd_domain(const qti_rimps_clkdom_t *clkdom)
{
	uintptr_t base = clkdom->acd_base;
	uint32_t auto_xfer_spm_count;
	uint32_t cpu;

	/* Match the Shikra downstream ACD setup; DVM tables come from VPP. */
	mmio_clrsetbits_32(clkdom->base + QTI_CLKDOM_CYCLE_CNTR_VERSION_SEL,
			   1U, 0U);
	for (cpu = 0U; cpu < clkdom->cpu_count; cpu++) {
		mmio_clrsetbits_32(clkdom->base +
				   QTI_CLKDOM_CORE_ACD_SW_OVERRIDE(cpu),
				   0xfU, 0x5U);
	}

	qti_rimps_acd_write(base, QTI_ACD_ACDCR, QTI_ACD_ACDCR_INIT);
	qti_rimps_acd_write(base, QTI_ACD_ACDTD, QTI_ACD_ACDTD_INIT);
	qti_rimps_acd_write(base, QTI_ACD_ACDSSCR, QTI_ACD_ACDSSCR_INIT);
	qti_rimps_acd_write(base, QTI_ACD_EXTINT_CFG,
			    QTI_ACD_EXTINT_CFG_INIT);
	qti_rimps_acd_write(base, QTI_ACD_DCVS_SW, QTI_ACD_DCVS_SW_INIT);
	qti_rimps_acd_write(base, QTI_ACD_GFMUX_CFG, 1U);

	auto_xfer_spm_count = (clkdom->domain_id == QTIPMLIB_OSM_L3) ?
			      0x15U : 0xaU;
	mmio_write_32(base + QTI_ACD_AUTOXFER_CTL,
		      (auto_xfer_spm_count << 1U) | 1U);
	mmio_clrsetbits_32(base + QTI_ACD_CONFIG, QTI_ACD_CONFIG_INIT,
			   QTI_ACD_CONFIG_INIT);
	qti_rimps_acd_write(base, QTI_ACD_DCVS_SW,
			    QTI_ACD_DCVS_SW_INIT | 1U);
	qti_rimps_acd_write(base, QTI_ACD_DCVS_SW,
			    QTI_ACD_DCVS_SW_INIT);

	udelay(15U);
	qti_rimps_acd_write(base, QTI_ACD_EXTINT_CFG,
			    QTI_ACD_EXTINT_CFG_FINAL);
	mmio_write_32(base + QTI_ACD_AUTOXFER_CFG,
		      QTI_ACD_AUTOXFER_CFG_INIT);
	mmio_clrsetbits_32(clkdom->clk_ctl_base + QTI_ACD_THRTL_CTL,
			   1U, 1U);
	qti_rimps_acd_write(base, QTI_ACD_AVG_CFG_0,
			    QTI_ACD_AVG_CFG_0_INIT);
	qti_rimps_acd_write(base, QTI_ACD_AVG_CFG_1,
			    QTI_ACD_AVG_CFG_1_INIT);
	qti_rimps_acd_write(base, QTI_ACD_AVG_CFG_2,
			    QTI_ACD_AVG_CFG_2_INIT);
	mmio_write_32(base + QTI_ACD_AUTOXFER_CFG,
		      QTI_ACD_AUTOXFER_CFG_AVG);
}

static void qti_rimps_program_acd(void)
{
	uint32_t i;

	if (qti_rimps_program_acd_tables() != QTIPMLIB_SUCCESS) {
		return;
	}

	for (i = 0U; i < ARRAY_SIZE(qti_rimps_clkdoms); i++) {
		const qti_rimps_clkdom_t *clkdom = &qti_rimps_clkdoms[i];
		const vpp_t *plan = qti_vpp_get_plan(clkdom->rail_name,
						    clkdom->clk_name);

		if ((plan != NULL) && (plan->acd_enabled != 0U) &&
		    (clkdom->acd_init != 0U) &&
		    (plan->cnt != 0U)) {
			qti_rimps_program_acd_domain(clkdom);
		}
	}
}

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
	qti_rimps_program_acd();
	(void)qtipmlib_init();
}
