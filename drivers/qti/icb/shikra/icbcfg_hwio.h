/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * ICB configuration hardware register address definitions for Shikra.
 */

#ifndef QTI_ICBCFG_HWIO_H
#define QTI_ICBCFG_HWIO_H

/* Base addresses */
#define SYSTEM_NOC_BASE		        0x01880000U
#define CONFIG_NOC_BASE		        0x01900000U

/*
 * DCD (dynamic clock divider) MAXDIV registers.
 * Offsets taken from IP Catalog (chip alias 'shikra_1.0').
 */
#define SNOC_DCD_PIMEM_QXCLK_MAXDIV_LOW                              (SYSTEM_NOC_BASE + 0x2008U)
#define SNOC_DCD_CENTER_QXCLK_MAXDIV_LOW                             (SYSTEM_NOC_BASE + 0x2088U)
#define SNOC_DCD_MM_SF_QXCLK_MAXDIV_LOW                              (SYSTEM_NOC_BASE + 0x2108U)
#define SNOC_DCD_MM_HF_QXCLK_MAXDIV_LOW                              (SYSTEM_NOC_BASE + 0x2188U)
#define SNOC_DCD_LPASS_QXCLK_MAXDIV_LOW                              (SYSTEM_NOC_BASE + 0x2208U)
#define SNOC_DCD_WCSS_QXCLK_MAXDIV_LOW                               (SYSTEM_NOC_BASE + 0x2288U)
#define SNOC_DCD_MODEM_QXCLK_MAXDIV_LOW                              (SYSTEM_NOC_BASE + 0x2308U)
#define SYSTEM_NOC_SNOC_DCD_APSS_QHCLK_DCD_MAXDIV_LOW                (SYSTEM_NOC_BASE + 0x2388U)
#define SYSTEM_NOC_SNOC_DCD_CDSP_QXCLK_DCD_MAXDIV_LOW                (SYSTEM_NOC_BASE + 0x2408U)
#define SYSTEM_NOC_SNOC_DCD_CENTER_PEIPH_CLK_DCD_MAXDIV_LOW          (SYSTEM_NOC_BASE + 0x2488U)
#define SYSTEM_NOC_SNOC_DCD_CNOC_SF_QXCLK_DCD_MAXDIV_LOW             (SYSTEM_NOC_BASE + 0x2508U)
#define SYSTEM_NOC_SNOC_DCD_CNOC_SLAVE_SF_QXCLK_DCD_MAXDIV_LOW       (SYSTEM_NOC_BASE + 0x2588U)
#define SYSTEM_NOC_SNOC_DCD_EMAC0_QXCLK_DCD_MAXDIV_LOW               (SYSTEM_NOC_BASE + 0x2608U)
#define SYSTEM_NOC_SNOC_DCD_EMAC1_QXCLK_DCD_MAXDIV_LOW               (SYSTEM_NOC_BASE + 0x2688U)
#define SYSTEM_NOC_SNOC_DCD_MCUSS_QXCLK_DCD_MAXDIV_LOW               (SYSTEM_NOC_BASE + 0x2708U)
#define SYSTEM_NOC_SNOC_DCD_MEMNOC_SF_QXCLK_DCD_MAXDIV_LOW           (SYSTEM_NOC_BASE + 0x2788U)
#define SYSTEM_NOC_SNOC_DCD_PCIE2_QXCLK_DCD_MAXDIV_LOW               (SYSTEM_NOC_BASE + 0x2808U)
#define SYSTEM_NOC_SNOC_DCD_QDSS_STM_CLK_DCD_MAXDIV_LOW              (SYSTEM_NOC_BASE + 0x2888U)
#define SYSTEM_NOC_SNOC_DCD_QUP0_PERIPH_CLK_DCD_MAXDIV_LOW           (SYSTEM_NOC_BASE + 0x2908U)
#define SYSTEM_NOC_SNOC_DCD_USB2_QXCLK_DCD_MAXDIV_LOW                (SYSTEM_NOC_BASE + 0x2988U)
#define SYSTEM_NOC_SNOC_DCD_USB3_QXCLK_DCD_MAXDIV_LOW                (SYSTEM_NOC_BASE + 0x2a08U)
#define SYSTEM_NOC_SNOC_DCD_WEST_PEIPH_CLK_DCD_MAXDIV_LOW            (SYSTEM_NOC_BASE + 0x2a88U)
#define SYSTEM_NOC_SNOC_DCD_WEST_QXCLK_DCD_MAXDIV_LOW                (SYSTEM_NOC_BASE + 0x2b08U)
#define CNOC_CENTER_DCD_QHCLK_MAXDIV_LOW                             (CONFIG_NOC_BASE + 0x2008U)
#define CNOC_WEST_DCD_QHCLK_MAXDIV_LOW                               (CONFIG_NOC_BASE + 0x2108U)
#define CNOC_NORTH_DCD_QHCLK_MAXDIV_LOW                              (CONFIG_NOC_BASE + 0x2208U)
#define CONFIG_NOC_CNOC_PCIE_DCD_QHCLK_DCD_MAXDIV_LOW                (CONFIG_NOC_BASE + 0x2308U)

#endif /* QTI_ICBCFG_HWIO_H */
