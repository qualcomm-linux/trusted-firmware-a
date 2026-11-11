/*
 * Copyright (c) 2026 Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef SMMU_CFG_H
#define SMMU_CFG_H

#include <drivers/qti/smmu/smmu.h>

#include <shikra_def.h>

/*
 * Shikra SMMU implementation-defined / secure register configuration.
 *
 * Addresses: IPCat shikra_1.0 SWI. Values: Shikra SMMU HSR (v2.0),
 * same set QTEE programs from smmu_hsr_shikra.xml.
 *
 * Only secure registers are programmed here. Intentionally NOT programmed:
 *   - non-secure regs: CUSTOM_CFG, NSACR, GPU TLBIS_CTRL, CBn_ACTLR (HLOS)
 *   - GPU ACR (owned by KVM)
 *
 * Apps SMMU base : QTI_SMMU_CFG_BASE     (0x0C600000)
 * GPU  SMMU base : QTI_GPU_SMMU_CFG_BASE (0x059A0000)
 */

/* Apps SMMU */

/* [4]=0 fast sync disabled */
#define SMMU_CUSTOM_CFG_SEC_ADDR		(QTI_SMMU_CFG_BASE + 0x0022fc)
#define SMMU_CUSTOM_CFG_SEC			0x00000000

/* Secure ACR (HSR lists it as "ACR"; IPCat: APPS_SMMU_SACR, secure) */
#define SMMU_SACR_ADDR				(QTI_SMMU_CFG_BASE + 0x000010)
#define SMMU_SACR				0x0000001c

/* [16]=0 internal clock gating csr disabled, hysteresis count 0xFF */
#define SMMU_CLOCK_GATING_SEC_ADDR		(QTI_SMMU_CFG_BASE + 0x180010)
#define SMMU_CLOCK_GATING_SEC			0x000000ff

/* [16]=1 enable HLOS & HYP client debug access (eCATS) */
#define SMMU_CLIENT_DBG_SSD_IDX_HYP_HLOS_EN_ANOC_1_SEC_ADDR	\
	(QTI_SMMU_CFG_BASE + 0x184010)
#define SMMU_CLIENT_DBG_SSD_IDX_HYP_HLOS_EN_ANOC_1_SEC		0x000103ff

#define SMMU_CLIENT_DBG_SSD_IDX_HYP_HLOS_EN_MM_RT_SEC_ADDR	\
	(QTI_SMMU_CFG_BASE + 0x188010)
#define SMMU_CLIENT_DBG_SSD_IDX_HYP_HLOS_EN_MM_RT_SEC		0x000103ff

#define SMMU_CLIENT_DBG_SSD_IDX_HYP_HLOS_EN_PCIE_SEC_ADDR	\
	(QTI_SMMU_CFG_BASE + 0x18c010)
#define SMMU_CLIENT_DBG_SSD_IDX_HYP_HLOS_EN_PCIE_SEC		0x000103ff

/* [31]=1 QNS4 bubble; PCIE also [13]=0 no MID compare, [14]=1 RAR hazard check */
#define SMMU_CLIENT_DBG_SID_HALT_PCIE_SEC_ADDR	(QTI_SMMU_CFG_BASE + 0x18c000)
#define SMMU_CLIENT_DBG_SID_HALT_PCIE_SEC	0x80102000

#define SMMU_CLIENT_DBG_SID_HALT_ANOC_1_SEC_ADDR	(QTI_SMMU_CFG_BASE + 0x184000)
#define SMMU_CLIENT_DBG_SID_HALT_ANOC_1_SEC	0x80102000

/* GPU GFX SMMU */

/* [7:0] min cycles for clock to come up after clk_on */
#define GPU_GFX_SMMU_SPARE_REG_1_SEC_ADDR	(QTI_GPU_SMMU_CFG_BASE + 0x038030)
#define GPU_GFX_SMMU_SPARE_REG_1_SEC		0x0000000f

#define GPU_GFX_SMMU_CLOCK_GATING_SEC_ADDR	(QTI_GPU_SMMU_CFG_BASE + 0x038010)
#define GPU_GFX_SMMU_CLOCK_GATING_SEC		0x000000ff

#define GPU_GFX_SMMU_CLIENT_DBG_SSD_IDX_HYP_HLOS_EN_GFX_0_SEC_ADDR	\
	(QTI_GPU_SMMU_CFG_BASE + 0x03c010)
#define GPU_GFX_SMMU_CLIENT_DBG_SSD_IDX_HYP_HLOS_EN_GFX_0_SEC		0x000103ff

/* [31]=1 QNS4 bubble, [19]=1 GPU IO coherency */
#define GPU_GFX_SMMU_CLIENT_DBG_SID_HALT_GFX_0_SEC_ADDR	\
	(QTI_GPU_SMMU_CFG_BASE + 0x03c000)
#define GPU_GFX_SMMU_CLIENT_DBG_SID_HALT_GFX_0_SEC		0x80182000

/* [13]=1 fast sync 3.0 */
#define GPU_GFX_SMMU_CLIENT_DBG_WUSER_GFX_0_SEC_ADDR	\
	(QTI_GPU_SMMU_CFG_BASE + 0x03c038)
#define GPU_GFX_SMMU_CLIENT_DBG_WUSER_GFX_0_SEC		0x00002000

#endif /* SMMU_CFG_H */
