/*
 * Copyright (c) 2026 Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef QTI_SECURE_IO_CFG_H
#define QTI_SECURE_IO_CFG_H

#include <stdint.h>

/* Registers non-secure software may access through the SiP service. */
#define QTI_DLOAD_COOKIE_REG	0x01FD9000U

static const uintptr_t qti_secure_io_allowed_regs[] = {
	QTI_DLOAD_COOKIE_REG,
};

#endif /* QTI_SECURE_IO_CFG_H */
