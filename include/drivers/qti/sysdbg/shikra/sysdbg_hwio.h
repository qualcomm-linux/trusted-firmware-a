#ifndef HWIO_H
#define HWIO_H
/*
 * ===========================================================================
 */

/* TFA Port: Add missing base addresses */
#ifndef QDSS_SOC_DBG_BASE
#define QDSS_SOC_DBG_BASE           0x08000000
#define QDSS_SOC_DBG_BASE_PHYS      0x08000000
#endif
#define ETFETB_BASE                 (QDSS_SOC_DBG_BASE + 0x00047000)
/**
 * @file hwio.h
 * @brief Auto-generated HWIO interface include file.
 *
 * Reference chip release:
 * Shikra [shikra_v1.0_p2q1r38_tiled_rtl.incr_upload]
 *
 * This file contains HWIO register definitions for the following modules:
 * TLMM_.*
 * SECURITY_CONTROL_CORE
 *
 *
 * Generation parameters:
 * { 'filename': 'hwio.h',
 * 'modules': ['TLMM_.*', 'SECURITY_CONTROL_CORE'],
 * 'output-fvals': True,
 * 'output-offsets': True,
 * 'output-phys': True}
 */
/*
 * ===========================================================================
 *
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * All rights reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 * ===========================================================================
 */

/* Include msmhwiobase.h for base address definitions
 * All conflicting definitions are now guarded with #ifndef
 */
#include "msmhwiobase.h"

/* QDSS CoreSight Lock/Unlock values */
#define QDSS_CORESIGHT_UNLOCK           0xC5ACCE55
#define QDSS_CORESIGHT_LOCK             0x00000000

#define MSS_TOP_BASE 0x6000000
#define MAPSS_BASE 0x4400000

