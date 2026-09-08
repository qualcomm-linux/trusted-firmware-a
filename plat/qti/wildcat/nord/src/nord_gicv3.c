/*
 * Copyright (c) 2015-2024, ARM Limited and Contributors. All rights reserved.
 * Copyright (c) 2018-2024, The Linux Foundation. All rights reserved.
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <lib/utils_def.h>

#include <drivers/arm/gicv3.h>

#include <nord_ints.h>
#include <qti_plat.h>

static const interrupt_prop_t qti_interrupt_props[] = {
	INTR_PROP_DESC(NORD_INT_ID_CPU_WAKEUP_SGI,
		       GIC_HIGHEST_SEC_PRIORITY, INTR_GROUP0,
		       GIC_INTR_CFG_EDGE),
	INTR_PROP_DESC(NORD_INT_ID_RESET_SGI, GIC_HIGHEST_SEC_PRIORITY,
		       INTR_GROUP0,
		       GIC_INTR_CFG_EDGE),
	INTR_PROP_DESC(NORD_INT_ID_SEC_WDOG_BARK, GIC_HIGHEST_SEC_PRIORITY,
		       INTR_GROUP0,
		       GIC_INTR_CFG_EDGE),
	INTR_PROP_DESC(NORD_INT_ID_NON_SEC_WDOG_BITE,
		       GIC_HIGHEST_SEC_PRIORITY, INTR_GROUP0,
		       GIC_INTR_CFG_LEVEL),
	INTR_PROP_DESC(NORD_INT_ID_VMIDMT_ERR_CLT_SEC,
		       GIC_HIGHEST_SEC_PRIORITY, INTR_GROUP0,
		       GIC_INTR_CFG_EDGE),
	INTR_PROP_DESC(NORD_INT_ID_XPU_SEC, GIC_HIGHEST_SEC_PRIORITY,
		       INTR_GROUP0,
		       GIC_INTR_CFG_EDGE),
};

const interrupt_prop_t *plat_qti_get_interrupt_props(unsigned int *num_props)
{
	*num_props = ARRAY_SIZE(qti_interrupt_props);

	return qti_interrupt_props;
}
