/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stddef.h>
#include <stdint.h>

#include <arch_helpers.h>
#include <assert.h>
#include <cdefs.h>
#include <lib/mmio.h>
#include <lib/spinlock.h>
#include <plat/common/platform.h>

#include <cpu_config.h>
#include <cpucp.h>
#include <cpucp_hwio.h>

/*
 * Lean CPUCP host driver for TF-A. Only the clock-domain enable path that is
 * required during secondary-core cold boot is implemented natively, using the
 * SCMI Clock Management protocol over the CPUCP shared-memory transport.
 */

/* SCMI message-type used for command requests. */
#define SCMI_MSG_TYPE_CMD			(0x0)

/* Initial SCMI packet length: the message-header word before the payload. */
#define SCMI_PKT_LEN_INIT			4U

/* Mailbox flag: do not request a completion interrupt. */
#define SCMI_MBOX_FLAG_INT_CMD_COMP_NOT_VIA_INT	(0)

/* Channel status */
#define CHAN_STAT_CHAN_FREE			(1 << 0)
#define CHAN_STAT_CHAN_BUSY			(0 << 0)

/* SCMI clock management protocol identifier (SCMI spec, Section 4.1.2). */
#define SCMI_CLOCK_MANAGEMENT_PROTOCOL		(0x14)

/* SCMI clock management protocol message ID (SCMI spec, Section 4.6.2). */
#define CLOCK_CONFIG_SET			(0x7)

/* CLOCK_CONFIG_SET attribute: enable the clock. */
#define SCMI_ATTR_CLK_CFG_SET_CLK_EN		(0x1)

/* Timeout for the SCMI channel handshake, in milliseconds. */
#define TIME_OUT_PERIOD_MS			500U

/* Error codes returned by cpucp_scmi_send(). */
#define CPUCP_SCMI_SEND_ERR_TX_NULL_PTR		(-1)
#define CPUCP_SCMI_SEND_ERR_TX_SIZE		(-2)
#define CPUCP_SCMI_SEND_ERR_RX_NULL_PTR		(-3)
#define CPUCP_SCMI_SEND_ERR_RX_SIZE		(-4)
#define CPUCP_SCMI_SEND_ERR_SCMI_STAT_NULL_PTR	(-5)
#define CPUCP_SCMI_SEND_ERR_TIMEOUT		(-6)
#define CPUCP_SCMI_SEND_ERR_RX_WINDOW		(-7)

/*
 * SCMI message header structure.
 *
 * @msg_id:   Unique 8-bit message identifier
 * @msg_type: Type of message (See SCMI specification)
 * @prot_id:  Unique 8-bit protocol identifier
 * @token:    Token usage varies with message type.
 */
struct __packed msg_hdr {
	uint32_t msg_id : 8;
	uint32_t msg_type : 2;
	uint32_t prot_id : 8;
	uint32_t token : 10;
};

/*
 * SCMI packet structure. Describes the layout of the shared-memory region used
 * for SCMI based communication between the agent and platform. See the
 * 'Transports' section of the SCMI specification for more details.
 */
struct scmi_packet {
	uint32_t res0;
	uint32_t chan_sts;
	uint64_t res1;
	uint32_t chan_flags;
	uint32_t len;
	struct msg_hdr msg_hdr;
	char payload[];
};

/* SCMI CLOCK_CONFIG_SET command payload. */
struct __packed __aligned(4) clk_config_set_payload {
	uint32_t clk_id;
	uint32_t attr;
};

static spinlock_t scmi_host_spinlock;
static spinlock_t cpucp_clkdom_init_spinlock;

/*
 * scmi_construct_msg_hdr - Construct an SCMI message header (32-bit) using the
 * @msg_id, @msg_type, @prot_id and @token arguments.
 */
static struct msg_hdr scmi_construct_msg_hdr(const unsigned int msg_id,
					     const unsigned int msg_type,
					     const unsigned int prot_id,
					     const unsigned int token)
{
	struct msg_hdr hdr;

	hdr.msg_id = msg_id;
	hdr.msg_type = msg_type;
	hdr.prot_id = prot_id;
	hdr.token = token;

	return hdr;
}

/*
 * cpucp_scmi_send - create an SCMI packet and send it to CPUCP
 *
 * This function creates an SCMI packet with the appropriate protocol ID and
 * message ID, and copies the TX-payload contents to shared-memory for
 * consumption by the target. It then rings the CPUCP doorbell to initiate SCMI
 * processing and polls the SCMI channel status until the remote processor
 * marks the channel free again or the timeout expires. Once processing is
 * complete, the SCMI status is retrieved and, if requested, the RX payload is
 * copied back from shared-memory.
 *
 * Returns 0 on a successful SCMI packet transfer, a negative
 * CPUCP_SCMI_SEND_ERR_* code otherwise.
 */
