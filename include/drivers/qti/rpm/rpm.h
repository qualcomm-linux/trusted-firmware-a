/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef QTI_RPM_H
#define QTI_RPM_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

enum qti_rpm_set {
	QTI_RPM_ACTIVE_SET = 0,
	QTI_RPM_SLEEP_SET = 1,
	QTI_RPM_NUM_SETS,
};

enum qti_rpm_resource_type {
	QTI_RPM_TEST_REQ		= 0x74736574U, /* 'test' */
	QTI_RPM_CLOCK_0_REQ		= 0x306b6c63U, /* 'clk0' */
	QTI_RPM_CLOCK_1_REQ		= 0x316b6c63U, /* 'clk1' */
	QTI_RPM_CLOCK_2_REQ		= 0x326b6c63U, /* 'clk2' */
	QTI_RPM_BUS_SLAVE_REQ		= 0x766c7362U, /* 'bslv' */
	QTI_RPM_BUS_MASTER_REQ		= 0x73616d62U, /* 'bmas' */
	QTI_RPM_BUS_SPDM_CLK_REQ	= 0x63707362U, /* 'bspc' */
	QTI_RPM_SMPS_A_REQ		= 0x61706d73U, /* 'smpa' */
	QTI_RPM_LDO_A_REQ		= 0x616f646cU, /* 'ldoa' */
	QTI_RPM_NCP_A_REQ		= 0x6170636eU, /* 'ncpa' */
	QTI_RPM_VS_A_REQ		= 0x00617376U, /* 'vsa' */
	QTI_RPM_CLK_BUFFER_A_REQ	= 0x616b6c63U, /* 'clka' */
	QTI_RPM_SMPS_B_REQ		= 0x62706d73U, /* 'smpb' */
	QTI_RPM_LDO_B_REQ		= 0x626f646cU, /* 'ldob' */
	QTI_RPM_NCP_B_REQ		= 0x6270636eU, /* 'ncpb' */
	QTI_RPM_VS_B_REQ		= 0x00627376U, /* 'vsb' */
	QTI_RPM_CLK_BUFFER_B_REQ	= 0x626b6c63U, /* 'clkb' */
	QTI_RPM_OCMEM_POWER_REQ		= 0x706d636fU, /* 'ocmp' */
};

enum qti_rpm_service_type {
	QTI_RPM_REQUEST_SERVICE	= 0x00716572U, /* 'req\0' */
	QTI_RPM_SYSTEMDB_SERVICE	= 0x00626473U, /* 'sdb\0' */
};

typedef enum qti_rpm_set rpm_set_type;
typedef enum qti_rpm_resource_type rpm_resource_type;
typedef enum qti_rpm_service_type rpm_service_type;

#define RPM_ACTIVE_SET		QTI_RPM_ACTIVE_SET
#define RPM_SLEEP_SET		QTI_RPM_SLEEP_SET
#define RPM_NUM_SETS		QTI_RPM_NUM_SETS

#define RPM_TEST_REQ		QTI_RPM_TEST_REQ
#define RPM_CLOCK_0_REQ		QTI_RPM_CLOCK_0_REQ
#define RPM_CLOCK_1_REQ		QTI_RPM_CLOCK_1_REQ
#define RPM_CLOCK_2_REQ		QTI_RPM_CLOCK_2_REQ
#define RPM_BUS_SLAVE_REQ	QTI_RPM_BUS_SLAVE_REQ
#define RPM_BUS_MASTER_REQ	QTI_RPM_BUS_MASTER_REQ
#define RPM_BUS_SPDM_CLK_REQ	QTI_RPM_BUS_SPDM_CLK_REQ
#define RPM_SMPS_A_REQ		QTI_RPM_SMPS_A_REQ
#define RPM_LDO_A_REQ		QTI_RPM_LDO_A_REQ
#define RPM_NCP_A_REQ		QTI_RPM_NCP_A_REQ
#define RPM_VS_A_REQ		QTI_RPM_VS_A_REQ
#define RPM_CLK_BUFFER_A_REQ	QTI_RPM_CLK_BUFFER_A_REQ
#define RPM_SMPS_B_REQ		QTI_RPM_SMPS_B_REQ
#define RPM_LDO_B_REQ		QTI_RPM_LDO_B_REQ
#define RPM_NCP_B_REQ		QTI_RPM_NCP_B_REQ
#define RPM_VS_B_REQ		QTI_RPM_VS_B_REQ
#define RPM_CLK_BUFFER_B_REQ	QTI_RPM_CLK_BUFFER_B_REQ
#define RPM_OCMEM_POWER_REQ	QTI_RPM_OCMEM_POWER_REQ

#define RPM_REQUEST_SERVICE	QTI_RPM_REQUEST_SERVICE
#define RPM_SYSTEMDB_SERVICE	QTI_RPM_SYSTEMDB_SERVICE

struct qti_rpm_kvp {
	uint32_t	key;
	uint32_t	size;
	const void	*value;
};

void qti_rpm_init(void);
bool qti_rpm_is_up(void);
void rpmclient_init(void);
bool rpm_is_up(void);

uint32_t qti_rpm_post_request(enum qti_rpm_set set,
			      enum qti_rpm_resource_type resource,
			      uint32_t resource_id, const void *kvps,
			      size_t kvps_len);
uint32_t qti_rpm_post_kvp(enum qti_rpm_set set,
			  enum qti_rpm_resource_type resource,
			  uint32_t resource_id, const struct qti_rpm_kvp *kvp);
void qti_rpm_barrier(uint32_t msg_id);
void rpm_barrier(uint32_t msg_id);
void rpm_churn_queue(uint32_t msg_id);
void qti_rpm_sync_request(enum qti_rpm_set set,
			  enum qti_rpm_resource_type resource,
			  uint32_t resource_id, const void *kvps,
			  size_t kvps_len);
void qti_rpm_sync_kvp(enum qti_rpm_set set, enum qti_rpm_resource_type resource,
		      uint32_t resource_id, const struct qti_rpm_kvp *kvp);
void qti_rpm_mask_interrupt(bool mask);
void rpm_mask_interrupt(bool mask);

/*
 * Baseline-compatible helper used by older QTI code. The input is a single
 * {key, size, value} tuple and is sent synchronously.
 */
void ubsp_rpm_send(uint32_t resource_type, uint32_t id, uint32_t set,
		   uint8_t *kvps, uint32_t len);

#endif /* QTI_RPM_H */
