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
#include <drivers/qti/ipcc/ipcc.h>
#include <lib/mmio.h>

#include "ipcc_internal.h"
#include "ipcc_regs.h"

static struct ipcc_drv_ctxt ipcc_drv_ctxt;

const struct ipcc_drv_ctxt *ipcc_get_drv_ctxt(void)
{
	return &ipcc_drv_ctxt;
}

static const struct ipcc_client_cfg *
ipcc_get_client(const struct ipcc_proto_cfg *proto, enum ipcc_client cid)
{
	uint32_t i;

	for (i = 0U; i < proto->num_clients; i++) {
		if (proto->clients[i].client == cid) {
			return &proto->clients[i];
		}
	}

	return NULL;
}

/* Packed pages: index is the clients[] row, else the client ID. */
static uint32_t ipcc_phys_idx(const struct ipcc_proto_cfg *proto,
			      const struct ipcc_client_cfg *entry,
			      enum ipcc_client cid)
{
	return (ipcc_drv_ctxt.cfg->flags & IPCC_CFG_HW_MEM_OPT) ?
	       (uint32_t)(entry - proto->clients) : (uint32_t)cid;
}

static int32_t ipcc_resolve_phys_idx(const struct ipcc_proto_cfg *proto,
				     enum ipcc_client cid, uint32_t *idx)
{
	const struct ipcc_client_cfg *entry = ipcc_get_client(proto, cid);

	if (entry != NULL) {
		*idx = ipcc_phys_idx(proto, entry, cid);
		return 0;
	}

	VERBOSE("IPCC: invalid client %u for protocol %u\n", (uint32_t)cid,
		proto->protocol_id);

	return -ENOTSUP;
}

static bool ipcc_has_router_window(const struct ipcc_proto_cfg *proto)
{
	return proto->has_router;
}

static int32_t ipcc_protocol_init(const struct ipcc_proto_cfg *proto)
{
	uint32_t idx = 0U;
	uintptr_t base;
	int32_t ret;

	if (!ipcc_has_router_window(proto)) {
		return 0;
	}

	ret = ipcc_resolve_phys_idx(proto, ipcc_drv_ctxt.cfg->client, &idx);
	if (ret != 0) {
		ERROR("IPCC: protocol %u: cannot resolve own client %u (%d)\n",
		      proto->protocol_id,
		      (uint32_t)ipcc_drv_ctxt.cfg->client, ret);
		return ret;
	}

	base = proto->proto_block_base +
	       ((uintptr_t)idx * proto->client_stride) +
	       ((uintptr_t)proto->protocol_id * proto->proto_stride);

	ipcc_drv_ctxt.router_pages[proto->protocol_id] = base;

	ipcc_drv_ctxt.hw_version =
		mmio_read_32(base + IPCC_VERSION_OFF) & IPCC_VERSION_MASK;

	VERBOSE("IPCC: protocol %u initialized, hw_version 0x%x\n",
		proto->protocol_id, ipcc_drv_ctxt.hw_version);

	return 0;
}

static void ipcc_apply_block_cfg(const struct ipcc_cfg *cfg)
{
	if (!cfg->has_ctrl_block) {
		ERROR("IPCC: no ctrl_block, block config not applied\n");
		return;
	}

	mmio_write_32(cfg->ctrl_block_base + IPCC_TOP_MODE_BLOCK_OFF,
		      (cfg->flags & IPCC_CFG_ROUTER_MODE) ? IPCC_TOP_MODE_BIT : 0U);

	mmio_write_32(cfg->ctrl_block_base + IPCC_TRACE_BLOCK_OFF,
		      IPCC_TRACE_ENABLE_BIT);
}

static void ipcc_init_protocols(const struct ipcc_cfg *cfg)
{
	uint32_t unavailable = 0U;
	uint32_t i;

	for (i = 0U; i < cfg->num_protocols; i++) {
		if (ipcc_protocol_init(&cfg->protocols[i]) != 0) {
			unavailable++;
		}
	}

	if (unavailable != 0U) {
		ERROR("IPCC: %u/%u protocols unavailable\n",
		      unavailable, cfg->num_protocols);
		return;
	}

	VERBOSE("IPCC: initialized, %u protocols\n", cfg->num_protocols);
}

void qti_ipcc_init(void)
{
	const struct ipcc_cfg *cfg = ipcc_chipset_cfg;

	if (cfg == NULL) {
		ERROR("IPCC: no chipset config, driver unavailable\n");
		return;
	}

	/* base[] holds one slot per protocol; a longer table cannot fit. */
	if (cfg->num_protocols > IPCC_PROTO_TOTAL) {
		ERROR("IPCC: %u protocols described, %u supported\n",
		      cfg->num_protocols, (uint32_t)IPCC_PROTO_TOTAL);
		return;
	}
	ipcc_drv_ctxt.cfg = cfg;

	if (cfg->flags & IPCC_CFG_NO_TME_CFG) {
		ipcc_apply_block_cfg(cfg);
	}

	ipcc_init_protocols(cfg);
}

static bool ipcc_proto_valid(enum ipcc_protocol protocol)
{
	return (protocol < IPCC_PROTO_TOTAL) &&
	       ((uint32_t)protocol < ipcc_drv_ctxt.cfg->num_protocols);
}

static int32_t ipcc_dispatch(enum ipcc_protocol protocol,
			     const struct ipcc_proto_cfg *proto,
			     const struct ipcc_client_cfg *target,
			     enum ipcc_client cid, uint16_t sig_lo,
			     uint16_t sig_hi)
{
	struct ipcc_signal_req req = {
		.router_page_base = ipcc_drv_ctxt.router_pages[protocol],
		.idx = ipcc_phys_idx(proto, target, cid),
		.num_sigs = proto->num_sigs,
		.sig_lo = sig_lo,
		.sig_hi = sig_hi,
		.cid = cid,
	};

	if (ipcc_backend_supported(proto->backends, target->backend)) {
		const struct ipcc_backend_ops *ops = proto->backend_ops[target->backend];
		if (ops != NULL && ops->signal != NULL) {
			return ops->signal(&req);
		}
	}

	VERBOSE("IPCC: backend %u unsupported for target %u\n",
		(uint32_t)target->backend, (uint32_t)cid);

	return -ENOTSUP;
}

int32_t qti_ipcc_signal(enum ipcc_protocol protocol, enum ipcc_client cid,
			uint16_t sig_lo, uint16_t sig_hi)
{
	const struct ipcc_proto_cfg *proto;
	const struct ipcc_client_cfg *target;

	if (ipcc_drv_ctxt.cfg == NULL) {
		return -ENODEV;
	}

	if (!ipcc_proto_valid(protocol)) {
		return -EINVAL;
	}

	proto = &ipcc_drv_ctxt.cfg->protocols[protocol];

	target = ipcc_get_client(proto, cid);
	if (target != NULL) {
		return ipcc_dispatch(protocol, proto, target, cid, sig_lo,
				     sig_hi);
	}

	VERBOSE("IPCC: client %u not wired on protocol %u\n",
		(uint32_t)cid, proto->protocol_id);

	return -ENOTSUP;
}