static int cpucp_scmi_send(unsigned int protocol_id, unsigned int message_id,
			   char *tx_payload, unsigned int tx_payload_size,
			   int *scmi_status, char *rx_payload,
			   unsigned int *rx_payload_size)
{
	uint32_t i;
	uint64_t current_tick;
	uint64_t max_tick;
	uint64_t ticks_to_wait;
	uint64_t freq;
	int ret = 0;
	uint32_t tx_payload_words;
	uint32_t rx_words_to_copy;
	uint32_t rx_bytes_to_copy;
	uint32_t volatile *remote_pkt_chan_status;
	struct scmi_packet *out_pkt =
		(struct scmi_packet *)CPUCP_IPC_SEC_BUF_BASE;
	char *payload_data = out_pkt->payload;
	struct msg_hdr message_header;

	/*
	 * If the TX payload size is non-zero, then the TX payload pointer
	 * cannot be NULL.
	 */
	if ((tx_payload == NULL) && (tx_payload_size > 0U)) {
		return CPUCP_SCMI_SEND_ERR_TX_NULL_PTR;
	}

	/* Check that the TX payload can fit in the shared-memory window. */
	if (tx_payload_size > (CPUCP_SECIPC_RAM_LENGTH -
			       sizeof(struct scmi_packet))) {
		return CPUCP_SCMI_SEND_ERR_TX_SIZE;
	}

	/* The TX copy loop transfers whole 32-bit words, so reject partial ones. */
	if ((tx_payload_size % 4U) != 0U) {
		return CPUCP_SCMI_SEND_ERR_TX_SIZE;
	}

	tx_payload_words = tx_payload_size / 4U;
	remote_pkt_chan_status = (uint32_t volatile *)(&out_pkt->chan_sts);

	spin_lock(&scmi_host_spinlock);

	freq = read_cntfrq_el0();
	ticks_to_wait = (freq * TIME_OUT_PERIOD_MS) / 1000U;

	current_tick = read_cntpct_el0();
	max_tick = current_tick + ticks_to_wait;

	/* Wait until the channel is marked free or the timeout is reached. */
	while ((*remote_pkt_chan_status != CHAN_STAT_CHAN_FREE) &&
	       (current_tick < max_tick)) {
		current_tick = read_cntpct_el0();
	}

	if (*remote_pkt_chan_status != CHAN_STAT_CHAN_FREE) {
		ret = CPUCP_SCMI_SEND_ERR_TIMEOUT;
		goto lock_free_and_return;
	}

	/*
	 * All subsequent writes into CPUCP shared-memory must occur only after
	 * the SCMI channel has been observed free.
	 */
	dsbsy();

	message_header = scmi_construct_msg_hdr(message_id, SCMI_MSG_TYPE_CMD,
						protocol_id, 0U);
	out_pkt->msg_hdr = message_header;

	/* Mark the SCMI channel as busy. */
	out_pkt->chan_sts = CHAN_STAT_CHAN_BUSY;

	/*
	 * Indicate that we do not expect a completion interrupt for the SCMI
	 * packet (true for the secure world only).
	 */
	out_pkt->chan_flags = SCMI_MBOX_FLAG_INT_CMD_COMP_NOT_VIA_INT;

	/* Initialize the SCMI payload before writing data. */
	out_pkt->len = SCMI_PKT_LEN_INIT;

	for (i = 0U; (i < tx_payload_words) && (tx_payload != NULL); i++) {
		/* Append payload word, advance length. */
		mmio_write_32((uintptr_t)out_pkt->payload +
			      (out_pkt->len - SCMI_PKT_LEN_INIT),
			      *((uint32_t *)tx_payload + i));
		out_pkt->len += 4U;
	}

	/* Read back the packet to ensure write completion. */
	(void)mmio_read_32((uintptr_t)out_pkt->payload + sizeof(uint32_t));

	/* Ring the CPUCP doorbell. */
	mmio_write_32(APSS_SHARED_TZ_IPC_INTERRUPT_ADDR,
		      1U << APSS_SHARED_TZ_IPC_INTERRUPT_OSM_IPC_SHFT);

	current_tick = read_cntpct_el0();
	max_tick = current_tick + ticks_to_wait;

	/*
	 * Wait until the channel is marked free or the timeout is reached
	 * (we are not expecting a completion interrupt).
	 */
	while ((*remote_pkt_chan_status != CHAN_STAT_CHAN_FREE) &&
	       (current_tick < max_tick)) {
		current_tick = read_cntpct_el0();
	}

	if (*remote_pkt_chan_status != CHAN_STAT_CHAN_FREE) {
		ret = CPUCP_SCMI_SEND_ERR_TIMEOUT;
		goto lock_free_and_return;
	}

	/*
	 * All subsequent reads from CPUCP shared-memory must occur only after
	 * the SCMI channel has been observed free.
	 */
	dsbsy();

	/* Reject a remote-set length that would read past the shmem window. */
	if (out_pkt->len > (CPUCP_SECIPC_RAM_LENGTH -
			    offsetof(struct scmi_packet, payload))) {
		ret = CPUCP_SCMI_SEND_ERR_RX_WINDOW;
		goto lock_free_and_return;
	}

	/* Retrieve the SCMI status output (4 bytes). */
	if (out_pkt->len >= 8U) {
		if (scmi_status != NULL) {
			*scmi_status = *((uint32_t *)payload_data);
		} else {
			ret = CPUCP_SCMI_SEND_ERR_SCMI_STAT_NULL_PTR;
			goto lock_free_and_return;
		}
	}

	/*
	 * Bail out if the caller is not interested in the RX payload, or if
	 * there is no valid RX payload content.
	 */
	if ((rx_payload_size == NULL) || (*rx_payload_size == 0U) ||
	    (out_pkt->len <= 8U)) {
		goto lock_free_and_return;
	}

	if (rx_payload == NULL) {
		ret = CPUCP_SCMI_SEND_ERR_RX_NULL_PTR;
		goto lock_free_and_return;
	}

	/*
	 * Return an error if rx_payload is not big enough to hold the SCMI
	 * output payload, reporting the correct size back to the caller.
	 */
	if (*rx_payload_size < (out_pkt->len - 8U)) {
		ret = CPUCP_SCMI_SEND_ERR_RX_SIZE;
		*rx_payload_size = out_pkt->len - 8U;
		goto lock_free_and_return;
	}

	/* Copy the payload from CPUCP shared-memory to APSS as 4-byte words. */
	rx_words_to_copy = (out_pkt->len - 8U) / 4U;

	for (i = 0U; i < rx_words_to_copy; i++) {
		*((uint32_t *)rx_payload + i) =
			*((uint32_t *)payload_data + 1 + i);
	}

	/* Copy the leftover payload from CPUCP shared-memory as bytes. */
	rx_bytes_to_copy = out_pkt->len % 4U;

	for (i = 0U; i < rx_bytes_to_copy; i++) {
		*((char *)((uint32_t *)rx_payload + rx_words_to_copy) + i) =
			*((char *)((uint32_t *)payload_data + 1 +
				   rx_words_to_copy) + i);
	}

	/* Notify the caller how many bytes were actually copied. */
	*rx_payload_size = out_pkt->len - 8U;

lock_free_and_return:
	spin_unlock(&scmi_host_spinlock);

	return ret;
}

