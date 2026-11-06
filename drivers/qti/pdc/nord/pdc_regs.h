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
#define PDC_IRQ_OWNER_DRV_MAX		2U

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

static inline void pdc_irq_owner_write(uint32_t irq, uint32_t owner)
{
	uint32_t bank = irq / PDC_IRQ_OWNER_BANK_BITS;
	uint32_t mask = 1U << (irq % PDC_IRQ_OWNER_BANK_BITS);
	uint32_t current_owner = PDC_IRQ_NONZERO_DRV_CFG_READ();
	uint32_t current_value;

	if (owner > PDC_IRQ_OWNER_DRV_MAX) {
		assert(owner <= PDC_IRQ_OWNER_DRV_MAX);
		return;
	}

	if (owner != 0U) {
		if ((current_owner != 0U) && (current_owner != owner)) {
			assert(current_owner == owner);
			return;
		}

		if (current_owner == 0U) {
			PDC_IRQ_NONZERO_DRV_CFG_WRITE(owner);
		}
	}

	if (bank <= PDC_IRQ_OWNER_BANK_MAX) {
		current_value = PDC_IRQ_OWNER_BANK_READ(bank);
		if (owner == 0U) {
			PDC_IRQ_OWNER_BANK_WRITE(bank, current_value & ~mask);
		} else {
			PDC_IRQ_OWNER_BANK_WRITE(bank, current_value | mask);
		}
	} else {
		current_value = PDC_IRQ_OWNER_LASTBANK_READ();
		if (owner == 0U) {
			PDC_IRQ_OWNER_LASTBANK_WRITE(current_value & ~mask);
		} else {
			PDC_IRQ_OWNER_LASTBANK_WRITE(current_value | mask);
		}
	}
}

#define PDC_IRQ_OWNER_WRITE(i, val) \
	pdc_irq_owner_write((uint32_t)(i), (uint32_t)(val))

/* --------------------------------------------------------------------------
 * HIDDEN_TCS_CMD2_DATA_DRV0  (rsc_base + 0x48)
 * --------------------------------------------------------------------------
 */
#define PDC_BRANCH_SLEEP_TCS0		0x000001U
#define PDC_BRANCH_SLEEP_TCS1		0x000002U
#define PDC_BRANCH_SLEEP_TCS2		0x000004U
#define PDC_BRANCH_SLEEP_TCS3		0x000008U
#define PDC_BRANCH_NO_AOSS_SLEEP	0x000010U
#define PDC_BRANCH_NO_WAKEUP_TIMER	0x000020U
#define PDC_BRANCH_MASK			0x00ffffU

#define PDC_HIDDEN_TCS_CMD2_DATA_DRV0_OFF	0x48U
#define PDC_SEQ_START_ADDR_VALID_BMSK		0x80000000U

#define PDC_SEQ_BRANCH_MASK_WRITE(rsc_base, val) \
	mmio_write_32(PDC_REG(rsc_base, PDC_HIDDEN_TCS_CMD2_DATA_DRV0_OFF), \
		      PDC_SEQ_START_ADDR_VALID_BMSK | \
		      ((uint32_t)(val) & PDC_BRANCH_MASK))

#endif /* PDC_REGS_H */
