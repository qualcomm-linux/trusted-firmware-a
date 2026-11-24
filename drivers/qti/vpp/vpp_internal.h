/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Internal VPP types and interfaces shared by the interpolation engine
 * and Shikra voltage-plan implementation.
 */

#ifndef VPP_INTERNAL_H
#define VPP_INTERNAL_H

#include <stdint.h>

#include <common/debug.h>
#include <drivers/qti/vpp/vpp.h>

#define VPP_CORE_VERIFY(x)					\
	do {							\
		if ((x) == 0) {				\
			ERROR("vpp: invariant failed: %s\n", #x); \
			panic();				\
		}						\
	} while (0)

/* Callback reading a fused value out of eFuse shadow registers. */
typedef int32_t (*fuse_cb_t)(void);

/*
 * KV (quotient-per-mV) plan. kv_min/kv_max bound the valid runtime-computed
 * kv value for the ring-oscillator select in effect (shikra uses the
 * non-OPTIMIZED_VP, per-RO-select layout from the vendor reference).
 */
typedef struct {
	vpp_vlvl_t min_vlvl;
	vpp_vlvl_t max_vlvl;
	uint32_t kv[16];
	uint32_t kv_min[16];
	uint32_t kv_max[16];
} vpp_kv_t;

/* Per-corner fuse adjustment, selected on aging-margin enable + CPR rev. */
typedef struct {
	vpp_vlvl_t vlvl;
	int16_t aging_off_olv_fuse_adj;
	int16_t aging_off_clv_fuse_adj;
	int16_t aging_on_olv_fuse_adj;
	int16_t aging_on_clv_fuse_adj;
	int16_t slt_offset_adj;
	uint16_t ceil_floor_range_mV;
} vpp_fuse_adj_t;

/* Per-corner virtual adjustment, selected by CPR local revision + VPID. */
typedef struct {
	vpp_vlvl_t vlvl;
	uint32_t freq_kHz;
	int16_t olv_adj_acd_mV;
	int16_t clv_adj_acd_mV;
	int16_t clv_vdelta_mv;
} vpp_virtual_adj_t;

typedef struct {
	vpp_vlvl_t vlvl;
	uint32_t freq_kHz;
	uint8_t en;
	uint8_t vc;
	uint16_t olv_ref_mV;
	uint16_t initial_ceil_mV;
	uint16_t initial_floor_mV;
	uint8_t olv_fused_point;
	uint8_t olv_interpolate;
	uint8_t quot_fused_point;
	uint8_t quot_interpolate;
	fuse_cb_t fused_olv;
	fuse_cb_t fused_quot;
	fuse_cb_t fused_quot_offset;
	fuse_cb_t fused_ro;
	int16_t olv_adj_acd_mV;
	int16_t clv_adj_acd_mV;
	int16_t clv_vdelta_mv;
	/* Populated at runtime by vpp_init_virtual_corner(). */
	vpp_cprh_t cprh;
	vpp_fuse_adj_t *olv_fuse_adj_table;
	vpp_fuse_adj_t *clv_fuse_adj_table;
} vpp_voltage_settings_t;

/* Internal per-rail/clock-domain context, built fresh for each vpp_t entry. */
typedef struct {
	const char *rail_name;
	const char *clk_name;
	vpp_voltage_settings_t *voltage_setting;
	vpp_osm_t *osm_setting;
	vpp_fuse_adj_t *fuse_adj;
	vpp_kv_t *kv;
} vpp_internal_t;

void vpp_init_virtual_corner(vpp_internal_t *vpp, const vpp_device_t *vpp_prop);

/* Shikra hooks used by the common interpolation engine. */
void vpp_validate_voltage_plan(vpp_internal_t *vpp, const vpp_device_t *vpp_prop);
void vpp_validate_virtual_corner(vpp_internal_t *vpp, uint8_t volt_setting_count);

#endif /* VPP_INTERNAL_H */
