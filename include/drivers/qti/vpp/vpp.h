/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * VPP (Voltage Plan Processor) public API.
 *
 * VPP turns fused per-corner calibration data into the ceil/floor voltage
 * and target quotient that CPRH programs into its closed-loop AVS
 * controller, and into the OSM LUT entries (L-val/clk-div/ACD config) a
 * clock domain runs at for each virtual corner. vpp_init() must run before
 * any client calls vpp_get_plan() or vpp_get_apm_vc().
 *
 * vpp_get_mx_vote_table() and vpp_get_memacc_vc() are not declared here:
 * the shikra reference always returns is_required = 0 / a VPP_CORE_VERIFY(0)
 * fatal for them, so they have no caller-visible behavior to port.
 */

#ifndef VPP_H
#define VPP_H

#include <stdint.h>

/* Maximum OSM LUT entries per rail/clock domain. */
#define VPP_MAX_LUT_SIZE	40U

typedef enum {
	VPP_VLVL_OFF        = 0,
	VPP_VLVL_RET        = 16,
	VPP_VLVL_MINSVS     = 48,
	VPP_VLVL_LOWSVS     = 64,
	VPP_VLVL_SVS        = 128,
	VPP_VLVL_SVS_L1     = 192,
	VPP_VLVL_NOM        = 256,
	VPP_VLVL_NOM_L1     = 320,
	VPP_VLVL_NOM_L2     = 336,
	VPP_VLVL_TUR        = 384,
	VPP_VLVL_TUR_L1     = 416,
	VPP_VLVL_TUR_L2     = 464,
	VPP_VLVL_TUR_L3     = 476,
	VPP_VLVL_TUR_BOOST  = 488,
	VPP_VLVL_BOOST      = VPP_VLVL_TUR_BOOST,
	VPP_VLVL_BOOSTP     = 500,
	VPP_VLVL_BIN        = 512,
	VPP_VLVL_INVALID    = 0x7fffffff,
} vpp_vlvl_t;

typedef struct {
	uint8_t acd_enable;
	uint8_t aging_enable;
	uint8_t aging_margin_enable;
	uint8_t avg_enable;
	uint8_t apm_auto_enable;
} vpp_prop_feature_t;

typedef struct {
	uint32_t chip_ver;
	uint8_t speed_bin;
	uint8_t cpr_local_rev;
	uint32_t pte_mcn_identifier;
	uint32_t vpid;
	vpp_prop_feature_t prop;
} vpp_device_t;

/* Per-virtual-corner OSM LUT entry for a clock domain. */
typedef struct {
	vpp_vlvl_t vlvl;
	uint32_t freq_kHz;
	uint16_t lval;
	uint8_t clk_div;
	uint8_t clk_src;
	uint8_t max_core_count;
	uint8_t droop_code;
	uint16_t dcvs_safe_freq;
	uint16_t cc_safe_freq;
	uint8_t mem_acc_lvl;
	uint8_t vc;
	uint32_t acd_acdcr;
	uint32_t acd_acdsscr;
	uint32_t acd_avg_cfg_2;
} vpp_osm_t;

/* Per-virtual-corner CPRh programming data, as consumed by the CPRH driver. */
typedef struct {
	uint8_t vc;
	uint8_t band;
	uint16_t ceil_mV;
	uint16_t floor_mV;
	uint16_t target_quot;
	uint8_t ro;
} vpp_cprh_t;

/* Public vpp context per rail/clock domain. */
typedef struct {
	const char *rail_name;
	const char *clk_name;
	uint16_t cnt;
	vpp_osm_t osm[VPP_MAX_LUT_SIZE];
	vpp_cprh_t cprh[VPP_MAX_LUT_SIZE];
} vpp_t;

enum vpp_apm_mode {
	APM_DISABLED = 0,
	APM_SW_MODE,
	APM_AUTO_MODE,
};

/*
 * vpp_get_apm_vc - virtual corner / threshold for the APM intermediate
 * voltage crossover on a rail/clock domain.
 *
 * is_required is set to an enum vpp_apm_mode value (APM_AUTO_MODE for
 * shikra, since VPP_AUTO_APM is enabled in vpp_cfg.h).
 */
void vpp_get_apm_vc(const char *rail, const char *clk, uint8_t *apm_vc,
		    uint16_t *apm_threshold_mV, uint16_t *apm_intermediate_mV,
		    uint8_t *is_required);

/* vpp_get_plan - osm/cprh data for a rail/clock domain, or NULL if unknown. */
const vpp_t *vpp_get_plan(const char *rail, const char *clk);

/* vpp_init - read device fuses and compute every rail/clock domain's plan. */
void vpp_init(void);

/* vpp_get_device_info - fused device info populated by vpp_init(). */
const vpp_device_t *vpp_get_device_info(void);

#endif /* VPP_H */
