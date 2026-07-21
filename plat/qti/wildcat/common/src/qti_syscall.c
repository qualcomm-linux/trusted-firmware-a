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

#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <arch.h>
#include <arch_features.h>
#include <arch_helpers.h>
#include <common/debug.h>
#include <common/runtime_svc.h>
#include <lib/coreboot.h>
#include <lib/cassert.h>
#include <lib/utils_def.h>
#include <smccc_helpers.h>
#include <tools_share/uuid.h>

#include <bl31qtilib_interface.h>
#include <bl31qtilib_spd_agnostic.h>

#if SPMD_SPM_AT_SEL2
#include <lib/gpt_rme/gpt_rme.h>
#endif

#if ENABLE_SPMD_LP
#include <services/el3_spmd_logical_sp.h>
#endif

/*
 * Arm SiP Service Call for the SPM to leverage RME to protect a memory range.
 * Protected memory range is one whose PAS was made secure. Unprotect relates
 * to reverting a protect operation.
 */
#if SPMD_SPM_AT_SEL2
#define PLAT_PROTECT_MEM_SMC64		U(0xC2000101)
#define PLAT_UNPROTECT_MEM_SMC64	U(0xC2000102)
#endif

/*
 * SIP service - SMC function IDs for SiP Service queries
 */
#define QTI_SIP_SVC_CALL_COUNT_ID			U(0x0200ff00)
#define QTI_SIP_SVC_UID_ID				U(0x0200ff01)
/*							0x8200ff02 is reserved */
#define QTI_SIP_SVC_VERSION_ID				U(0x0200ff03)
#define QTI_SIP_SVC_AVAILABLE_ID			U(0x02000601)
#define QTI_SIP_SVC_AVAILABLE_ID_PARAM_ID		U(0x1)

#define QTI_SIP_SVC_CONFIG_HW_FOR_OFFLINE_RAM_DUMP	U(0x02000109)
#define QTI_SIP_SVC_CONFIG_HW_FOR_OFFLINE_RAM_DUMP_PARAM_ID	U(0x2)
/*
 * Syscall's to allow Non Secure world accessing peripheral/IO memory
 * those are secure/proteced BUT not required to be secure.
 */
#define QTI_SIP_SVC_SECURE_IO_READ_ID			U(0x02000501)
#define QTI_SIP_SVC_SECURE_IO_WRITE_ID			U(0x02000502)

#define QTI_SIP_SVC_SECURE_IO_READ_PARAM_ID		U(0x1)
#define QTI_SIP_SVC_SECURE_IO_WRITE_PARAM_ID		U(0x2)

#define QTI_SIP_SVC_CONFIG_CPU_ERRATA_ID		U(0x02000112)
#define QTI_SIP_SVC_CONFIG_CPU_ERRATA_ID_PARAM_ID	U(0x1)
#define QTI_SIP_SVC_IS_ARM_FEATURE_ALLOWED_FOR_NS	U(0x0200060B)

/*
 * TZ_OWNER_SIP / TZ_SVC_READ_DBG_POLICY (0x14) / func 0x02
 * x1 carries the dbgil_subsys_t subsystem ID; 0 defaults to DBGIL_SUBSYS_NONE.
 */
#define QTI_SIP_SVC_GET_SUBSYSTEM_DEBUG_OPTIONS_ID	U(0x02001402)

#if QTI_HWTRACE_SUPPORT
#define QTI_SIP_NCC_HWTRACE_SET_ATID			U(0x02001310)
#define QTI_SIP_NCC_HWTRACE_SET_ENABLED			U(0x02001311)
#define QTI_SIP_NCC_HWTRACE_SET_OPTIONS			U(0x02001312)
#define QTI_SIP_NCC_HWTRACE_GET_FEATURES		U(0x02001313)

#define QTI_SIP_SVC_HWTRACE_CALL_COUNT			U(0x4)
#endif

