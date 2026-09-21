/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <lib/mmio.h>
#include <lib/utils_def.h>

#include <qti_secure_io.h>
#include <qti_secure_io_cfg.h>

static const qti_secure_io_allowed_reg_t *
qti_get_secure_io_allowed_reg(uintptr_t addr)
{
	if ((addr & (sizeof(uint32_t) - 1U)) != 0U) {
		return NULL;
	}

	for (size_t i = 0U; i < ARRAY_SIZE(qti_secure_io_allowed_regs); i++) {
		if (addr == qti_secure_io_allowed_regs[i].addr) {
			return &qti_secure_io_allowed_regs[i];
		}
	}

	return NULL;
}

static bool qti_secure_io_permission_allowed(
		const qti_secure_io_allowed_reg_t *reg, uint32_t permission)
{
	return (reg->permissions & permission) != 0U;
}

bool qti_secure_io_read(uintptr_t addr, uint32_t *value)
{
	const qti_secure_io_allowed_reg_t *reg;

	reg = qti_get_secure_io_allowed_reg(addr);
	if ((reg == NULL) || (value == NULL) ||
	    !qti_secure_io_permission_allowed(reg,
					QTI_SECURE_IO_PERMISSION_READ)) {
		return false;
	}

	*value = mmio_read_32(reg->addr) & reg->mask;
	return true;
}

bool qti_secure_io_write(uintptr_t addr, uint32_t value)
{
	const qti_secure_io_allowed_reg_t *reg;
	uint32_t old_val;
	uint32_t new_val;

	reg = qti_get_secure_io_allowed_reg(addr);
	if ((reg == NULL) ||
	    !qti_secure_io_permission_allowed(reg,
					QTI_SECURE_IO_PERMISSION_WRITE)) {
		return false;
	}

	if (reg->mask == UINT32_MAX) {
		mmio_write_32(reg->addr, value);
		return true;
	}

	old_val = mmio_read_32(reg->addr);
	new_val = (old_val & ~reg->mask) | (value & reg->mask);
	mmio_write_32(reg->addr, new_val);
	return true;
}
