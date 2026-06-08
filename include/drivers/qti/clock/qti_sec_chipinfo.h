/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Minimal chip info interface used by the QTI clock driver.
 */

#ifndef CLOCK_QTI_SEC_CHIPINFO_H
#define CLOCK_QTI_SEC_CHIPINFO_H

#include <stdint.h>

/* Encode a major.minor chip revision the way the clock back-end compares it. */
#define CHIPINFO_VERSION(major, minor)	(((major) << 16) | (minor))

#define SOC_ID_TALOS		0x60070101U

#define DEVICE_ID_SA6155P	0xEEU
#define DEVICE_ID_SA4155P	0xF2U
#define DEVICE_ID_SA6155	0xF5U
#define DEVICE_ID_QCS615	0x2E9U
#define DEVICE_ID_QCS9075	0x2EBU

uint32_t qti_sec_get_soc_id(void);
uint32_t qti_sec_get_device_id(void);
uint32_t qti_sec_get_chip_version(void);

#endif /* CLOCK_QTI_SEC_CHIPINFO_H */
