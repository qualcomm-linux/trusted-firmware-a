/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * NoC error register address definitions for Shikra.
 */

#ifndef QTI_NOC_ERROR_HWIO_H
#define QTI_NOC_ERROR_HWIO_H

#include <stdint.h>

#include "icbcfg_hwio.h"

/* DDR subsystem base, per IP Catalog (chip alias 'shikra_1.0'). */
#define DDR_SS_BASE                     0x00c00000U
/* GCC clock-control base, per IP Catalog (chip alias 'shikra_1.0'). */
#define CLK_CTL_BASE                    0x01400000U

#define MEMNOC_HM_MEM_NOC_BASE          (DDR_SS_BASE + 0x00100000U)
#define DC_NOC_BASE                     (DDR_SS_BASE + 0x000e0000U)
#define GCC_CLK_CTL_REG_BASE            (CLK_CTL_BASE + 0x00000000U)

/* CONFIG_NOC */
#define CNOC_OBS_SWID_LOW               (CONFIG_NOC_BASE + 0x0U)
#define CNOC_SBM_SWID_LOW               (CONFIG_NOC_BASE + 0x200U)
#define CNOC_NORTH_FM_SWID_LOW          (CONFIG_NOC_BASE + 0x400U)
#define CNOC_PCIE_FM_SWID_LOW           (CONFIG_NOC_BASE + 0x600U)
#define CNOC_WEST_FM_SWID_LOW           (CONFIG_NOC_BASE + 0x800U)
#define CONFIG_NOC_REFGEN_MAINCTL_LOW   (CONFIG_NOC_BASE + 0xc08U)
#define CONFIG_NOC_REFGEN_REFCTLn(n)    (CONFIG_NOC_BASE + 0xc40U + ((n) * 8U))
#define CNOC_SBM_TIC_SWID_LOW           (CONFIG_NOC_BASE + 0x3000U)

/* SYSTEM_NOC */
#define SNOC_OBS_ERRORLOGGER_SWID_LOW   (SYSTEM_NOC_BASE + 0x0U)
#define SYSTEM_NOC_SBM_SWID_LOW         (SYSTEM_NOC_BASE + 0x200U)
#define SYSTEM_NOC_REFGEN_MAINCTL_LOW   (SYSTEM_NOC_BASE + 0x408U)
#define SYSTEM_NOC_REFGEN_REFCTLn(n)    (SYSTEM_NOC_BASE + 0x440U + ((n) * 8U))

/* MEMNOC_HM_MEM_NOC: core */
#define MEMNOC_HM_MEM_NOC_CORE_ERL_SWID_LOW        (MEMNOC_HM_MEM_NOC_BASE + 0x0U)
#define MEMNOC_HM_MEM_NOC_CORE_FAULT_SBM_SWID_LOW  (MEMNOC_HM_MEM_NOC_BASE + 0x200U)
#define MEMNOC_HM_MEM_NOC_CORE_REFGEN_MAINCTL_LOW  (MEMNOC_HM_MEM_NOC_BASE + 0x408U)
#define MEMNOC_HM_MEM_NOC_CORE_REFGEN_REFCTLn(n)   (MEMNOC_HM_MEM_NOC_BASE + 0x440U + ((n) * 8U))

/* MEMNOC_HM_MEM_NOC: defrag */
#define MEMNOC_HM_DEFRAG_NOC_ERL_SWID_LOW       (MEMNOC_HM_MEM_NOC_BASE + 0x40000U)
#define MEMNOC_HM_DEFRAG_NOC_SBM_SWID_LOW       (MEMNOC_HM_MEM_NOC_BASE + 0x40400U)
#define MEMNOC_HM_DEFRAG_NOC_REFGEN_MAINCTL_LOW (MEMNOC_HM_MEM_NOC_BASE + 0x40808U)
#define MEMNOC_HM_DEFRAG_NOC_REFGEN_REFCTLn(n)  (MEMNOC_HM_MEM_NOC_BASE + 0x40840U + ((n) * 8U))

/* DC_NOC */
#define DC_NOC_DC_NOC_ERL_SWID_LOW          (DC_NOC_BASE + 0x0U)
#define DC_NOC_DC_NOC_ERROR_SBM_SWID_LOW    (DC_NOC_BASE + 0x200U)
#define DC_NOC_DC_NOC_REFGEN_MAINCTL_LOW    (DC_NOC_BASE + 0x1008U)
#define DC_NOC_DC_NOC_REFGEN_REFCTLn(n)     (DC_NOC_BASE + 0x1040U + ((n) * 8U))

/* GCC_CLK_CTL_REG */
#define GCC_NOC_BUS_TIMEOUT_EXTREF_CBCR         (GCC_CLK_CTL_REG_BASE + 0x2c004U)
#define GCC_NOC_BUS_TIMEOUT_EXTREF_CDIVR        (GCC_CLK_CTL_REG_BASE + 0x2c008U)
#define GCC_NOC_BUS_TIMEOUT_EXTREF_DIV512_CDIVR (GCC_CLK_CTL_REG_BASE + 0x2c010U)

#endif /* QTI_NOC_ERROR_HWIO_H */
