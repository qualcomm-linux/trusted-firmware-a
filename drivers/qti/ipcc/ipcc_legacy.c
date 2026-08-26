/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <errno.h>
#include <stddef.h>
#include <stdint.h>

#include <common/debug.h>
#include <lib/mmio.h>

#include "ipcc_priv.h"

static const struct ipcc_legacy_client *ipcc_legacy_get_client(
						enum ipcc_client cid)
{
	if ((ipcc_chipset_legacy_config == NULL) ||
	    ((uint32_t)cid >= ipcc_chipset_legacy_config->num_clients)) {
		return NULL;
	}

	return &ipcc_chipset_legacy_config->clients[cid];
}

/*
 * Legacy outbound trigger: one write per signal of the signal's fixed bit mask
 * to the target client's dedicated trigger register. Unlike router mode there
 * is no target encoding in the data, the register itself selects the client.
 */
int ipcc_legacy_trigger(enum ipcc_client target_id, uint16_t signal_low,
			uint16_t signal_high)
{
	const struct ipcc_legacy_client *client =
		ipcc_legacy_get_client(target_id);
	uint16_t sig;

	if ((client == NULL) || (client->reg_base == 0UL) ||
	    (signal_high >= client->num_signals)) {
		return -EINVAL;
	}

	/* Reject the whole range if any signal in it has no legacy encoding. */
	for (sig = signal_low; sig <= signal_high; sig++) {
		if (client->signals[sig].out_mask == 0U) {
			VERBOSE("IPCC: legacy signal %u unsupported for target %u\n",
				sig, (uint32_t)target_id);
			return -ENOTSUP;
		}
	}

	for (sig = signal_low; sig <= signal_high; sig++) {
		mmio_write_32(client->reg_base, client->signals[sig].out_mask);
	}

	VERBOSE("IPCC: legacy triggered target %u signals [%u, %u]\n",
		(uint32_t)target_id, signal_low, signal_high);

	return 0;
}
