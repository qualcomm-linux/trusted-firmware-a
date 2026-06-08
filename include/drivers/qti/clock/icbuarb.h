/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Minimal interconnect bus arbiter (ICB) interface used by the QTI clock
 * driver. The functions are provided by the platform ICB back-end.
 */

#ifndef CLOCK_ICBUARB_H
#define CLOCK_ICBUARB_H

#include <stdbool.h>
#include <stdint.h>

/* Bus master/slave identifiers (opaque to the clock driver). */
typedef uint32_t icb_master_id;
typedef uint32_t icb_slave_id;

/*
 * Bus master/slave identifiers referenced by the lemans BSP bandwidth votes.
 * These are opaque tags; only the small subset the clock BSP uses is defined.
 */
enum {
	ICBID_MASTER_MDP0	= 0,
	ICBID_MASTER_IPA_CORE,
	ICBID_MASTER_CDSP_PROC,
	ICBID_MASTER_CDSP_PROC_B,
	ICBID_MASTER_PCIE_0,
	ICBID_MASTER_PKA_CORE,
};

enum {
	ICBID_SLAVE_EBI1	= 0,
	ICBID_SLAVE_IPA_CORE,
	ICBID_SLAVE_PKA_CORE,
};

/* Opaque ICB client handle. */
typedef struct icb_client *icb_client_handle;

/* Bandwidth request (arbitrated and instantaneous). */
struct icb_bw_req {
	uint64_t	ab;
	uint64_t	ib;
};

icb_client_handle icbuarb_create_client(icb_master_id master,
					icb_slave_id slave);
bool icbuarb_issue_request(icb_client_handle handle, struct icb_bw_req *req);

#endif /* CLOCK_ICBUARB_H */
