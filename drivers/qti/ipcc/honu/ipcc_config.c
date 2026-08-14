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
 * Honu packs its client pages: client IDs run to 84, but the controller keeps
 * only 25 pages per protocol and packs the clients this chipset wires
 * contiguously from page 0. A client's physical index is therefore its position
 * in this array, not its client ID, and both the register address and the SEND
 * register's CLIENT_ID field take that index.
 *
 * Each protocol has its own physical client map, so the two arrays below are
 * indexed independently -- a client wired on both, us included, generally lands
 * on a different page in each.
 *
 * Order is load-bearing and must match the physical client map for the protocol
 * -- reordering silently retargets every signal.
 *
 * Every client stays on the router.
 */
static struct ipcc_client_bsp mproc_clients[] = {
	{ .client = IPCC_CLIENT_AOP      },	/* physical index 0 */
	{ .client = IPCC_CLIENT_APSS_S   },	/* physical index 1, us */
	{ .client = IPCC_CLIENT_MPSS     },
	{ .client = IPCC_CLIENT_LPASS    },
	{ .client = IPCC_CLIENT_SDC      },
	{ .client = IPCC_CLIENT_NSP0     },
	{ .client = IPCC_CLIENT_APSS_NS0 },
	{ .client = IPCC_CLIENT_APSS_NS1 },
	{ .client = IPCC_CLIENT_APSS_NS2 },
	{ .client = IPCC_CLIENT_APSS_NS3 },
	{ .client = IPCC_CLIENT_PDP0     },
	{ .client = IPCC_CLIENT_PDP1     },
	{ .client = IPCC_CLIENT_DUMMY    },
	{ .client = IPCC_CLIENT_SOCCP    },
	{ .client = IPCC_CLIENT_APCP     },
	{ .client = IPCC_CLIENT_DCP      },
	{ .client = IPCC_CLIENT_LMCU     },
	{ .client = IPCC_CLIENT_QECP     },
	{ .client = IPCC_CLIENT_SPSS     },
	{ .client = IPCC_CLIENT_TMESS    },	/* physical index 19 */
};

/* Carries the QUP and PCIe clients, which MPROC does not wire. */
static struct ipcc_client_bsp periph_clients[] = {
	{ .client = IPCC_CLIENT_APSS_S   },	/* physical index 0, us */
	{ .client = IPCC_CLIENT_MPSS     },
	{ .client = IPCC_CLIENT_LPASS    },
	{ .client = IPCC_CLIENT_NSP0     },
	{ .client = IPCC_CLIENT_APSS_NS0 },
	{ .client = IPCC_CLIENT_PCIE0    },
	{ .client = IPCC_CLIENT_PCIE1    },
	{ .client = IPCC_CLIENT_APSS_NS1 },
	{ .client = IPCC_CLIENT_APSS_NS2 },
	{ .client = IPCC_CLIENT_APSS_NS3 },
	{ .client = IPCC_CLIENT_DUMMY    },
	{ .client = IPCC_CLIENT_SOCCP    },
	{ .client = IPCC_CLIENT_DCP      },
	{ .client = IPCC_CLIENT_LMCU     },
	{ .client = IPCC_CLIENT_QECP     },
	{ .client = IPCC_CLIENT_QUP_TOP0 },
	{ .client = IPCC_CLIENT_QUP_TOP1 },
	{ .client = IPCC_CLIENT_QUP_TOP2 },
	{ .client = IPCC_CLIENT_QUP_TOP3 },
	{ .client = IPCC_CLIENT_QUP_SSC0 },
	{ .client = IPCC_CLIENT_QUP_SSC1 },
	{ .client = IPCC_CLIENT_QUP_TOP4 },	/* physical index 21 */
};

/*
 * MPROC and PERIPH are described; the hardware also instantiates COMPUTE_L0 and
 * COMPUTE_L1, but nothing in EL3 sends on them. Their slots stay zeroed, which
 * costs two rows in protocols[] and is what keeps the array indexable by
 * protocol ID -- a zeroed slot has no clients, so every trigger aimed at one is
 * rejected by the client lookup in ipcc_core.c.
 *
 * protocol_id is the block index the address math multiplies by
 * IPCC_PROTO_STRIDE, so it always equals the slot a protocol sits in.
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
	[IPCC_PROTO_PERIPH] = {
		.protocol_id = IPCC_PROTO_PERIPH,
		.num_sigs = IPCC_PERIPH_NUM_SIGS,
		.num_clients = ARRAY_SIZE(periph_clients),
		.phys_base = IPCC_BASE,
		.clients = periph_clients,
#if QTI_IPCC_LEGACY
		.ipcc_mode_supported = true,
#endif
	},
};

static const struct ipcc_bsp_data bsp_data = {
	.protocols = protocols,
	.num_protocols = ARRAY_SIZE(protocols),
	.client = IPCC_CLIENT_APSS_S,
#if QTI_IPCC_NO_TME
	/*
	 * Value for IPC_CONFIG.TOP_MODE.MODE: true = router. Honu has TME to
	 * program TOP_MODE, so QTI_IPCC_NO_TME is expected to stay 0 here and
	 * this value to go unused.
	 */
	.ipcc_mode = true,
#endif
	.hw_mem_opt = true,
};

const struct ipcc_bsp_data *const ipcc_chipset_config = &bsp_data;

#if QTI_IPCC_LEGACY
/* Honu does not use the legacy path for any client. */
const struct ipcc_legacy_bsp *const ipcc_chipset_legacy_config = NULL;
#endif
