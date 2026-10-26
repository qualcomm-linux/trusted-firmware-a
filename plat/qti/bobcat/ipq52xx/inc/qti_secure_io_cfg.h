/*
 * Copyright (c) 2019, The Linux Foundation. All rights reserved.
 * Changes from Qualcomm Technologies, Inc. are provided under the following license:
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef QTI_SECURE_IO_CFG_H
#define QTI_SECURE_IO_CFG_H

#include <stdint.h>

/*
 * List of peripheral/IO memory areas that are protected from
 * non-secure world but not required to be secure.
 */

#define TCSR_BOOT_MISC_DETECT	0x195C100
#define TCSR_BOOT_INFO		0x195C158
#define EUD_EUD_EN2		0x7A000

static const uintptr_t qti_secure_io_allowed_regs[] = {
	TCSR_BOOT_MISC_DETECT,
	TCSR_BOOT_INFO,
	EUD_EUD_EN2
};

#endif /* QTI_SECURE_IO_CFG_H */
