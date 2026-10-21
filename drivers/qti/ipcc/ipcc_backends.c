/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <errno.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <common/debug.h>
#include <lib/mmio.h>

#include "ipcc_internal.h"
#include "ipcc_regs.h"

/* Both ends are bounded explicitly: neither is implied by the other. */
static bool ipcc_sig_range_valid(uint16_t sig_lo, uint16_t sig_hi,
				 uint32_t limit)
{
	return (sig_lo <= sig_hi) && ((uint32_t)sig_lo < limit) &&
	       ((uint32_t)sig_hi < limit);
}

static uint32_t ipcc_router_client_id(const struct ipcc_drv_ctxt *ctxt,
				      uint32_t idx, enum ipcc_client cid)
{
	uint32_t id = (ctxt->hw_version >= IPCC_VERSION(3, 0)) ?
		      idx : (uint32_t)cid;

	return id & IPCC_SEND_CLIENT_ID_MASK;
}

static bool ipcc_direct_range_encoded(const struct ipcc_direct_client *dc,
				      enum ipcc_client cid, uint16_t sig_lo,
				      uint16_t sig_hi)
{
	uint16_t sig;

	for (sig = sig_lo; sig <= sig_hi; sig++) {
		if (dc->signals[sig].out_mask != 0U) {
			continue;
		}

		VERBOSE("IPCC: direct sig %u unsupported, target %u\n",
			sig, (uint32_t)cid);
		return false;
	}

	return true;
}

int32_t ipcc_direct_signal(const struct ipcc_signal_req *req)
{
	const struct ipcc_direct_client *dc;
	enum ipcc_client cid = req->cid;
	uint16_t sig;

	if (ipcc_chipset_direct_cfg == NULL) {
		return -ENODEV;
	}

	if ((uint32_t)cid >= ipcc_chipset_direct_cfg->num_clients) {
		return -ENOTSUP;
	}

	dc = &ipcc_chipset_direct_cfg->clients[cid];
	if (!dc->is_supported) {
		return -ENOTSUP;
	}

	if (!ipcc_sig_range_valid(req->sig_lo, req->sig_hi, dc->num_signals)) {
		return -EINVAL;
	}

	if (!ipcc_direct_range_encoded(dc, cid, req->sig_lo, req->sig_hi)) {
		return -ENOTSUP;
	}

	for (sig = req->sig_lo; sig <= req->sig_hi; sig++) {
		mmio_write_32(dc->trigger_reg, dc->signals[sig].out_mask);
	}

	VERBOSE("IPCC: direct triggered target %u signals [%u, %u]\n",
		(uint32_t)cid, req->sig_lo, req->sig_hi);

	return 0;
}

int32_t ipcc_router_signal(const struct ipcc_signal_req *req)
{
	const struct ipcc_drv_ctxt *ctxt = ipcc_get_drv_ctxt();
	uint32_t client_id;
	uint16_t sig;

	if (req->router_page_base == 0UL) {
		return -ENODEV;
	}

	if (!ipcc_sig_range_valid(req->sig_lo, req->sig_hi, req->num_sigs)) {
		return -EINVAL;
	}

	client_id = ipcc_router_client_id(ctxt, req->idx, req->cid);

	for (sig = req->sig_lo; sig <= req->sig_hi; sig++) {
		uint32_t signal_id = (uint32_t)sig & IPCC_SEND_SIGNAL_ID_MASK;

		mmio_write_32(req->router_page_base + IPCC_SEND_OFF,
			      (client_id << IPCC_SEND_CLIENT_ID_SHIFT) |
			      (signal_id << IPCC_SEND_SIGNAL_ID_SHIFT));
	}

	VERBOSE("IPCC: router triggered target %u signals [%u, %u]\n",
		(uint32_t)req->cid, req->sig_lo, req->sig_hi);

	return 0;
}

/* Backend operations structures for protocol configuration. */
const struct ipcc_backend_ops ipcc_direct_ops = {
	.signal = ipcc_direct_signal,
};

const struct ipcc_backend_ops ipcc_router_ops = {
	.signal = ipcc_router_signal,
};