#ifndef QTI_SIP_SVC_HWTRACE_CALL_COUNT
#define QTI_SIP_SVC_HWTRACE_CALL_COUNT			U(0x0)
#endif

/* UPDATE QTI_SIP_SVC_CALL_COUNT when adding a new call */
/* QTI_SIP_SVC_HWTRACE_CALL_COUNT is added in qti_sip_handler */
#define QTI_SIP_SVC_CALL_COUNT				U(0x11)

#define QTI_SIP_SVC_FATAL_ERR_DUMP_ID			U(0x02000316)
#define QTI_SIP_SVC_FATAL_ERR_DUMP_PARAM_ID		U(0x3)

#define QTI_SIP_SVC_VERSION_MAJOR			U(0x0)
#define QTI_SIP_SVC_VERSION_MINOR			U(0x0)

#define FUNCID_OEN_NUM_MASK						\
	((FUNCID_OEN_MASK << FUNCID_OEN_SHIFT) |			\
	 (FUNCID_NUM_MASK << FUNCID_NUM_SHIFT))

enum {
	QTI_SIP_SUCCESS = 0,
	QTI_SIP_NOT_SUPPORTED = -1,
	QTI_SIP_PREEMPTED = -2,
	QTI_SIP_INVALID_PARAM = -3,
	QTI_SIP_CALL_FAILED = -4,
};

enum arm_feature_status_shift {
	MTE_ENABLED_STATUS_SHIFT = 0x0,
	SVE_ENABLED_STATUS_SHIFT = 0x1,
	SELF_HOSTED_TRACE_ENABLED_STATUS_SHIFT = 0x2,
	/* Debug access registers controlled by TDA Bit */
	DEBUG_ACCESS_ENABLED_STATUS_SHIFT = 0x3,
	/* Performance monitor registers controlled by TPM Bit */
	PERF_MONITOR_ENABLED_STATUS_SHIFT = 0x4,
	/* Activity monitor registers controlled by TAM Bit */
	ACTIVITY_MONITOR_ENABLED_STATUS_SHIFT = 0x5,
	SW_CTX_NUMBER_ENABLED_STATUS_SHIFT = 0x6,
	PAUTH_API_ACCESS_ENABLED_STATUS_SHIFT = 0x7,
	MPAM_ENABLED_STATUS_SHIFT = 0x8,
	/*
	 * This bit is set if EL2 has access to HAFGRTR_EL2, HDFGRTR_EL2,
	 * HDFGWTR_EL2, HFGRTR_EL2, HFGITR_EL2 and HFGWTR_EL2 registers.
	 */
	FGT_ENABLED_STATUS_SHIFT = 0x9,
	/* This bit is set if EL2 has access HCRX_EL2 register */
	HCX_ENABLED_STATUS_SHIFT = 0xA,
	/*
	 * This bit is set if lower ELs has access to CPTR_EL3.ESM(12) and
	 * SCR_EL3.EnTP2(41) register.
	 */
	SME_ENABLED_STATUS_SHIFT = 0xB,
	/*
	 * This bit is set if lower ELs has access to SMCR_EL3.FA64(31)
	 * register.
	 */
	SME_FA64_ENABLED_STATUS_SHIFT = 0xC,
	/* This bit is set if the PE supports SVE2 instructions */
	SVE2_ENABLED_STATUS_SHIFT = 0xD,
	/*
	 * This bit is set if EL2 has access to CNTPOFF_EL2 register. In
	 * SCR_EL3, ECVEn bit is enabled.
	 */
	ECV_ENABLED_STATUS_SHIFT = 0xE,
};

/* QTI SiP Service UUID */
DEFINE_SVC_UUID2(qti_sip_svc_uid, 0x43864748, 0x217f, 0x41ad, 0xaa, 0x5a,
		 0xba, 0xe7, 0x0f, 0xa5, 0x52, 0xaf);

