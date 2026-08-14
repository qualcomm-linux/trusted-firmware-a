/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef IPCC_HWIO_H
#define IPCC_HWIO_H

#include <lib/utils_def.h>

/*
 * Target HWIO for the IPCC controller on Lemans. Everything a respin or a
 * different SoC can change lives here: block address, block geometry and the
 * per-protocol signal count. The register offsets and bit fields, which the IP
 * keeps common across parts, are in drivers/qti/ipcc/ipcc_regs.h.
 *
 * The block holds the core pages at 0x400000 (size 0xfc000), the config block at
 * 0x4fc000 and the trace block at 0x4fd000. The whole window falls inside the
 * platform's flat device mapping (QTI_DEVICE_BASE..0x1C000000), so no extra mmap
 * entry is needed.
 */
#define IPCC_BASE			UL(0x00400000)
#define IPCC_SIZE			UL(0x00100000)

/*
 * Core geometry: 4 protocol blocks of 0x40000, each addressing its clients
 * 0x1000 apart, so a register lives at
 *
 *   IPCC_BASE + off + IPCC_PROTO_STRIDE * protocol_id +
 *		       IPCC_CLIENT_STRIDE * phys_idx
 *
 * Both strides are per-target: the protocol stride scales with the client count
 * and only the client stride happens to agree across the parts supported so far.
 */
#define IPCC_PROTO_STRIDE		U(0x40000)
#define IPCC_CLIENT_STRIDE		U(0x1000)

/*
 * Signals per client, per protocol. Not a property of the IP -- it varies by
 * target and by protocol -- so each protocol the config describes needs its own
 * count here.
 */
#define IPCC_MPROC_NUM_SIGS		U(8)

/*
 * TOP_MODE and TRACE block offsets from IPCC_BASE. They sit in the tail of the
 * last protocol block rather than past it, so they cannot be derived from the
 * stride.
 */
#define IPCC_TOP_MODE_BLOCK_OFF		U(0xfc000)
#define IPCC_TRACE_BLOCK_OFF		U(0xfd000)

#endif /* IPCC_HWIO_H */
