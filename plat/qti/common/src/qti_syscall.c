/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 * Copyright (c) 2018-2020, The Linux Foundation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <stdint.h>

#include <common/runtime_svc.h>
#include <lib/utils_def.h>
#include <smccc_helpers.h>
#include <tools_share/uuid.h>

#include <qti_sip_svc.h>

#define QTI_SIP_SVC_VERSION_MAJOR		U(0x0)
#define QTI_SIP_SVC_VERSION_MINOR		U(0x0)

#define FUNCID_OEN_NUM_MASK	((FUNCID_OEN_MASK << FUNCID_OEN_SHIFT)\
				|(FUNCID_NUM_MASK << FUNCID_NUM_SHIFT))

/* QTI SiP Service UUID */
DEFINE_SVC_UUID2(qti_sip_svc_uid,
		 0x43864748, 0x217f, 0x41ad, 0xaa, 0x5a,
		 0xba, 0xe7, 0x0f, 0xa5, 0x52, 0xaf);

/*
 * This function handles QTI specific syscalls. Currently only SiP calls are
 * present. Both FAST & YIELD type call land here.
 */
static uintptr_t qti_sip_handler(uint32_t smc_fid,
				 u_register_t x1,
				 u_register_t x2,
				 u_register_t x3,
				 u_register_t x4,
				 void *cookie, void *handle, u_register_t flags)
{
	uint32_t l_smc_fid = smc_fid & FUNCID_OEN_NUM_MASK;
	int ret;
	int operation_ret = QTI_SIP_NOT_SUPPORTED;
	u_register_t value = 0U;
	u_register_t response[3] = { 0U };

	if (GET_SMC_CC(smc_fid) == SMC_32) {
		x1 = (uint32_t)x1;
		x2 = (uint32_t)x2;
		x3 = (uint32_t)x3;
		x4 = (uint32_t)x4;
	}

	switch (l_smc_fid) {
	case QTI_SIP_SVC_CALL_COUNT_ID:
		SMC_RET1(handle, qti_sip_call_count());
		break;
	case QTI_SIP_SVC_UID_ID:
		SMC_UUID_RET(handle, qti_sip_svc_uid);
		break;
	case QTI_SIP_SVC_VERSION_ID:
		SMC_RET2(handle, QTI_SIP_SVC_VERSION_MAJOR,
			 QTI_SIP_SVC_VERSION_MINOR);
		break;
	case QTI_SIP_SVC_AVAILABLE_ID:
		if (x1 != QTI_SIP_SVC_AVAILABLE_ID_PARAM_ID) {
			SMC_RET1(handle, QTI_SIP_INVALID_PARAM);
		}
		SMC_RET2(handle, QTI_SIP_SUCCESS,
			 qti_sip_is_available(
				 x2, is_caller_secure(flags)) ? U(1) : U(0));
		break;
	case QTI_SIP_SVC_SECURE_IO_READ_ID:
		if (is_caller_secure(flags)) {
			SMC_RET1(handle, QTI_SIP_NOT_SUPPORTED);
		}
		if (x1 != QTI_SIP_SVC_SECURE_IO_READ_PARAM_ID) {
			SMC_RET1(handle, QTI_SIP_INVALID_PARAM);
		}
		ret = qti_sip_secure_io_read(x2, &value);
		if (ret != QTI_SIP_SUCCESS) {
			SMC_RET1(handle, ret);
		}
		SMC_RET2(handle, QTI_SIP_SUCCESS, value);
		break;
	case QTI_SIP_SVC_SECURE_IO_WRITE_ID:
		if (is_caller_secure(flags)) {
			SMC_RET1(handle, QTI_SIP_NOT_SUPPORTED);
		}
		if (x1 != QTI_SIP_SVC_SECURE_IO_WRITE_PARAM_ID) {
			SMC_RET1(handle, QTI_SIP_INVALID_PARAM);
		}
		SMC_RET1(handle, qti_sip_secure_io_write(x2, x3));
		break;
	case QTI_SIP_SVC_MEM_ASSIGN_ID:
		if (is_caller_secure(flags)) {
			SMC_RET1(handle, QTI_SIP_NOT_SUPPORTED);
		}
		ret = qti_sip_mem_assign(handle, GET_SMC_CC(smc_fid),
					 x1, x2, x3, x4, &operation_ret);
		if (ret == QTI_SIP_NOT_SUPPORTED) {
			SMC_RET1(handle, ret);
		}
		SMC_RET2(handle, ret, operation_ret);
		break;
	case QTI_SIP_SVC_CONFIG_CPU_ERRATA_ID:
		SMC_RET1(handle, qti_sip_config_cpu_errata(x1));
		break;
	case QTI_SIP_SVC_IS_ARM_FEATURE_ALLOWED_FOR_NS:
		ret = qti_sip_get_arm_feature_status(&value);
		if (ret != QTI_SIP_SUCCESS) {
			SMC_RET1(handle, ret);
		}
		SMC_RET2(handle, QTI_SIP_SUCCESS, value);
		break;
	case QTI_SIP_SVC_CONFIG_HW_FOR_OFFLINE_RAM_DUMP:
		SMC_RET1(handle,
			 qti_sip_config_hw_for_offline_ram_dump(x2, x3));
		break;
	case QTI_SIP_SVC_GET_SUBSYSTEM_DEBUG_OPTIONS_ID:
		ret = qti_sip_get_subsystem_debug_options(
			x1, &response[0], &response[1], &response[2]);
		if (ret != QTI_SIP_SUCCESS) {
			SMC_RET1(handle, ret);
		}
		SMC_RET4(handle, QTI_SIP_SUCCESS, response[0], response[1],
			 response[2]);
		break;
	default:
		SMC_RET1(handle, QTI_SIP_NOT_SUPPORTED);
	}

	return (uintptr_t)handle;
}

/* Define a runtime service descriptor for both fast & yield SiP calls */
DECLARE_RT_SVC(qti_sip_fast_svc, OEN_SIP_START,
	       OEN_SIP_END, SMC_TYPE_FAST, NULL, qti_sip_handler);

DECLARE_RT_SVC(qti_sip_yield_svc, OEN_SIP_START,
	       OEN_SIP_END, SMC_TYPE_YIELD, NULL, qti_sip_handler);
