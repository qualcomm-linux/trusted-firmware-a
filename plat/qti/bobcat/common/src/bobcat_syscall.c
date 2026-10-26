/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 * Copyright (c) 2018-2020, The Linux Foundation. All rights reserved.
 * Changes from Qualcomm Technologies, Inc. are provided under the following license:
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <common/runtime_svc.h>
#include <lib/utils_def.h>
#include <smccc_helpers.h>
#include <tools_share/uuid.h>

#include <qti_plat.h>
#include <qti_secure_io_cfg.h>

/*
 * SIP service - SMC function IDs for SiP Service queries
 *
 */
#define	QTI_SIP_SVC_CALL_COUNT_ID			U(0x0200ff00)
#define	QTI_SIP_SVC_UID_ID				U(0x0200ff01)
/*							0x8200ff02 is reserved*/
#define	QTI_SIP_SVC_VERSION_ID				U(0x0200ff03)
#define QTI_SIP_SVC_AVAILABLE_ID			U(0x02000601)
/*
 * Syscall's to allow Non Secure world accessing peripheral/IO memory
 * those are secure/proteced BUT not required to be secure.
 */
#define	QTI_SIP_SVC_SECURE_IO_READ_ID		U(0x02000501)
#define	QTI_SIP_SVC_SECURE_IO_WRITE_ID		U(0x02000502)

#define	QTI_SIP_SVC_SECURE_IO_READ_PARAM_ID	U(0x1)
#define	QTI_SIP_SVC_SECURE_IO_WRITE_PARAM_ID	U(0x2)

#define	QTI_SIP_SVC_CALL_COUNT			U(0x3)
#define QTI_SIP_SVC_VERSION_MAJOR		U(0x0)
#define	QTI_SIP_SVC_VERSION_MINOR		U(0x0)

#define	FUNCID_OEN_NUM_MASK	((FUNCID_OEN_MASK << FUNCID_OEN_SHIFT)\
				|(FUNCID_NUM_MASK << FUNCID_NUM_SHIFT))

enum {
	QTI_SIP_SUCCESS = 0,
	QTI_SIP_NOT_SUPPORTED = -1,
	QTI_SIP_PREEMPTED = -2,
	QTI_SIP_INVALID_PARAM = -3,
};

/* QTI SiP Service UUID */
DEFINE_SVC_UUID2(qti_sip_svc_uid,
		 0x43864748, 0x217f, 0x41ad, 0xaa, 0x5a,
		 0xba, 0xe7, 0x0f, 0xa5, 0x52, 0xaf);

static bool qti_is_secure_io_access_allowed(u_register_t addr)
{
	size_t i;

	for (i = 0U; i < ARRAY_SIZE(qti_secure_io_allowed_regs); i++) {
		if ((uintptr_t) addr == qti_secure_io_allowed_regs[i]) {
			return true;
		}
	}

	return false;
}

static bool qti_check_syscall_availability(u_register_t smc_fid)
{
	switch (smc_fid) {
	case QTI_SIP_SVC_CALL_COUNT_ID:
	case QTI_SIP_SVC_UID_ID:
	case QTI_SIP_SVC_VERSION_ID:
	case QTI_SIP_SVC_AVAILABLE_ID:
		return true;
	case QTI_SIP_SVC_SECURE_IO_READ_ID:
	case QTI_SIP_SVC_SECURE_IO_WRITE_ID:
		return true;
	default:
		return false;
	}
}

/*
 * This function handles QTI specific syscalls. Currently only SiP calls are present.
 * Both FAST & YIELD type call land here.
 */
static uintptr_t qti_sip_handler(uint32_t smc_fid,
				 u_register_t x1,
				 u_register_t x2,
				 u_register_t x3,
				 u_register_t x4,
				 void *cookie, void *handle, u_register_t flags)
{
	uint32_t l_smc_fid = smc_fid & FUNCID_OEN_NUM_MASK;

	if (GET_SMC_CC(smc_fid) == SMC_32) {
		x1 = (uint32_t) x1;
		x2 = (uint32_t) x2;
		x3 = (uint32_t) x3;
		x4 = (uint32_t) x4;
	}

	switch (l_smc_fid) {
	case QTI_SIP_SVC_CALL_COUNT_ID:
		{
			SMC_RET1(handle, QTI_SIP_SVC_CALL_COUNT);
			break;
		}
	case QTI_SIP_SVC_UID_ID:
		{
			/* Return UID to the caller */
			SMC_UUID_RET(handle, qti_sip_svc_uid);
			break;
		}
	case QTI_SIP_SVC_VERSION_ID:
		{
			/* Return the version of current implementation */
			SMC_RET2(handle, QTI_SIP_SVC_VERSION_MAJOR,
				 QTI_SIP_SVC_VERSION_MINOR);
			break;
		}
	case QTI_SIP_SVC_AVAILABLE_ID:
		{
			if (x1 != 1) {
				SMC_RET1(handle, QTI_SIP_INVALID_PARAM);
			}
			if (qti_check_syscall_availability(x2) == true) {
				SMC_RET2(handle, QTI_SIP_SUCCESS, 1);
			} else {
				SMC_RET2(handle, QTI_SIP_SUCCESS, 0);
			}
			break;
		}
	case QTI_SIP_SVC_SECURE_IO_READ_ID:
		{
			if (is_caller_secure(flags)) {
				SMC_RET1(handle, QTI_SIP_NOT_SUPPORTED);
			}
			if ((x1 == QTI_SIP_SVC_SECURE_IO_READ_PARAM_ID) &&
			    qti_is_secure_io_access_allowed(x2)) {
				SMC_RET2(handle, QTI_SIP_SUCCESS,
					 *((volatile uint32_t *)x2));
			}
			SMC_RET1(handle, QTI_SIP_INVALID_PARAM);
			break;
		}
	case QTI_SIP_SVC_SECURE_IO_WRITE_ID:
		{
			if (is_caller_secure(flags)) {
				SMC_RET1(handle, QTI_SIP_NOT_SUPPORTED);
			}
			if ((x1 == QTI_SIP_SVC_SECURE_IO_WRITE_PARAM_ID) &&
			    qti_is_secure_io_access_allowed(x2)) {
				*((volatile uint32_t *)x2) = x3;
				SMC_RET1(handle, QTI_SIP_SUCCESS);
			}
			SMC_RET1(handle, QTI_SIP_INVALID_PARAM);
			break;
		}
	default:
		{
			SMC_RET1(handle, QTI_SIP_NOT_SUPPORTED);
		}
	}
	return (uintptr_t) handle;
}

/* Define a runtime service descriptor for both fast & yield SiP calls */
DECLARE_RT_SVC(qti_sip_fast_svc, OEN_SIP_START,
	       OEN_SIP_END, SMC_TYPE_FAST, NULL, qti_sip_handler);

DECLARE_RT_SVC(qti_sip_yield_svc, OEN_SIP_START,
	       OEN_SIP_END, SMC_TYPE_YIELD, NULL, qti_sip_handler);
