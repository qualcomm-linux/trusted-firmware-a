/*
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>

#include <arch_helpers.h>
#include <common/debug.h>
#include <drivers/arm/css/scmi.h>

#include "scmi_private.h"

/*
 * SCMI Reset Domain Management protocol message and response lengths.
 * Calculated as sum of header (4 bytes) + payload bytes.
 *
 * RESET_DOMAIN_ATTRIBUTES request:  domain_id (4) = 4
 * RESET_DOMAIN_ATTRIBUTES response: status (4) + attributes (4) +
 *                                   latency (4) + name (16) = 28
 *
 * RESET_REQUEST payload:  domain_id (4) + flags (4) + reset_state (4) = 12
 * RESET_REQUEST response: status (4)
 */
#define SCMI_RESET_DOMAIN_ATTR_MSG_LEN		8U	/* 4 hdr + 4 domain_id */
#define SCMI_RESET_DOMAIN_ATTR_RESP_LEN		32U	/* 4 hdr + 28 payload  */

#define SCMI_RESET_DOMAIN_RESET_MSG_LEN		16U	/* 4 hdr + 12 payload */
#define SCMI_RESET_DOMAIN_RESET_RESP_LEN	8U	/* 4 hdr + 4 status  */

/*
 * Initialize the SCMI reset domain protocol.
 *
 * Queries the firmware for the protocol version and verifies it is compatible
 * with the driver's expected version (SCMI_RESET_DOMAIN_PROTO_VER).
 *
 * Returns 0 on success, -EPROTONOSUPPORT if the protocol is not supported or
 * the version is incompatible, or -EPROTO on other errors.
 */
int scmi_reset_domain_init(scmi_channel_t *ch)
{
	uint32_t version;
	int ret;

	ret = scmi_proto_version(ch, SCMI_RESET_DOMAIN_PROTO_ID, &version);
	if (ret != SCMI_E_SUCCESS) {
		WARN("SCMI reset domain protocol version message failed\n");

		return (ret == SCMI_E_NOT_SUPPORTED) ? -EPROTONOSUPPORT :
						       -EPROTO;
	}

	if (!is_scmi_version_compatible(SCMI_RESET_DOMAIN_PROTO_VER, version)) {
		WARN("SCMI reset domain protocol version 0x%x incompatible with driver version 0x%x\n",
		     version, SCMI_RESET_DOMAIN_PROTO_VER);

		return -EPROTONOSUPPORT;
	}

	VERBOSE("SCMI reset domain protocol version 0x%x detected\n", version);

	return 0;
}

/*
 * API to get the SCMI reset domain attributes
 * (SCMI spec §4.7.2.2 RESET_DOMAIN_ATTRIBUTES command).
 *
 * @p:          Opaque pointer to the initialized SCMI channel (scmi_channel_t *)
 * @domain_id:  Identifier for the reset domain.
 * @attributes: Pointer to store the domain attributes bitmask:
 *              Bit[31]: async_support  - 1 = asynchronous reset supported
 *              Bit[30]: notify_support - 1 = reset notifications supported
 *              Bits[29:0]: reserved (SBZ)
 * @latency:    Pointer to store the worst-case reset latency in microseconds.
 *              0xFFFFFFFF indicates latency is unknown.
 *
 * Returns SCMI_E_SUCCESS (0) on success, or a negative SCMI error code.
 */
int scmi_reset_domain_attributes(void *p, uint32_t domain_id,
				 uint32_t *attributes, uint32_t *latency)
{
	mailbox_mem_t *mbx_mem;
	unsigned int token = 0;
	int ret;
	scmi_channel_t *ch = (scmi_channel_t *)p;

	validate_scmi_channel(ch);

	scmi_get_channel(ch);

	mbx_mem = (mailbox_mem_t *)(ch->info->scmi_mbx_mem);
	mbx_mem->msg_header = SCMI_MSG_CREATE(SCMI_RESET_DOMAIN_PROTO_ID,
					      SCMI_RESET_DOMAIN_ATTRIBUTES_MSG,
					      token);
	mbx_mem->len = SCMI_RESET_DOMAIN_ATTR_MSG_LEN;
	mbx_mem->flags = SCMI_FLAG_RESP_POLL;
	SCMI_PAYLOAD_ARG1(mbx_mem->payload, domain_id);

	scmi_send_sync_command(ch);

	/* Get the return values */
	SCMI_PAYLOAD_RET_VAL3(mbx_mem->payload, ret, *attributes, *latency);
	assert(mbx_mem->len == SCMI_RESET_DOMAIN_ATTR_RESP_LEN);
	assert(token == SCMI_MSG_GET_TOKEN(mbx_mem->msg_header));

	scmi_put_channel(ch);

	return ret;
}

/*
 * API to send an SCMI Reset Domain Management RESET_REQUEST command
 * (SCMI spec §4.7.2.3).
 *
 * @p:           Opaque pointer to the initialized SCMI channel (scmi_channel_t *)
 * @domain_id:   Identifier for the reset domain.
 *               For QTI CPUCP: bits[15:8] = cluster (AFF1), bits[7:0] = core (AFF0)
 * @flags:       Reset flags (SCMI_RESET_FLAG_SYNC / SCMI_RESET_FLAG_AUTONOMOUS etc.)
 * @reset_state: Reset state (SCMI_RESET_STATE_ARCH / SCMI_RESET_STATE_IMPL | value)
 *
 * Returns SCMI_E_SUCCESS (0) on success, or a negative SCMI error code.
 */
int scmi_reset_domain_request(void *p, uint32_t domain_id,
			      uint32_t flags, uint32_t reset_state)
{
	mailbox_mem_t *mbx_mem;
	unsigned int token = 0;
	int ret;
	scmi_channel_t *ch = (scmi_channel_t *)p;

	validate_scmi_channel(ch);

	scmi_get_channel(ch);

	mbx_mem = (mailbox_mem_t *)(ch->info->scmi_mbx_mem);
	mbx_mem->msg_header = SCMI_MSG_CREATE(SCMI_RESET_DOMAIN_PROTO_ID,
					      SCMI_RESET_DOMAIN_RESET_MSG, token);
	mbx_mem->len = SCMI_RESET_DOMAIN_RESET_MSG_LEN;
	mbx_mem->flags = SCMI_FLAG_RESP_POLL;
	SCMI_PAYLOAD_ARG3(mbx_mem->payload, domain_id, flags, reset_state);

	scmi_send_sync_command(ch);

	/* Get the return values */
	SCMI_PAYLOAD_RET_VAL1(mbx_mem->payload, ret);
	assert(mbx_mem->len == SCMI_RESET_DOMAIN_RESET_RESP_LEN);
	assert(token == SCMI_MSG_GET_TOKEN(mbx_mem->msg_header));

	scmi_put_channel(ch);

	return ret;
}