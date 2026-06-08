/*
 * Copyright (c) 2026 Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Interconnect bus arbiter (ICB) seam for the lemans (qcs9075) clock driver.
 *
 * The clock BSP issues a handful of fixed bandwidth "votes" while bringing up
 * clock groups. A full ICB/BCM bandwidth-arbitration driver is not yet
 * available in TF-A, so this provides a minimal static client allocator and a
 * no-op request path that lets clock-group enable proceed.
 *
 * TODO: replace with a real ICB/BCM driver once it lands.
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <drivers/qti/clock/icbuarb.h>

/* Per-client record. No heap: clients are handed out from a static pool. */
struct icb_client {
	bool		in_use;
	icb_master_id	master;
	icb_slave_id	slave;
};

/*
 * Sized to the number of ICB bandwidth requests in the lemans clock BSP
 * (currently 6). Over-provision slightly so adding a vote does not require
 * touching this file.
 */
#define ICB_MAX_CLIENTS		8U

static struct icb_client icb_clients[ICB_MAX_CLIENTS];

icb_client_handle icbuarb_create_client(icb_master_id master, icb_slave_id slave)
{
	for (uint32_t i = 0U; i < ICB_MAX_CLIENTS; i++) {
		if (!icb_clients[i].in_use) {
			icb_clients[i].in_use = true;
			icb_clients[i].master = master;
			icb_clients[i].slave = slave;
			return &icb_clients[i];
		}
	}

	return NULL;
}

bool icbuarb_issue_request(icb_client_handle handle, struct icb_bw_req *req)
{
	(void)req;

	/*
	 * TODO: program the actual bandwidth request through the ICB/BCM path.
	 * For now the vote is accepted so clock-group bring-up can continue.
	 */
	return handle != NULL;
}
