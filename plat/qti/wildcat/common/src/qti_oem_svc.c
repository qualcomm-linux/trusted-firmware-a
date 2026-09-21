/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 * Copyright (c) 2018-2020, The Linux Foundation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * Changes from Qualcomm Technologies, Inc. are provided under the following
 * license:
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <common/runtime_svc.h>
#include <smccc_helpers.h>

#include <bl31qtilib_interface.h>
#include <bl31qtilib_spd_agnostic.h>

#define FUNCID_OEN_NUM_MASK						\
	((FUNCID_OEN_MASK << FUNCID_OEN_SHIFT) |			\
	 (FUNCID_NUM_MASK << FUNCID_NUM_SHIFT))

#define TZ_PSCI_WARM_RESET	(U(0xC3000922) & FUNCID_OEN_NUM_MASK)

enum {
	QTI_OEM_SVC_SUCCESS = 0,
	QTI_OEM_SVC_NOT_SUPPORTED = -1,
	QTI_OEM_SVC_PREEMPTED = -2,
	QTI_OEM_SVC_INVALID_PARAM = -3,
};

/* OEM SVC handler */
static uintptr_t oem_svc_smc_handler(uint32_t smc_fid, u_register_t x1,
				     u_register_t x2, u_register_t x3,
				     u_register_t x4, void *cookie,
				     void *handle, u_register_t flags)
{
	bool forward_to_spd = false;
	uint32_t l_smc_fid = smc_fid & FUNCID_OEN_NUM_MASK;

	switch (l_smc_fid) {
	case TZ_PSCI_WARM_RESET:
		SMC_RET1(handle, bl31qtilib_psci_warm_reset());
	default:
		/* Allow the call to be forwarded to QTEE if using qteed */
		forward_to_spd = true;
		break;
	}

	if (forward_to_spd) {
		return bl31qtilib_spd_smc_handler(smc_fid, x1, x2, x3, x4,
						  cookie, handle, flags);
	}

	return (uintptr_t)handle;
}

/* Register OEM Service Calls as runtime service */
DECLARE_RT_SVC(oem_svc_smc_handler_fast, OEN_OEM_START, OEN_OEM_END,
	       SMC_TYPE_FAST, NULL, oem_svc_smc_handler);

DECLARE_RT_SVC(oem_svc_smc_handler_yield, OEN_OEM_START, OEN_OEM_END,
	       SMC_TYPE_YIELD, NULL, oem_svc_smc_handler);
