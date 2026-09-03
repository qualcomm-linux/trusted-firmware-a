/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Lightweight legacy RPM request service client.
 *
 * Shikra TZ sends RPM requests over the "rpm_requests" GLINK channel to the
 * remote "rpm" subsystem. This driver keeps only the wire packet format and
 * synchronous request/barrier behavior needed by TF-A. The actual transport is
 * supplied by the platform through qti_rpm_transport_*().
 */

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <common/debug.h>
#include <drivers/qti/rpm/rpm.h>
#include <drivers/qti/rpm/rpm_transport.h>
#include <lib/spinlock.h>
#include <lib/utils_def.h>

#ifndef QTI_RPM_MAX_PACKET_BYTES
#define QTI_RPM_MAX_PACKET_BYTES	256U
#endif

#ifndef QTI_RPM_POLL_COUNT
#define QTI_RPM_POLL_COUNT		1000000U
#endif

#define QTI_RPM_HEADER_WORDS		7U
#define QTI_RPM_HEADER_BYTES		(QTI_RPM_HEADER_WORDS * sizeof(uint32_t))
#define QTI_RPM_SERVICE_IDX		0U
#define QTI_RPM_SIZE_IDX		1U
#define QTI_RPM_MSG_ID_IDX		2U
#define QTI_RPM_SET_IDX		3U
#define QTI_RPM_RESOURCE_TYPE_IDX	4U
#define QTI_RPM_RESOURCE_ID_IDX		5U
#define QTI_RPM_DATA_LEN_IDX		6U

#define QTI_RPM_MSG_KEY		0x2367736dU /* 'msg#' */
#define QTI_RPM_ERR_KEY		0x00727265U /* 'err\0' */

#define QTI_RPM_ALIGN4(_len)		(((_len) + 3U) & ~3U)

static uint8_t rpm_tx_buf[QTI_RPM_MAX_PACKET_BYTES];
static spinlock_t rpm_lock;
static bool rpm_initialized;
static bool rpm_transport_up;
static uint32_t rpm_next_msg_id = 1U;
static uint32_t rpm_completed_msg_id;

static uint32_t rpm_alloc_msg_id(void)
{
	uint32_t msg_id = rpm_next_msg_id++;

	if (msg_id == 0U) {
		msg_id = rpm_next_msg_id++;
	}

	return msg_id;
}

static bool rpm_msg_completed(uint32_t msg_id)
{
	return (int32_t)(rpm_completed_msg_id - msg_id) >= 0;
}

static bool rpm_kvp_get(const uint8_t *buf, size_t len, size_t *offset,
			uint32_t *key, uint32_t *value_len,
			const uint8_t **value)
{
	size_t aligned_len;

	if ((buf == NULL) || (offset == NULL) || (key == NULL) ||
	    (value_len == NULL) || (value == NULL)) {
		return false;
	}

	if ((*offset > len) || ((len - *offset) < (2U * sizeof(uint32_t)))) {
		return false;
	}

	memcpy(key, &buf[*offset], sizeof(*key));
	*offset += sizeof(*key);
	memcpy(value_len, &buf[*offset], sizeof(*value_len));
	*offset += sizeof(*value_len);

	aligned_len = QTI_RPM_ALIGN4(*value_len);
	if ((*offset > len) || ((len - *offset) < aligned_len)) {
		return false;
	}

	*value = (*value_len == 0U) ? NULL : &buf[*offset];
	*offset += aligned_len;

	return true;
}

static size_t rpm_kvp_put(uint8_t *buf, size_t len, uint32_t key,
			  const void *value, size_t value_len)
{
	uint32_t value_len32 = (uint32_t)value_len;
	size_t aligned_len = QTI_RPM_ALIGN4(value_len);
	size_t total_len = (2U * sizeof(uint32_t)) + aligned_len;

	assert(buf != NULL);

	if ((value_len > UINT32_MAX) || (total_len > len)) {
		ERROR("RPM: KVP is too large\n");
		panic();
	}

	memcpy(buf, &key, sizeof(key));
	memcpy(buf + sizeof(key), &value_len32, sizeof(value_len32));

	if (value_len != 0U) {
		if (value == NULL) {
			ERROR("RPM: KVP value is NULL\n");
			panic();
		}
		memcpy(buf + (2U * sizeof(uint32_t)), value, value_len);
	}

	if (aligned_len != value_len) {
		memset(buf + (2U * sizeof(uint32_t)) + value_len, 0,
		       aligned_len - value_len);
	}

	return total_len;
}

