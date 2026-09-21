/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef QTI_SECURE_IO_H
#define QTI_SECURE_IO_H

#include <stdbool.h>
#include <stdint.h>

#define QTI_DLOAD_COOKIE_MASK	0x00000030U
#define QTI_SECURE_IO_PERMISSION_READ	0x1U
#define QTI_SECURE_IO_PERMISSION_WRITE	0x2U

typedef struct qti_secure_io_allowed_reg {
	uintptr_t addr;
	uint32_t mask;
	uint32_t permissions;
} qti_secure_io_allowed_reg_t;

bool qti_secure_io_read(uintptr_t addr, uint32_t *value);
bool qti_secure_io_write(uintptr_t addr, uint32_t value);

#endif /* QTI_SECURE_IO_H */
