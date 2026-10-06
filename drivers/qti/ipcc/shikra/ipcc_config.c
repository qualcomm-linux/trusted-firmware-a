/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stddef.h>
#include <stdint.h>

#include <drivers/qti/ipcc/ipcc.h>
#include <lib/utils_def.h>

#include <ipcc_hwio.h>

#include "ipcc_internal.h"

/* AOP signals: RPM_TZ_IPC field [2:0]; the IRQs are the inbound side. */
static const struct ipcc_direct_signal aop_signals[] = {
	[0] = { .out_mask = IPCC_DIRECT_AOP_SIG0_MASK },	/* IRQ 11 */
	[1] = { .out_mask = IPCC_DIRECT_AOP_SIG1_MASK },	/* IRQ 12 */
	[2] = { .out_mask = IPCC_DIRECT_AOP_SIG2_MASK },	/* IRQ 13 */
};

/* Indexed by client ID; AOP is the only outbound path on this target. */
static const struct ipcc_direct_client direct_clients[] = {
	[IPCC_CLIENT_AOP] = {
		.num_signals = ARRAY_SIZE(aop_signals),
		.signals = aop_signals,
		.trigger_reg = IPCC_DIRECT_AOP_TRIG_REG,
		.is_supported = true,
	},
};

static const struct ipcc_direct_cfg direct_cfg = {
	.num_clients = ARRAY_SIZE(direct_clients),
	.clients = direct_clients,
};

const struct ipcc_direct_cfg *const ipcc_chipset_direct_cfg = &direct_cfg;

/* The router is not wired here, so only the direct client is listed. */
static const struct ipcc_client_cfg mproc_clients[] = {
	{
		.client = IPCC_CLIENT_AOP,
		.backend = IPCC_BACKEND_DIRECT,
	},
};

/* Only MPROC is described; COMPUTE_L0 and L1 are unsupported on this target. */
static const struct ipcc_proto_cfg protocols[] = {
	[IPCC_PROTO_MPROC] = {
		.protocol_id = IPCC_PROTO_MPROC,
		.num_sigs = IPCC_MPROC_NUM_SIGS,
		.num_clients = ARRAY_SIZE(mproc_clients),
		.proto_block_base = IPCC_BASE,
		.has_router = false,
		.clients = mproc_clients,
		.backends = IPCC_CAP_DIRECT,
		.proto_stride = IPCC_PROTO_STRIDE,
		.client_stride = IPCC_CLIENT_STRIDE,
		.backend_ops = {
			[IPCC_BACKEND_DIRECT] = &ipcc_direct_ops,
		},
	},
};

/* A TME owns the block registers, and client pages are not packed here. */
static const struct ipcc_cfg ipcc_cfg = {
	.protocols = protocols,
	.num_protocols = ARRAY_SIZE(protocols),
	.client = IPCC_CLIENT_TZ,
	.ctrl_block_base = IPCC_BASE,
	.has_ctrl_block = false,
};

const struct ipcc_cfg *const ipcc_chipset_cfg = &ipcc_cfg;
