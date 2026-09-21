/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef QTI_SECURE_IO_CFG_H
#define QTI_SECURE_IO_CFG_H

#include <qti_secure_io.h>

/*
 * List of peripheral register addresses that non-secure software may access
 * through the secure I/O SiP service.
 */
static const qti_secure_io_allowed_reg_t qti_secure_io_allowed_regs[] = {
	{ 0x01fd9000, QTI_DLOAD_COOKIE_MASK,
	  QTI_SECURE_IO_PERMISSION_READ | QTI_SECURE_IO_PERMISSION_WRITE },
};

#endif /* QTI_SECURE_IO_CFG_H */
