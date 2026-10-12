/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef RPMH_TARGET_H
#define RPMH_TARGET_H

#include <rpmh_common.h>

#define RPMH_MSG_RAM_SECTION_SIZE	0x1000U

/* APSS RSC register layout for Nord. */
#define RSC_REG_BASE			0x18900000U
#define RSC_TCS_BASE_OFF		0xd20U
#define RSC_CMD_STRIDE			0x18U
#define RSC_CMD_BASE_OFF		0xd34U

#endif /* RPMH_TARGET_H */