#if ENABLE_FEAT_MTE2 && ENABLE_FEAT_MTE2_PLATFORM_CONTROL
bool plat_mte_enabled(void);
#endif

static bool qti_check_syscall_availability(u_register_t smc_fid)
{
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
#if QTI_HWTRACE_SUPPORT
	case QTI_SIP_NCC_HWTRACE_SET_ATID:
	case QTI_SIP_NCC_HWTRACE_SET_ENABLED:
	case QTI_SIP_NCC_HWTRACE_SET_OPTIONS:
	case QTI_SIP_NCC_HWTRACE_GET_FEATURES:
#endif
	case QTI_SIP_SVC_FATAL_ERR_DUMP_ID:
		return true;
	default:
		return false;
	}
}

/* macro to set the status bit */
#define STATUS_BIT(cond, bit_shift)	\
	((uint32_t)(((cond) ? 0x1 : 0x0) << (bit_shift)))

/*
 * Enum for arm feature status shift -
 * A mirror to this list is present at:
 * core/kernel/hypervisor-rm/hyp/hyp/platform/qhee/qhee.tc Note: If any
 * changes are made to this list, please inform the hypervisor team.
 */
static uint32_t qti_sip_get_arm_feature_status(void)
{
	uint32_t status = 0U;

#if ENABLE_FEAT_MTE2
	bool mte_enabled = is_feat_mte2_supported();

#if ENABLE_FEAT_MTE2_PLATFORM_CONTROL
	mte_enabled = mte_enabled && plat_mte_enabled();
#endif
	status |= STATUS_BIT(mte_enabled, MTE_ENABLED_STATUS_SHIFT);
#else
	status |= STATUS_BIT(false, MTE_ENABLED_STATUS_SHIFT);
#endif
	status |= STATUS_BIT(is_feat_sve_supported(), SVE_ENABLED_STATUS_SHIFT);
	/*
	 * if Trap trace filter control bit is set, then access to Trace Filter
	 * registers at EL2 and EL1 generate a trap exception to EL3. if TTRF
	 * bit is not set, non-secure access is allowed to Trace filter
	 * registers
	 */
	status |= STATUS_BIT((read_mdcr_el3() & MDCR_TTRF_BIT) == 0U,
			     SELF_HOSTED_TRACE_ENABLED_STATUS_SHIFT);
	/*
	 * Trap Debug Access bit is set then EL0, EL1, and EL2 accesses to the
	 * debug registers, other than the registers that can be trapped by
	 * MDCR_EL3.TDOSA, are trapped to EL3. if TDA bit is not set,
	 * non-secure access is allowed to Trace filter registers
	 */
	status |= STATUS_BIT((read_mdcr_el3() & MDCR_TDA_BIT) == 0U,
			     DEBUG_ACCESS_ENABLED_STATUS_SHIFT);
	/*
	 * Trap Performance Monitor bit is set then EL2, EL1, and EL0 System
	 * register accesses to all Performance Monitor registers are trapped
	 * to EL3, unless it is trapped by HDCR.TPM or MDCR_EL2.TPM. if TPM
	 * bit is not set, non-secure access is allowed to Performance Monitor
	 * registers
	 */
	status |= STATUS_BIT((read_mdcr_el3() & MDCR_TPM_BIT) == 0U,
			     PERF_MONITOR_ENABLED_STATUS_SHIFT);
	/*
	 * Trap Activity Monitor bit is set. Traps EL2, EL1 and EL0 accesses to
	 * all Activity Monitor registers to EL3 if TAM bit is not set,
	 * non-secure access is allowed to Activity Monitor registers
	 */
	status |= STATUS_BIT((read_cptr_el3() & TAM_BIT) == 0U,
			     ACTIVITY_MONITOR_ENABLED_STATUS_SHIFT);
	/*
	 * EnSCXT bit is set enable access to the SCXTNUM_EL2, SCXTNUM_EL1, and
	 * SCXTNUM_EL0 registers if not set, Accesses to the specified
	 * registers from EL1 and EL2 generate a Trap exception to EL3.
	 */
	status |= STATUS_BIT((read_scr_el3() & SCR_EnSCXT_BIT) ==
			     SCR_EnSCXT_BIT,
			     SW_CTX_NUMBER_ENABLED_STATUS_SHIFT);
	/*
	 * API bit is set, allows non-secure access to instructions related to
	 * pointer authentication if not set, use of any instruction related to
	 * pointer authentication in any Exception level except EL3 when the
	 * instructions are enabled are trapped to EL3 unless they are trapped
	 * to EL2 as a result of the HCR_EL2.API bit.
	 */
	status |= STATUS_BIT((read_scr_el3() & SCR_API_BIT) == SCR_API_BIT,
			     PAUTH_API_ACCESS_ENABLED_STATUS_SHIFT);
	status |= STATUS_BIT(is_feat_mpam_supported(), MPAM_ENABLED_STATUS_SHIFT);
	/*
	 * Checks if EL2 has access to HAFGRTR_EL2, HDFGRTR_EL2, HDFGWTR_EL2,
	 * HFGRTR_EL2, HFGITR_EL2 and HFGWTR_EL2 registers
	 */
	status |= STATUS_BIT(is_feat_fgt_supported(), FGT_ENABLED_STATUS_SHIFT);
	/* Checks if EL2 has access to the HCRX_EL2 register */
	status |= STATUS_BIT(is_feat_hcx_supported(), HCX_ENABLED_STATUS_SHIFT);
	/*
	 * Checks if lower ELs has access to CPTR_EL3.ESM(12) and
	 * SCR_EL3.EnTP2(41) register
	 */
	status |= STATUS_BIT(is_feat_sme_supported(), SME_ENABLED_STATUS_SHIFT);
	/* Checks if lower ELs has access to SMCR_EL3.FA64(31) */
	status |= STATUS_BIT(is_feat_sme_fa64_present(),
			     SME_FA64_ENABLED_STATUS_SHIFT);
	/* Checks if the PE supports SVE2 instructions */
	status |= STATUS_BIT(false, SVE2_ENABLED_STATUS_SHIFT);
	status |= STATUS_BIT(is_feat_ecv_supported(), ECV_ENABLED_STATUS_SHIFT);

	return status;
}

