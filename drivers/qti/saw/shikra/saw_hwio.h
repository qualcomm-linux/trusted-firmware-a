/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * shikra SAW4 AVS register map. Only SAW4_AVS_CTL is needed: the other
 * SAW4 registers (VCTL, STS, LIMIT, DLY, PMIC_STS) belong exclusively to
 * the aging/voltage-ramp HAL functions, which are not ported.
 */

#ifndef SAW_HWIO_H
#define SAW_HWIO_H

#define HWIO_SAW4_AVS_CTL_ADDR(x)		((x) + 0x104U)

#define HWIO_SAW4_AVS_CTL_VLVL_STEP_DN_BMSK	0xff000000UL
#define HWIO_SAW4_AVS_CTL_VLVL_STEP_DN_SHFT	24U
#define HWIO_SAW4_AVS_CTL_VLVL_STEP_UP_BMSK	0xff0000UL
#define HWIO_SAW4_AVS_CTL_VLVL_STEP_UP_SHFT	16U
#define HWIO_SAW4_AVS_CTL_CPR_RAMP_EN_BMSK	0x8000U
#define HWIO_SAW4_AVS_CTL_CPR_RAMP_EN_SHFT	15U
#define HWIO_SAW4_AVS_CTL_VCTL_RAMP_EN_BMSK	0x4000U
#define HWIO_SAW4_AVS_CTL_VCTL_RAMP_EN_SHFT	14U
#define HWIO_SAW4_AVS_CTL_TMR_CLK_DIV_BMSK	0x1f00U
#define HWIO_SAW4_AVS_CTL_TMR_CLK_DIV_SHFT	8U
#define HWIO_SAW4_AVS_CTL_VLVL_WIDTH_BMSK	0x20U
#define HWIO_SAW4_AVS_CTL_VLVL_WIDTH_SHFT	5U
#define HWIO_SAW4_AVS_CTL_CTL_SEL_BMSK		0x10U
#define HWIO_SAW4_AVS_CTL_CTL_SEL_SHFT		4U
#define HWIO_SAW4_AVS_CTL_EN_BMSK		0x1U
#define HWIO_SAW4_AVS_CTL_EN_SHFT		0U

/* shikra has a single AVS rail (APC0). */
#define SAW_AVS_BASE_ADDR			0x0F900800U

#endif /* SAW_HWIO_H */
