/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * TME service of the FF-A LSP.
 *
 * The service is a thin layer over the TME passthrough API: SEND_REQ is
 * tme_passthrough_cmd() and AWAIT_RESP is tme_passthrough_await().
 */

#include <errno.h>
#include <stddef.h>
#include <stdint.h>

#include <smccc_helpers.h>

#include "ffa_tme_handler.h"
#include "el3_spmc_logical_sp.h"
#include "tme_interfaces.h"

/* Index in rsp_regs[] of the results of the commands */
#define FFA_TME_RSP_STATUS		0	/* SEND_REQ and AWAIT_RESP */
#define FFA_TME_RSP_TXN			1	/* SEND_REQ */

/*
 * Translate an error of the passthrough API, an errno value of TF-A, to the
 * code of the service that OP-TEE understands. The API does not document the
 * sign of its errors, so both signs are accepted. The magnitude is taken in
 * unsigned arithmetic, which is defined for any int. Errors that have no code
 * of their own are TME_ERR_FAILURE.
 */
static tme_err_t ffa_tme_err(int ret)
{
	const unsigned int err = (ret < 0) ? (0U - (unsigned int)ret) :
					     (unsigned int)ret;

	switch (err) {
	case EINVAL:
		return TME_ERR_INVALID_ARG;
	case EAGAIN:
		return TME_ERR_AGAIN;
	case EINPROGRESS:
		return TME_ERR_IN_PROGRESS;
	case ENODEV:
		return TME_ERR_NO_DEV;
	case EMSGSIZE:
	case ENOSPC:
		return TME_ERR_DATA_TOO_LARGE;
	case EBADMSG:
		return TME_ERR_BAD_DATA;
	case EIO:
	default:
		return TME_ERR_FAILURE;
	}
}

/* Status of the service for the value returned by the passthrough API. */
static uint64_t ffa_tme_status(int ret)
{
	return (ret == 0) ? 0U : (uint64_t)ffa_tme_err(ret);
}

/*
 * SEND_REQ: hand the request, the first req_len bytes of buf, to the
 * passthrough API.
 */
static uint64_t ffa_tme_cmd_send(void *buf, size_t req_len,
				 uint64_t *rsp_regs)
{
	uint32_t txn = 0U;
	int ret;

	ret = tme_passthrough_cmd(buf, req_len, &txn);

	rsp_regs[FFA_TME_RSP_STATUS] = ffa_tme_status(ret);
	rsp_regs[FFA_TME_RSP_TXN] = txn;

	return FFA_LSP_ST_SUCCESS;
}

/*
 * AWAIT_RESP: one poll for the response of an earlier SEND_REQ, which the API
 * writes at the start of buf.
 */
static uint64_t ffa_tme_cmd_await(void *handle, void *buf, size_t buf_size,
				  size_t *out_size, uint64_t *rsp_regs)
{
	const uint64_t txn = SMC_GET_GP(handle, CTX_GPREG_X7);
	const uint64_t rsp_cap = SMC_GET_GP(handle, CTX_GPREG_X8);
	uint64_t status = FFA_LSP_ST_SUCCESS;
	size_t rsp_size = (size_t)rsp_cap;
	int ret;

	/* The transaction ID is 32 bit, the response has to fit the buffer. */
	if ((txn > UINT32_MAX) || (rsp_cap > buf_size)) {
		return FFA_LSP_ST_INVALID_PARAM;
	}

	ret = tme_passthrough_await((uint32_t)txn, buf, &rsp_size);

	/*
	 * The size is only meaningful if the response was written, and then it
	 * cannot exceed the buffer. Do not hand out any other value.
	 */
	if (ret != 0) {
		rsp_size = 0U;
	} else if (rsp_size > rsp_cap) {
		rsp_size = 0U;
		status = FFA_LSP_ST_FAILURE;
	}

	*out_size = rsp_size;
	rsp_regs[FFA_TME_RSP_STATUS] = ffa_tme_status(ret);

	return status;
}

uint64_t ffa_tme_handler(void *handle, void *buf, size_t buf_size,
			 size_t in_size, size_t *out_size, uint64_t *rsp_regs)
{
	const uint64_t tme_cmd = SMC_GET_GP(handle, CTX_GPREG_X6);

	switch (tme_cmd) {
	case FFA_TME_CMD_SEND_REQ:
		return ffa_tme_cmd_send(buf, in_size, rsp_regs);
	case FFA_TME_CMD_AWAIT_RESP:
		return ffa_tme_cmd_await(handle, buf, buf_size, out_size,
					 rsp_regs);
	default:
		return FFA_LSP_ST_NOT_SUPPORTED;
	}
}
