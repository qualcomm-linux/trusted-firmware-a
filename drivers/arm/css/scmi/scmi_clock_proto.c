/*
 * Copyright (c) 2026, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <arch_helpers.h>
#include <common/debug.h>
#include <drivers/arm/css/scmi.h>

#include "scmi_private.h"

/*
 * API to get the SCMI clock management protocol attributes.
 */
int scmi_clock_protocol_attributes(void *p, uint32_t *max_num_pending_async_rate_chgs,
					uint32_t *num_clocks)
{
	mailbox_mem_t *mbx_mem;
	unsigned int token = 0;
	int ret;
	uint32_t attributes;
	scmi_channel_t *ch = (scmi_channel_t *)p;

	validate_scmi_channel(ch);

	scmi_get_channel(ch);

	mbx_mem = (mailbox_mem_t *)(ch->info->scmi_mbx_mem);
	mbx_mem->msg_header = SCMI_MSG_CREATE(SCMI_CLOCK_PROTO_ID,
			SCMI_PROTO_ATTR_MSG, token);
	mbx_mem->len = SCMI_PROTO_ATTR_MSG_LEN;
	mbx_mem->flags = SCMI_FLAG_RESP_POLL;

	scmi_send_sync_command(ch);

	/* Get the return values */
	SCMI_PAYLOAD_RET_VAL2(mbx_mem->payload, ret, attributes);
	assert(mbx_mem->len == SCMI_PROTO_ATTR_RESP_LEN);
	assert(token == SCMI_MSG_GET_TOKEN(mbx_mem->msg_header));

	scmi_put_channel(ch);

	if (ret == SCMI_E_SUCCESS) {
		if (num_clocks != NULL) {
			*num_clocks = attributes & 0xFFFF;
		}

		if (max_num_pending_async_rate_chgs != NULL) {
			*max_num_pending_async_rate_chgs = (attributes >> 16) & 0xFF;
		}
	}

	return ret;
}

/*
 * API to get the attributes of a SCMI clock device.
 */
int scmi_clock_attributes(void *p, uint32_t clock_id, uint32_t *attributes,
					char *clock_name, uint32_t *clock_enable_delay)
{
	mailbox_mem_t *mbx_mem;
	unsigned int token = 0;
	int ret;
	scmi_channel_t *ch = (scmi_channel_t *)p;

	validate_scmi_channel(ch);

	scmi_get_channel(ch);

	mbx_mem = (mailbox_mem_t *)(ch->info->scmi_mbx_mem);
	mbx_mem->msg_header = SCMI_MSG_CREATE(SCMI_CLOCK_PROTO_ID,
			SCMI_CLOCK_ATTRIBUTES_MSG, token);
	mbx_mem->len = SCMI_CLOCK_ATTRIBUTES_MSG_LEN;
	mbx_mem->flags = SCMI_FLAG_RESP_POLL;
	SCMI_PAYLOAD_ARG1(mbx_mem->payload, clock_id);

	scmi_send_sync_command(ch);

	/* Get the return values */
	if (attributes != NULL) {
		SCMI_PAYLOAD_RET_VAL2(mbx_mem->payload, ret, *attributes);
	} else {
		SCMI_PAYLOAD_RET_VAL1(mbx_mem->payload, ret);
	}
	assert(mbx_mem->len == SCMI_CLOCK_ATTRIBUTES_RESP_LEN);
	assert(token == SCMI_MSG_GET_TOKEN(mbx_mem->msg_header));

	if (clock_name != NULL) {
		memcpy(clock_name, (void *)&mbx_mem->payload[2], 16);
	}

	if (clock_enable_delay != NULL) {
		*clock_enable_delay = mmio_read_32((uintptr_t)&mbx_mem->payload[6]);
	}

	scmi_put_channel(ch);

	return ret;
}

/*
 * API to discover the rates supported by a SCMI clock device.
 */
