/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Generic read-modify-write helpers over the CPRH register map. Every
 * HWIO_<reg>_ADDR() macro supplied by the chipset register header takes an
 * explicit base address (the controller's base_addr) as its first
 * argument, rather than relying on an implicit variable in scope.
 */

#ifndef CPRH_HAL_INTERNAL_H
#define CPRH_HAL_INTERNAL_H

#include <lib/mmio.h>

#define CPR_HWIO_IN(base, reg)						\
	mmio_read_32(HWIO_##reg##_ADDR(base))

#define CPR_HWIO_OUT(base, reg, val)					\
	mmio_write_32(HWIO_##reg##_ADDR(base), (val))

#define CPR_HWIO_OUTI(base, reg, idx, val)				\
	mmio_write_32(HWIO_##reg##_ADDR(base, idx), (val))

#define CPR_HWIO_INM(base, reg, field)					\
	((mmio_read_32(HWIO_##reg##_ADDR(base)) &			\
	  HWIO_##reg##_##field##_BMSK) >> HWIO_##reg##_##field##_SHFT)

#define CPR_HWIO_OUTM(base, reg, field, val)				\
	mmio_write_32(HWIO_##reg##_ADDR(base),				\
		(mmio_read_32(HWIO_##reg##_ADDR(base)) &		\
		 ~HWIO_##reg##_##field##_BMSK) |			\
		(((val) << HWIO_##reg##_##field##_SHFT) &		\
		 HWIO_##reg##_##field##_BMSK))

#define CPR_HWIO_OUTMI(base, reg, idx, field, val)			\
	mmio_write_32(HWIO_##reg##_ADDR(base, idx),			\
		(mmio_read_32(HWIO_##reg##_ADDR(base, idx)) &		\
		 ~HWIO_##reg##_##field##_BMSK) |			\
		(((val) << HWIO_##reg##_##field##_SHFT) &		\
		 HWIO_##reg##_##field##_BMSK))

#define CPR_HWIO_OUTMI2(base, reg, idx1, idx2, field, val)		\
	mmio_write_32(HWIO_##reg##_ADDR(base, idx1, idx2),		\
		(mmio_read_32(HWIO_##reg##_ADDR(base, idx1, idx2)) &	\
		 ~HWIO_##reg##_##field##_BMSK) |			\
		(((val) << HWIO_##reg##_##field##_SHFT) &		\
		 HWIO_##reg##_##field##_BMSK))

#endif /* CPRH_HAL_INTERNAL_H */
