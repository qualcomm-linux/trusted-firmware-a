/*
 * Copyright (c) 2026 Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <arch_features.h>
#include <arch_helpers.h>
#include <lib/utils_def.h>

#include <bl31qtilib_interface.h>
#include <qti_sip_svc.h>

#ifndef QTI_SIP_SVC_HWTRACE_CALL_COUNT
#define QTI_SIP_SVC_HWTRACE_CALL_COUNT			U(0x0)
#endif

#define QTI_SIP_SVC_CALL_COUNT				U(0x10)

enum arm_feature_status_shift {
	MTE_ENABLED_STATUS_SHIFT = 0x0,
	SVE_ENABLED_STATUS_SHIFT = 0x1,
	SELF_HOSTED_TRACE_ENABLED_STATUS_SHIFT = 0x2,
	DEBUG_ACCESS_ENABLED_STATUS_SHIFT = 0x3,
	PERF_MONITOR_ENABLED_STATUS_SHIFT = 0x4,
	ACTIVITY_MONITOR_ENABLED_STATUS_SHIFT = 0x5,
	SW_CTX_NUMBER_ENABLED_STATUS_SHIFT = 0x6,
	PAUTH_API_ACCESS_ENABLED_STATUS_SHIFT = 0x7,
	MPAM_ENABLED_STATUS_SHIFT = 0x8,
	FGT_ENABLED_STATUS_SHIFT = 0x9,
	HCX_ENABLED_STATUS_SHIFT = 0xA,
	SME_ENABLED_STATUS_SHIFT = 0xB,
	SME_FA64_ENABLED_STATUS_SHIFT = 0xC,
	SVE2_ENABLED_STATUS_SHIFT = 0xD,
	ECV_ENABLED_STATUS_SHIFT = 0xE,
};

#define STATUS_BIT(cond, bit_shift)	\
	((uint32_t)(((cond) ? 0x1 : 0x0) << (bit_shift)))

u_register_t qti_sip_call_count(void)
{
	return QTI_SIP_SVC_CALL_COUNT +
	       QTI_SIP_SVC_HWTRACE_CALL_COUNT;
}

bool qti_sip_is_available(u_register_t smc_fid, bool caller_secure)
{
	(void)caller_secure;

	switch (smc_fid) {
	case QTI_SIP_SVC_CALL_COUNT_ID:
	case QTI_SIP_SVC_UID_ID:
	case QTI_SIP_SVC_VERSION_ID:
	case QTI_SIP_SVC_AVAILABLE_ID:
	case QTI_SIP_SVC_CONFIG_CPU_ERRATA_ID:
	case QTI_SIP_SVC_IS_ARM_FEATURE_ALLOWED_FOR_NS:
	case QTI_SIP_SVC_CONFIG_HW_FOR_OFFLINE_RAM_DUMP:
	case QTI_SIP_SVC_SECURE_IO_READ_ID:
	case QTI_SIP_SVC_SECURE_IO_WRITE_ID:
	case QTI_SIP_SVC_GET_SUBSYSTEM_DEBUG_OPTIONS_ID:
		return true;
	default:
		return false;
	}
}

int qti_sip_secure_io_read(u_register_t addr, u_register_t *value)
{
	struct smc_rsp_s response = { 0 };
	int ret;

	ret = bl31qtilib_secure_io_read((uintptr_t)addr, &response);
	if (ret != 0) {
		return QTI_SIP_CALL_FAILED;
	}

	*value = response.rsp[0];
	return QTI_SIP_SUCCESS;
}

int qti_sip_secure_io_write(u_register_t addr, u_register_t value)
{
	/* Preserve the existing write return-value ABI. */
	return bl31qtilib_secure_io_write((uintptr_t)addr, (uint32_t)value);
}

int qti_sip_config_cpu_errata(u_register_t x1)
{
	if (x1 == QTI_SIP_SVC_CONFIG_CPU_ERRATA_PARAM_ID) {
		return QTI_SIP_SUCCESS;
	}

	return QTI_SIP_INVALID_PARAM;
}

