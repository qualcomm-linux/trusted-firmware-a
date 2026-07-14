/*
 * Copyright (c) 2026 Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef QTI_SIP_SVC_H
#define QTI_SIP_SVC_H

#include <stdbool.h>
#include <stdint.h>

#include <arch.h>
#include <lib/utils_def.h>

/*
 * QTI SiP calls common to Hoya and Wildcat.
 */
#define QTI_SIP_SVC_CALL_COUNT_ID			U(0x0200ff00)
#define QTI_SIP_SVC_UID_ID				U(0x0200ff01)
#define QTI_SIP_SVC_VERSION_ID				U(0x0200ff03)
#define QTI_SIP_SVC_AVAILABLE_ID			U(0x02000601)
#define QTI_SIP_SVC_SECURE_IO_READ_ID			U(0x02000501)
#define QTI_SIP_SVC_SECURE_IO_WRITE_ID			U(0x02000502)

/*
 * QTI SiP call supported by Hoya. Wildcat currently returns
 * QTI_SIP_NOT_SUPPORTED.
 */
#define QTI_SIP_SVC_MEM_ASSIGN_ID			U(0x02000C16)

/*
 * QTI SiP calls available only on Wildcat.
 */
#define QTI_SIP_SVC_CONFIG_HW_FOR_OFFLINE_RAM_DUMP	U(0x02000109)
#define QTI_SIP_SVC_CONFIG_CPU_ERRATA_ID		U(0x02000112)
#define QTI_SIP_SVC_IS_ARM_FEATURE_ALLOWED_FOR_NS	U(0x0200060B)
#define QTI_SIP_SVC_GET_SUBSYSTEM_DEBUG_OPTIONS_ID	U(0x02001402)

/* Parameters for QTI SiP calls common to Hoya and Wildcat. */
#define QTI_SIP_SVC_AVAILABLE_ID_PARAM_ID		U(0x1)
#define QTI_SIP_SVC_SECURE_IO_READ_PARAM_ID		U(0x1)
#define QTI_SIP_SVC_SECURE_IO_WRITE_PARAM_ID		U(0x2)

/* Parameter for the Hoya-only MEM_ASSIGN call. */
#define QTI_SIP_SVC_MEM_ASSIGN_PARAM_ID		U(0x1117)

/* Parameter for the Wildcat-only CPU errata call. */
#define QTI_SIP_SVC_CONFIG_CPU_ERRATA_PARAM_ID	U(0x1)

enum qti_sip_return_code {
	QTI_SIP_SUCCESS = 0,
	QTI_SIP_NOT_SUPPORTED = -1,
	QTI_SIP_PREEMPTED = -2,
	QTI_SIP_INVALID_PARAM = -3,
	QTI_SIP_CALL_FAILED = -4,
};

u_register_t qti_sip_call_count(void);
bool qti_sip_is_available(u_register_t smc_fid, bool caller_secure);

/*
 * Return the exact value to report in SMC x0. For reads, value is valid only
 * when QTI_SIP_SUCCESS is returned.
 */
int qti_sip_secure_io_read(u_register_t addr, u_register_t *value);
int qti_sip_secure_io_write(u_register_t addr, u_register_t value);

int qti_sip_config_cpu_errata(u_register_t x1);
int qti_sip_get_arm_feature_status(u_register_t *status);
int qti_sip_config_hw_for_offline_ram_dump(u_register_t x2,
					    u_register_t x3);
int qti_sip_get_subsystem_debug_options(u_register_t x1,
					u_register_t *response0,
					u_register_t *response1,
					u_register_t *response2);

/*
 * Return the SMC x0 status. For every result other than NOT_SUPPORTED,
 * operation_ret is returned in SMC x1.
 */
int qti_sip_mem_assign(void *handle, uint32_t smc_cc,
		       u_register_t x1, u_register_t x2,
		       u_register_t x3, u_register_t x4,
		       int *operation_ret);

#endif /* QTI_SIP_SVC_H */
