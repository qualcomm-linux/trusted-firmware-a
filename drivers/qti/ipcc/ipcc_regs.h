/*
 * Copyright (c) 2026 Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef IPCC_REGS_H
#define IPCC_REGS_H

#include <lib/utils_def.h>
#include <ipcc_hwio.h>

/* Offsets within a client page and their bit fields, common to every part. */
#define IPCC_VERSION_OFF			U(0x0)
#define IPCC_VERSION_MASK			U(0xffffff)
#define IPCC_VERSION_MAJOR_SHIFT		16
#define IPCC_VERSION_MINOR_SHIFT		8
#define IPCC_VERSION(maj, min)			\
	((U(maj) << IPCC_VERSION_MAJOR_SHIFT) |	\
	 (U(min) << IPCC_VERSION_MINOR_SHIFT))

/* SEND is write-only: SIGNAL_ID 15:0, CLIENT_ID 30:16, BROADCAST bit 31. */
#define IPCC_SEND_OFF				U(0xC)
#define IPCC_SEND_SIGNAL_ID_SHIFT		0
#define IPCC_SEND_SIGNAL_ID_MASK		U(0xffff)
#define IPCC_SEND_CLIENT_ID_SHIFT		16
#define IPCC_SEND_CLIENT_ID_MASK		U(0x7fff)

/* TOP_MODE.MODE selects 0 = direct, 1 = router; offsets are target data. */
#define IPCC_TOP_MODE_BIT			BIT_32(0)
#define IPCC_TRACE_ENABLE_BIT			BIT_32(0)

#endif /* IPCC_REGS_H */