#define GCC_CLK_CTL_REG_REG_BASE                                                                       (CLK_CTL_BASE      + 0x00000000)
#define HWIO_GCC_QDSS_DAP_CBCR_CLK_ENABLE_BMSK                                                                0x1
#define HWIO_GCC_APSS_QDSS_APB_CBCR_ADDR                                                               (GCC_CLK_CTL_REG_REG_BASE      + 0x2b174)
#define HWIO_GCC_APSS_QDSS_APB_CBCR_CLK_ENABLE_BMSK                                                           0x1
#define HWIO_GCC_APCS_CLOCK_BRANCH_ENA_VOTE_ADDR                                                       (GCC_CLK_CTL_REG_REG_BASE      + 0x79004)
#define HWIO_GCC_APCS_CLOCK_BRANCH_ENA_VOTE_QDSS_CFG_AHB_CLK_ENA_BMSK                                     0x20000
#define HWIO_GCC_APCS_CLOCK_BRANCH_ENA_VOTE_IMEM_AXI_CLK_ENA_BMSK                                            0x80
#define HWIO_GCC_APCS_VOTE_QDSS_APB_CLK_ADDR                                                           (GCC_CLK_CTL_REG_REG_BASE      + 0x79014)
#define HWIO_GCC_APCS_VOTE_QDSS_APB_CLK_CLK_ENABLE_BMSK                                                       0x1
#define HWIO_GCC_APCS_TZ_CLOCK_BRANCH_ENA_VOTE_IMEM_AXI_CLK_ENA_BMSK                                         0x80
#define HWIO_GCC_APCS_TZ_VOTE_QDSS_APB_CLK_ADDR                                                        (GCC_CLK_CTL_REG_REG_BASE      + 0x7a014)
#define HWIO_GCC_APCS_TZ_VOTE_QDSS_APB_CLK_CLK_ENABLE_BMSK                                                    0x1
#define HWIO_GCC_RESET_DEBUG_ENABLE_ADDR                                                               (GCC_CLK_CTL_REG_REG_BASE      + 0x90004)
#define HWIO_GCC_RESET_DEBUG_ENABLE_ALLOW_SW_BASED_RST_DBG_EN_BMSK                                          0x800
#define HWIO_GCC_RESET_DEBUG_ENABLE_SECURE_WDOG_RESET_DEBUG_EN_BMSK                                          0x40
#define HWIO_GCC_RESET_DEBUG_ENABLE_SECURE_WDOG_RESET_DEBUG_EN_SHFT                                             6
#define HWIO_GCC_RESET_DEBUG_ENABLE_MSM_TSENSE_RESET_DEBUG_EN_BMSK                                           0x20
#define HWIO_GCC_RESET_DEBUG_ENABLE_PMIC_RESIN_RESET_DEBUG_EN_BMSK                                           0x10
#define HWIO_GCC_RESET_DEBUG_ENABLE_PMIC_RESIN_RESET_DEBUG_EN_SHFT                                              4
#define HWIO_GCC_RESET_DEBUG_ENABLE_AUTO_PS_HOLD_SET_EN_BMSK                                                  0x4
#define HWIO_GCC_RESET_DEBUG_ENABLE_ACCESS_RESET_FIRST_PASS_EN_BMSK                                           0x2
#define HWIO_GCC_RESET_DEBUG_ENABLE_DEBUG_RESET_FIRST_PASS_EN_BMSK                                            0x1
#define HWIO_GCC_RESET_FSM_CTRL_ADDR                                                                   (GCC_CLK_CTL_REG_REG_BASE      + 0x90008)
#define HWIO_GCC_RESET_FSM_CTRL_FIRST_PASS_TMR_EN_BMSK                                                        0x4
#define HWIO_GCC_RESET_FSM_CTRL_FIRST_PASS_COMPLETE_BMSK                                                      0x2
#define HWIO_GCC_WIND_DOWN_TIMER_ADDR                                                                  (GCC_CLK_CTL_REG_REG_BASE      + 0x9000c)
#define HWIO_GCC_SAVE_CONTENTS_TIMER_ADDR                                                              (GCC_CLK_CTL_REG_REG_BASE      + 0x90010)
#define HWIO_GCC_STOP_CAPTURE_TIMER_ADDR                                                               (GCC_CLK_CTL_REG_REG_BASE      + 0x90014)
#define HWIO_GCC_PREPARE_FOR_RESET_TIMER_ADDR                                                          (GCC_CLK_CTL_REG_REG_BASE      + 0x90018)
#define HWIO_GCC_FIRST_PASS_TIMEOUT_ADDR                                                               (GCC_CLK_CTL_REG_REG_BASE      + 0x9001c)
#define HWIO_GCC_RESET_STATUS_ADDR                                                                     (GCC_CLK_CTL_REG_REG_BASE      + 0x90024)
#define HWIO_GCC_RESET_STATUS_RMSK                                                                     0xffffffff
#define HWIO_GCC_RESET_STATUS_PMIC_ABNORMAL_RESIN_RESET_STATUS_SHFT                                             6
#define HWIO_GCC_RESET_STATUS_PROC_HALT_CTI_STATUS_BMSK                                                      0x10
#define HWIO_GCC_RESET_STATUS_SRST_RESET_STATUS_BMSK                                                          0x8
#define HWIO_GCC_RESET_STATUS_SECURE_WDOG_EXPIRE_RESET_STATUS_SHFT                                              0
#define HWIO_GCC_RESET_FSM_STATUS_ADDR                                                                 (GCC_CLK_CTL_REG_REG_BASE      + 0x90028)
#define HWIO_GCC_RESET_FSM_STATUS_FSM_STATE_BMSK                                                              0xf
#define QDSS_QDSS_DCC_DCC_RAM_REG_BASE (QDSS_SOC_DBG_BASE      + 0x00080000)
#define QDSS_QDSS_QDSS_CSR_REG_BASE                                                                (QDSS_SOC_DBG_BASE      + 0x00001000)
#define HWIO_QDSS_QDSS_CS_QDSSCSR_LOCKACCESS_ADDR                                                  (QDSS_QDSS_QDSS_CSR_REG_BASE      + 0xfb0)
#define QDSS_QDSS_DCC_DCC_CFG_REG_BASE                                                  (QDSS_SOC_DBG_BASE      + 0x000ff000)
#define HWIO_QDSS_QDSS_DCC_DCC_LL_LOCKm_ADDR(m)                                         (QDSS_QDSS_DCC_DCC_CFG_REG_BASE      + 0X34 + (0x80*(m)))
#define HWIO_QDSS_QDSS_DCC_DCC_LL_LOCKm_S_BIT_BMSK                                             0x8
#define HWIO_QDSS_QDSS_DCC_DCC_LL_LOCKm_ACTIVE_BMSK                                            0x1
#define HWIO_QDSS_QDSS_DCC_DCC_LL_CONFIGm_ADDR(m)                                       (QDSS_QDSS_DCC_DCC_CFG_REG_BASE      + 0X38 + (0x80*(m)))
#define HWIO_QDSS_QDSS_DCC_DCC_LL_CONFIGm_MAXm                                                   7
#define HWIO_QDSS_QDSS_DCC_DCC_LL_CONFIGm_HW_TRIGGER_EN_BMSK                                 0x100
#define HWIO_QDSS_QDSS_DCC_DCC_LL_CONFIGm_HW_TRIGGER_EN_SHFT                                     8
#define HWIO_QDSS_QDSS_DCC_DCC_LL_CONFIGm_SINK_BMSK                                           0x10
#define HWIO_QDSS_QDSS_DCC_DCC_LL_CONFIGm_SINK_SHFT                                              4
#define HWIO_QDSS_QDSS_DCC_DCC_LL_CONFIGm_FUNC_BMSK                                            0x1
#define HWIO_QDSS_QDSS_DCC_DCC_LL_CONFIGm_FUNC_SHFT                                              0
#define HWIO_QDSS_QDSS_DCC_DCC_LL_BASEm_ADDR(m)                                         (QDSS_QDSS_DCC_DCC_CFG_REG_BASE      + 0X3C + (0x80*(m)))
#define HWIO_QDSS_QDSS_DCC_DCC_LL_BASEm_MAXm                                                     7
#define HWIO_QDSS_QDSS_DCC_DCC_LL_BASEm_ADDRESS_BMSK                                        0x1fff
#define HWIO_QDSS_QDSS_DCC_DCC_LL_BASEm_ADDRESS_SHFT                                             0
#define HWIO_QDSS_QDSS_DCC_DCC_FD_BASEm_ADDR(m)                                         (QDSS_QDSS_DCC_DCC_CFG_REG_BASE      + 0X40 + (0x80*(m)))
#define HWIO_QDSS_QDSS_DCC_DCC_FD_BASEm_ADDRESS_BMSK                                        0x1fff
#define HWIO_QDSS_QDSS_DCC_DCC_FD_BASEm_ADDRESS_SHFT                                             0
#define HWIO_QDSS_QDSS_DCC_DCC_LL_TIMEOUTm_ADDR(m)                                      (QDSS_QDSS_DCC_DCC_CFG_REG_BASE      + 0X44 + (0x80*(m)))
#define HWIO_QDSS_QDSS_DCC_DCC_LL_TIMEOUTm_VALUE_BMSK                                   0xffffffff
#define HWIO_QDSS_QDSS_DCC_DCC_LL_TIMEOUTm_VALUE_SHFT                                            0
#define HWIO_QDSS_QDSS_DCC_DCC_TRANS_TIMEOUTm_ADDR(m)                                   (QDSS_QDSS_DCC_DCC_CFG_REG_BASE      + 0X48 + (0x80*(m)))
#define HWIO_QDSS_QDSS_DCC_DCC_TRANS_TIMEOUTm_VALUE_BMSK                                    0xffff
#define HWIO_QDSS_QDSS_DCC_DCC_TRANS_TIMEOUTm_VALUE_SHFT                                         0
#define QDSS_QDSS_DCC_DCC_APU_REG_BASE                                           (QDSS_SOC_DBG_BASE      + 0x000e0000)
#define QDSS_APSS_APSS_APSS_APB_SDC_ALL_REG_BASE                         (QDSS_SOC_DBG_BASE      + 0x01870000)
#define HWIO_QDSS_APSS_APSS_APB_SDC_CONTROL_ADDR                         (QDSS_APSS_APSS_APSS_APB_SDC_ALL_REG_BASE      + 0x0)
#define HWIO_QDSS_APSS_APSS_SCAN_DUMP_MODE_ADDR                          (QDSS_APSS_APSS_APSS_APB_SDC_ALL_REG_BASE      + 0x4)
#define HWIO_QDSS_APSS_APSS_CTRL_0_EN_ADDR                               (QDSS_APSS_APSS_APSS_APB_SDC_ALL_REG_BASE      + 0x8)
#define HWIO_QDSS_APSS_APSS_CTRL_1_EN_ADDR                               (QDSS_APSS_APSS_APSS_APB_SDC_ALL_REG_BASE      + 0xc)
#define HWIO_QDSS_APSS_APSS_CTRL_2_EN_ADDR                               (QDSS_APSS_APSS_APSS_APB_SDC_ALL_REG_BASE      + 0x10)
#define HWIO_QDSS_APSS_APSS_BOOT_TRIGGER_ADDR                            (QDSS_APSS_APSS_APSS_APB_SDC_ALL_REG_BASE      + 0x18)
#define APSS_ALIAS_0_APSS_CPU_IPM_REG_BASE                                                (APSS_HM_BASE      + 0x00800000)
#define HWIO_APSS_ALIAS_0_CPU_SEQ_FORCE_PWR_CTL_EN_ADDR                                   (APSS_ALIAS_0_APSS_CPU_IPM_REG_BASE      + 0x1c)
#define HWIO_APSS_ALIAS_0_CPU_SEQ_FORCE_PWR_CTL_VAL_ADDR                                  (APSS_ALIAS_0_APSS_CPU_IPM_REG_BASE      + 0x20)
#define APSS_APSS_CL_IPM_REG_BASE                                                         (APSS_HM_BASE      + 0x00880000)
#define HWIO_APSS_L3_SEQ_FORCE_PWR_CTL_EN_ADDR                                            (APSS_APSS_CL_IPM_REG_BASE      + 0x1c)
#define HWIO_APSS_L3_SEQ_FORCE_PWR_CTL_VAL_ADDR                                           (APSS_APSS_CL_IPM_REG_BASE      + 0x20)
#define HWIO_APSS_CL_QCHANNEL_FSM_EN_CTL_ADDR                                             (APSS_APSS_CL_IPM_REG_BASE      + 0x54)
#define APSS_SHARED_APSS_SHARED_REG_BASE                                                    (APSS_HM_BASE      + 0x00600000)
#define HWIO_APSS_SHARED_CPU_PRESENT_ADDR                                                   (APSS_SHARED_APSS_SHARED_REG_BASE      + 0x24)
#define HWIO_APSS_SHARED_CPU_PRESENT_CORE_PRESENT_BMSK                                            0xff

#endif /* HWIO_H */
