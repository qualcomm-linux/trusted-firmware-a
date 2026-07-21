/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef BL31QTILIB_SPD_AGNOSTIC_H
#define BL31QTILIB_SPD_AGNOSTIC_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <arch.h>
#include <cdefs.h>

#define BL31QTILIB_SPD_NOT_SUPPORTED		(-1)
#define BL31QTILIB_SPD_BOOT_QSEE_INTERFACE	(1U)

uintptr_t bl31qtilib_spd_smc_handler(uint32_t smc_fid, u_register_t x1,
				      u_register_t x2, u_register_t x3,
				      u_register_t x4, void *cookie,
				      void *handle, u_register_t flags);

bool bl31qtilib_spd_is_available(void);

void *bl31qtilib_spd_share_object(uint32_t object_id, void *ptr, size_t size);

int bl31qtilib_spd_register_isr(uint32_t intnum, const char *int_desc,
				void *(*fn)(void *), void *ctx,
				uint32_t flags, bool enable);

int bl31qtilib_spd_disable_isr(uint32_t intnum);

void bl31qtilib_spd_store_intr_context(uint32_t intid, uint32_t flags,
				       void *handle, void *cookie);

__dead2 void bl31qtilib_spd_plat_error_handler(int error);
__dead2 void bl31qtilib_spd_error_handler(int error);

void bl31qtilib_spd_set_error_fatal(int error);
void bl31qtilib_spd_set_error_fatal_with_cond(int error, bool return_to_hlos);

bool bl31qtilib_spd_owns_interrupts(void);

#endif /* BL31QTILIB_SPD_AGNOSTIC_H */