#if (defined(SPD_spmd) && SPMD_SPM_AT_SEL2 == 1)
static uint64_t plat_protect_memory(bool protect, bool secure_origin,
				    const uint64_t base, const size_t size,
				    void *handle)
{
	if (!is_feat_rme_supported()) {
		SMC_RET1(handle, SMC_UNK);
	}

	if (!secure_origin) {
		SMC_RET1(handle, SMC_UNK);
		/* Shall not be reached. */
	}

	if (((base % PAGE_SIZE_4KB) != 0U) &&
	    ((size % PAGE_SIZE_4KB) != 0U)) {
		VERBOSE("Base address must be aligned to 4k.\n");
		SMC_RET1(handle, SMC_INVALID_PARAM);
		/* Shall not be reached. */
	}

	if ((ULONG_MAX - base) < size) {
		VERBOSE("Base + Size results in overflow.\n");
		SMC_RET1(handle, SMC_INVALID_PARAM);
		/* Shall not be reached. */
	}

#if defined(AC_NOT_STUBBED) && AC_NOT_STUBBED /* NEEDSWORK */
	uint64_t ret = SMC_INVALID_PARAM;
	uint64_t last_updated = 0;
	uint64_t granule_count;

	for (uint64_t it = base; it < (base + size); it += PAGE_SIZE_4KB) {
		/*
		 * If protect is true, add memory to secure PAS.
		 * Else unprotect it, making part of non-secure PAS.
		 */
		granule_count = 1;

		if (protect) {
			ret = gpt_transition_pas(it, &granule_count,
						 GPT_GPI_SECURE,
						 SMC_FROM_SECURE);
		} else {
			ret = gpt_transition_pas(it, &granule_count,
						 GPT_GPI_NS, SMC_FROM_SECURE);
		}

		switch (ret) {
		case 0:
			last_updated = it;
			break;
		case -EINVAL:
			SMC_RET2(handle, SMC_INVALID_PARAM, last_updated);
			break; /* Shall not be reached. */
		case -EPERM:
			SMC_RET2(handle, SMC_DENIED, last_updated);
			break; /* Shall not be reached. */
		default:
			ERROR("Unexpected return\n");
			panic();
		}
	}
#endif /* defined(AC_NOT_STUBBED) && AC_NOT_STUBBED */

	SMC_RET1(handle, SMC_OK);
}
#endif /* defined(SPD_spmd) && SPMD_SPM_AT_SEL2 == 1 */

