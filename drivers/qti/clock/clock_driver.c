/*
 * Copyright (c) 2026 Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * ATF clock driver implementation.
 *
 * The lemans (qcs9075) boot flow only needs clock-group bring-up/teardown and
 * source enables, so the driver exposes just that surface: qti_clock_init(),
 * qti_clock_init_done(), clock_enable_clock_group(), clock_disable_clock_group()
 * and clock_enable_source(). The generic by-name clock-control and
 * frequency-scaling APIs are intentionally not provided for this target.
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <assert.h>

#include <drivers/qti/clock/clock.h>
#include <drivers/qti/clock/clock_bsp.h>
#include <drivers/qti/clock/clock_driver.h>
#include <drivers/qti/clock/icbuarb.h>
#include <drivers/qti/clock/qti_sec_chipinfo.h>
#include <drivers/qti/cmd_db/cmd_db.h>
#include <drivers/qti/pwr_utils/pwr_utils.h>
#include <drivers/qti/rpmh/rpmh_client.h>

/*
 * BSP configuration data, provided by the chipset back-end.
 */
extern struct clock_tz_bsp clock_tz_bsp_config;

/*
 * RPMh command set indices.
 */
#define CLOCK_CX_SET_IDX		0U
#define CLOCK_MX_SET_IDX		1U

/*
 * Value to enable the path to XO.
 */
#define CLOCK_XO_ENABLE_VAL		3U

/*
 * Clock driver context.
 */
static struct clock_drv_ctxt clock_drv_ctxt = {
	.bsp = &clock_tz_bsp_config,
	.cx_cmd_set = {
		.set = RPMH_SET_ACTIVE,
		.num_commands = 2U,
		.commands[CLOCK_CX_SET_IDX].completion = true,
		.commands[CLOCK_MX_SET_IDX].completion = true,
	},
};

static void clock_init_rpmh(void)
{
	struct clock_drv_ctxt *drv_ctxt = &clock_drv_ctxt;
	rpmh_command_set_t *cmd_set = &drv_ctxt->cx_cmd_set;
	uint32_t cx_addr, xo_addr;
	uint32_t req_id;
	uint32_t device_id = qti_sec_get_device_id();
	uint32_t soc_id = qti_sec_get_soc_id();

	/* RPMh has no common init hook; first consumer initialises. */
	rpmh_client_init();

	/* Set up handles and query data. */
	drv_ctxt->rpmh = rpmh_create_handle(RSC_DRV_TZ, "clock");
	drv_ctxt->cx_lvl_idx = pwr_utils_lvl_resource_idx("cx.lvl");
	cx_addr = cmd_db_query_addr("cx.lvl");
	xo_addr = cmd_db_query_addr("xo.lvl");

	/* Verify everything. */
	assert(drv_ctxt->rpmh != NULL);
	assert(drv_ctxt->cx_lvl_idx != -1);
	assert(cx_addr != 0U);
	assert(xo_addr != 0U);

	if ((soc_id == SOC_ID_TALOS) &&
	    (device_id != DEVICE_ID_SA6155P) &&
	    (device_id != DEVICE_ID_SA4155P) &&
	    (device_id != DEVICE_ID_SA6155) &&
	    (device_id != DEVICE_ID_QCS615)) {
		uint32_t mx_addr;

		drv_ctxt->mx_lvl_idx = pwr_utils_lvl_resource_idx("mx.lvl");
		mx_addr = cmd_db_query_addr("mx.lvl");
		assert(mx_addr != 0U);
		cmd_set->commands[CLOCK_MX_SET_IDX].address = mx_addr;
	}

	/* Set command set addresses. */
	cmd_set->commands[CLOCK_CX_SET_IDX].address = cx_addr;

	/*
	 * Send a single, active set request for XO. There is no need to do
	 * this dynamically based on usage, since SP never needs a request in
	 * its sleep set.
	 */
	req_id = rpmh_issue_command(drv_ctxt->rpmh, RPMH_SET_ACTIVE, true,
				    xo_addr, CLOCK_XO_ENABLE_VAL);
	rpmh_barrier_single(drv_ctxt->rpmh, req_id);
}

