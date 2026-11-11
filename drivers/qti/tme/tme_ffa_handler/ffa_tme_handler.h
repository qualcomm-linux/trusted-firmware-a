/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef FFA_TME_HANDLER_H
#define FFA_TME_HANDLER_H

#include <stddef.h>
#include <stdint.h>

/*
 * TME service of the logical partition
 *
 * OP-TEE reaches the TME through two commands, which map one to one onto the
 * TME passthrough API. The bytes of the TME messages (CBOR) are in the shared
 * page of the LSP and are handed to and received from the
 * passthrough API in place. The LSP keeps x4 and x5 for its framing: x5 is the
 * size of the buffer in the shared page, in the request and in the response
 * (see el3_spmc_logical_sp.h). The registers of the service are x6..x17,
 * the command is in x6.
 *
 *   SEND_REQ (tme_passthrough_cmd())
 *     in : x5 = length in bytes of the request in the shared page
 *          x6 = FFA_TME_CMD_SEND_REQ
 *     out: x4 = status of the LSP (FFA_LSP_ST_*)
 *          x5 = 0, nothing is written to the shared page
 *          x6 = status of the service: 0 if the TME accepted the request,
 *               TME_ERR_AGAIN if it is busy, another tme_err_t otherwise
 *          x7 = transaction ID for AWAIT_RESP, valid if x6 is 0
 *
 *   AWAIT_RESP (tme_passthrough_await())
 *     in : x6 = FFA_TME_CMD_AWAIT_RESP
 *          x7 = transaction ID returned by SEND_REQ
 *          x8 = length in bytes of the response buffer, the shared page or
 *               less
 *     out: x4 = status of the LSP (FFA_LSP_ST_*)
 *          x5 = length in bytes of the response that tme_passthrough_await()
 *               wrote in the shared page, 0 unless the response is there
 *          x6 = status of the service: 0 if the response is there,
 *               TME_ERR_IN_PROGRESS if the TME has not answered yet, another
 *               tme_err_t otherwise
 *
 * Status of the service. 0 is success, any other value is a tme_err_t. The
 * errors of the passthrough API are errno values of TF-A, which OP-TEE cannot
 * use, so the service translates them (ffa_tme_err() in ffa_tme_handler.c):
 *
 *   EINVAL             TME_ERR_INVALID_ARG
 *   EAGAIN             TME_ERR_AGAIN, the TME is busy: SEND_REQ again later
 *   EINPROGRESS        TME_ERR_IN_PROGRESS: AWAIT_RESP again later
 *   ENODEV             TME_ERR_NO_DEV
 *   EMSGSIZE           TME_ERR_DATA_TOO_LARGE, the request is above the limit
 *   ENOSPC             TME_ERR_DATA_TOO_LARGE, the response is above the buffer
 *   EBADMSG            TME_ERR_BAD_DATA, version, CRC or transaction ID
 *   EIO, anything else TME_ERR_FAILURE
 *
 * UUID of the TME service: 4dd829e1-7cf3-4dcf-9ecc-bda410aa53e1. Its 16 bytes
 * in the order of the text form are read as two little-endian 64-bit numbers.
 */
#define FFA_TME_UUID_X2			0xcf4df37ce129d84dULL
#define FFA_TME_UUID_X3			0xe153aa10a4bdcc9eULL

/* Commands (REQ2 x6). */
#define FFA_TME_CMD_SEND_REQ		0x1
#define FFA_TME_CMD_AWAIT_RESP		0x2

/* Status of the service (x6 of the response), 0 is success. */
typedef enum {
	TME_ERR_FAILURE = 1,
	TME_ERR_AGAIN,
	TME_ERR_IN_PROGRESS,
	TME_ERR_NO_MEMORY,
	TME_ERR_INVALID_ARG,
	TME_ERR_BAD_DATA,
	TME_ERR_DATA_TOO_LARGE,
	TME_ERR_NO_DEV,
	TME_ERR_TIMER_EXP,
} tme_err_t;

/*
 * Handler of the TME service, to be listed in the services of the LSP. Same
 * prototype as lsp_service_handler_t.
 */
uint64_t ffa_tme_handler(void *handle, void *buf, size_t buf_size,
			 size_t in_size, size_t *out_size, uint64_t *rsp_regs);

#endif /* FFA_TME_HANDLER_H */