int qti_sip_get_arm_feature_status(u_register_t *status)
{
	uint32_t arm_feature_status = 0U;

	arm_feature_status |= STATUS_BIT(false, MTE_ENABLED_STATUS_SHIFT);
	arm_feature_status |= STATUS_BIT(is_feat_sve_supported(),
					 SVE_ENABLED_STATUS_SHIFT);
	arm_feature_status |= STATUS_BIT((read_mdcr_el3() & MDCR_TTRF_BIT) == 0U,
					 SELF_HOSTED_TRACE_ENABLED_STATUS_SHIFT);
	arm_feature_status |= STATUS_BIT((read_mdcr_el3() & MDCR_TDA_BIT) == 0U,
					 DEBUG_ACCESS_ENABLED_STATUS_SHIFT);
	arm_feature_status |= STATUS_BIT((read_mdcr_el3() & MDCR_TPM_BIT) == 0U,
					 PERF_MONITOR_ENABLED_STATUS_SHIFT);
	arm_feature_status |= STATUS_BIT((read_cptr_el3() & TAM_BIT) == 0U,
					 ACTIVITY_MONITOR_ENABLED_STATUS_SHIFT);
	arm_feature_status |= STATUS_BIT((read_scr_el3() & SCR_EnSCXT_BIT) ==
					 SCR_EnSCXT_BIT,
					 SW_CTX_NUMBER_ENABLED_STATUS_SHIFT);
	arm_feature_status |= STATUS_BIT((read_scr_el3() & SCR_API_BIT) ==
					 SCR_API_BIT,
					 PAUTH_API_ACCESS_ENABLED_STATUS_SHIFT);
	arm_feature_status |= STATUS_BIT(is_feat_mpam_supported(),
					 MPAM_ENABLED_STATUS_SHIFT);
	arm_feature_status |= STATUS_BIT(is_feat_fgt_supported(),
					 FGT_ENABLED_STATUS_SHIFT);
	arm_feature_status |= STATUS_BIT(is_feat_hcx_supported(),
					 HCX_ENABLED_STATUS_SHIFT);
	arm_feature_status |= STATUS_BIT(is_feat_sme_supported(),
					 SME_ENABLED_STATUS_SHIFT);
	arm_feature_status |= STATUS_BIT(is_feat_sme_fa64_present(),
					 SME_FA64_ENABLED_STATUS_SHIFT);
	arm_feature_status |= STATUS_BIT(false, SVE2_ENABLED_STATUS_SHIFT);
	arm_feature_status |= STATUS_BIT(is_feat_ecv_supported(),
					 ECV_ENABLED_STATUS_SHIFT);

	*status = arm_feature_status;
	return QTI_SIP_SUCCESS;
}

int qti_sip_config_hw_for_offline_ram_dump(u_register_t x2,
					    u_register_t x3)
{
	int ret;

	ret = bl31qtilib_config_hw_for_offline_ram_dump((uint32_t)x2,
							(uint32_t)x3);
	return ret == 0 ? QTI_SIP_SUCCESS : QTI_SIP_CALL_FAILED;
}

int qti_sip_get_subsystem_debug_options(u_register_t x1,
					u_register_t *response0,
					u_register_t *response1,
					u_register_t *response2)
{
	struct smc_rsp_s response = { 0 };
	int ret;

	ret = bl31qtilib_get_subsystem_debug_options((uint32_t)x1, &response);
	if (ret != 0) {
		return QTI_SIP_CALL_FAILED;
	}

	*response0 = response.rsp[0];
	*response1 = response.rsp[1];
	*response2 = response.rsp[2];
	return QTI_SIP_SUCCESS;
}

int qti_sip_mem_assign(void *handle, uint32_t smc_cc,
		       u_register_t x1, u_register_t x2,
		       u_register_t x3, u_register_t x4,
		       int *operation_ret)
{
	(void)handle;
	(void)smc_cc;
	(void)x1;
	(void)x2;
	(void)x3;
	(void)x4;

	*operation_ret = QTI_SIP_NOT_SUPPORTED;
	return QTI_SIP_NOT_SUPPORTED;
}