int clock_enable_source(struct clock_source *source)
{
	if (source == NULL) {
		return -1;
	}

	if (source->source != NULL) {
		if (clock_enable_source(source->source) != 0) {
			return -1;
		}
	}

	/*
	 * Only votable PLLs are driven here. RPM-owned source voting is not
	 * supported on this target (no source declares an RPM resource).
	 */
	if (source->ref_count++ == 0U) {
		clock_hal_enable_source(&source->hw_source);
	}

	return 0;
}

static int clock_enable_clock_group_internal(struct clock_group *group)
{
	struct clock_clk_desc *clock;
	struct clock_power_domain_desc *pd;
	bool timeout = false;
	struct clock_icb_request *icb;
	struct icb_bw_req icb_req = {0};
	struct clock_voltage_request *vrequest;
	uint32_t rpm_req_id;
	struct clock_drv_ctxt *drv_ctxt = &clock_drv_ctxt;

	if (group->voltage_requests != NULL) {
		group->num_voltage_requests = 0U;
		for (vrequest = group->voltage_requests;
		     vrequest->rail != NULL; vrequest++) {
			group->num_voltage_requests++;

			if (vrequest->rail_addr == 0) {
				vrequest->rail_addr = (int)cmd_db_query_addr(
					vrequest->rail);
				vrequest->hlvl =
					pwr_utils_hlvl_named_resource(
						vrequest->rail,
						vrequest->vlvl, NULL);
				assert(vrequest->rail_addr != 0);
				assert(vrequest->hlvl != -1);
			}

			rpm_req_id = rpmh_issue_command(drv_ctxt->rpmh,
							RPMH_SET_ACTIVE, true,
							(uint32_t)vrequest->rail_addr,
							(uint32_t)vrequest->hlvl);
			rpmh_barrier_single(drv_ctxt->rpmh, rpm_req_id);
		}
	}

	if (group->icb_requests != NULL) {
		group->num_icb_requests = 0U;
		for (icb = group->icb_requests; icb->request != 0U; icb++) {
			group->num_icb_requests++;

			if (icb->icb == NULL) {
				icb->icb = icbuarb_create_client(
					icb->master, icb->slave);
				assert(icb->icb != NULL);
			}

			/*
			 * Request both ib/ab, since some routes only support
			 * one or the other.
			 */
			icb_req.ib = icb->request;
			icb_req.ab = icb->request;
			icbuarb_issue_request(icb->icb, &icb_req);
		}
	}

	if (group->access_clocks != NULL) {
		group->num_access_clocks = 0U;
		for (clock = group->access_clocks; clock->cbcr_addr != 0U;
		     clock++) {
			group->num_access_clocks++;
			/*
			 * Do not invoke the accessor if the resource is
			 * already enabled by TZ. This would be the case if
			 * there is a second call to clock_enable_clock_group
			 * after the first one failed. Without this condition,
			 * the driver would think some other entity enabled the
			 * clock and clock_disable_clock_group would not disable
			 * it.
			 */
			if (!clock->tz_enabled) {
				clock_hal_enable_clock(clock);
			}
			if (clock_hal_wait_for_clock_on(clock) != 0) {
				return -1;
			}
		}
	}

	if (group->power_domains != NULL) {
		group->num_power_domains = 0U;
		for (pd = group->power_domains; pd->gdscr_addr != 0U; pd++) {
			group->num_power_domains++;
			if (!pd->tz_enabled) {
				clock_hal_enable_power_domain(pd);
			}
		}
		for (pd = group->power_domains; pd->gdscr_addr != 0U; pd++) {
			timeout |= (clock_hal_wait_for_power_domain_on(pd) != 0);
		}
	}

	group->num_clocks = 0U;
	for (clock = group->clocks; clock->cbcr_addr != 0U; clock++) {
		group->num_clocks++;
		if (!clock->tz_enabled) {
			clock_hal_enable_clock(clock);
		}
	}
	for (clock = group->clocks; clock->cbcr_addr != 0U; clock++) {
		timeout |= (clock_hal_wait_for_clock_on(clock) != 0);
	}

	if (timeout) {
		return -1;
	}

	return 0;
}

