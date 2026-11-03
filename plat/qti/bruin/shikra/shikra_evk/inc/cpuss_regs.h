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
#define APSS_ALIAS_n_CORE_HANG_CONFIG(n)		\
	(APSS_ALIAS_0_CORE_HANG_CONFIG + ((n) * APSS_ALIAS_n_APSS_CPU_IPM_PCPU_STRIDE))
#define APSS_ALIAS_n_CORE_HANG_THRESHOLD(n)		\
	(APSS_ALIAS_0_CORE_HANG_THRESHOLD + ((n) * APSS_ALIAS_n_APSS_CPU_IPM_PCPU_STRIDE))
#define CORE_HANG_CONFIG_CORE_HANG_RESET_SHIFT		3
#define CORE_HANG_CONFIG_CORE_HANG_COUNTER_EN_SHIFT	1

/* ACE/GNOC Hang Detection Registers */
#define APSS_SHARED_GNOC_HANG_CONFIG_ADDR		0x0F600434UL
#define APSS_SHARED_GNOC_HANG_THRESHOLD_ADDR		0x0F60041CUL
#define GNOC_HANG_CONFIG_ACE_COUNTER_EN_SHIFT		0

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
/* CPUSS init feature flags - explicit enable/disable                        */
/*----------------------------------------------------------------------------*/

/* Core hang detection - ENABLED */
#define CORE_HANG_DETECTION_EN			1
#define CORE_HANG_THRESHOLD_VALUE		0xB0101DUL /* 600 ms, value x 52ns */

/* ACE/GNOC hang detection - ENABLED */
#define CPUSS_ACE_HANG_DETECTION_EN		1
#define CPUSS_ACE_HANG_THRESHOLD_VALUE		0x92B814UL /* 500 ms, value x 52ns */

/* ACG - DISABLED (dead code removed from bruin11_cpuss_init.c) */
#define BRUIN_ACG_EN				0

/* Error handling - DISABLED */
#define CPUSS_ERRORS_AS_FATAL_EN		0
#define GIC_FAULT_ERR_INTR_EN			0

/*----------------------------------------------------------------------------*/
/* Compile-time checks: Ensure all feature flags are defined                 */
/*----------------------------------------------------------------------------*/
#ifndef CORE_HANG_DETECTION_EN
#error "CORE_HANG_DETECTION_EN must be defined (0 or 1)"
#endif

#ifndef CPUSS_ACE_HANG_DETECTION_EN
#error "CPUSS_ACE_HANG_DETECTION_EN must be defined (0 or 1)"
#endif

#ifndef CPUSS_ERRORS_AS_FATAL_EN
#error "CPUSS_ERRORS_AS_FATAL_EN must be defined (0 or 1)"
#endif

#ifndef GIC_FAULT_ERR_INTR_EN
#error "GIC_FAULT_ERR_INTR_EN must be defined (0 or 1)"
#endif

/*----------------------------------------------------------------------------*/
/* APSS INTU - interrupt type configuration registers                         */
/* HWIO_APSS_SHARED_SPI_CONFIG_n_ADDR(0) = APSS_INTU_BASE + 0x00F0           */
/* (APSS_HM_BASE=0x0F000000, INTU offset=0x400000)                           */
/*----------------------------------------------------------------------------*/
#define APSS_INTU_BASE			0x0F400000
#define APSS_INTU_SIZE			0x1000
#define APSS_SPI_CONFIG_BASE		(APSS_INTU_BASE + 0x00F0U)	/* 0x0F4000F0 */
#define APSS_SPI_NUM_WORDS		19U			/* SPI[0:607] */

#endif /* CPUSS_REGS_H */
