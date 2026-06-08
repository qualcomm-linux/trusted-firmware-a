/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * BSP data types for the QTI clock driver.
 *
 * Scoped to the data the lemans (qcs9075) boot flow consumes: clock groups
 * (clocks, power domains, access clocks, ICB and voltage votes) and the PLL
 * source tree. The per-clock-node, clock-domain and frequency-plan types from
 * the original vendor BSP are omitted along with the APIs that used them.
 */

#ifndef CLOCK_BSP_H
#define CLOCK_BSP_H

#include <stdbool.h>
#include <stdint.h>

#include <drivers/qti/clock/clock_types.h>
#include <drivers/qti/clock/icbuarb.h>
#include <drivers/qti/pwr_utils/voltage_level.h>

/* Clock source node. */
struct clock_source {
	struct clock_source_desc	hw_source;
	uint32_t			ref_count;
	struct clock_source		*source;
};

/* ICB bandwidth request descriptor. */
struct clock_icb_request {
	icb_master_id		master;
	icb_slave_id		slave;
	uint32_t		request;
	icb_client_handle	icb;
};

/* Rail voltage request descriptor. */
struct clock_voltage_request {
	const char		*rail;
	enum rail_voltage_level	vlvl;
	int			rail_addr;
	int			hlvl;
};

/* Group of clocks/power-domains/requests managed together. */
struct clock_group {
	struct clock_clk_desc		*clocks;
	struct clock_power_domain_desc	*power_domains;
	struct clock_clk_desc		*access_clocks;
	struct clock_icb_request	*icb_requests;
	struct clock_voltage_request	*voltage_requests;
	uint32_t			ref_count;
	uint32_t			num_clocks;
	uint32_t			num_power_domains;
	uint32_t			num_access_clocks;
	uint32_t			num_icb_requests;
	uint32_t			num_voltage_requests;
	bool				prev_failed;
};

/* Top-level BSP data structure. */
struct clock_tz_bsp {
	struct clock_group	*clock_groups;
	struct clock_source	*sources;
};

#endif /* CLOCK_BSP_H */
