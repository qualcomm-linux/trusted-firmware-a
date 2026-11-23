/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Shikra VPP feature-enable configuration.
 *
 * VPP_AGING_ON is kept at the vendor's value for device-info reporting, but
 * the aging-adjustment contribution itself (VPP_OLV_AGING_ADJ) is always 0
 * in this port: see drivers/qti/vpp/common/vpp_interpolation.c.
 */

#ifndef VPP_CFG_H
#define VPP_CFG_H

#define VPP_AGING_ON			1
#define VPP_AGING_MARGIN_ON		1
#define VPP_ACD_ON			1
#define VPP_AVG_ON			0
#define VPP_AUTO_APM			1

/* Set to 1 to enable run-time KV calculation from fused olv/quot points. */
#define CALCULATE_KV			1

/* Target PMIC step size in mV. */
#define VPP_PMIC_STEP_SIZE		4

/* APM threshold / intermediate voltage in mV. */
#define VPP_APM_THRESHOLD_VOLTAGE	752
#define VPP_APM_INTERMEDIATE_VOLTAGE	880

#endif /* VPP_CFG_H */
