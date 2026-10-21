/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef IPCC_INTERNAL_H
#define IPCC_INTERNAL_H

#include <stdbool.h>
#include <stdint.h>

#include <drivers/qti/ipcc/ipcc.h>

/* Backend selection: which mechanism to use for signaling. */
enum ipcc_backend {
	IPCC_BACKEND_ROUTER = 0,
	IPCC_BACKEND_DIRECT = 1,
};

/* Backend capability flags for protocols. */
#define IPCC_CAP_DIRECT  (1U << IPCC_BACKEND_DIRECT)
#define IPCC_CAP_ROUTER  (1U << IPCC_BACKEND_ROUTER)

/* Chipset configuration flags. */
#define IPCC_CFG_NO_TME_CFG   (1U << 0)
#define IPCC_CFG_ROUTER_MODE  (1U << 1)
#define IPCC_CFG_HW_MEM_OPT   (1U << 2)

/* Only the clients a chipset instantiates are listed; absence rejects. */
struct ipcc_client_cfg {
	enum ipcc_client client;
	enum ipcc_backend backend;	/* Client's chosen backend; validated at dispatch. */
};

/* One request for either backend; router_page_base and idx are router-only. */
struct ipcc_signal_req {
	uintptr_t router_page_base;	/* resolved SEND page */
	enum ipcc_client cid;
	uint32_t idx;			/* physical client index */
	uint16_t sig_lo;
	uint16_t sig_hi;
	uint16_t num_sigs;		/* protocol signals per client */
};

/* Backend operations: extensible for future requirements (register, unregister, etc.). */
struct ipcc_backend_ops {
	int32_t (*signal)(const struct ipcc_signal_req *req);
};

struct ipcc_proto_cfg {
	uint16_t protocol_id;
	/* Signals per client; varies by target and by protocol. */
	uint16_t num_sigs;
	uint32_t num_clients;
	uintptr_t proto_block_base;	/* protocol block base */
	bool has_router;		/* true if router window is wired */
	const struct ipcc_client_cfg *clients;
	uint32_t backends;		/* Capability bitmask: IPCC_CAP_DIRECT | IPCC_CAP_ROUTER */
	uint32_t proto_stride;		/* bytes between protocol blocks */
	uint32_t client_stride;		/* bytes between client pages */
	/* Backend operations per mechanism; NULL if not supported. */
	const struct ipcc_backend_ops *backend_ops[2];	/* indexed by enum ipcc_backend */
};

struct ipcc_cfg {
	const struct ipcc_proto_cfg *protocols;
	uint32_t num_protocols;
	enum ipcc_client client;	/* client ID of this EL3 */
	uintptr_t ctrl_block_base;	/* IPC_CONFIG.TOP_MODE and IPC_TRACE base */
	bool has_ctrl_block;		/* true if EL3 programs control block */
	uint32_t flags;			/* Bitmask: IPCC_CFG_* flags */
};

/*
 * Mutable driver state, kept apart from the const config so that table can
 * live in read-only memory. Holds addresses only, never code pointers.
 */
struct ipcc_drv_ctxt {
	/* Chipset description this build was handed, NULL until init. */
	const struct ipcc_cfg *cfg;
	/* Router page base resolved per protocol at init; 0 = unresolved. */
	uintptr_t router_pages[IPCC_PROTO_TOTAL];
	uint32_t hw_version;
};

/* Provided by <chipset>/ipcc_config.c. */
extern const struct ipcc_cfg *const ipcc_chipset_cfg;

const struct ipcc_drv_ctxt *ipcc_get_drv_ctxt(void);

int32_t ipcc_router_signal(const struct ipcc_signal_req *req);

/* Backend operations exported for protocol configuration. */
extern const struct ipcc_backend_ops ipcc_direct_ops;
extern const struct ipcc_backend_ops ipcc_router_ops;

/* A signal with no direct encoding has out_mask == 0 and is rejected. */
struct ipcc_direct_signal {
	uint32_t out_mask;
};

struct ipcc_direct_client {
	uint32_t num_signals;
	const struct ipcc_direct_signal *signals;
	uintptr_t trigger_reg;		/* outgoing trigger register */
	bool is_supported;		/* true if this client is supported */
};

struct ipcc_direct_cfg {
	uint32_t num_clients;
	const struct ipcc_direct_client *clients;
};

/* Provided by <chipset>/ipcc_config.c, NULL when no direct path exists. */
extern const struct ipcc_direct_cfg *const ipcc_chipset_direct_cfg;

int32_t ipcc_direct_signal(const struct ipcc_signal_req *req);

static inline bool ipcc_backend_supported(uint32_t backends, enum ipcc_backend backend)
{
	return (backends & (1U << backend)) != 0U;
}

#endif /* IPCC_INTERNAL_H */
