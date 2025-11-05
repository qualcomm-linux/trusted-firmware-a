/********************************************************************
 * Copyright (c) 2025 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *********************************************************************/

#ifndef SMMU_SOC_CFG_H
#define SMMU_SOC_CFG_H

#include "smmu_cfg.h"


#define APPS_SMMU_CUSTOM_CFG_ADDR                                             0x15002300
#define APPS_SMMU_CUSTOM_CFG                                                  0x60480010

#define APPS_SMMU_CUSTOM_CFG_SEC_ADDR                                         0x150022fc
#define APPS_SMMU_CUSTOM_CFG_SEC                                              0x000006f0

#define APPS_SMMU_ACR_ADDR                                                    0x15000010
#define APPS_SMMU_ACR                                                         0x0000001c

#define APPS_SMMU_NSACR_ADDR                                                  0x15000410
#define APPS_SMMU_NSACR                                                       0x0000001c

#define APPS_SMMU_SAFE_SEC_CFG_ADDR                                           0x15002648
#define APPS_SMMU_SAFE_SEC_CFG                                                0x00000000

#define APPS_SMMU_MTLB_DEPTH_ADDR                                             0x15002524
#define APPS_SMMU_MTLB_DEPTH                                                  0x000001ff

#define APPS_SMMU_PFB_DEPTH_ADDR                                              0x15002564
#define APPS_SMMU_PFB_DEPTH                                                   0x000001ff

#define APPS_SMMU_SPARE_REG_1_SEC_ADDR                                        0x151d8030
#define APPS_SMMU_SPARE_REG_1_SEC                                             0x0000080f

#define APPS_SMMU_CLOCK_GATING_SEC_ADDR                                       0x151d8010
#define APPS_SMMU_CLOCK_GATING_SEC                                            0x000400ff

#define APPS_SMMU_CLIENT_DEBUG_SSD_INDEX_HYP_HLOS_EN_ANOC_1_SEC_ADDR          0x151dc010
#define APPS_SMMU_CLIENT_DEBUG_SSD_INDEX_HYP_HLOS_EN_ANOC_1_SEC               0x000103ff

#define APPS_SMMU_CLIENT_DEBUG_SSD_INDEX_HYP_HLOS_EN_ANOC_2_SEC_ADDR          0x151e0010
#define APPS_SMMU_CLIENT_DEBUG_SSD_INDEX_HYP_HLOS_EN_ANOC_2_SEC               0x000103ff

#define APPS_SMMU_CLIENT_DEBUG_SSD_INDEX_HYP_HLOS_EN_MNOC_HF_0_SEC_ADDR       0x151e4010
#define APPS_SMMU_CLIENT_DEBUG_SSD_INDEX_HYP_HLOS_EN_MNOC_HF_0_SEC            0x000103ff

#define APPS_SMMU_CLIENT_DEBUG_SSD_INDEX_HYP_HLOS_EN_MNOC_HF_1_SEC_ADDR       0x151e8010
#define APPS_SMMU_CLIENT_DEBUG_SSD_INDEX_HYP_HLOS_EN_MNOC_HF_1_SEC            0x000103ff

#define APPS_SMMU_CLIENT_DEBUG_SSD_INDEX_HYP_HLOS_EN_COMPUTE_DSP_1_SEC_ADDR   0x151ec010
#define APPS_SMMU_CLIENT_DEBUG_SSD_INDEX_HYP_HLOS_EN_COMPUTE_DSP_1_SEC        0x000103ff

#define APPS_SMMU_CLIENT_DEBUG_SSD_INDEX_HYP_HLOS_EN_COMPUTE_DSP_0_SEC_ADDR   0x151f0010
#define APPS_SMMU_CLIENT_DEBUG_SSD_INDEX_HYP_HLOS_EN_COMPUTE_DSP_0_SEC        0x000103ff

#define APPS_SMMU_CLIENT_DEBUG_SSD_INDEX_HYP_HLOS_EN_LPASS_SEC_ADDR           0x151f4010
#define APPS_SMMU_CLIENT_DEBUG_SSD_INDEX_HYP_HLOS_EN_LPASS_SEC                0x000103ff

#define APPS_SMMU_CLIENT_DEBUG_SSD_INDEX_HYP_HLOS_EN_ANOC_PCIE_SEC_ADDR       0x151f8010
#define APPS_SMMU_CLIENT_DEBUG_SSD_INDEX_HYP_HLOS_EN_ANOC_PCIE_SEC            0x000103ff

