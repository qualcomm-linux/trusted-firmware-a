/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef IPCC_HWIO_H
#define IPCC_HWIO_H

#include <lib/utils_def.h>

/*
 * Target HWIO for the IPCC controller on Honu. Everything a respin or a
 * different SoC can change lives here: block address, block geometry and the
 * per-protocol signal count. The register offsets and bit fields, which the IP
 * keeps common across parts, are in drivers/qti/ipcc/ipcc_regs.h.
 *
 * The window must be covered by the platform's device mapping before
 * qti_ipcc_init() runs.
 */
#define IPCC_BASE			UL(0x06000000)
#define IPCC_SIZE			UL(0x05000000)

/*
 * Core geometry: 5 protocol blocks of 0x1000000 (25 clients 0x10000 apart), so a
 * register lives at
 *
 *   IPCC_BASE + off + IPCC_PROTO_STRIDE * protocol_id +
 *		       IPCC_CLIENT_STRIDE * phys_idx
 *
 * Both strides are wider here than on earlier parts, which is why they are
 * per-target and not shared.
 */
#define IPCC_PROTO_STRIDE		U(0x1000000)
#define IPCC_CLIENT_STRIDE		U(0x10000)

/*
 * Signals per client, per protocol. Not a property of the IP -- it varies by
 * target and by protocol -- so each protocol the config describes needs its own
 * count here.
 */
#define IPCC_MPROC_NUM_SIGS		U(16)
#define IPCC_PERIPH_NUM_SIGS		U(32)

/*
 * TOP_MODE and TRACE offsets from a protocol block base. Unlike earlier parts
 * they share one block, TRACE_ENABLE sitting two registers past TOP_MODE.
 */
#define IPCC_TOP_MODE_BLOCK_OFF		U(0xff0000)
#define IPCC_TRACE_BLOCK_OFF		U(0xff0008)

#endif /* IPCC_HWIO_H */
