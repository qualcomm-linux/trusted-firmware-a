/*
 * Copyright (c) 2026 Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Default no-op ICB seam for the QTI clock driver. Hands out dummy client
 * handles and accepts every bandwidth vote, so clock-group bring-up
 * succeeds on platforms that do not link the real ICB/BCM driver. Platforms
 * that do should skip this file and link the real driver instead.
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <drivers/qti/clock/icbuarb.h>

/*
 * Sized to the number of ICB bandwidth requests an in-tree clock BSP is
 * expected to make (currently <= 8 for lemans). Bump if a new BSP needs
 * more.
 */
#define ICB_MAX_CLIENTS		8U

/* Dummy client pool; only the addresses are handed back as handles. */
static uint8_t icb_clients[ICB_MAX_CLIENTS];
static uint32_t icb_clients_used;

icb_client_handle icbuarb_create_client(enum icbid_master master,
					enum icbid_slave slave)
{
	(void)master;
	(void)slave;

	if (icb_clients_used >= ICB_MAX_CLIENTS) {
		return NULL;
	}

	return (icb_client_handle)&icb_clients[icb_clients_used++];
}

bool icbuarb_issue_request(icb_client_handle handle, struct icb_bw_req *req)
{
	(void)req;

	/* No bandwidth arbitration in the stub; just acknowledge the vote. */
	return handle != NULL;
}