#define APPS_SMMU_CLIENT_DEBUG_SSD_INDEX_HYP_HLOS_EN_MNOC_SF_0_SEC_ADDR       0x151fc010
#define APPS_SMMU_CLIENT_DEBUG_SSD_INDEX_HYP_HLOS_EN_MNOC_SF_0_SEC            0x000103ff

#define APPS_SMMU_CLIENT_DEBUG_SID_HALT_MNOC_HF_1_SEC_ADDR                    0x151e8000
#define APPS_SMMU_CLIENT_DEBUG_SID_HALT_MNOC_HF_1_SEC                         0x80104000

#define APPS_SMMU_CLIENT_DEBUG_SID_HALT_MNOC_HF_0_SEC_ADDR                    0x151e4000
#define APPS_SMMU_CLIENT_DEBUG_SID_HALT_MNOC_HF_0_SEC                         0x80104000

#define APPS_SMMU_CLIENT_DEBUG_SID_HALT_ANOC_1_SEC_ADDR                       0x151dc000
#define APPS_SMMU_CLIENT_DEBUG_SID_HALT_ANOC_1_SEC                            0x80100000

#define APPS_SMMU_CLIENT_DEBUG_SID_HALT_ANOC_2_SEC_ADDR                       0x151e0000
#define APPS_SMMU_CLIENT_DEBUG_SID_HALT_ANOC_2_SEC                            0x80100000

#define APPS_SMMU_CLIENT_DEBUG_SID_HALT_COMPUTE_DSP_1_SEC_ADDR                0x151ec000
#define APPS_SMMU_CLIENT_DEBUG_SID_HALT_COMPUTE_DSP_1_SEC                     0x80100000

#define APPS_SMMU_CLIENT_DEBUG_SID_HALT_COMPUTE_DSP_0_SEC_ADDR                0x151f0000
#define APPS_SMMU_CLIENT_DEBUG_SID_HALT_COMPUTE_DSP_0_SEC                     0x80100000

#define APPS_SMMU_CLIENT_DEBUG_SID_HALT_LPASS_SEC_ADDR                        0x151f4000
#define APPS_SMMU_CLIENT_DEBUG_SID_HALT_LPASS_SEC                             0x80100000

#define APPS_SMMU_CLIENT_DEBUG_SID_HALT_ANOC_PCIE_SEC_ADDR                    0x151f8000
#define APPS_SMMU_CLIENT_DEBUG_SID_HALT_ANOC_PCIE_SEC                         0x80100000

#define APPS_SMMU_CLIENT_DEBUG_SID_HALT_MNOC_SF_0_SEC_ADDR                    0x151fc000
#define APPS_SMMU_CLIENT_DEBUG_SID_HALT_MNOC_SF_0_SEC                         0x80100000

#define APPS_SMMU_CLIENT_DEBUG_WUSER_ANOC_1_SEC_ADDR                          0x151dc038
#define APPS_SMMU_CLIENT_DEBUG_WUSER_ANOC_1_SEC                               0x00000000

#define APPS_SMMU_CLIENT_DEBUG_WUSER_ANOC_2_SEC_ADDR                          0x151e0038
#define APPS_SMMU_CLIENT_DEBUG_WUSER_ANOC_2_SEC                               0x00000000

#define APPS_SMMU_CLIENT_DEBUG_WUSER_MNOC_HF_0_SEC_ADDR                       0x151e4038
#define APPS_SMMU_CLIENT_DEBUG_WUSER_MNOC_HF_0_SEC                            0x00000000

#define APPS_SMMU_CLIENT_DEBUG_WUSER_MNOC_HF_1_SEC_ADDR                       0x151e8038
#define APPS_SMMU_CLIENT_DEBUG_WUSER_MNOC_HF_1_SEC                            0x00000000

#define APPS_SMMU_CLIENT_DEBUG_WUSER_COMPUTE_DSP_1_SEC_ADDR                   0x151ec038
#define APPS_SMMU_CLIENT_DEBUG_WUSER_COMPUTE_DSP_1_SEC                        0x00000000

#define APPS_SMMU_CLIENT_DEBUG_WUSER_COMPUTE_DSP_0_SEC_ADDR                   0x151f0038
#define APPS_SMMU_CLIENT_DEBUG_WUSER_COMPUTE_DSP_0_SEC                        0x00000000

#define APPS_SMMU_CLIENT_DEBUG_WUSER_LPASS_SEC_ADDR                           0x151f4038
#define APPS_SMMU_CLIENT_DEBUG_WUSER_LPASS_SEC                                0x00000000

#define APPS_SMMU_CLIENT_DEBUG_WUSER_ANOC_PCIE_SEC_ADDR                       0x151f8038
#define APPS_SMMU_CLIENT_DEBUG_WUSER_ANOC_PCIE_SEC                            0x00000000