void qti_rpm_init(void)
{
	const struct qti_rpm_transport_config cfg = {
		.channel_name = "rpm_requests",
		.remote_ss = "rpm",
	};
	int ret;

	if (rpm_initialized) {
		return;
	}

	ret = qti_rpm_transport_init(&cfg);
	rpm_transport_up = (ret == 0);
	rpm_initialized = true;

	if (!rpm_transport_up) {
		WARN("RPM: transport backend is not available (%d)\n", ret);
	}
}

bool qti_rpm_is_up(void)
{
	return rpm_initialized && rpm_transport_up;
}

void rpmclient_init(void)
{
	qti_rpm_init();
}

bool rpm_is_up(void)
{
	return qti_rpm_is_up();
}

uint32_t qti_rpm_post_request(enum qti_rpm_set set,
			      enum qti_rpm_resource_type resource,
			      uint32_t resource_id, const void *kvps,
			      size_t kvps_len)
{
	uint32_t *header = (uint32_t *)rpm_tx_buf;
	uint32_t msg_id;
	size_t packet_len;
	int ret;

	assert(rpm_initialized);
	assert(set < QTI_RPM_NUM_SETS);
	assert((kvps_len == 0U) || (kvps != NULL));

	if ((set >= QTI_RPM_NUM_SETS) || (kvps_len > UINT32_MAX) ||
	    ((kvps_len != 0U) && (kvps == NULL))) {
		ERROR("RPM: invalid request\n");
		panic();
	}

	if (!rpm_transport_up) {
		ERROR("RPM: cannot send request without a transport backend\n");
		panic();
	}

	packet_len = QTI_RPM_HEADER_BYTES + kvps_len;
	if (packet_len > sizeof(rpm_tx_buf)) {
		ERROR("RPM: request is too large\n");
		panic();
	}

	spin_lock(&rpm_lock);

	msg_id = rpm_alloc_msg_id();

	header[QTI_RPM_SERVICE_IDX] = QTI_RPM_REQUEST_SERVICE;
	header[QTI_RPM_SIZE_IDX] = (uint32_t)(packet_len -
					      (2U * sizeof(uint32_t)));
	header[QTI_RPM_MSG_ID_IDX] = msg_id;
	header[QTI_RPM_SET_IDX] = (uint32_t)set;
	header[QTI_RPM_RESOURCE_TYPE_IDX] = (uint32_t)resource;
	header[QTI_RPM_RESOURCE_ID_IDX] = resource_id;
	header[QTI_RPM_DATA_LEN_IDX] = (uint32_t)kvps_len;

	if (kvps_len != 0U) {
		memcpy(&rpm_tx_buf[QTI_RPM_HEADER_BYTES], kvps, kvps_len);
	}

	ret = qti_rpm_transport_tx(rpm_tx_buf, packet_len);
	if (ret != 0) {
		spin_unlock(&rpm_lock);
		ERROR("RPM: transport send failed (%d)\n", ret);
		panic();
	}

	spin_unlock(&rpm_lock);

	return msg_id;
}

uint32_t qti_rpm_post_kvp(enum qti_rpm_set set,
			  enum qti_rpm_resource_type resource,
			  uint32_t resource_id, const struct qti_rpm_kvp *kvp)
{
	uint8_t kvp_buf[QTI_RPM_MAX_PACKET_BYTES - QTI_RPM_HEADER_BYTES];
	size_t kvp_len;

	assert(kvp != NULL);

	if (kvp == NULL) {
		ERROR("RPM: KVP request is NULL\n");
		panic();
	}

	kvp_len = rpm_kvp_put(kvp_buf, sizeof(kvp_buf), kvp->key, kvp->value,
			      kvp->size);

	return qti_rpm_post_request(set, resource, resource_id, kvp_buf,
				    kvp_len);
}

