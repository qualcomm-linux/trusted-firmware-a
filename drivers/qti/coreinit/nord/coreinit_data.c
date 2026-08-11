/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Core initialization driver - target-specific configuration data.
 * Provides coreinit_info, consumed by coreinit_init() / coreinit_deinit().
 *
 * To add a new chip variant:
 *   1. Define its routes[], requests[], and handle-storage arrays.
 *   2. Define a coreinit_vote[] and a coreinit_config for it.
 *   3. Add a pointer to the new coreinit_config in coreinit_configs[].
 */

#include <stddef.h>

#include <drivers/qti/chipinfo/chipinfo.h>
#include <drivers/qti/icb/icbuarb.h>
#include <drivers/qti/icb/icbid.h>

#include "coreiniti.h"

/* =========================================================================
 * nord v1.0
 * ======================================================================= */

/*
 * ICB master/slave routes for nord v1.
 * routes[i] corresponds to requests[i] and lemans_v1_vote0_handles[i].
 */
static struct coreinit_route routes[] = {
	[0] = {
		.master = ICBID_MASTER_MDP0,
		.slave  = ICBID_SLAVE_EBI1,
	},
	[1] = {
		.master = ICBID_MASTER_NSP0_PROC,
		.slave  = ICBID_SLAVE_EBI1,
	},
	[2] = {
		.master = ICBID_MASTER_NSP1_PROC,
		.slave  = ICBID_SLAVE_EBI1,
	},
	[3] = {
		.master = ICBID_MASTER_NSP2_PROC,
		.slave  = ICBID_SLAVE_EBI1,
	},
	[4] = {
		.master = ICBID_MASTER_NSP3_PROC,
		.slave  = ICBID_SLAVE_EBI1,
	},
};

/*
 * Bandwidth requests for nord v1.
 * ab: average bandwidth (bytes/sec); ib: instantaneous bandwidth (bytes/sec).
 * ib = 1 requests a minimum instantaneous vote without a sustained average.
 */
static struct icb_bw_req requests[] = {
	[0] = {
		.ab = 0ULL,
		.ib = 1ULL,
	},
	[1] = {
		.ab = 0ULL,
		.ib = 1ULL,
	},
	[2] = {
		.ab = 0ULL,
		.ib = 1ULL,
	},
};

/*
 * ICB client handle storage for nord v1, vote 0.
 * One slot per route; initialised to NULL and populated at runtime by
 * coreinit_init().
 */
static icb_client_handle nord_v1_vote0_handles[ARRAY_SIZE(routes)] = {
	NULL
};

/*
 * Vote configurations for nord v1.
 */
static struct coreinit_vote nord_v1_votes[] = {
	[0] = {
		.num_routes = ARRAY_SIZE(routes),
		.routes     = routes,
		.requests   = requests,
		.handles    = nord_v1_vote0_handles,
	},
};

/*
 * nord v1.0 configuration.
 * match = false: applies to all versions of CHIPINFO_FAMILY_NORDAU.
 */
static struct coreinit_config nord_v1 = {
	.family    = CHIPINFO_FAMILY_NORDAU,
	.match     = false,
	.version   = CHIPINFO_VERSION(1, 0),
	.num_votes = ARRAY_SIZE(nord_v1_votes),
	.votes     = nord_v1_votes,
};

/*
 * Array of all chip configurations, searched in order by coreinit_init().
 */
static struct coreinit_config *coreinit_configs[] = {
	[0] = &nord_v1,
};

/*
 * Exported top-level symbol.
 * Declared extern in coreinit.c; resolved by the linker at build time.
 */
struct coreinit_info coreinit_info = {
	.num_configs = ARRAY_SIZE(coreinit_configs),
	.configs     = coreinit_configs,
};