int scmi_clock_describe_rates(void *p, uint32_t clock_id, uint32_t rate_index,
					uint32_t *num_rates_flags, uint32_t *rate_array,
					size_t rate_array_size)
{
	mailbox_mem_t *mbx_mem;
	unsigned int token = 0;
	int ret;
	uint32_t num_rates;
	scmi_channel_t *ch = (scmi_channel_t *)p;

	validate_scmi_channel(ch);

	scmi_get_channel(ch);

	mbx_mem = (mailbox_mem_t *)(ch->info->scmi_mbx_mem);
	mbx_mem->msg_header = SCMI_MSG_CREATE(SCMI_CLOCK_PROTO_ID,
			SCMI_CLOCK_DESCRIBE_RATES_MSG, token);
	mbx_mem->len = SCMI_CLOCK_DESCRIBE_RATES_MSG_LEN;
	mbx_mem->flags = SCMI_FLAG_RESP_POLL;
	SCMI_PAYLOAD_ARG2(mbx_mem->payload, clock_id, rate_index);

	scmi_send_sync_command(ch);

	/* Get the return values */
	SCMI_PAYLOAD_RET_VAL2(mbx_mem->payload, ret, num_rates);
	assert(token == SCMI_MSG_GET_TOKEN(mbx_mem->msg_header));

	if (num_rates_flags != NULL) {
		*num_rates_flags = num_rates;
	}

	if (ret == SCMI_E_SUCCESS && rate_array != NULL) {
		unsigned int words = (num_rates & 0xFFF) * 2U;

		if (words > rate_array_size) {
			words = rate_array_size;
		}

		for (unsigned int i = 0U; i < words; i++) {
			rate_array[i] = mmio_read_32(
					(uintptr_t)&mbx_mem->payload[2 + i]);
		}
	}

	scmi_put_channel(ch);

	return ret;
}

/*
 * API to set the rate of a SCMI clock device.
 */
int scmi_clock_rate_set(void *p, uint32_t clock_id, uint32_t flags, uint64_t rate)
{
	mailbox_mem_t *mbx_mem;
	unsigned int token = 0;
	int ret;
	scmi_channel_t *ch = (scmi_channel_t *)p;

	validate_scmi_channel(ch);

	scmi_get_channel(ch);

	mbx_mem = (mailbox_mem_t *)(ch->info->scmi_mbx_mem);
	mbx_mem->msg_header = SCMI_MSG_CREATE(SCMI_CLOCK_PROTO_ID,
			SCMI_CLOCK_RATE_SET_MSG, token);
	mbx_mem->len = SCMI_CLOCK_RATE_SET_MSG_LEN;
	mbx_mem->flags = SCMI_FLAG_RESP_POLL;
	SCMI_PAYLOAD_ARG4(mbx_mem->payload, flags, clock_id,
					rate & 0xffffffff, rate >> 32);

	scmi_send_sync_command(ch);

	/* Get the return values */
	SCMI_PAYLOAD_RET_VAL1(mbx_mem->payload, ret);
	assert(mbx_mem->len == SCMI_CLOCK_RATE_SET_RESP_LEN);
	assert(token == SCMI_MSG_GET_TOKEN(mbx_mem->msg_header));

	scmi_put_channel(ch);

	return ret;
}

/*
 * API to get the current rate of a SCMI clock device.
 */
