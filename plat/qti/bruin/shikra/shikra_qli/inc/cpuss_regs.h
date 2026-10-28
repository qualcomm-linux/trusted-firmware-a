/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CPUSS_REGS_H
#define CPUSS_REGS_H

/*----------------------------------------------------------------------------*/
/* CPUSS init register definitions                                           */
/*----------------------------------------------------------------------------*/

/* CPU Present Register */
#define APSS_SHARED_CPU_PRESENT			0x0F600024UL

/* Core Hang Detection Registers */
#define APSS_ALIAS_0_CORE_HANG_CONFIG			0x0F800060UL
#define APSS_ALIAS_0_CORE_HANG_THRESHOLD		0x0F800058UL
#define APSS_ALIAS_n_APSS_CPU_IPM_PCPU_STRIDE		0x10000UL
#define APSS_ALIAS_n_CORE_HANG_CONFIG(n)		(APSS_ALIAS_0_CORE_HANG_CONFIG + \
							 ((n) * APSS_ALIAS_n_APSS_CPU_IPM_PCPU_STRIDE))
#define APSS_ALIAS_n_CORE_HANG_THRESHOLD(n)		(APSS_ALIAS_0_CORE_HANG_THRESHOLD + \
							 ((n) * APSS_ALIAS_n_APSS_CPU_IPM_PCPU_STRIDE))
#define CORE_HANG_CONFIG_CORE_HANG_RESET_SHIFT		3
#define CORE_HANG_CONFIG_CORE_HANG_COUNTER_EN_SHIFT	1
#define CORE_HANG_THRESHOLD_VALUE			0xB0101DUL /* 600 ms, value x 52ns */

/* ACE/GNOC Hang Detection Registers */
#define APSS_SHARED_GNOC_HANG_CONFIG_ADDR		0x0F600434UL
#define APSS_SHARED_GNOC_HANG_THRESHOLD_ADDR		0x0F60041CUL
#define GNOC_HANG_CONFIG_ACE_COUNTER_EN_SHIFT		0
#define CPUSS_ACE_HANG_THRESHOLD_VALUE			0x92B814UL /* 500 ms, value x 52ns */

/* ACG (Automatic Clock Gating) Registers */
#define APSS_CL_IPM_REG_BASE				0x0F880000UL
#define APSS_CL_SCLK_QCHANNEL_FSM_HYST_CTL_ADDR	(APSS_CL_IPM_REG_BASE + 0x58)
#define APSS_CL_GICCLK_QCHANNEL_FSM_HYST_CTL_ADDR	(APSS_CL_IPM_REG_BASE + 0x5C)
#define APSS_CL_PCLK_QCHANNEL_FSM_HYST_CTL_ADDR	(APSS_CL_IPM_REG_BASE + 0x60)
#define APSS_CL_ATCLK_QCHANNEL_FSM_HYST_CTL_ADDR	(APSS_CL_IPM_REG_BASE + 0x64)
#define APSS_CL_GNOC_QCHANNEL_FSM_HYST_CTL_ADDR	(APSS_CL_IPM_REG_BASE + 0x68)
#define QCHANNEL_FSM_HYST_CTL_ACG_HYST_CNT_SHFT	8
#define QCHANNEL_FSM_HYST_CTL_CBC_HYST_CNT_SHFT	0

#define APSS_CL_QCHANNEL_FSM_EN_CTL_ADDR		(APSS_CL_IPM_REG_BASE + 0x54)
#define QCHANNEL_FSM_EN_CTL_GNOC_ACG_EN_SHFT		9
#define QCHANNEL_FSM_EN_CTL_GNOC_CLK_ON_SHFT		8
#define QCHANNEL_FSM_EN_CTL_ATCLK_ACG_EN_SHFT		7
#define QCHANNEL_FSM_EN_CTL_ATCLK_CLK_ON_SHFT		6
#define QCHANNEL_FSM_EN_CTL_GICCLK_ACG_EN_SHFT		5
#define QCHANNEL_FSM_EN_CTL_GICCLK_CLK_ON_SHFT		4
#define QCHANNEL_FSM_EN_CTL_PCLK_ACG_EN_SHFT		3
#define QCHANNEL_FSM_EN_CTL_PCLK_CLK_ON_SHFT		2
#define QCHANNEL_FSM_EN_CTL_SCLK_ACG_EN_SHFT		1
#define QCHANNEL_FSM_EN_CTL_SCLK_CLK_ON_SHFT		0