/*
 * This function handles QTI specific syscalls. Currently only SiP calls are
 * present. Both FAST & YIELD type call land here.
 */
#if defined(SPD_spmd) && \
	defined(ENABLE_SPMD_QTI_SVC) && ENABLE_SPMD_QTI_SVC == 1
extern uintptr_t spmd_qti_smc_handler(uint32_t, u_register_t, u_register_t,
				      u_register_t, u_register_t, void *,
				      void *, u_register_t);
#endif

static uintptr_t qti_sip_handler(uint32_t smc_fid, u_register_t x1,
				 u_register_t x2, u_register_t x3,
				 u_register_t x4, void *cookie, void *handle,
				 u_register_t flags)
{
	uint32_t l_smc_fid = smc_fid & FUNCID_OEN_NUM_MASK;
	bool forward_to_spd = false;
	bool secure_origin;

	if (GET_SMC_CC(smc_fid) == SMC_32) {
		x1 = (uint32_t)x1;
		x2 = (uint32_t)x2;
		x3 = (uint32_t)x3;
		x4 = (uint32_t)x4;
	}

	/* Determine which security state this SMC originated from. */
	secure_origin = is_caller_secure(flags);
	(void)secure_origin;

	switch (l_smc_fid) {
	case QTI_SIP_SVC_CALL_COUNT_ID:
		/*
		 * Return Supported QTI SVC call count - SPD forwarded calls
		 * not included
		 */
		SMC_RET1(handle, QTI_SIP_SVC_CALL_COUNT +
			 QTI_SIP_SVC_HWTRACE_CALL_COUNT);
	case QTI_SIP_SVC_UID_ID:
		/* Return UID to the caller */
		SMC_UUID_RET(handle, qti_sip_svc_uid);
	case QTI_SIP_SVC_VERSION_ID:
		/* Return the version of current implementation */
		SMC_RET2(handle, QTI_SIP_SVC_VERSION_MAJOR,
			 QTI_SIP_SVC_VERSION_MINOR);
	case QTI_SIP_SVC_IS_ARM_FEATURE_ALLOWED_FOR_NS:
		SMC_RET2(handle, QTI_SIP_SUCCESS,
			 qti_sip_get_arm_feature_status());
	case QTI_SIP_SVC_CONFIG_HW_FOR_OFFLINE_RAM_DUMP: {
		int ret = bl31qtilib_config_hw_for_offline_ram_dump(x2, x3);

		if (ret != 0) {
			SMC_RET1(handle, QTI_SIP_CALL_FAILED);
		} else {
			SMC_RET1(handle, QTI_SIP_SUCCESS);
		}
	}
	case QTI_SIP_SVC_SECURE_IO_READ_ID: {
		smc_rsp_t response = { 0 };
		int ret = bl31qtilib_secure_io_read(x2, &response);

		if (ret != 0) {
			SMC_RET1(handle, QTI_SIP_CALL_FAILED);
		} else {
			SMC_RET2(handle, QTI_SIP_SUCCESS, response.rsp[0]);
		}
	}
	case QTI_SIP_SVC_SECURE_IO_WRITE_ID:
		SMC_RET1(handle, bl31qtilib_secure_io_write(x2, x3));
	case QTI_SIP_SVC_GET_SUBSYSTEM_DEBUG_OPTIONS_ID: {
		smc_rsp_t response = { 0 };
		int ret = bl31qtilib_get_subsystem_debug_options(
			(uint32_t)x1, &response);

		if (ret != 0) {
			SMC_RET1(handle, QTI_SIP_CALL_FAILED);
		} else {
			SMC_RET4(handle, QTI_SIP_SUCCESS, response.rsp[0],
				 response.rsp[1], response.rsp[2]);
		}
	}
	case QTI_SIP_SVC_CONFIG_CPU_ERRATA_ID:
		if (x1 == QTI_SIP_SVC_CONFIG_CPU_ERRATA_ID_PARAM_ID) {
#ifdef QTI_ARCH_HAS_CPU_CONFIG_ERRATA
			/*
			 * If the CPU has Errata to configure, this function
			 * must be implemented
			 */
			qti_config_cpu_errata(x2);
#endif
			SMC_RET1(handle, QTI_SIP_SUCCESS);
		} else {
			SMC_RET1(handle, QTI_SIP_INVALID_PARAM);
		}
	case QTI_SIP_SVC_AVAILABLE_ID:
		if (x1 != QTI_SIP_SVC_AVAILABLE_ID_PARAM_ID) {
			SMC_RET1(handle, QTI_SIP_INVALID_PARAM);
		}
		if (qti_check_syscall_availability(x2)) {
			SMC_RET2(handle, QTI_SIP_SUCCESS, 1);
		} else {
			if (bl31qtilib_spd_is_available()) {
				/*
				 * If the SIP call is not handled here, forward
				 * to SPD
				 */
				forward_to_spd = true;
				break;
			}
#if ENABLE_SPMD_LP
			forward_to_spd = true;
			break;
#endif
			/* Syscall call is not available */
			SMC_RET2(handle, QTI_SIP_SUCCESS, 0);
		}
#if QTI_HWTRACE_SUPPORT
	case QTI_SIP_NCC_HWTRACE_SET_ATID: {
		int ret = bl31qtilib_ncc_hwtrace_set_atid(x1, x2);

		if (ret != 0) {
			ERROR("HWTRACE: Failed to set atid (ret=%d)\n", ret);
			SMC_RET1(handle, QTI_SIP_CALL_FAILED);
		}
		SMC_RET1(handle, QTI_SIP_SUCCESS);
	}
	case QTI_SIP_NCC_HWTRACE_SET_ENABLED: {
		int ret = bl31qtilib_ncc_hwtrace_set_enabled(x1, x2);

		if (ret != 0) {
			ERROR("HWTRACE: Failed to set enabled (ret=%d)\n", ret);
			SMC_RET1(handle, QTI_SIP_CALL_FAILED);
		}
		SMC_RET1(handle, QTI_SIP_SUCCESS);
	}
	case QTI_SIP_NCC_HWTRACE_SET_OPTIONS: {
		int ret = bl31qtilib_ncc_hwtrace_set_options(x1, x2);

		if (ret != 0) {
			ERROR("HWTRACE: Failed to set options (ret=%d)\n", ret);
			SMC_RET1(handle, QTI_SIP_CALL_FAILED);
		}
		SMC_RET1(handle, QTI_SIP_SUCCESS);
	}
	case QTI_SIP_NCC_HWTRACE_GET_FEATURES: {
		u_register_t features = 0;
		int ret = bl31qtilib_ncc_hwtrace_get_features(&features);

		if (ret != 0) {
			ERROR("HWTRACE: Failed to get features (ret=%d)\n", ret);
			SMC_RET2(handle, QTI_SIP_CALL_FAILED, 0);
		}
		SMC_RET2(handle, QTI_SIP_SUCCESS, features);
	}
#endif
#if (defined(SPD_spmd) && SPMD_SPM_AT_SEL2 == 1)
	case PLAT_PROTECT_MEM_SMC64:
		VERBOSE("Sip Call - Protect memory\n");
		return plat_protect_memory(true, secure_origin, x1, x2,
					   handle);
	case PLAT_UNPROTECT_MEM_SMC64:
		VERBOSE("Sip Call - Unprotect memory\n");
		return plat_protect_memory(false, secure_origin, x1, x2,
					   handle);
#endif /* defined(SPD_spmd) && SPMD_SPM_AT_SEL2 == 1 */
	case QTI_SIP_SVC_FATAL_ERR_DUMP_ID: {
		size_t out_bytes = 0;
		int ret;

		if (x1 != QTI_SIP_SVC_FATAL_ERR_DUMP_PARAM_ID) {
			SMC_RET1(handle, QTI_SIP_INVALID_PARAM);
		}

		if (!is_caller_non_secure(flags)) {
			SMC_RET1(handle, QTI_SIP_NOT_SUPPORTED);
		}

		/*
		 * We assume the SMC comes from HLOS with an HLOS VA that will
		 * be translated by TF-A.
		 */
		ret = bl31qtilib_fatal_err_dump((uintptr_t)x2, (size_t)x3,
						MODE_EL1, &out_bytes);
		if (ret != 0) {
			ERROR("Failed to retrieve fatal error logs (ret=%d)\n",
			      ret);
			SMC_RET1(handle, QTI_SIP_CALL_FAILED);
		}
		SMC_RET2(handle, QTI_SIP_SUCCESS, (u_register_t)out_bytes);
	}
	default:
		/* Allow the call to be forwarded to an SPD if present. */
		forward_to_spd = true;
		break;
	}

	if (forward_to_spd) {
		if (bl31qtilib_spd_is_available()) {
			return bl31qtilib_spd_smc_handler(smc_fid, x1, x2,
							  x3, x4, cookie,
							  handle, flags);
		}
#if ENABLE_SPMD_LP
		return plat_spmd_logical_sp_smc_handler(smc_fid, x1, x2,
							x3, x4, cookie,
							handle, flags);
#else
		SMC_RET1(handle, QTI_SIP_NOT_SUPPORTED);
#endif
	}

	return (uintptr_t)handle;
}