/*
 * cpucp_clkdom_init - request CPUCP to enable the clock domain owning the
 * calling core during cold boot.
 *
 * If the calling core is the first core of its clock domain to cold boot, issue
 * an SCMI Clock Management "config set" command to CPUCP to enable the clock
 * domain.
 */
void cpucp_clkdom_init(void)
{
	int32_t ret_status;
	int32_t scmi_status = -1;
	unsigned int core_pos = plat_my_core_pos();
	uint32_t linear_id;
	struct clk_config_set_payload tx_payload_data;

	/* CPU masks are 32-bit, so the core index must fit a 32-bit shift. */
	assert(core_pos < 32U);
	linear_id = 1U << core_pos;

	for (int i = 0; i < CD_MAX; i++) {
		if ((clkdom_init_status[i] != 0U) ||
		    ((linear_id & clkdom_cpumasks[i].cpumask) == 0U)) {
			continue;
		}

		spin_lock(&cpucp_clkdom_init_spinlock);

		/* Bail out if the clock domain has since been enabled. */
		if (clkdom_init_status[i] != 0U) {
			spin_unlock(&cpucp_clkdom_init_spinlock);
			break;
		}

		tx_payload_data.clk_id = clkdom_cpumasks[i].clkdom;
		tx_payload_data.attr = SCMI_ATTR_CLK_CFG_SET_CLK_EN;

		ret_status = cpucp_scmi_send(SCMI_CLOCK_MANAGEMENT_PROTOCOL,
					     CLOCK_CONFIG_SET,
					     (char *)&tx_payload_data,
					     sizeof(tx_payload_data),
					     &scmi_status, NULL, NULL);

		if ((ret_status == 0) && (scmi_status == 0)) {
			clkdom_init_status[i] = 1U;
		}

		spin_unlock(&cpucp_clkdom_init_spinlock);

		break;
	}
}
