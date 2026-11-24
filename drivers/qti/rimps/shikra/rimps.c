/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * RIMPS DCVS runtime bring-up for shikra: loads the CPUCP (RIMPS/EPSS)
 * firmware, derives each clock domain's OSM LUT/ACD/AVG/BCL configuration
 * from the VPP voltage plan, programs APM/MEMACC, and enables the L3,
 * Silver and Gold OSM domains with an initial perf-state request.
 *
 * This is a 1:1 port of the vendor TZ reference's Clock_InitCPU/Clock_InitOSM
 * call tree (ClockCPU.c), with the following vendor code paths deliberately
 * NOT ported because they are dead or unreachable for shikra (grep-verified
 * against the vendor source -- zero behavioral difference from omitting
 * them):
 *   - Clock_CPUColdBoot() and the gold-domain CPR workaround
 *     (are_both_gold_cores_disabled()/cprh_disable_gold_domain_thread()/
 *     cprh_thread_override_ctl()): per-CPU hotplug boot entry point and a
 *     fused-CPU-disable workaround, both out of scope for BL31 cold boot.
 *   - The ACC-fuse mapping in Clock_InitOSM (nACCFuse): written once, never
 *     read anywhere afterward.
 *   - nFirstTurboL3VC, nInitialVC/Clock_GetInitialVC, nMEMACCCrossover
 *     (Hop)VC, nAPMCrossover(Hop)VC/nCrossoverVoltageAPM, nCPUMin/nCPUMax,
 *     nOSMSeqBase, CoreInactivePwrIdx, C1VisibilityParkingVC: all dead
 *     fields/branches, gated behind commented-out vendor feature flags or
 *     never read.
 *   - The Clock_InitOSM L3-voting readback branch: RIMPSTOP_L3_VOTING_EN is
 *     written to 0 immediately before being read back, so the branch it
 *     guards never executes; every domain always takes the plain
 *     COREd_PERF_STATE_DESIRED write.
 *   - Clock_InitAPM's SW-mode body: unreachable because VPP_AUTO_APM is set
 *     in vpp_cfg.h, so eAPMMode is always APM_AUTO_MODE; the scan that
 *     decides SW- vs HW-mode is still performed for real so this stays
 *     correct if that ever changes.
 *   - NS_LOGBUF_ACCESS_EN, SKU_ID, QULTIVATE_FUSE in the EPSS shared data:
 *     stubbed to 0, no TF-A equivalent exists for tzbsp_is_sys_dbg_enable(),
 *     vpp_device_t has no sku_identifier field, and there is no
 *     ChipInfo_GetDisabledCPUs() port.
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <lib/mmio.h>
#include <lib/utils_def.h>

#include <drivers/qti/rimps/rimps.h>
#include <drivers/qti/vpp/vpp.h>

#include "rimps_hwio.h"