#define APPS_SMMU_CLIENT_DEBUG_WUSER_MNOC_SF_0_SEC_ADDR                       0x151fc038
#define APPS_SMMU_CLIENT_DEBUG_WUSER_MNOC_SF_0_SEC                            0x00000000

#define GPU_GFX_SMMU_CUSTOM_CFG_ADDR                                          0x03da2300
#define GPU_GFX_SMMU_CUSTOM_CFG                                               0x20400053

#define GPU_GFX_SMMU_CUSTOM_CFG_SEC_ADDR                                      0x03da22fc
#define GPU_GFX_SMMU_CUSTOM_CFG_SEC                                           0x000007f0

#define GPU_GFX_SMMU_ACR_ADDR                                                 0x03da0010
#define GPU_GFX_SMMU_ACR                                                      0x0000001c

#define GPU_GFX_SMMU_NSACR_ADDR                                               0x03da0410
#define GPU_GFX_SMMU_NSACR                                                    0x0000001c

#define GPU_GFX_SMMU_SPARE_REG_1_SEC_ADDR                                     0x03dd4030
#define GPU_GFX_SMMU_SPARE_REG_1_SEC                                          0x0000000f

#define GPU_GFX_SMMU_CLOCK_GATING_SEC_ADDR                                    0x03dd4010
#define GPU_GFX_SMMU_CLOCK_GATING_SEC                                         0x000000ff

#define GPU_GFX_SMMU_CLIENT_DEBUG_SSD_INDEX_HYP_HLOS_EN_GFX_0_SEC_ADDR        0x03dd8010
#define GPU_GFX_SMMU_CLIENT_DEBUG_SSD_INDEX_HYP_HLOS_EN_GFX_0_SEC             0x000103ff

#define GPU_GFX_SMMU_CLIENT_DEBUG_SSD_INDEX_HYP_HLOS_EN_GFX_1_SEC_ADDR        0x03ddc010
#define GPU_GFX_SMMU_CLIENT_DEBUG_SSD_INDEX_HYP_HLOS_EN_GFX_1_SEC             0x000103ff

#define GPU_GFX_SMMU_CLIENT_DEBUG_SID_HALT_GFX_0_SEC_ADDR                     0x03dd8000
#define GPU_GFX_SMMU_CLIENT_DEBUG_SID_HALT_GFX_0_SEC                          0x80182000

#define GPU_GFX_SMMU_CLIENT_DEBUG_SID_HALT_GFX_1_SEC_ADDR                     0x03ddc000
#define GPU_GFX_SMMU_CLIENT_DEBUG_SID_HALT_GFX_1_SEC                          0x80182000

#define GPU_GFX_SMMU_TLBIS_CTRL_ADDR                                          0x03da2634
#define GPU_GFX_SMMU_TLBIS_CTRL                                               0x00001000

#define GPU_GFX_SMMU_CLIENT_DEBUG_WUSER_GFX_0_SEC_ADDR                        0x03dd8038
#define GPU_GFX_SMMU_CLIENT_DEBUG_WUSER_GFX_0_SEC                             0x00000000

#define GPU_GFX_SMMU_CLIENT_DEBUG_WUSER_GFX_1_SEC_ADDR                        0x03ddc038
#define GPU_GFX_SMMU_CLIENT_DEBUG_WUSER_GFX_1_SEC                             0x00000000

#define SMMU_GPU_AHB_4K_APERTURE_S1CB0_HWIO_ADDR                              0x03D60000
#define SMMU_GPU_AHB_4K_APERTURE_S1CB1_HWIO_ADDR                              0x03D60004
#define SMMU_GPU_AHB_4K_APERTURE_CTL_HWIO_ADDR                                0x03D60008
#define SMMU_GPU_AHB_4K_APERTURE_S1CB6_HWIO_ADDR                              0x03D6000C
#define SMMU_GPU_AHB_4K_APERTURE_S1CB7_HWIO_ADDR                              0x03D60010

#define SMMU_GPU_AHB_4K_APERTURE_S1CB0_REG_ADDR                               0x03DB0000
#define SMMU_GPU_AHB_4K_APERTURE_S1CB1_REG_ADDR                               0x03DB1000
#define SMMU_GPU_AHB_4K_APERTURE_S1CB2_REG_ADDR                               0x03DB2000
#define GPU_REG_OFFSET_MASK                                                   0xFFFFF
#define SMMU_GPU_AHB_4K_APERTURE_CTL_REG_ADDR                                 0x2

extern smmu_cfg_t qti_smmu_cfg[];
extern const size_t qti_smmu_cfg_count;

#endif