/* Errors as Fatal Registers */
#define APSS_SHARED_APSS_INTU_REG_BASE			0x0F400000UL
#define APSS_SHARED_WDOG_BITE_INT0_CONFIG_ADDR		(APSS_SHARED_APSS_INTU_REG_BASE + 0x38)
#define WDOG_BITE_INT0_CONFIG_GIC_FAULT_INT_SEL_SHFT	4
#define WDOG_BITE_INT0_CONFIG_GIC_ERR_INT_SEL_SHFT	3
#define WDOG_BITE_INT0_CONFIG_GNOC_HANG_STS_SEL_SHFT	2
#define WDOG_BITE_INT0_CONFIG_CORE_HANG_STS_SEL_SHFT	1
#define WDOG_BITE_INT0_CONFIG_WDOG_BITE_INT_SEL_SHFT	0

/* GIC Fault/Error Interrupt Registers */
#define APSS_GIC600_GICD_APSS_REG_BASE			0x0F200000UL
#define APSS_GICT_ERR1CTLR_ADDR			(APSS_GIC600_GICD_APSS_REG_BASE + 0x00020048)
#define APSS_GICT_ERR2CTLR_ADDR			(APSS_GIC600_GICD_APSS_REG_BASE + 0x00020088)
#define GICT_ERR1CTLR_CFI_SHFT				8
#define GICT_ERR2CTLR_FI_SHFT				3
#define GICT_ERR2CTLR_UI_SHFT				2


/*----------------------------------------------------------------------------*/
/* CPUSS init feature flags                                                  */
/*----------------------------------------------------------------------------*/
#define CORE_HANG_DETECTION_EN			1
#define CPUSS_ACE_HANG_DETECTION_EN		1
#define BRUIN_ACG_EN				0
#define BRUIN_GNOC_ACG_EN			1
#define BRUIN_PCLK_ACG_EN			1
#define BRUIN_GICCLK_ACG_EN			1
#define BRUIN_ATCLK_ACG_EN			1
#define BRUIN_SCLK_ACG_EN			1
#define CPUSS_ERRORS_AS_FATAL_EN		0
#define GIC_FAULT_ERR_INTR_EN			0

/* ACG Hysteresis Values */
#define SCLK_QCHANNEL_ACG_HYST_CNT		0x20
#define SCLK_QCHANNEL_CBC_HYST_CNT		0x4
#define GNOC_QCHANNEL_ACG_HYST_CNT		0x20
#define GNOC_QCHANNEL_CBC_HYST_CNT		0x8
#define GICCLK_QCHANNEL_ACG_HYST_CNT		0x20
#define GICCLK_QCHANNEL_CBC_HYST_CNT		0x8
#define PCLK_QCHANNEL_ACG_HYST_CNT		0x20
#define PCLK_QCHANNEL_CBC_HYST_CNT		0x8
#define ATCLK_QCHANNEL_ACG_HYST_CNT		0x20
#define ATCLK_QCHANNEL_CBC_HYST_CNT		0x8

/* Shikra-Specific Overrides */
#define GNOC_ACG_DISABLE			1
#define SCLK_ACG_DISABLE			1

/*----------------------------------------------------------------------------*/
/* APSS INTU - interrupt type configuration registers                         */
/* HWIO_APSS_SHARED_SPI_CONFIG_n_ADDR(0) = APSS_INTU_BASE + 0x00F0           */
/* (APSS_HM_BASE=0x0F000000, INTU offset=0x400000)                           */
/*----------------------------------------------------------------------------*/
#define APSS_INTU_BASE			0x0F400000
#define APSS_SPI_CONFIG_BASE		(APSS_INTU_BASE + 0x00F0U)	/* 0x0F4000F0 */
#define APSS_SPI_NUM_WORDS		19U			/* SPI[0:607] */

#endif /* CPUSS_REGS_H */