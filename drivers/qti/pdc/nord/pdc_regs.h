/*
 * Copyright (c) 2026 Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PDC_REGS_H
#define PDC_REGS_H

#include <assert.h>

#include <lib/mmio.h>

#include <platform_def.h>

#include "pdc_grp0_branch.h"

/* Base addresses */
#define PDC_BASE		(QTI_AOSS_BASE + 0x200000U)

/* Register address helpers */
#define PDC_REG(base, off)		((uintptr_t)(base) + (uint32_t)(off))
#define PDC_REG_DRV(base, off, d)	((uintptr_t)(base) + (uint32_t)(off) + \
					 0x10000U * (uint32_t)(d))

/* --------------------------------------------------------------------------
 * ENABLE_PDC  (base + 0x4500)
 * --------------------------------------------------------------------------
 */
#define PDC_ENABLE_PDC_OFF		0x4500U
#define PDC_ENABLE_PDC_BMSK		0x1U

#define PDC_ENABLE_PDC_RMW(base, val) \
	mmio_write_32(PDC_REG(base, PDC_ENABLE_PDC_OFF), \
		      (mmio_read_32(PDC_REG(base, PDC_ENABLE_PDC_OFF)) & \
		       ~PDC_ENABLE_PDC_BMSK) | \
		      ((uint32_t)(val) & PDC_ENABLE_PDC_BMSK))

/* --------------------------------------------------------------------------
 * PDC_PARAM_RESOURCE  (base + 0x1004 + 0x10000*d)
 * --------------------------------------------------------------------------
 */
#define PDC_PARAM_RESOURCE_OFF			0x1004U
#define PDC_PARAM_RESOURCE_PROFILING_UNIT_BMSK	0xf000U
#define PDC_PARAM_RESOURCE_PROFILING_UNIT_SHFT	12U

#define PDC_PARAM_RESOURCE_READ(base, d) \
	mmio_read_32(PDC_REG_DRV(base, PDC_PARAM_RESOURCE_OFF, d))

#define PDC_PARAM_PROFILING_UNIT(base) \
	((PDC_PARAM_RESOURCE_READ(base, 0) & \
	  PDC_PARAM_RESOURCE_PROFILING_UNIT_BMSK) >> \
	 PDC_PARAM_RESOURCE_PROFILING_UNIT_SHFT)

#define PDC_SEQ_BR_ADDR_WRITE(base, b, val) \
	do { \
		(void)(base); \
		(void)(b); \
		(void)(val); \
	} while (0)

#define PDC_SEQ_DELAY_WRITE(base, v, val) \
	do { \
		(void)(base); \
		(void)(v); \
		(void)(val); \
	} while (0)

/* --------------------------------------------------------------------------
 * IRQ ownership
 *
 * Nord uses packed ownership registers. Each bit in IRQ_OWNER_BANKb /
 * IRQ_OWNER_LASTBANK selects either DRV0 or the single non-zero owner stored
 * in IRQ_NONZERO_DRV_CFG.
 * --------------------------------------------------------------------------
 */
#define PDC_IRQ_OWNER_BANK_OFF(b)	(0x4600U + 0x4U * (uint32_t)(b))
#define PDC_IRQ_OWNER_LASTBANK_OFF	0x4614U
#define PDC_IRQ_NONZERO_DRV_CFG_OFF	0x4640U
#define PDC_IRQ_OWNER_BANK_BITS		32U
#define PDC_IRQ_OWNER_BANK_MAX		4U
#define PDC_IRQ_OWNER_DRV_BMSK		0x3U

#define PDC_IRQ_OWNER_BANK_READ(b) \
	mmio_read_32(PDC_REG(PDC_BASE, PDC_IRQ_OWNER_BANK_OFF(b)))

#define PDC_IRQ_OWNER_BANK_WRITE(b, val) \
	mmio_write_32(PDC_REG(PDC_BASE, PDC_IRQ_OWNER_BANK_OFF(b)), \
		      (uint32_t)(val))

#define PDC_IRQ_OWNER_LASTBANK_READ() \
	mmio_read_32(PDC_REG(PDC_BASE, PDC_IRQ_OWNER_LASTBANK_OFF))

#define PDC_IRQ_OWNER_LASTBANK_WRITE(val) \
	mmio_write_32(PDC_REG(PDC_BASE, PDC_IRQ_OWNER_LASTBANK_OFF), \
		      (uint32_t)(val))

#define PDC_IRQ_NONZERO_DRV_CFG_READ() \
	(mmio_read_32(PDC_REG(PDC_BASE, PDC_IRQ_NONZERO_DRV_CFG_OFF)) & \
	 PDC_IRQ_OWNER_DRV_BMSK)

#define PDC_IRQ_NONZERO_DRV_CFG_WRITE(val) \
	mmio_write_32(PDC_REG(PDC_BASE, PDC_IRQ_NONZERO_DRV_CFG_OFF), \
		      (uint32_t)(val) & PDC_IRQ_OWNER_DRV_BMSK)

#define PDC_IRQ_OWNER_WRITE(i, val) \
	do { \
		uint32_t _bank = (uint32_t)(i) / PDC_IRQ_OWNER_BANK_BITS; \
		uint32_t _mask = 1U << ((uint32_t)(i) % PDC_IRQ_OWNER_BANK_BITS); \
		uint32_t _owner = (uint32_t)(val) & PDC_IRQ_OWNER_DRV_BMSK; \
		uint32_t _cur_owner = PDC_IRQ_NONZERO_DRV_CFG_READ(); \
		uint32_t _cur_value; \
		if (_bank <= PDC_IRQ_OWNER_BANK_MAX) { \
			_cur_value = PDC_IRQ_OWNER_BANK_READ(_bank); \
			if (_owner == 0U) { \
				PDC_IRQ_OWNER_BANK_WRITE(_bank, _cur_value & ~_mask); \
			} else { \
				assert((_cur_owner == 0U) || (_cur_owner == _owner)); \
				PDC_IRQ_OWNER_BANK_WRITE(_bank, _cur_value | _mask); \
				if (_cur_owner == 0U) { \
					PDC_IRQ_NONZERO_DRV_CFG_WRITE(_owner); \
				} \
			} \
		} else { \
			_cur_value = PDC_IRQ_OWNER_LASTBANK_READ(); \
			if (_owner == 0U) { \
				PDC_IRQ_OWNER_LASTBANK_WRITE(_cur_value & ~_mask); \
			} else { \
				assert((_cur_owner == 0U) || (_cur_owner == _owner)); \
				PDC_IRQ_OWNER_LASTBANK_WRITE(_cur_value | _mask); \
				if (_cur_owner == 0U) { \
					PDC_IRQ_NONZERO_DRV_CFG_WRITE(_owner); \
				} \
			} \
		} \
	} while (0)

/* --------------------------------------------------------------------------
 * HIDDEN_TCS_CMD2_DATA_DRV0  (rsc_base + 0x48)
 * --------------------------------------------------------------------------
 */
#define PDC_HIDDEN_TCS_CMD2_DATA_DRV0_OFF	0x48U
#define PDC_SEQ_START_ADDR_VALID_BMSK		0x80000000U

#define PDC_SEQ_BRANCH_MASK_WRITE(rsc_base, val) \
	mmio_write_32(PDC_REG(rsc_base, PDC_HIDDEN_TCS_CMD2_DATA_DRV0_OFF), \
		      PDC_SEQ_START_ADDR_VALID_BMSK | \
		      ((uint32_t)(val) & PDC_BRANCH_MASK))

#endif /* PDC_REGS_H */
