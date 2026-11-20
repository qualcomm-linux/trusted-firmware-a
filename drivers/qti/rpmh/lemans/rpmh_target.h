/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef RPMH_TARGET_H
#define RPMH_TARGET_H

#include <platform_def.h>
#include <rpmh_common.h>

#define RPMH_MSG_RAM_SECTION_SIZE	0x10000U

/* APSS RSC register layout for LeMans. */
#define RSC_REG_BASE			(QTI_APSS_HM_BASE + 0x00a00000U)
#define RSC_TCS_BASE_OFF		0xd10U
#define RSC_CMD_STRIDE			0x14U
#define RSC_CMD_BASE_OFF		0xd30U

#endif /* RPMH_TARGET_H */