static int clock_disable_clock_group_internal(struct clock_group *group)
{
	struct clock_clk_desc *clock;
	struct clock_power_domain_desc *pd;
	struct clock_icb_request *icb;
	struct icb_bw_req icb_no_req = {0};
	struct clock_voltage_request *vrequest;
	struct clock_drv_ctxt *drv_ctxt = &clock_drv_ctxt;

	for (clock = group->clocks + group->num_clocks - 1U;
	     clock >= group->clocks; clock--) {
		/* Disable the resource only if TZ enabled it. */
		if (clock->tz_enabled) {
			clock_hal_disable_clock(clock);
		}
	}

	if (group->power_domains != NULL) {
		for (pd = group->power_domains + group->num_power_domains - 1U;
		     pd >= group->power_domains; pd--) {
			if (pd->tz_enabled) {
				clock_hal_disable_power_domain(pd);
			}
		}
	}

	if (group->access_clocks != NULL) {
		for (clock = group->access_clocks +
			     group->num_access_clocks - 1U;
		     clock >= group->access_clocks; clock--) {
			if (clock->tz_enabled) {
				clock_hal_disable_clock(clock);
			}
		}
	}

	if (group->icb_requests != NULL) {
		for (icb = group->icb_requests + group->num_icb_requests - 1U;
		     icb >= group->icb_requests; icb--) {
			if (icb->icb != NULL) {
				icbuarb_issue_request(icb->icb, &icb_no_req);
			}
		}
	}

	if (group->voltage_requests != NULL) {
		for (vrequest = group->voltage_requests +
				group->num_voltage_requests - 1U;
		     vrequest >= group->voltage_requests; vrequest--) {
			if (vrequest->rail_addr != 0) {
				rpmh_issue_command(drv_ctxt->rpmh,
						   RPMH_SET_ACTIVE, true,
						   (uint32_t)vrequest->rail_addr,
						   0U);
			}
		}
	}

	return 0;
}

void qti_clock_init(void)
{
	if (clock_drv_ctxt.initialized) {
		return;
	}

	clock_init_rpmh();

	assert(clock_init_image(&clock_drv_ctxt) == 0);

	clock_drv_ctxt.initialized = true;
}

void qti_clock_init_done(void)
{
	assert(clock_post_init_image(&clock_drv_ctxt) == 0);
}

int clock_enable_clock_group(enum clock_group_type group_type)
{
	struct clock_group *group;

	if ((group_type >= CLOCK_GROUP_TOTAL) ||
	    (clock_drv_ctxt.bsp->clock_groups == NULL)) {
		return -1;
	}

	group = &clock_drv_ctxt.bsp->clock_groups[group_type];
	if (group->clocks == NULL) {
		return -1;
	}

	/*
	 * If the previous clock_enable_clock_group invocation failed, retry
	 * enabling the resources and return an error if that fails again.
	 * Without this condition, subsequent clock_enable_clock_group calls
	 * return a misleading success.
	 */
	if ((group->ref_count++ == 0U) || group->prev_failed) {
		if (clock_enable_clock_group_internal(group) != 0) {
			group->prev_failed = true;
			return -1;
		}
	}

	/*
	 * Clear the failure flag in case the subsequent clock_enable_clock_group
	 * calls succeeded.
	 */
	group->prev_failed = false;

	return 0;
}

int clock_disable_clock_group(enum clock_group_type group_type)
{
	struct clock_group *group;

	if ((group_type >= CLOCK_GROUP_TOTAL) ||
	    (clock_drv_ctxt.bsp->clock_groups == NULL)) {
		return -1;
	}

	group = &clock_drv_ctxt.bsp->clock_groups[group_type];
	if (group->clocks == NULL) {
		return -1;
	}

	if ((group->ref_count > 0U) && (group->ref_count-- == 1U)) {
		if (clock_disable_clock_group_internal(group) != 0) {
			return -1;
		}
	}

	return 0;
}
