/*
 * Copyright (c) 2026 Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PDC_REGS_H
#define PDC_REGS_H

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
