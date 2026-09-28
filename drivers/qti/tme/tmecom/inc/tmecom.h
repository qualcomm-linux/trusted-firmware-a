/*===========================================================================
  Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
  All rights reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
===========================================================================*/

#ifndef TMECOM_H
#define TMECOM_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* Opaque handle returned when a client registers with tmecom. */
typedef struct tmecom_client_t tmecom_client_t;

#define TMECOM_VERSION(major, minor) (((major) << 8) | (minor))
#define TMECOM_VERSION_NONE          TMECOM_VERSION(0, 0)
#define TMECOM_VERSION_MAJOR         1
#define TMECOM_VERSION_MINOR         0

#define TMECOM_PROTOCOL_FAILURE   100
#define TMECOM_TME_ERROR_FATAL    132

/**
 * tmecom_msg_hdr_t - Request/Response message header between TFA and TME.
 *
 * This header is proceeding any request specific parameters.
 * The transaction id is used to match request with response.
 */
typedef struct tmecom_msg_hdr {
	uint16_t version;       /* TMECom Version */
	uint16_t crc;           /* Message CRC    */
	uint32_t txn_id;         /* transaction ID */
} __attribute__((__packed__)) tmecom_msg_hdr_t;

/**
 * Max allocation for TME COM mailbox
 */
#define TMECOM_MAX_MAILBOX_SIZE (2048)

/**
 * First part of mailbox memory is taken up by QMP descriptor
 *
 * @note  Same as sizeof(xport_qmp_ch_desc_type) but cannot figure out how to
 *        get the header included here
 */
#define TMECOM_LOCAL_MAILBOX_OFFSET  (0xC0)
#define TMECOM_REMOTE_MAILBOX_OFFSET (0x00)

/**
 * Request buffer size for outbound message
 */
/* Round a byte count up to the nearest number of uint32_t words. */
#define TME_BYTES2WORDS(n)  (((n) + sizeof(uint32_t) - 1U) / sizeof(uint32_t))

#define TMECOM_MAX_REQUEST_SIZE      (TMECOM_MAX_MAILBOX_SIZE     - \
                                      TMECOM_LOCAL_MAILBOX_OFFSET - \
                                      sizeof(tmecom_msg_hdr_t))

/**
 * Response buffer size for inbound message
 */
#define TMECOM_MAX_RESPONSE_SIZE     (TMECOM_MAX_MAILBOX_SIZE      - \
                                      TMECOM_REMOTE_MAILBOX_OFFSET - \
                                      sizeof(tmecom_msg_hdr_t))

typedef struct tmecom_req {
	uint8_t buf[TMECOM_MAX_REQUEST_SIZE];
	bool in_use;
} __attribute__((aligned(sizeof(uint32_t)))) tmecom_req_t;

typedef struct tmecom_rsp {
	uint8_t buf[TMECOM_MAX_RESPONSE_SIZE];
	bool in_use;
} __attribute__((aligned(sizeof(uint32_t)))) tmecom_rsp_t;

typedef struct tmecom_msg_req {
	tmecom_msg_hdr_t  hdr;
	uint8_t       enc_req_buf[TMECOM_MAX_REQUEST_SIZE];
} __attribute__((aligned(sizeof(uint32_t)))) tmecom_msg_req_t;

typedef struct tmecom_msg_rsp {
	tmecom_msg_hdr_t  hdr;
	uint8_t       enc_rsp_buf[TMECOM_MAX_RESPONSE_SIZE];
} __attribute__((aligned(sizeof(uint32_t)))) tmecom_msg_rsp_t;

/**
  * @addtogroup TMECom
  * @{
  */

/**
  * Used by tmecom to allocate the request buffer used
  * to communicate with the TME-FW.
  *
  * @param  [in]  size    Size of the request buffer.
  *
  * @return Pointer to the memory if successfully obtained,
  *         @c NULL otherwise
  */
void *tmecom_alloc_req(size_t size);

/**
  * Used by tmecom to free the request buffer used
  * to communicate with the TME-FW.
  *
  * @param  [in]  p_mem    Pointer to the request buffer to be released
  *
  */
void tmecom_free_req(void *p_mem);

/**
  * Used by tmecom to allocate the response buffer used
  * to communicate with the TME-FW.
  *
  * @param  [in]  size    Size of the response buffer.
  *
  * @return Pointer to the memory if successfully obtained,
  *         @c NULL otherwise
  */
void *tmecom_alloc_rsp(size_t size);

/**
  * Used by tmecom to release the response buffer used
  * to communicate with the TME-FW.
  *
  * @param  [in]  p_mem    Pointer to the response buffer to be released
  *
  */
void tmecom_free_rsp(void *p_mem);

/** @} */ /* end addtogroup TMECom */

#endif  /* TMECOM_H */