void qti_rpm_barrier(uint32_t msg_id)
{
	uint32_t poll;

	if (msg_id == 0U) {
		return;
	}

	assert(rpm_initialized);

	for (poll = 0U; poll < QTI_RPM_POLL_COUNT; poll++) {
		if (rpm_msg_completed(msg_id)) {
			return;
		}

		if (qti_rpm_transport_poll() != 0) {
			break;
		}
	}

	ERROR("RPM: request %u did not complete\n", msg_id);
	panic();
}

void rpm_barrier(uint32_t msg_id)
{
	qti_rpm_barrier(msg_id);
}

void rpm_churn_queue(uint32_t msg_id)
{
	qti_rpm_barrier(msg_id);
}

void qti_rpm_sync_request(enum qti_rpm_set set,
			  enum qti_rpm_resource_type resource,
			  uint32_t resource_id, const void *kvps,
			  size_t kvps_len)
{
	qti_rpm_barrier(qti_rpm_post_request(set, resource, resource_id, kvps,
					     kvps_len));
}

void qti_rpm_sync_kvp(enum qti_rpm_set set, enum qti_rpm_resource_type resource,
		      uint32_t resource_id, const struct qti_rpm_kvp *kvp)
{
	qti_rpm_barrier(qti_rpm_post_kvp(set, resource, resource_id, kvp));
}

void qti_rpm_mask_interrupt(bool mask)
{
	if (qti_rpm_transport_mask_rx_interrupt(mask) != 0) {
		WARN("RPM: transport interrupt mask request failed\n");
	}
}

void rpm_mask_interrupt(bool mask)
{
	qti_rpm_mask_interrupt(mask);
}

void qti_rpm_rx(const void *buf, size_t len)
{
	const uint8_t *bytes = buf;
	const uint8_t *service_payload = NULL;
	const uint8_t *value;
	size_t offset = 0U;
	size_t service_len = 0U;
	uint32_t key;
	uint32_t value_len;
	uint32_t msg_id;

	if ((buf == NULL) || (len == 0U)) {
		return;
	}

	if (!rpm_kvp_get(bytes, len, &offset, &key, &value_len, &value)) {
		WARN("RPM: dropping malformed RX packet\n");
		return;
	}

	if (key == QTI_RPM_REQUEST_SERVICE) {
		service_payload = value;
		service_len = value_len;
	} else {
		service_payload = bytes;
		service_len = len;
	}

	offset = 0U;
	while (rpm_kvp_get(service_payload, service_len, &offset, &key,
			   &value_len, &value)) {
		if ((key == QTI_RPM_MSG_KEY) &&
		    (value_len == sizeof(msg_id)) && (value != NULL)) {
			memcpy(&msg_id, value, sizeof(msg_id));
			rpm_completed_msg_id = msg_id;
		} else if (key == QTI_RPM_ERR_KEY) {
			ERROR("RPM: request error response\n");
		}
	}
}

struct ubsp_rpm_message {
	uint32_t key;
	uint32_t size;
	uint32_t value;
};

void ubsp_rpm_send(uint32_t resource_type, uint32_t id, uint32_t set,
		   uint8_t *kvps, uint32_t len)
{
	const struct ubsp_rpm_message *msg =
		(const struct ubsp_rpm_message *)kvps;
	struct qti_rpm_kvp kvp;

	assert(len >= sizeof(*msg));
	assert(set < QTI_RPM_NUM_SETS);

	if ((kvps == NULL) || (len < sizeof(*msg)) ||
	    (set >= QTI_RPM_NUM_SETS)) {
		ERROR("RPM: invalid UBSP request\n");
		panic();
	}

	kvp.key = msg->key;
	kvp.size = msg->size;
	kvp.value = &msg->value;

	qti_rpm_sync_kvp((enum qti_rpm_set)set,
			 (enum qti_rpm_resource_type)resource_type, id, &kvp);
}
