/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Rail voltage voting for the QTI clock driver (RPMh/BCM back-end). See
 * clock_rail.h for what these votes are for and when the stubs are built.
 */

#include <stdbool.h>
#include <stdint.h>

#include <assert.h>

#include <drivers/qti/chipinfo/chipinfo.h>
#include <drivers/qti/clock/clock_bsp.h>
#include <drivers/qti/clock/clock_rail.h>
#include <drivers/qti/cmd_db/cmd_db.h>
#include <drivers/qti/pwr_utils/pwr_utils.h>
#include <drivers/qti/rpmh/rpmh_client.h>

/* Value to enable the path to XO. */
#define CLOCK_XO_ENABLE_VAL	3U

static struct rpmh_client *clock_rpmh;

/*
 * cx/mx rail addresses, resolved once in clock_rail_init(). On targets where
 * mx is merged into cx it has no cmd-DB entry, so clock_mx_addr stays 0 and
 * the mx vote/clear is skipped — cx alone then covers the merged rail.
 */
static uint32_t clock_cx_addr;
static uint32_t clock_mx_addr;

void clock_rail_init(void)
{
	uint32_t xo_addr;
	uint32_t req_id;
	int cx_hlvl;

	clock_rpmh = rpmh_create_handle(RSC_DRV_TZ, "clock");
	xo_addr = cmd_db_query_addr("xo.lvl");
	clock_cx_addr = cmd_db_query_addr("cx.lvl");
	clock_mx_addr = cmd_db_query_addr("mx.lvl");

	assert(clock_rpmh != NULL);
	assert(xo_addr != 0U);
	assert(clock_cx_addr != 0U);

	/* SP never needs a request in its sleep set, so a static active vote suffices. */
	req_id = rpmh_issue_command(clock_rpmh, RPMH_SET_ACTIVE, true,
				    xo_addr, CLOCK_XO_ENABLE_VAL);
	rpmh_barrier_single(clock_rpmh, req_id);

	/*
	 * Hold cx (and mx, when not merged into cx) up across clock init, the
	 * same static NOM hold applied to mmcx/gfx for xPU/secure programming.
	 * Released in clock_rail_deinit() at init-done.
	 */
	cx_hlvl = pwr_utils_hlvl_named_resource("cx.lvl",
						RAIL_VOLTAGE_LEVEL_NOM, NULL);
	assert(cx_hlvl != -1);
	req_id = rpmh_issue_command(clock_rpmh, RPMH_SET_ACTIVE, true,
				    clock_cx_addr, (uint32_t)cx_hlvl);
	rpmh_barrier_single(clock_rpmh, req_id);

	if (clock_mx_addr != 0U) {
		int mx_hlvl = pwr_utils_hlvl_named_resource(
			"mx.lvl", RAIL_VOLTAGE_LEVEL_NOM, NULL);

		assert(mx_hlvl != -1);
		req_id = rpmh_issue_command(clock_rpmh, RPMH_SET_ACTIVE, true,
					    clock_mx_addr, (uint32_t)mx_hlvl);
		rpmh_barrier_single(clock_rpmh, req_id);
	}
}

void clock_rail_deinit(void)
{
	/* Release the cx/mx holds taken in clock_rail_init(). */
	rpmh_issue_command(clock_rpmh, RPMH_SET_ACTIVE, true,
			   clock_cx_addr, 0U);

	if (clock_mx_addr != 0U) {
		rpmh_issue_command(clock_rpmh, RPMH_SET_ACTIVE, true,
				   clock_mx_addr, 0U);
	}
}

void clock_rail_vote(struct clock_group *group)
{
	struct clock_voltage_request *vrequest;
	uint32_t req_id;

	if (group->voltage_requests == NULL) {
		return;
	}

	group->num_voltage_requests = 0U;
	for (vrequest = group->voltage_requests; vrequest->rail != NULL;
	     vrequest++) {
		group->num_voltage_requests++;

		if (chipinfo_is_part_disabled(vrequest->part,
					      vrequest->part_idx)) {
			continue;
		}

		if (vrequest->rail_addr == 0) {
			vrequest->rail_addr =
				(int)cmd_db_query_addr(vrequest->rail);
			vrequest->hlvl = pwr_utils_hlvl_named_resource(
				vrequest->rail, vrequest->vlvl, NULL);
			assert(vrequest->rail_addr != 0);
			assert(vrequest->hlvl != -1);
		}

		req_id = rpmh_issue_command(clock_rpmh, RPMH_SET_ACTIVE, true,
					    (uint32_t)vrequest->rail_addr,
					    (uint32_t)vrequest->hlvl);
		rpmh_barrier_single(clock_rpmh, req_id);
	}
}

void clock_rail_clear(struct clock_group *group)
{
	struct clock_voltage_request *vrequest;
	uint32_t i;

	if (group->voltage_requests == NULL) {
		return;
	}

	for (i = group->num_voltage_requests; i > 0U; i--) {
		vrequest = &group->voltage_requests[i - 1U];
		if (vrequest->rail_addr != 0) {
			rpmh_issue_command(clock_rpmh, RPMH_SET_ACTIVE, true,
					   (uint32_t)vrequest->rail_addr, 0U);
		}
	}
}
