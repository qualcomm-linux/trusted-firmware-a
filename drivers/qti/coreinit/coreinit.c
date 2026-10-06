/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Core initialization driver.
 * Manages ICB bus votes required during platform core initialization.
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <common/debug.h>
#include <drivers/qti/chipinfo/chipinfo.h>
#include <drivers/qti/icb/icbuarb.h>
#include <drivers/qti/coreinit/coreinit.h>

#include "coreiniti.h"

/* Defined per-target in coreinit_data.c; kept out of the public header. */
extern struct coreinit_info coreinit_info;

/* Cached so coreinit_deinit() need not repeat the chip-family search. */
static struct coreinit_config *coreinit_cfg;

static struct coreinit_config *coreinit_find_config(void)
{
	enum chipinfo_family family = chipinfo_get_chip_family();
	uint32_t version = (uint32_t)chipinfo_get_chip_version();
	uint32_t i = 0U;

	for (i = 0U; i < coreinit_info.num_configs; i++) {
		struct coreinit_config *cfg = coreinit_info.configs[i];

		if (cfg == NULL)
			continue;

		if (cfg->family != family)
			continue;

		if (cfg->match && (cfg->version != version))
			continue;

		return cfg;
	}

	return NULL;
}

/* -------------------------------------------------------------------------
 * Public API
 * ---------------------------------------------------------------------- */

void qti_coreinit_init(void)
{
	uint32_t i = 0U;
	uint32_t j = 0U;

	coreinit_cfg = coreinit_find_config();
	if (coreinit_cfg == NULL)
		return;

	for (i = 0U; i < coreinit_cfg->num_votes; i++) {
		struct coreinit_vote *vote = &coreinit_cfg->votes[i];

		for (j = 0U; j < vote->num_routes; j++) {
			if (vote->handles[j] == NULL) {
				vote->handles[j] = icbuarb_create_client(
					vote->routes[j].master,
					vote->routes[j].slave);
			}

			if (vote->handles[j] == NULL) {
				ERROR("coreinit: icbuarb_create_client"
				      " failed: master %u slave %u\n",
				      (unsigned int)vote->routes[j].master,
				      (unsigned int)vote->routes[j].slave);
				panic();
			}

			icbuarb_issue_request(vote->handles[j],
					      &vote->requests[j]);
		}
	}
}

void qti_coreinit_deinit(void)
{
	struct icb_bw_req zero_req = { .ab = 0ULL, .ib = 0ULL };
	uint32_t i = 0U;
	uint32_t j = 0U;

	if (coreinit_cfg == NULL)
		return;

	for (i = 0U; i < coreinit_cfg->num_votes; i++) {
		struct coreinit_vote *vote = &coreinit_cfg->votes[i];

		for (j = 0U; j < vote->num_routes; j++) {
			if (vote->handles[j] == NULL)
				continue;

			icbuarb_issue_request(vote->handles[j], &zero_req);
		}
	}
}