#define RIMPS_CORE_VERIFY(x)					\
	do {								\
		if ((x) == 0) {						\
			ERROR("rimps: invariant failed: %s\n", #x); \
			panic();					\
		}							\
	} while (0)

#define RIMPS_MAX_VC			0x3fU
#define RIMPS_MAX_VOLTAGE			0xffffU
#define RIMPS_LUT_MAX_ROWS			12U

#define RIMPS_ACD_MAX_DVM_VALS			7U
#define RIMPS_ACD_MAX_AVG_VALS			7U
#define RIMPS_ACD_MAX_ACDSSCR_VALS		7U
#define RIMPS_BCL_ALARM_MAX_LVLS		4U

#define RIMPS_RAIL_APC0			0U
#define RIMPS_MAX_RAIL			1U
#define RIMPS_SILVER_AVS_DELAY_PERF_DOWN	59U

/* ipeak_measurments() MPMM_GEAR thresholds/slope, from the vendor source. */
#define RIMPS_IPEAK_SLOPE			60U
#define RIMPS_IPEAK_IDDQ_TT			52340U
#define RIMPS_IPEAK_THRESHOLD_0		1125000U
#define RIMPS_IPEAK_THRESHOLD_1		1151000U
#define RIMPS_IPEAK_THRESHOLD_2		1173000U

enum rimps_domain_id {
	RIMPS_DOMAIN_L3 = 0,
	RIMPS_DOMAIN_SILVER,
	RIMPS_DOMAIN_GOLD,
	RIMPS_DOMAIN_COUNT
};

/*
 * EPSS shared-data region consumed by the CPUCP firmware; verbatim layout of
 * the vendor epssSharedDataType (RESERVEDn kept as real members, not
 * compiler padding, so the layout matches regardless of toolchain). Fields
 * never written by either the vendor's live code paths or this port
 * (PARK_OLV, VC_880MV, GPLUS_VMAX_*) are kept for layout only.
 */
struct rimps_epss_shared_data {
	uint32_t version_id;
	uint8_t apm_crossover_corner[RIMPS_DOMAIN_COUNT];
	uint8_t reserved1;
	uint8_t acd_dvm_vc[RIMPS_DOMAIN_COUNT][RIMPS_ACD_MAX_DVM_VALS];
	uint32_t acd_dvm_val[RIMPS_DOMAIN_COUNT][RIMPS_ACD_MAX_DVM_VALS];
	uint8_t acd_avg_vc[RIMPS_DOMAIN_COUNT][RIMPS_ACD_MAX_AVG_VALS];
	uint32_t acd_avg_cfg[RIMPS_DOMAIN_COUNT][RIMPS_ACD_MAX_AVG_VALS];
	uint8_t acd_acdsscr_vc[RIMPS_DOMAIN_COUNT][RIMPS_ACD_MAX_ACDSSCR_VALS];
	uint32_t acd_acdsscr_val[RIMPS_DOMAIN_COUNT][RIMPS_ACD_MAX_ACDSSCR_VALS];
	uint8_t bcl_lval[RIMPS_DOMAIN_COUNT][RIMPS_BCL_ALARM_MAX_LVLS];
	uint8_t park_pll_lval[RIMPS_DOMAIN_COUNT];
	uint8_t park_vc[RIMPS_DOMAIN_COUNT];
	uint16_t park_olv[RIMPS_DOMAIN_COUNT];
	uint8_t vc_880mv[RIMPS_DOMAIN_COUNT];
	uint32_t reserved2;
	uint8_t mem_acc_nom_vc[RIMPS_DOMAIN_COUNT];
	uint8_t mem_acc_tur_vc[RIMPS_DOMAIN_COUNT];
	uint32_t reserved3;
	uint8_t mx_nom_data;
	uint8_t mx_tur_data;
	uint32_t dcvs_down_saw_avs_dly[RIMPS_MAX_RAIL];
	uint16_t gplus_vmax_75c;
	uint16_t gplus_vmax_80c;
	uint16_t gplus_vmax_85c;
	uint16_t gplus_vmax_90c;
	uint16_t gplus_vmax_95c;
	uint16_t gplus_vmax_100c;
	uint32_t ns_logbuf_access_en;
	uint32_t speed_bin;
	uint32_t sku_id;
	uint32_t iddq;
	uint32_t qultivate_fuse;
	uint32_t reserved4;
	uint32_t mpmm_gear;
};

static volatile struct rimps_epss_shared_data *const epss_shared_data =
	(volatile struct rimps_epss_shared_data *)RIMPS_EPSS_SHARED_DATA_BASE;

struct rimps_ctrl_settings {
	uint8_t cc_based_boost_enable;
	uint8_t shared_rail_boost_enable;
	uint8_t per_core_dcvs_enable;
	uint8_t cycle_counter_enable;
	uint8_t boost_fsm_enable;
	uint8_t pll_lock_sel;
	uint8_t l3_voting_pwr_state;
	uint8_t lval_chng_wait;
	uint8_t throttle_gain;
	uint8_t max_core_count;
	uint16_t cc_increase_timer;
	uint16_t cc_decrease_timer;
	uint16_t vmin_0c_violation;
	uint16_t llm_fmax_increase;
	uint16_t llm_fmax_decrease;
	uint16_t llm_throttle_increase;
	uint16_t llm_throttle_decrease;
	uint16_t llm_vmax_increase;
	uint16_t llm_vmax_decrease;
	uint16_t llm_vmin_increase;
	uint16_t llm_vmin_decrease;
	uint32_t in_act2act_timer;
	uint32_t act2inact_timer;
};

struct rimps_domain {
	const char *rail_name;
	const char *clk_name;
	uint8_t domain_id;
	uint8_t flag_enable;
	uint8_t flag_init_request;
	uint8_t clkdom_num_cpus;
	uintptr_t osm_top_base;
	uintptr_t acd_base;
	uintptr_t clk_ctl_base;
	struct rimps_ctrl_settings ctrl;

	/* Runtime state, populated by rimps_parse_vpp(). */
	bool enabled;
	const vpp_t *vpp;
	uint8_t apm_mode;
	uint8_t first_svs_l1_vc;
	uint8_t first_turbo_vc;
	uint32_t dvm_vals[RIMPS_ACD_MAX_DVM_VALS];
	uint8_t dvm_last_vcs[RIMPS_ACD_MAX_DVM_VALS];
	uint32_t avg_vals[RIMPS_ACD_MAX_AVG_VALS];
	uint8_t avg_last_vcs[RIMPS_ACD_MAX_AVG_VALS];
	uint32_t acdsscr_vals[RIMPS_ACD_MAX_ACDSSCR_VALS];
	uint8_t acdsscr_last_vcs[RIMPS_ACD_MAX_ACDSSCR_VALS];
	uint8_t bcl_alarm_lvals[RIMPS_BCL_ALARM_MAX_LVLS];
};

/*
 * L3 must be configured before Silver/Gold: L3 and Silver share a rail, and
 * enabling Silver first snaps the rail down to Silver's VC0.
 *
 * Gold's Flags diverge from the vendor reference ({bEnable=0,bInitRequest=0}
 * there): shikra enables all three domains at init, Gold included.
 */
static struct rimps_domain rimps_dcvs_domains[RIMPS_DOMAIN_COUNT] = {
	[RIMPS_DOMAIN_L3] = {
		.rail_name = "APC",
		.clk_name = "L3",
		.domain_id = RIMPS_DOMAIN_L3,
		.flag_enable = 1U,
		.flag_init_request = 1U,
		.clkdom_num_cpus = 1U,
		.osm_top_base = L3_OSM_TOP_BASE,
		.acd_base = L3_ACD_BASE,
		.clk_ctl_base = L3_APSS_KRYO_CLK_CTL_BASE,
		.ctrl = {
			.cc_based_boost_enable = 0U,
			.shared_rail_boost_enable = 0U,
			.per_core_dcvs_enable = 0U,
			.cycle_counter_enable = 1U,
			.boost_fsm_enable = 0U,
			.pll_lock_sel = 1U,
			.l3_voting_pwr_state = 0x1U,
			.lval_chng_wait = 0x5U,
			.throttle_gain = 0x10U,
			.max_core_count = 0x0U,
			.cc_increase_timer = 0U,
			.cc_decrease_timer = 0U,
			.vmin_0c_violation = RIMPS_MAX_VOLTAGE,
			.in_act2act_timer = 0U,
			.act2inact_timer = 0U,
		},
	},
	[RIMPS_DOMAIN_SILVER] = {
		.rail_name = "APC",
		.clk_name = "Silver",
		.domain_id = RIMPS_DOMAIN_SILVER,
		.flag_enable = 1U,
		.flag_init_request = 1U,
		.clkdom_num_cpus = 3U,
		.osm_top_base = SILVER_OSM_TOP_BASE,
		.acd_base = SILVER_ACD_BASE,
		.clk_ctl_base = SILVER_APSS_KRYO_CLK_CTL_BASE,
		.ctrl = {
			.cc_based_boost_enable = 1U,
			.shared_rail_boost_enable = 0U,
			.per_core_dcvs_enable = 0U,
			.cycle_counter_enable = 1U,
			.boost_fsm_enable = 1U,
			.pll_lock_sel = 1U,
			.l3_voting_pwr_state = 0x1U,
			.lval_chng_wait = 0x5U,
			.throttle_gain = 0x10U,
			.max_core_count = 0x3U,
			.cc_increase_timer = 0U,
			.cc_decrease_timer = 0U,
			.vmin_0c_violation = RIMPS_MAX_VOLTAGE,
			.in_act2act_timer = 0U,
			.act2inact_timer = 0U,
		},
	},
	[RIMPS_DOMAIN_GOLD] = {
		.rail_name = "APC",
		.clk_name = "Gold",
		.domain_id = RIMPS_DOMAIN_GOLD,
		.flag_enable = 1U,
		.flag_init_request = 1U,
		.clkdom_num_cpus = 1U,
		.osm_top_base = GOLD_OSM_TOP_BASE,
		.acd_base = GOLD_ACD_BASE,
		.clk_ctl_base = GOLD_APSS_KRYO_CLK_CTL_BASE,
		.ctrl = {
			.cc_based_boost_enable = 1U,
			.shared_rail_boost_enable = 0U,
			.per_core_dcvs_enable = 0U,
			.cycle_counter_enable = 1U,
			.boost_fsm_enable = 1U,
			.pll_lock_sel = 1U,
			.l3_voting_pwr_state = 0x1U,
			.lval_chng_wait = 0x5U,
			.throttle_gain = 0x10U,
			.max_core_count = 0x1U,
			.cc_increase_timer = 0U,
			.cc_decrease_timer = 0U,
			.vmin_0c_violation = RIMPS_MAX_VOLTAGE,
			.in_act2act_timer = 0U,
			.act2inact_timer = 0U,
		},
	},
};

/*
 * rimps_acd_latch - mirror of the vendor Clock_ACDTransfer(): ACD
 * registers at byte-offset/4 < 32 have a shadow copy that must be latched
 * into the live ACD block via a SELECT + UPDATE_EN handshake, polled on
 * ACD_WRITE_STATUS.ACK; registers at idx >= 32 have no such latch and the
 * call is a no-op.
 */
static void rimps_acd_latch(uintptr_t acd_base, uint32_t offset)
{
	uint32_t idx = offset / 4U;
	uint32_t mask;

	if (idx >= 32U) {
		return;
	}

	mask = 1U << idx;
	mmio_write_32(ACD_WRITE_CTL(acd_base), 0U);
	mmio_clrsetbits_32(ACD_WRITE_CTL(acd_base), ACD_WRITE_CTL_SELECT_BMSK,
			   idx << ACD_WRITE_CTL_SELECT_SHFT);
	mmio_setbits_32(ACD_WRITE_CTL(acd_base), ACD_WRITE_CTL_UPDATE_EN_BMSK);
	while ((mmio_read_32(ACD_WRITE_STATUS(acd_base)) & mask) == 0U) {
	}
}

/* io(0) extracts just the byte-offset literal out of a (base)-only register macro. */
#define RIMPS_ACD_FIELD(acd_base, io, field, val)				\
	do {									\
		mmio_clrsetbits_32(io(acd_base), io##_##field##_BMSK,		\
				    (uint32_t)(val) << io##_##field##_SHFT);	\
		rimps_acd_latch((acd_base), io(0));			\
	} while (0)

#define RIMPS_ACD_RAW(acd_base, io, val)					\
	do {									\
		mmio_write_32(io(acd_base), (uint32_t)(val));			\
		rimps_acd_latch((acd_base), io(0));			\
	} while (0)

static void rimps_config_common(struct rimps_domain *dom)
{
	const vpp_device_t *vpp_config = vpp_get_device_info();
	uintptr_t top = dom->osm_top_base;
	uintptr_t acd = dom->acd_base;
	uint32_t i;

	mmio_clrsetbits_32(CC_HYS_INC(top), CC_HYS_INC_VALUE_BMSK,
			    (uint32_t)dom->ctrl.cc_increase_timer << CC_HYS_INC_VALUE_SHFT);
	mmio_clrsetbits_32(CC_HYS_DEC(top), CC_HYS_DEC_VALUE_BMSK,
			    (uint32_t)dom->ctrl.cc_decrease_timer << CC_HYS_DEC_VALUE_SHFT);
	mmio_clrsetbits_32(MAX_CORE_CNT(top), MAX_CORE_CNT_VALUE_BMSK,
			    (uint32_t)dom->ctrl.max_core_count << MAX_CORE_CNT_VALUE_SHFT);
	mmio_clrsetbits_32(LLM_DCVS_VOL_VOTE(top), LLM_DCVS_VOL_VOTE_VMIN_BMSK,
			    (uint32_t)dom->ctrl.vmin_0c_violation << LLM_DCVS_VOL_VOTE_VMIN_SHFT);
	mmio_clrsetbits_32(LLM_FMAX_HYS_TIMER_INC(top), LLM_FMAX_HYS_TIMER_INC_VALUE_BMSK,
			    (uint32_t)dom->ctrl.llm_fmax_increase << LLM_FMAX_HYS_TIMER_INC_VALUE_SHFT);
	mmio_clrsetbits_32(LLM_FMAX_HYS_TIMER_DEC(top), LLM_FMAX_HYS_TIMER_DEC_VALUE_BMSK,
			    (uint32_t)dom->ctrl.llm_fmax_decrease << LLM_FMAX_HYS_TIMER_DEC_VALUE_SHFT);
	mmio_clrsetbits_32(LLM_THROTTLE_AMNT_HYS_INC(top), LLM_THROTTLE_AMNT_HYS_INC_CNT_BMSK,
			    (uint32_t)dom->ctrl.llm_throttle_increase << LLM_THROTTLE_AMNT_HYS_INC_CNT_SHFT);
	mmio_clrsetbits_32(LLM_THROTTLE_AMNT_HYS_DEC(top), LLM_THROTTLE_AMNT_HYS_DEC_CNT_BMSK,
			    (uint32_t)dom->ctrl.llm_throttle_decrease << LLM_THROTTLE_AMNT_HYS_DEC_CNT_SHFT);
	mmio_clrsetbits_32(LLM_VMAX_HYS_TIMER_INC(top), LLM_VMAX_HYS_TIMER_INC_VALUE_BMSK,
			    (uint32_t)dom->ctrl.llm_vmax_increase << LLM_VMAX_HYS_TIMER_INC_VALUE_SHFT);
	mmio_clrsetbits_32(LLM_VMAX_HYS_TIMER_DEC(top), LLM_VMAX_HYS_TIMER_DEC_VALUE_BMSK,
			    (uint32_t)dom->ctrl.llm_vmax_decrease << LLM_VMAX_HYS_TIMER_DEC_VALUE_SHFT);
	mmio_clrsetbits_32(LLM_VMIN_HYS_TIMER_INC(top), LLM_VMIN_HYS_TIMER_INC_VALUE_BMSK,
			    (uint32_t)dom->ctrl.llm_vmin_increase << LLM_VMIN_HYS_TIMER_INC_VALUE_SHFT);
	mmio_clrsetbits_32(LLM_VMIN_HYS_TIMER_DEC(top), LLM_VMIN_HYS_TIMER_DEC_VALUE_BMSK,
			    (uint32_t)dom->ctrl.llm_vmin_decrease << LLM_VMIN_HYS_TIMER_DEC_VALUE_SHFT);
	mmio_clrsetbits_32(CORE_PS_HYS(top), CORE_PS_HYS_INACT_TO_ACT_TIMER_BMSK,
			    dom->ctrl.in_act2act_timer << CORE_PS_HYS_INACT_TO_ACT_TIMER_SHFT);
	mmio_clrsetbits_32(CORE_PS_HYS(top), CORE_PS_HYS_ACT_TO_INACT_TIMER_BMSK,
			    dom->ctrl.act2inact_timer << CORE_PS_HYS_ACT_TO_INACT_TIMER_SHFT);
	mmio_clrsetbits_32(CC_DCVS(top), CC_DCVS_EN_BMSK,
			    (uint32_t)dom->ctrl.cc_based_boost_enable << CC_DCVS_EN_SHFT);
	mmio_clrsetbits_32(SHARED_RAIL_BOOST(top), SHARED_RAIL_BOOST_EN_BMSK,
			    (uint32_t)dom->ctrl.shared_rail_boost_enable << SHARED_RAIL_BOOST_EN_SHFT);
	mmio_clrsetbits_32(CORE_DCVS_CTRL(top), CORE_DCVS_CTRL_PER_CORE_DCVS_EN_BMSK,
			    (uint32_t)dom->ctrl.per_core_dcvs_enable << CORE_DCVS_CTRL_PER_CORE_DCVS_EN_SHFT);
	mmio_clrsetbits_32(CORE_DCVS_CTRL(top), CORE_DCVS_CTRL_L3_VOTING_PWR_STATE_BMSK,
			    (uint32_t)dom->ctrl.l3_voting_pwr_state << CORE_DCVS_CTRL_L3_VOTING_PWR_STATE_SHFT);
	mmio_clrsetbits_32(CORE_DCVS_CTRL(top), CORE_DCVS_CTRL_CYCLE_CNTR_EN_BMSK,
			    (uint32_t)dom->ctrl.cycle_counter_enable << CORE_DCVS_CTRL_CYCLE_CNTR_EN_SHFT);
	mmio_clrsetbits_32(PLL_FSM_CTRL(top), PLL_FSM_CTRL_LVAL_CHNG_WAIT_BMSK,
			    (uint32_t)dom->ctrl.lval_chng_wait << PLL_FSM_CTRL_LVAL_CHNG_WAIT_SHFT);
	mmio_clrsetbits_32(PLL_FSM_CTRL(top), PLL_FSM_CTRL_THROTTLE_GAIN_BMSK,
			    (uint32_t)dom->ctrl.throttle_gain << PLL_FSM_CTRL_THROTTLE_GAIN_SHFT);
	mmio_clrsetbits_32(PLL_FSM_CTRL(top), PLL_FSM_CTRL_BOOST_FSM_EN_BMSK,
			    (uint32_t)dom->ctrl.boost_fsm_enable << PLL_FSM_CTRL_BOOST_FSM_EN_SHFT);
	mmio_clrsetbits_32(PLL_FSM_CTRL(top), PLL_FSM_CTRL_PLL_LOCK_SEL_BMSK,
			    (uint32_t)dom->ctrl.pll_lock_sel << PLL_FSM_CTRL_PLL_LOCK_SEL_SHFT);
	mmio_clrsetbits_32(LLM_DCVS_VC_VOTE(top), LLM_DCVS_VC_VOTE_THROTTLE_EN_BMSK,
			    1U << LLM_DCVS_VC_VOTE_THROTTLE_EN_SHFT);
	mmio_clrsetbits_32(PLL_MIN_FREQ(top), PLL_MIN_FREQ_LVAL_BMSK,
			    0x20U << PLL_MIN_FREQ_LVAL_SHFT);

	if (vpp_config->prop.acd_enable != 0U) {
		mmio_clrsetbits_32(CYCLE_CNTR_VERSION_SEL(top),
				    CYCLE_CNTR_VERSION_SEL_VERSION_SEL_BMSK, 0U);

		for (i = 0U; i < dom->clkdom_num_cpus; i++) {
			uintptr_t ov = CORE_CYCLE_CNTR_ACD_SW_OVERRIDE(top, i);

			mmio_clrsetbits_32(ov, CYCLE_CNTR_ACD_SW_OVERRIDE_SW_ACD_CLK_GATING_OVERRIDE_VAL_BMSK, 0U);
			mmio_clrsetbits_32(ov, CYCLE_CNTR_ACD_SW_OVERRIDE_SW_ACD_CLK_DIV2_OVERRIDE_VAL_BMSK, 0U);
			mmio_clrsetbits_32(ov, CYCLE_CNTR_ACD_SW_OVERRIDE_SW_ACD_CLK_GATING_OVERRIDE_SEL_BMSK,
					    1U << CYCLE_CNTR_ACD_SW_OVERRIDE_SW_ACD_CLK_GATING_OVERRIDE_SEL_SHFT);
			mmio_clrsetbits_32(ov, CYCLE_CNTR_ACD_SW_OVERRIDE_SW_ACD_CLK_DIV2_OVERRIDE_SEL_BMSK,
					    1U << CYCLE_CNTR_ACD_SW_OVERRIDE_SW_ACD_CLK_DIV2_OVERRIDE_SEL_SHFT);
		}

		RIMPS_ACD_FIELD(acd, ACDCR, AUTOCALDLY, 0x2U);
		RIMPS_ACD_FIELD(acd, ACDCR, HWAUTOCALEN, 0x1U);
		RIMPS_ACD_FIELD(acd, ACDCR, AUTOCALMGN, 0x4U);
		RIMPS_ACD_FIELD(acd, ACDCR, DVMEN, 0x1U);
		RIMPS_ACD_FIELD(acd, ACDCR, ACUCNTRST, 0x1U);
		RIMPS_ACD_FIELD(acd, ACDCR, ACUTMRCNT, 0xffU);
		RIMPS_ACD_FIELD(acd, ACDCR, ACUMODE, 0x3U);
		RIMPS_ACD_FIELD(acd, ACDCR, ACDEN, 0x1U);
		RIMPS_ACD_FIELD(acd, ACDTD, XTRTDECFG, 0xb6U);
		RIMPS_ACD_FIELD(acd, ACDTD, XTRTDEEN, 0x1U);
		RIMPS_ACD_FIELD(acd, ACDTD, MODE, 0x1U);
		RIMPS_ACD_FIELD(acd, ACDSSCR, ACUTMRTRIPCNT, 0x9U);
		RIMPS_ACD_FIELD(acd, ACDSSCR, LARGE_MGN_INITIAL, 0x6U);
		RIMPS_ACD_FIELD(acd, ACDSSCR, MODE, 0x1U);
		RIMPS_ACD_FIELD(acd, ACD_EXTINT_CFG, DIV2_BADCAL_VOLT_NONDCVS, 0x1U);
		RIMPS_ACD_FIELD(acd, ACD_EXTINT_CFG, DIV2_BADCAL_VOLT_DCVS, 0x1U);
		RIMPS_ACD_FIELD(acd, ACD_EXTINT_CFG, DIV2_BADCAL_OSM_DCVS, 0x0U);
		RIMPS_ACD_FIELD(acd, ACD_EXTINT_CFG, DCVS_EVENT_EN, 0x1U);
		RIMPS_ACD_FIELD(acd, ACD_EXTINT_CFG, RC_FULL_FREQ_MASK, 0x0U);
		RIMPS_ACD_FIELD(acd, ACD_EXTINT_CFG, OSM_FULL_FREQ_MASK, 0x0U);
		RIMPS_ACD_FIELD(acd, ACD_EXTINT_CFG, FORCE_DIV2_LF_RC_NONDCVS, 0x0U);
		RIMPS_ACD_FIELD(acd, ACD_DCVS_SW, CPR_CLAIB_TIMER, 0xffffU);
		RIMPS_ACD_FIELD(acd, ACD_DCVS_SW, CPR_CLAIB_FEATURE_ENABLE, 0x1U);
		RIMPS_ACD_FIELD(acd, ACD_GFMUX_CFG, SELECT, 0x1U);
		RIMPS_ACD_FIELD(acd, ACD_AUTOXFER_CTL, AUTO_CONFIGURE, 0x1U);

		if (dom->domain_id == RIMPS_DOMAIN_L3) {
			RIMPS_ACD_FIELD(acd, ACD_AUTOXFER_CTL, AUTO_XFER_SPM_CNT, 0x15U);
		} else {
			RIMPS_ACD_FIELD(acd, ACD_AUTOXFER_CTL, AUTO_XFER_SPM_CNT, 0xaU);
		}

		RIMPS_ACD_FIELD(acd, ACDCONFIG, ACD_ACTIVE_FEATURE_ENABLE, 0x1U);
		RIMPS_ACD_FIELD(acd, ACDCONFIG, ACD_ACTIVE_DISABLE, 0x1U);
		RIMPS_ACD_FIELD(acd, ACDCONFIG, CPR_AVG_ENABLE, 0x1U);
		RIMPS_ACD_FIELD(acd, ACDCONFIG, AVG_TICK_COUNTER_ENABLE, 0x1U);
		RIMPS_ACD_FIELD(acd, ACDCONFIG, TICK_COUNTER_ENABLE, 0x1U);

		RIMPS_ACD_FIELD(acd, ACD_DCVS_SW, SW_DCVS_IN_PROGRESS, 1U);
		RIMPS_ACD_FIELD(acd, ACD_DCVS_SW, SW_DCVS_IN_PROGRESS, 0U);

		/* Wait >= 210 CPU clocks (10us @ 19.2MHz). */
		udelay(15U);

		RIMPS_ACD_RAW(acd, ACD_EXTINT_CFG, 0x24f9ae6U);
		mmio_write_32(ACD_AUTOXFER_CFG(acd), 0x940eU);
		mmio_clrsetbits_32(ACD_THRTL_CTL(dom->clk_ctl_base),
				    ACD_THRTL_CTL_LLM_ACD_THROTTLE_EN_BMSK,
				    1U << ACD_THRTL_CTL_LLM_ACD_THROTTLE_EN_SHFT);

		if (vpp_config->prop.avg_enable != 0U) {
			RIMPS_ACD_FIELD(acd, ACD_AVG_CFG_0, HYSTERESIS_ITER_DECREASE, 0x4U);
			RIMPS_ACD_FIELD(acd, ACD_AVG_CFG_0, HYSTERESIS_ITER_INCREASE, 0x1U);
			RIMPS_ACD_FIELD(acd, ACD_AVG_CFG_0, REF_CYCLES, 0x7a1U);
			RIMPS_ACD_FIELD(acd, ACD_AVG_CFG_1, LOST_CYCLES_DIFF, 0x64U);
			RIMPS_ACD_FIELD(acd, ACD_AVG_CFG_1, LOST_CYCLES_LOWER_BOUND, 0xc8U);
			RIMPS_ACD_FIELD(acd, ACD_AVG_CFG_2, MARGIN_FIRST, 0x1U);
			RIMPS_ACD_FIELD(acd, ACD_AVG_CFG_2, DELTA_M, 0x1U);
			RIMPS_ACD_FIELD(acd, ACD_AVG_CFG_2, DELTA_V, 0x1U);
			RIMPS_ACD_FIELD(acd, ACD_AVG_CFG_2, MAX_VOL_REDUCTION, 0x7U);
			RIMPS_ACD_RAW(acd, ACD_AUTOXFER_CFG, 0x38f40eU);
		}
	}

	mmio_clrsetbits_32(RIMPS_INTR_EN(top), RIMPS_INTR_EN_RV_TO_APSS_IRQ_EN_BMSK,
			    1U << RIMPS_INTR_EN_RV_TO_APSS_IRQ_EN_SHFT);
	mmio_clrsetbits_32(RIMPS_INTR_EN(top), RIMPS_INTR_EN_GT_IRQ_EN_BMSK,
			    1U << RIMPS_INTR_EN_GT_IRQ_EN_SHFT);
	mmio_clrsetbits_32(RIMPS_INTR_EN(top), RIMPS_INTR_EN_LT_IRQ_EN_BMSK, 0U);
	mmio_clrsetbits_32(RIMPS_INTR_EN(top), RIMPS_INTR_EN_EQ_IRQ_EN_BMSK, 0U);
	mmio_clrsetbits_32(FVA_INT_EN(top), FVA_INT_EN_INT_EN_BMSK,
			    1U << FVA_INT_EN_INT_EN_SHFT);

	mmio_write_32(RIMPSFAST_CPU_LPM_IRQ_EN, 0x47U);
	mmio_write_32(RIMPSFAST_CL_LPM_IRQ_EN, 0x1U);
}

/*
 * rimps_init_apm - APM (Analog Power Mux) bring-up. The vendor SW-mode
 * body (gated on any domain using APM_SW_MODE) is not ported: VPP_AUTO_APM
 * is set in vpp_cfg.h, so every domain's eAPMMode is always APM_AUTO_MODE
 * and that body never runs. The scan that decides which branch applies is
 * still performed for real, so this stays correct if that ever changes.
 */
static void rimps_init_apm(void)
{
	bool hw_mode = false;
	uint32_t i;

	for (i = 0U; i < RIMPS_DOMAIN_COUNT; i++) {
		if (rimps_dcvs_domains[i].apm_mode == APM_AUTO_MODE) {
			hw_mode = true;
		}
	}

	mmio_clrsetbits_32(APSS_APM_CTL_OVERRIDE,
			    APSS_APM_CTL_OVERRIDE_APM_CTL_SER_FREEZE_DIS_BMSK,
			    0x3U << APSS_APM_CTL_OVERRIDE_APM_CTL_SER_FREEZE_DIS_SHFT);

	mmio_clrsetbits_32(APSS_APM_DLY_CNTR_0, APSS_APM_DLY_CNTR_POST_HALT_DLY_BMSK,
			    0xbU << APSS_APM_DLY_CNTR_POST_HALT_DLY_SHFT);
	mmio_clrsetbits_32(APSS_APM_DLY_CNTR_0, APSS_APM_DLY_CNTR_HALT_CLK_DLY_BMSK,
			    0xcU << APSS_APM_DLY_CNTR_HALT_CLK_DLY_SHFT);
	mmio_clrsetbits_32(APSS_APM_DLY_CNTR_0, APSS_APM_DLY_CNTR_RESUME_CLK_DLY_BMSK,
			    0xdU << APSS_APM_DLY_CNTR_RESUME_CLK_DLY_SHFT);
	mmio_clrsetbits_32(APSS_APM_DLY_CNTR_0, APSS_APM_DLY_CNTR_SEL_SWITCH_DLY_BMSK,
			    0xeU << APSS_APM_DLY_CNTR_SEL_SWITCH_DLY_SHFT);

	mmio_clrsetbits_32(APSS_APM_DLY_CNTR_1, APSS_APM_DLY_CNTR_POST_HALT_DLY_BMSK,
			    0xbU << APSS_APM_DLY_CNTR_POST_HALT_DLY_SHFT);
	mmio_clrsetbits_32(APSS_APM_DLY_CNTR_1, APSS_APM_DLY_CNTR_HALT_CLK_DLY_BMSK,
			    0xcU << APSS_APM_DLY_CNTR_HALT_CLK_DLY_SHFT);
	mmio_clrsetbits_32(APSS_APM_DLY_CNTR_1, APSS_APM_DLY_CNTR_RESUME_CLK_DLY_BMSK,
			    0xdU << APSS_APM_DLY_CNTR_RESUME_CLK_DLY_SHFT);
	mmio_clrsetbits_32(APSS_APM_DLY_CNTR_1, APSS_APM_DLY_CNTR_SEL_SWITCH_DLY_BMSK,
			    0xeU << APSS_APM_DLY_CNTR_SEL_SWITCH_DLY_SHFT);

	mmio_clrsetbits_32(APSS_MAS_DLY0_0, APSS_MAS_DLY0_0_RET_BMSK,
			    0xfU << APSS_MAS_DLY0_0_RET_SHFT);
	mmio_clrsetbits_32(APSS_MAS_DLY0_0, APSS_MAS_DLY0_0_SEL_BMSK,
			    0xfU << APSS_MAS_DLY0_0_SEL_SHFT);
	mmio_clrsetbits_32(APSS_MAS_DLY0_0, APSS_MAS_DLY0_0_CLAMP_BMSK,
			    0xfU << APSS_MAS_DLY0_0_CLAMP_SHFT);

	mmio_clrsetbits_32(APSS_MAS_DLY1_0, APSS_MAS_DLY1_0_ENF1_BMSK,
			    0xfU << APSS_MAS_DLY1_0_ENF1_SHFT);
	mmio_clrsetbits_32(APSS_MAS_DLY1_0, APSS_MAS_DLY1_0_ENF0_BMSK,
			    0xfU << APSS_MAS_DLY1_0_ENF0_SHFT);
	mmio_clrsetbits_32(APSS_MAS_DLY1_0, APSS_MAS_DLY1_0_ENR_BMSK,
			    0xfU << APSS_MAS_DLY1_0_ENR_SHFT);

	if (!hw_mode) {
		return;
	}

	if (rimps_dcvs_domains[RIMPS_DOMAIN_SILVER].apm_mode == APM_AUTO_MODE) {
		mmio_clrsetbits_32(APSS_APM_CTL_0, APSS_APM_CTL_0_APM_CTL_EN_CSR_MODE_BMSK, 0U);
		mmio_clrsetbits_32(APSS_APM_CTL_0, APSS_APM_CTL_0_APM_CTL_APC0_AUTO_MODE_EN_BMSK,
				    1U << APSS_APM_CTL_0_APM_CTL_APC0_AUTO_MODE_EN_SHFT);
		mmio_clrsetbits_32(APSS_APM_CTL_0, APSS_APM_CTL_0_APM_CTL_MODE_SW_HW_SEL_BMSK,
				    1U << APSS_APM_CTL_0_APM_CTL_MODE_SW_HW_SEL_SHFT);
	}

	if (rimps_dcvs_domains[RIMPS_DOMAIN_GOLD].apm_mode == APM_AUTO_MODE) {
		mmio_clrsetbits_32(APSS_APM_CTL_1, APSS_APM_CTL_1_APM_CTL_EN_CSR_MODE_BMSK, 0U);
		mmio_clrsetbits_32(APSS_APM_CTL_1, APSS_APM_CTL_1_APM_CTL_APC1_AUTO_MODE_EN_BMSK,
				    1U << APSS_APM_CTL_1_APM_CTL_APC1_AUTO_MODE_EN_SHFT);
		mmio_clrsetbits_32(APSS_APM_CTL_1, APSS_APM_CTL_1_APM_CTL_MODE_SW_HW_SEL_BMSK,
				    1U << APSS_APM_CTL_1_APM_CTL_MODE_SW_HW_SEL_SHFT);
	}

	for (i = 0U; i < 10U; i++) {
		mmio_write_32(APSS_MAS_OVERRIDE(i), 0x1000000U);
	}

	mmio_clrsetbits_32(APSS_APM_CTL_0, APSS_APM_CTL_0_APM_CTL_MODE_FORCE_CLK_DIS_BMSK, 0U);
	mmio_clrsetbits_32(APSS_APM_CTL_1, APSS_APM_CTL_1_APM_CTL_MODE_FORCE_CLK_DIS_BMSK, 0U);
	mmio_clrsetbits_32(APSS_APM_CTL_0, APSS_APM_CTL_0_APM_CONT_LOAD_SHIFTING_EN_BMSK, 0U);
	mmio_clrsetbits_32(APSS_APM_CTL_1, APSS_APM_CTL_1_APM_CONT_LOAD_SHIFTING_EN_BMSK, 0U);
	mmio_clrsetbits_32(APSS_APM_CTL_0, APSS_APM_CTL_0_APM_CTL_USE_CLK_GATE_ACK_BMSK, 0U);
	mmio_clrsetbits_32(APSS_APM_CTL_1, APSS_APM_CTL_1_APM_CTL_USE_CLK_GATE_ACK_BMSK, 0U);
}

static void rimps_init_memacc(void)
{
	mmio_write_32(APSS_SHARED_GOLD_SVS_0, 0U);
	mmio_write_32(APSS_SHARED_GOLD_SVS_1, 0U);
	mmio_write_32(APSS_SHARED_GOLD_NOM_0, 0U);
	mmio_write_32(APSS_SHARED_GOLD_NOM_1, 0U);
	mmio_write_32(APSS_SHARED_GOLD_TURBO_0, 0U);
	mmio_write_32(APSS_SHARED_GOLD_TURBO_1, 0U);

	mmio_write_32(APSS_SHARED_SILVER_SVS_0, 0U);
	mmio_write_32(APSS_SHARED_SILVER_SVS_1, 0U);
	mmio_write_32(APSS_SHARED_SILVER_NOM_0, 0U);
	mmio_write_32(APSS_SHARED_SILVER_NOM_1, 0U);
	mmio_write_32(APSS_SHARED_SILVER_TURBO_0, 0U);
	mmio_write_32(APSS_SHARED_SILVER_TURBO_1, 0U);

	mmio_write_32(APSS_SHARED_L3_SVS_0, 0U);
	mmio_write_32(APSS_SHARED_L3_SVS_1, 0U);
	mmio_write_32(APSS_SHARED_L3_NOM_0, 0U);
	mmio_write_32(APSS_SHARED_L3_NOM_1, 0U);
	mmio_write_32(APSS_SHARED_L3_TURBO_0, 0U);
	mmio_write_32(APSS_SHARED_L3_TURBO_1, 0U);
}

/*
 * rimps_ipeak_measure - port of ipeak_measurments(): reads the
 * IDDQ_APC_ACTIVE fuse and derives MPMM_GEAR from it plus the Gold domain's
 * turbo-corner ceiling voltage (each domain's rimps_update_epss_shared_data()
 * call overwrites MPMM_GEAR unconditionally, so after L3/Silver/Gold run in
 * order, the stored value always reflects Gold's turbo corner -- same as
 * the vendor reference, which makes the same three calls in the same order).
 */
static void rimps_ipeak_measure(uint32_t vturbo_l3_mV)
{
	uint32_t fuse_raw = (mmio_read_32(QFPROM_CORR_PTE_ROW1_MSB) &
			     QFPROM_CORR_PTE_ROW1_MSB_IDDQ_APC_ACTIVE_BMSK) >>
			    QFPROM_CORR_PTE_ROW1_MSB_IDDQ_APC_ACTIVE_SHFT;
	uint32_t iddq_mA_x1000 = (fuse_raw >> 2U) * 1000U + (fuse_raw & 0x3U) * 25U;
	uint32_t ipeak_cal = vturbo_l3_mV * 1000U +
			     RIMPS_IPEAK_SLOPE * (iddq_mA_x1000 * 1000U / RIMPS_IPEAK_IDDQ_TT);
	uint32_t gear;

	if (fuse_raw == 0U) {
		gear = 0x2U;
	} else if (ipeak_cal < RIMPS_IPEAK_THRESHOLD_0) {
		gear = 0x3U;
	} else if (ipeak_cal < RIMPS_IPEAK_THRESHOLD_1) {
		gear = 0x0U;
	} else if (ipeak_cal < RIMPS_IPEAK_THRESHOLD_2) {
		gear = 0x1U;
	} else {
		gear = 0x2U;
	}

	epss_shared_data->mpmm_gear = gear;
}

static void rimps_config_lut_row(const struct rimps_domain *dom, uint32_t row,
				      const vpp_osm_t *osm, const vpp_cprh_t *cprh)
{
	mmio_clrsetbits_32(DCVS_LUT_FREQUENCY(dom->osm_top_base, row),
			    DCVS_LUT_FREQUENCY_CLKSRC_BMSK,
			    (uint32_t)osm->clk_src << DCVS_LUT_FREQUENCY_CLKSRC_SHFT);
	mmio_clrsetbits_32(DCVS_LUT_FREQUENCY(dom->osm_top_base, row),
			    DCVS_LUT_FREQUENCY_CC_BMSK,
			    (uint32_t)osm->max_core_count << DCVS_LUT_FREQUENCY_CC_SHFT);
	mmio_clrsetbits_32(DCVS_LUT_FREQUENCY(dom->osm_top_base, row),
			    DCVS_LUT_FREQUENCY_LVAL_BMSK,
			    (uint32_t)osm->lval << DCVS_LUT_FREQUENCY_LVAL_SHFT);

	mmio_clrsetbits_32(DCVS_LUT_VOLTAGE(dom->osm_top_base, row),
			    DCVS_LUT_VOLTAGE_VC_BMSK,
			    (uint32_t)osm->vc << DCVS_LUT_VOLTAGE_VC_SHFT);
	mmio_clrsetbits_32(DCVS_LUT_VOLTAGE(dom->osm_top_base, row),
			    DCVS_LUT_VOLTAGE_OLV_BMSK,
			    (uint32_t)cprh->ceil_mV << DCVS_LUT_VOLTAGE_OLV_SHFT);
}

static void rimps_config_lut(struct rimps_domain *dom)
{
	uint32_t osm_row;

	for (osm_row = 0U; osm_row < RIMPS_LUT_MAX_ROWS; osm_row++) {
		uint32_t vpp_row = MIN(dom->vpp->cnt - 1U, osm_row);

		rimps_config_lut_row(dom, osm_row, &dom->vpp->osm[vpp_row],
					  &dom->vpp->cprh[vpp_row]);
	}
}

/*
 * rimps_parse_vpp - derive per-domain OSM/ACD/AVG/BCL runtime state from
 * the VPP voltage plan. Port of Clock_ParseVPP(), minus the vendor's
 * MEMACC/APM crossover-VC tracking (nMEMACCCrossover*, nAPMCrossover*,
 * nCrossoverVoltageAPM): those feed only the dead APM_SW_MODE branch of
 * Clock_UpdateEPSSSharedData() and the unported vpp_get_memacc_vc(), so
 * tracking them here would have no observable effect.
 */
static void rimps_parse_vpp(struct rimps_domain *dom)
{
	const vpp_osm_t *osm;
	const vpp_osm_t *osm_prev;
	const vpp_cprh_t *cprh;
	uint32_t row;
	uint32_t dvm_idx, acdsscr_idx, avg_idx, bcl_idx;
	uint32_t bcl_min_lval;
	uint8_t apm_vc_unused;
	uint16_t apm_threshold_unused, apm_intermediate_unused;
	uint8_t apm_mode;

	dom->vpp = vpp_get_plan(dom->rail_name, dom->clk_name);
	RIMPS_CORE_VERIFY(dom->vpp != NULL);

	dom->first_svs_l1_vc = (uint8_t)RIMPS_MAX_VC;
	dom->first_turbo_vc = (uint8_t)RIMPS_MAX_VC;

	vpp_get_apm_vc(dom->rail_name, dom->clk_name, &apm_vc_unused,
		       &apm_threshold_unused, &apm_intermediate_unused, &apm_mode);
	dom->apm_mode = apm_mode;

	dvm_idx = 0U;
	acdsscr_idx = 0U;
	avg_idx = 0U;
	bcl_idx = 0U;
	bcl_min_lval = 0U;
	osm_prev = NULL;

	for (row = 0U; row < dom->vpp->cnt; row++) {
		osm = &dom->vpp->osm[row];
		cprh = &dom->vpp->cprh[row];

		/*
		 * Shikra's Huyara PLL needs >= 724mV at NOM or above when cold;
		 * capture the L-val of the first row that crosses into NOM so
		 * the BCL alarm levels below can be floor-clamped to it.
		 */
		if (dom->ctrl.vmin_0c_violation == RIMPS_MAX_VOLTAGE &&
		    osm->vlvl >= VPP_VLVL_NOM) {
			dom->ctrl.vmin_0c_violation =
				(cprh->ceil_mV > 724U) ? cprh->ceil_mV : 724U;
			bcl_min_lval = osm->lval;
		}

		if (dom->first_svs_l1_vc == (uint8_t)RIMPS_MAX_VC &&
		    osm->vlvl >= VPP_VLVL_SVS_L1) {
			dom->first_svs_l1_vc = osm->vc;
		}

		if (dom->first_turbo_vc == (uint8_t)RIMPS_MAX_VC &&
		    osm->vlvl >= VPP_VLVL_TUR) {
			dom->first_turbo_vc = osm->vc;
		}

		if ((osm_prev != NULL) && (osm->acd_acdcr != osm_prev->acd_acdcr)) {
			dom->dvm_last_vcs[dvm_idx] = osm_prev->vc;
			dvm_idx++;
		}
		dom->dvm_vals[dvm_idx] = osm->acd_acdcr;

		if ((osm_prev != NULL) && (osm->acd_acdsscr != osm_prev->acd_acdsscr)) {
			dom->acdsscr_last_vcs[acdsscr_idx] = osm_prev->vc;
			acdsscr_idx++;
		}
		dom->acdsscr_vals[acdsscr_idx] = osm->acd_acdsscr;

		if ((osm_prev != NULL) && (osm->acd_avg_cfg_2 != osm_prev->acd_avg_cfg_2)) {
			dom->avg_last_vcs[avg_idx] = osm_prev->vc;
			avg_idx++;
		}
		dom->avg_vals[avg_idx] = osm->acd_avg_cfg_2;

		osm_prev = osm;
	}

	osm = &dom->vpp->osm[dom->vpp->cnt - 1U];

	for (; dvm_idx < RIMPS_ACD_MAX_DVM_VALS; dvm_idx++) {
		dom->dvm_vals[dvm_idx] = osm->acd_acdcr;
		dom->dvm_last_vcs[dvm_idx] = osm->vc;
	}

	for (; acdsscr_idx < RIMPS_ACD_MAX_ACDSSCR_VALS; acdsscr_idx++) {
		dom->acdsscr_vals[acdsscr_idx] = osm->acd_acdsscr;
		dom->acdsscr_last_vcs[acdsscr_idx] = osm->vc;
	}

	for (; avg_idx < RIMPS_ACD_MAX_AVG_VALS; avg_idx++) {
		dom->avg_vals[avg_idx] = osm->acd_avg_cfg_2;
		dom->avg_last_vcs[avg_idx] = osm->vc;
	}

	/*
	 * BCL alarm levels: 0 = 100%, 1 = 80%, 2 = 60%, 3 = 40% of the top
	 * L-val, floor-clamped to bcl_min_lval (the L-val at the first
	 * NOM-or-above row) per 0C guidance.
	 */
	for (; bcl_idx < RIMPS_BCL_ALARM_MAX_LVLS; bcl_idx++) {
		uint32_t lval = osm->lval - ((osm->lval * 20U * bcl_idx) / 100U);

		if (bcl_min_lval != 0U && lval < bcl_min_lval) {
			lval = bcl_min_lval;
		}
		dom->bcl_alarm_lvals[bcl_idx] = (uint8_t)lval;
	}
}

static void rimps_update_epss_shared_data(struct rimps_domain *dom)
{
	const vpp_device_t *vpp_config = vpp_get_device_info();
	uint32_t turbo_corner;
	uint32_t vturbo_l3_mV;
	uint32_t idx;
	uint8_t d = dom->domain_id;

	/*
	 * L3_MX_VOTE/AVG_MASK_VC_THRESHOLD tracking is dead (gated behind
	 * ENABLE_L3_MX_VOTING/avg_enable-version-2, both unreachable on
	 * shikra); every domain converges on the same APM_CROSSOVER_CORNER
	 * sentinel and L3's MX_NOM/MX_TUR_DATA value.
	 */
	epss_shared_data->apm_crossover_corner[d] = 0xffU;
	if (d == RIMPS_DOMAIN_L3) {
		epss_shared_data->mx_tur_data = 0U;
		epss_shared_data->mx_nom_data = 0U;
	}

	epss_shared_data->mem_acc_nom_vc[d] = dom->first_svs_l1_vc;
	epss_shared_data->mem_acc_tur_vc[d] = dom->first_turbo_vc;

	for (idx = 0U; idx < RIMPS_ACD_MAX_DVM_VALS; idx++) {
		epss_shared_data->acd_dvm_vc[d][idx] = dom->dvm_last_vcs[idx];
	}
	for (idx = 0U; idx < RIMPS_ACD_MAX_ACDSSCR_VALS; idx++) {
		epss_shared_data->acd_acdsscr_vc[d][idx] = dom->acdsscr_last_vcs[idx];
	}
	for (idx = 0U; idx < RIMPS_ACD_MAX_AVG_VALS; idx++) {
		epss_shared_data->acd_avg_vc[d][idx] = dom->avg_last_vcs[idx];
	}
	for (idx = 0U; idx < RIMPS_ACD_MAX_DVM_VALS; idx++) {
		epss_shared_data->acd_dvm_val[d][idx] = dom->dvm_vals[idx];
	}
	for (idx = 0U; idx < RIMPS_ACD_MAX_AVG_VALS; idx++) {
		epss_shared_data->acd_avg_cfg[d][idx] = dom->avg_vals[idx];
	}
	for (idx = 0U; idx < RIMPS_ACD_MAX_ACDSSCR_VALS; idx++) {
		epss_shared_data->acd_acdsscr_val[d][idx] = dom->acdsscr_vals[idx];
	}
	for (idx = 0U; idx < RIMPS_BCL_ALARM_MAX_LVLS; idx++) {
		epss_shared_data->bcl_lval[d][idx] = dom->bcl_alarm_lvals[idx];
	}

	epss_shared_data->park_pll_lval[d] = (uint8_t)dom->vpp->osm[0].lval;
	epss_shared_data->park_vc[d] = dom->vpp->osm[0].vc;

	epss_shared_data->dcvs_down_saw_avs_dly[RIMPS_RAIL_APC0] =
		RIMPS_SILVER_AVS_DELAY_PERF_DOWN;

	/* No TF-A equivalent for tzbsp_is_sys_dbg_enable(); always deny access. */
	epss_shared_data->ns_logbuf_access_en = 0U;

	turbo_corner = dom->vpp->cnt - 1U;
	vturbo_l3_mV = dom->vpp->cprh[turbo_corner].ceil_mV;
	rimps_ipeak_measure(vturbo_l3_mV);

	epss_shared_data->speed_bin = vpp_config->speed_bin;
	epss_shared_data->sku_id = 0U; /* vpp_device_t has no sku_identifier field. */
	epss_shared_data->qultivate_fuse = 0U; /* No ChipInfo_GetDisabledCPUs() port. */
	epss_shared_data->iddq = 0U; /* Vendor literal: "TBD". */
}

/*
 * rimps_get_max_vc - highest LUT row index before DCVS_LUT_r_FREQUENCY's
 * L-val stops changing (i.e. the highest meaningfully-distinct row).
 */
static uint8_t rimps_get_max_vc(const struct rimps_domain *dom)
{
	uint8_t next_index = 1U;
	uint32_t cur_lval, next_lval;

	if (dom == NULL || dom->osm_top_base == 0U) {
		return (uint8_t)(RIMPS_LUT_MAX_ROWS - 1U);
	}

	cur_lval = (mmio_read_32(DCVS_LUT_FREQUENCY(dom->osm_top_base, 0U)) &
		    DCVS_LUT_FREQUENCY_LVAL_BMSK) >> DCVS_LUT_FREQUENCY_LVAL_SHFT;
	next_lval = (mmio_read_32(DCVS_LUT_FREQUENCY(dom->osm_top_base, next_index)) &
		    DCVS_LUT_FREQUENCY_LVAL_BMSK) >> DCVS_LUT_FREQUENCY_LVAL_SHFT;

	while (cur_lval != next_lval && next_index < RIMPS_LUT_MAX_ROWS - 1U) {
		cur_lval = next_lval;
		next_index++;
		next_lval = (mmio_read_32(DCVS_LUT_FREQUENCY(dom->osm_top_base, next_index)) &
			    DCVS_LUT_FREQUENCY_LVAL_BMSK) >> DCVS_LUT_FREQUENCY_LVAL_SHFT;
	}

	return (uint8_t)(next_index - 1U);
}

static void rimps_enable(struct rimps_domain *dom)
{
	if (dom->enabled) {
		return;
	}

	rimps_config_common(dom);
	mmio_clrsetbits_32(PLL_FSM_CTRL(dom->osm_top_base), PLL_FSM_CTRL_FSM_EN_BMSK,
			    1U << PLL_FSM_CTRL_FSM_EN_SHFT);
	mmio_clrsetbits_32(RIMPS_ENABLE(dom->osm_top_base), RIMPS_ENABLE_RIMPS_EN_BMSK,
			    1U << RIMPS_ENABLE_RIMPS_EN_SHFT);
	dom->enabled = true;
}

/*
 * rimps_init - port of Clock_InitOSM(). The gold-domain CPR workaround
 * (are_both_gold_cores_disabled()/cprh_disable_gold_domain_thread()/
 * cprh_thread_override_ctl()) depends on fused-CPU-disable state and CPRH
 * internals with no TF-A port, and does not apply when (as on this board)
 * neither Gold core is fused off; it is not ported. The L3-voting readback
 * branch is also not ported: RIMPSTOP_L3_VOTING_EN is written to 0 right
 * before Clock_InitOSM would read it back, so that branch is permanently
 * dead and every domain always takes the plain COREd_PERF_STATE_DESIRED
 * write below.
 */
static void rimps_init(void)
{
	uint32_t i;

	mmio_clrsetbits_32(RIMPSTOP_SECURE_ACCESS_OVERRIDE,
			    RIMPSTOP_SECURE_ACCESS_OVERRIDE_SECURE_ACCESS_OVERRIDE_BMSK, 0U);
	mmio_clrsetbits_32(RIMPSTOP_SECURE_ACCESS_OVERRIDE,
			    RIMPSTOP_SECURE_ACCESS_OVERRIDE_CLKDOM_TIMER_NON_SECURE_ACCESS_EN_BMSK,
			    1U << RIMPSTOP_SECURE_ACCESS_OVERRIDE_CLKDOM_TIMER_NON_SECURE_ACCESS_EN_SHFT);
	mmio_clrsetbits_32(RIMPSTOP_GLOBAL_ENABLE, RIMPSTOP_GLOBAL_ENABLE_GLOBAL_ENABLE_BMSK,
			    1U << RIMPSTOP_GLOBAL_ENABLE_GLOBAL_ENABLE_SHFT);

	rimps_fw_load();

	for (i = 0U; i < RIMPS_DOMAIN_COUNT; i++) {
		struct rimps_domain *dom = &rimps_dcvs_domains[i];

		rimps_parse_vpp(dom);
		rimps_config_lut(dom);
		rimps_update_epss_shared_data(dom);
	}

	rimps_init_apm();
	rimps_init_memacc();

	mmio_clrsetbits_32(RIMPSFAST_RIMPS_MUC_CLK_CTRL,
			    RIMPSFAST_RIMPS_MUC_CLK_CTRL_CORE_CLK_EN_BMSK,
			    1U << RIMPSFAST_RIMPS_MUC_CLK_CTRL_CORE_CLK_EN_SHFT);
	mmio_write_32(RIMPSFAST_CPU_LPM_ARB_CTL, 0x5U);
	mmio_write_32(RIMPSTOP_L3_VOTING_EN, 0U);

	for (i = 0U; i < RIMPS_DOMAIN_COUNT; i++) {
		struct rimps_domain *dom = &rimps_dcvs_domains[i];

		if (dom->domain_id == RIMPS_DOMAIN_L3) {
			mmio_clrsetbits_32(PLL_USER_CTL(dom->clk_ctl_base),
					    PLL_USER_CTL_POSTDIV_CTL_BMSK,
					    1U << PLL_USER_CTL_POSTDIV_CTL_SHFT);
		}

		if (dom->flag_init_request != 0U) {
			mmio_clrsetbits_32(CORE_PERF_STATE_DESIRED(dom->osm_top_base, 0U),
					    CORE_PERF_STATE_DESIRED_SW_INDEX_BMSK,
					    (uint32_t)rimps_get_max_vc(dom) <<
					    CORE_PERF_STATE_DESIRED_SW_INDEX_SHFT);
		}

		if (dom->flag_enable != 0U) {
			rimps_enable(dom);
		}
	}
}

/*
 * rimps_dcvs_init - port of Clock_InitCPU(): configures the misc APSS
 * clock muxes/dividers RIMPS depends on, enables the CPUCP subsystem clock,
 * then runs the full OSM/DCVS bring-up above.
 */
void rimps_dcvs_init(void)
{
	mmio_clrsetbits_32(APSS_AHB_CLK_CFG, APSS_AHB_CLK_CFG_CDIV_COUNT_BMSK,
			    5U << APSS_AHB_CLK_CFG_CDIV_COUNT_SHFT);
	mmio_clrsetbits_32(APSS_AHB_CLK_CFG, APSS_AHB_CLK_CFG_GFMUX_SRC_SEL_BMSK,
			    1U << APSS_AHB_CLK_CFG_GFMUX_SRC_SEL_SHFT);

	mmio_clrsetbits_32(APSS_LMH_GFMUX_CFG, APSS_LMH_GFMUX_CFG_SRC_SEL_BMSK,
			    1U << APSS_LMH_GFMUX_CFG_SRC_SEL_SHFT);
	mmio_clrsetbits_32(APSS_LMH_CDIV_COUNT, APSS_LMH_CDIV_COUNT_BMSK,
			    2U << APSS_LMH_CDIV_COUNT_SHFT);

	mmio_clrsetbits_32(APSS_OSM_GFMUX_CFG, APSS_OSM_GFMUX_CFG_SRC_SEL_BMSK,
			    1U << APSS_OSM_GFMUX_CFG_SRC_SEL_SHFT);
	mmio_clrsetbits_32(APSS_OSM_CDIV_COUNT, APSS_OSM_CDIV_COUNT_BMSK,
			    1U << APSS_OSM_CDIV_COUNT_SHFT);

	mmio_clrsetbits_32(APSS_PERIPHCLK_GFMUX_CFG, APSS_PERIPHCLK_GFMUX_CFG_SRC_SEL_BMSK,
			    1U << APSS_PERIPHCLK_GFMUX_CFG_SRC_SEL_SHFT);
	mmio_clrsetbits_32(APSS_PERIPH_CDIV_COUNT, APSS_PERIPH_CDIV_COUNT_BMSK,
			    1U << APSS_PERIPH_CDIV_COUNT_SHFT);

	mmio_clrsetbits_32(APSS_IPM_GFMUX_CFG, APSS_IPM_GFMUX_CFG_SRC_SEL_BMSK,
			    1U << APSS_IPM_GFMUX_CFG_SRC_SEL_SHFT);
	mmio_clrsetbits_32(APSS_IPM_CDIV_COUNT, APSS_IPM_CDIV_COUNT_BMSK,
			    3U << APSS_IPM_CDIV_COUNT_SHFT);

	mmio_clrsetbits_32(RIMPSFAST_RIMPS_MUC_CLK_CTRL,
			    RIMPSFAST_RIMPS_MUC_CLK_CTRL_CORE_SS_CLK_EN_BMSK,
			    1U << RIMPSFAST_RIMPS_MUC_CLK_CTRL_CORE_SS_CLK_EN_SHFT);
	while ((mmio_read_32(RIMPSFAST_RIMPS_MUC_CLK_CTRL) &
		RIMPSFAST_RIMPS_MUC_CLK_CTRL_CORE_SS_CLK_EN_BMSK) == 0U) {
	}

	rimps_init();
}
