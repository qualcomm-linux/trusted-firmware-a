/*
 * Copyright (c) 2026 Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef IPCC_HWIO_H
#define IPCC_HWIO_H

#include <lib/utils_def.h>

/* No router window: this target reaches AOP over the trigger register below. */
#define IPCC_BASE			UL(0x00000000)

/* Protocol blocks of 0x40000, each holding 64 client pages 0x1000 apart. */
#define IPCC_PROTO_STRIDE		U(0x40000)
#define IPCC_CLIENT_STRIDE		U(0x1000)

/* Signals per client on MPROC. Varies by target and by protocol. */
#define IPCC_MPROC_NUM_SIGS		U(4)

/* A TME owns the block config here, so neither offset is described. */
#define IPCC_TOP_MODE_BLOCK_OFF		U(0x0)
#define IPCC_TRACE_BLOCK_OFF		U(0x0)

/* APSS_SHARED_APSS_INTU, the window the platform maps on this target. */
#define IPCC_DIRECT_BASE		UL(0x0f400000)

/* APSS_SHARED_TZ_IPC_INTERRUPT, write-only; RPM is the older name for AOP. */
#define IPCC_DIRECT_AOP_TRIG_REG	(IPCC_DIRECT_BASE + UL(0x8))

/* Field RPM_TZ_IPC [2:0], so three signals is the hardware limit. */
#define IPCC_DIRECT_AOP_SIG0_MASK	BIT_32(0)
#define IPCC_DIRECT_AOP_SIG1_MASK	BIT_32(1)
#define IPCC_DIRECT_AOP_SIG2_MASK	BIT_32(2)

#endif /* IPCC_HWIO_H */
