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

#include "ipcc_priv.h"

/*
 * Lemans does not pack its client pages: they sit at their client ID positions
 * across the 0..36 span, only the clients wired on MPROC are real, and every
 * other position in that span is a hole. So a client's physical index is its
 * client ID, the order of the list below does not matter, and what the list
 * contributes is exactly which positions are not holes -- a client with no entry
 * here is rejected by the client lookup in ipcc_core.c. Hence bsp_data.hw_mem_opt
 * stays false.
 *
 * Note the list excludes APSS_NS4 (36), which is inside the instantiated span but
 * not permitted on this protocol.
 *
 * Every client stays on the router.
 */
static struct ipcc_client_bsp mproc_clients[] = {
	{ .client = IPCC_CLIENT_AOP     },
	{ .client = IPCC_CLIENT_APSS_S  },
	{ .client = IPCC_CLIENT_LPASS   },
	{ .client = IPCC_CLIENT_NSP0    },
	{ .client = IPCC_CLIENT_APSS_NS0},
	{ .client = IPCC_CLIENT_GPU     },
	{ .client = IPCC_CLIENT_CVP     },
	{ .client = IPCC_CLIENT_CAM     },
	{ .client = IPCC_CLIENT_VPU     },
	{ .client = IPCC_CLIENT_NSP1    },
	{ .client = IPCC_CLIENT_SAIL0   },
	{ .client = IPCC_CLIENT_SAIL1   },
	{ .client = IPCC_CLIENT_SAIL2   },
	{ .client = IPCC_CLIENT_SAIL3   },
	{ .client = IPCC_CLIENT_GPDSP0  },
	{ .client = IPCC_CLIENT_GPDSP1  },
	{.client = IPCC_CLIENT_APSS_NS1 },
	{.client = IPCC_CLIENT_APSS_NS2 },
	{.client = IPCC_CLIENT_APSS_NS3 },
};

/*
 * Only MPROC is described. The hardware also instantiates COMPUTE_L0,
 * COMPUTE_L1 and PERIPH, plus a separate SAIL-domain controller, but nothing in
 * EL3 uses them.
 */
static struct ipcc_protocol_cfg protocols[] = {
	[IPCC_PROTO_MPROC] = {
		.protocol_id = IPCC_PROTO_MPROC,
		.num_sigs = IPCC_MPROC_NUM_SIGS,
		.num_clients = ARRAY_SIZE(mproc_clients),
		.phys_base = IPCC_BASE,
		.clients = mproc_clients,
#if QTI_IPCC_LEGACY
		/* Router only: legacy_supported keeps its false default. */
		.ipcc_mode_supported = true,
#endif
	},
};

static const struct ipcc_bsp_data bsp_data = {
	.protocols = protocols,
	.num_protocols = ARRAY_SIZE(protocols),
	.client = IPCC_CLIENT_TZ,
#if QTI_IPCC_NO_TME
	/*
	 * Value for IPC_CONFIG.TOP_MODE.MODE: true = router. Lemans has no TME, so
	 * the platform sets QTI_IPCC_NO_TME=1 and EL3 programs TOP_MODE itself at
	 * init rather than relying on its reset value.
	 */
	.ipcc_mode = true,
#endif
};

const struct ipcc_bsp_data *const ipcc_chipset_config = &bsp_data;

#if QTI_IPCC_LEGACY
/* Lemans does not use the legacy path for any client. */
const struct ipcc_legacy_bsp *const ipcc_chipset_legacy_config = NULL;
#endif
