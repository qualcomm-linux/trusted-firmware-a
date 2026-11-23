/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Curated PCU/RIMPS register definitions for shikra.
 */

#ifndef PCU_HWIO_H
#define PCU_HWIO_H

#include <shikra_def.h>

/* Per-core APSS CPU IPM aliases: CPU_SEQ_CTL, branch addresses, LPM counter. */
#define APSS_CPU_IPM_BASE(core)		(QTI_APSS_HM_BASE + 0x00800000U + \
					 ((core) * 0x10000U))

#define CPU_SEQ_CTL(core)		(APSS_CPU_IPM_BASE(core) + 0x24U)
#define CPU_SEQ_CTL_SEQ_START_ADDR_BMSK	0x1ff0000U
#define CPU_SEQ_CTL_SEQ_START_ADDR_SHFT		16U
#define CPU_SEQ_CTL_SEQ_PWR_STATE_IDX_BMSK	0xf00U
#define CPU_SEQ_CTL_SEQ_PWR_STATE_IDX_SHFT		8U
#define CPU_SEQ_CTL_SEQ_EN_BMSK			0x1U

#define CPU_SEQ_BRANCH_START_ADDR_n(core, n)	(APSS_CPU_IPM_BASE(core) + \
						 0x28U + (0x4U * (n)))
#define CPU_SEQ_LPM_COUNTER_CFG(core)	(APSS_CPU_IPM_BASE(core) + 0x54U)

/* Index into the per-core branch-address array used by the C3 BEQ opcode. */
#define CPU_SEQ_BRANCH_C3_SHORT_IDX	0U

/* L3/cluster IPM: same CPU_SEQ_CTL layout, different base + extra registers. */
#define APSS_CL_IPM_BASE		(QTI_APSS_HM_BASE + 0x00880000U)
#define L3_SEQ_CTL			(APSS_CL_IPM_BASE + 0x24U)
#define L3_SEQ_LPM_COUNTER_CFG		(APSS_CL_IPM_BASE + 0x98U)

/* Shared CPU+L3 opcode/residency-counter memory. */
#define APSS_SEQ_MEM_BASE		(QTI_APSS_HM_BASE + 0x008c0000U)
#define APSS_SEQ_COMMON_INSTR_n(n)	(APSS_SEQ_MEM_BASE + (0x4U * (n)))
#define APSS_CPU_LPM_RESIDENCY_CNTR_CFG_n(n) \
					(APSS_SEQ_MEM_BASE + 0xc004U + (0x4U * (n)))
#define APSS_CL_LPM_RESIDENCY_CNTR_CFG	(APSS_SEQ_MEM_BASE + 0xc030U)

/* SAW4 SPM sequencer (Silver rail), coordinated with the CPU C3 sequence. */
#define SILVER_SAW4_SRM_BASE		(QTI_APSS_HM_BASE + 0x00900000U)
#define SILVER_SAW4_SRM_CTL		(SILVER_SAW4_SRM_BASE + 0x0U)
#define SILVER_SAW4_CFG			(SILVER_SAW4_SRM_BASE + 0xcU)
#define SILVER_SAW4_CFG_CLK_DIV_BMSK	0x1fU
#define SILVER_SAW4_CFG_CLK_DIV_SHFT	0U
#define SILVER_SAW4_SRM_PMIC_DATA_n(n)	(SILVER_SAW4_SRM_BASE + 0x40U + (0x4U * (n)))

/* RIMPS opcode-sequencer memory, shared by the Silver/Gold/L3 PLL domains. */
#define RIMPSFAST_BASE			(QTI_APSS_HM_BASE + 0x00d80000U)
#define RIMPSFAST_RIMPS_MUC_CLK_CTRL	(RIMPSFAST_BASE + 0x0U)
#define RIMPSFAST_MUC_CLK_CTRL_CPU_PCU_SEQ_HALT_BMSK	0xff0U
#define RIMPSFAST_MUC_CLK_CTRL_CORE_CLK_EN_BMSK	0x2U
#define RIMPSFAST_MUC_CLK_CTRL_CORE_SS_CLK_EN_BMSK	0x1U
#define RIMPSFAST_SEQ_MEM_n(n)		(RIMPSFAST_BASE + 0x200U + (0x4U * (n)))

/* RIMPS clock-domain sequencers: CLKDOM0 = L3, CLKDOM1 = Silver, CLKDOM2 = Gold. */
#define RIMPSSLOW_CLKDOM0_BASE		(QTI_APSS_HM_BASE + 0x00d90000U)
#define RIMPSSLOW_CLKDOM1_BASE		(QTI_APSS_HM_BASE + 0x00d91000U)
#define RIMPSSLOW_CLKDOM2_BASE		(QTI_APSS_HM_BASE + 0x00d92000U)

#define RIMPSSLOW_RIMPS_ENABLE(clkdom_base)	((clkdom_base) + 0x0U)
#define RIMPSSLOW_RIMPS_ENABLE_RIMPS_EN_BMSK	0x1U
#define RIMPSSLOW_SEQ_CTRL1(clkdom_base)	((clkdom_base) + 0x348U)
#define RIMPSSLOW_SEQ_CTRL1_PLL_SEQ_START_ADDR_BMSK	0x1ffU
#define RIMPSSLOW_SEQ_CTRL1_PLL_SEQ_START_ADDR_SHFT	0U

/* Gold's branch slot for the C4 BEQR opcode (PLL-off -> rail-off handoff). */
#define RIMPSSLOW_CLKDOM2_SEQ_BR_CTRL_n(n)	(RIMPSSLOW_CLKDOM2_BASE + \
						 0x350U + (0x4U * (n)))
#define RIMPSSLOW_SEQ_BR_CTRL_BRANCH_IDX	0U

/* RIMPS top-level enable, shared by the opcode sequencer and the CPUCP MUC. */
#define RIMPSTOP_BASE			(QTI_APSS_HM_BASE + 0x00d98000U)
#define RIMPSTOP_GLOBAL_ENABLE		(RIMPSTOP_BASE + 0x4U)
#define RIMPSTOP_GLOBAL_ENABLE_BMSK	0x1U

#endif /* PCU_HWIO_H */