/* Define a runtime service descriptor for both fast & yield SiP calls */
DECLARE_RT_SVC(qti_sip_fast_svc, OEN_SIP_START, OEN_SIP_END, SMC_TYPE_FAST,
	       NULL, qti_sip_handler);

DECLARE_RT_SVC(qti_sip_yield_svc, OEN_SIP_START, OEN_SIP_END, SMC_TYPE_YIELD,
	       NULL, qti_sip_handler);

#if defined(SPD_spmd) && \
	defined(ENABLE_SPMD_QTI_SVC) && ENABLE_SPMD_QTI_SVC == 1
#define TZ_OWNER_EL3	53U
CASSERT((TZ_OWNER_EL3 >= OEN_TOS_START) && (TZ_OWNER_EL3 <= OEN_TOS_END),
	assert_tz_owner_el3_out_of_range);

/* Define a runtime service descriptor for both fast & yield TOS calls. */
DECLARE_RT_SVC(qti_tos_fast_svc, TZ_OWNER_EL3, TZ_OWNER_EL3, SMC_TYPE_FAST,
	       NULL, spmd_qti_smc_handler);

DECLARE_RT_SVC(qti_tos_yield_svc, TZ_OWNER_EL3, TZ_OWNER_EL3, SMC_TYPE_YIELD,
	       NULL, spmd_qti_smc_handler);
#endif /* SPD_spmd && ENABLE_SPMD_QTI_SVC */