int scmi_clock_rate_get(void *p, uint32_t clock_id, uint64_t *rate)
{
	mailbox_mem_t *mbx_mem;
	unsigned int token = 0;
	int ret;
	uint32_t lo_rate, hi_rate;
	scmi_channel_t *ch = (scmi_channel_t *)p;

	validate_scmi_channel(ch);

	scmi_get_channel(ch);

	mbx_mem = (mailbox_mem_t *)(ch->info->scmi_mbx_mem);
	mbx_mem->msg_header = SCMI_MSG_CREATE(SCMI_CLOCK_PROTO_ID,
			SCMI_CLOCK_RATE_GET_MSG, token);
	mbx_mem->len = SCMI_CLOCK_RATE_GET_MSG_LEN;
	mbx_mem->flags = SCMI_FLAG_RESP_POLL;
	SCMI_PAYLOAD_ARG1(mbx_mem->payload, clock_id);

	scmi_send_sync_command(ch);

	/* Get the return values */
	SCMI_PAYLOAD_RET_VAL3(mbx_mem->payload, ret, lo_rate, hi_rate);
	*rate = lo_rate | (uint64_t)hi_rate << 32;
	assert(mbx_mem->len == SCMI_CLOCK_RATE_GET_RESP_LEN);
	assert(token == SCMI_MSG_GET_TOKEN(mbx_mem->msg_header));

	scmi_put_channel(ch);

	return ret;
}

/*
 * API to set the configuration of a SCMI clock device.
 */
int scmi_clock_config_set(void *p, uint32_t clock_id, uint32_t attributes,
					uint32_t extended_config_val)
{
	mailbox_mem_t *mbx_mem;
	unsigned int token = 0;
	int ret;
	scmi_channel_t *ch = (scmi_channel_t *)p;

	validate_scmi_channel(ch);

	scmi_get_channel(ch);

	mbx_mem = (mailbox_mem_t *)(ch->info->scmi_mbx_mem);
	mbx_mem->msg_header = SCMI_MSG_CREATE(SCMI_CLOCK_PROTO_ID,
			SCMI_CLOCK_CONFIG_SET_MSG, token);
	mbx_mem->len = SCMI_CLOCK_CONFIG_SET_MSG_LEN;
	mbx_mem->flags = SCMI_FLAG_RESP_POLL;
	SCMI_PAYLOAD_ARG3(mbx_mem->payload, clock_id, attributes,
					extended_config_val);

	scmi_send_sync_command(ch);

	/* Get the return values */
	SCMI_PAYLOAD_RET_VAL1(mbx_mem->payload, ret);
	assert(mbx_mem->len == SCMI_CLOCK_CONFIG_SET_RESP_LEN);
	assert(token == SCMI_MSG_GET_TOKEN(mbx_mem->msg_header));

	scmi_put_channel(ch);

	return ret;
}

/*
 * API to get the configuration of a SCMI clock device.
 */
int scmi_clock_config_get(void *p, uint32_t clock_id, uint32_t flags,
					uint32_t *attributes, uint32_t *config,
					uint32_t *extended_config_val)
{
	mailbox_mem_t *mbx_mem;
	unsigned int token = 0;
	int ret;
	uint32_t resp_attributes, resp_config, resp_extended_config_val;
	scmi_channel_t *ch = (scmi_channel_t *)p;

	validate_scmi_channel(ch);

	scmi_get_channel(ch);

	mbx_mem = (mailbox_mem_t *)(ch->info->scmi_mbx_mem);
	mbx_mem->msg_header = SCMI_MSG_CREATE(SCMI_CLOCK_PROTO_ID,
			SCMI_CLOCK_CONFIG_GET_MSG, token);
	mbx_mem->len = SCMI_CLOCK_CONFIG_GET_MSG_LEN;
	mbx_mem->flags = SCMI_FLAG_RESP_POLL;
	SCMI_PAYLOAD_ARG2(mbx_mem->payload, clock_id, flags);

	scmi_send_sync_command(ch);

	/* Get the return values */
	SCMI_PAYLOAD_RET_VAL4(mbx_mem->payload, ret, resp_attributes,
				resp_config, resp_extended_config_val);
	assert(mbx_mem->len == SCMI_CLOCK_CONFIG_GET_RESP_LEN);
	assert(token == SCMI_MSG_GET_TOKEN(mbx_mem->msg_header));

	scmi_put_channel(ch);

	if (attributes != NULL) {
		*attributes = resp_attributes;
	}

	if (config != NULL) {
		*config = resp_config;
	}

	if (extended_config_val != NULL) {
		*extended_config_val = resp_extended_config_val;
	}

	return ret;
}
