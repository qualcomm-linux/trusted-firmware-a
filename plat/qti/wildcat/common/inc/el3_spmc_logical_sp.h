/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef EL3_SPMC_LOGICAL_SP_H
#define EL3_SPMC_LOGICAL_SP_H

#include <stddef.h>
#include <stdint.h>

/*
 * Logical partition (LSP) of the EL3 SPMC.
 *
 * The LSP lets OP-TEE (the S-EL1 FF-A SP) reach services that run in EL3.
 * Each service brings a handler, which the LSP
 * runs for the requests that carry the UUID of the service. This header has
 * the interface between OP-TEE and the LSP and what a service needs.
 *
 * A service is selected by its UUID, and defines its own commands and the
 * meaning of the registers x6..x17 for each of them.
 *
 * Transport: FFA_MSG_SEND_DIRECT_REQ2 / FFA_MSG_SEND_DIRECT_RESP2
 * SMC64 only, FF-A version >= 1.2 is required.
 *
 *   REQ2  (OP-TEE -> LSP)
 *     x0 = FFA_MSG_SEND_DIRECT_REQ2
 *     w1 = (sender << 16) | receiver        sender = ID of OP-TEE
 *                                           receiver = LSP ID
 *     x2, x3 = UUID of the service
 *     x4 = reserved, 0
 *     x5 = size in bytes of the input buffer in the shared page, 0 if none
 *     x6..x17 = arguments, defined by the service, which carry its command;
 *               unused registers are zero
 *
 *   RESP2 (LSP -> OP-TEE)
 *     x0 = FFA_MSG_SEND_DIRECT_RESP2
 *     w1 = (receiver << 16) | sender        i.e. (LSP ID << 16) | ID of OP-TEE
 *     x2 = x3 = 0                           (SBZ per FF-A 1.2)
 *     x4 = status of the LSP (FFA_LSP_ST_*), in every response
 *     x5 = size in bytes of the output buffer that the LSP wrote to the shared
 *          page, 0 if it wrote nothing
 *     x6..x17 = results, defined by the service; unused registers are zero
 *
 * x4 and x5 belong to the framing of the LSP, the service never sees them as
 * its own registers. The status in x4 says whether the LSP accepted and handled
 * the request. It is not the result of the operation of the service, which
 * every command returns in its own registers or in the shared buffer.
 *
 * The LSP never answers with FFA_ERROR; every LSP-level failure is a RESP2 
 * carrying an FFA_LSP_ST_* value.
 *
 * Sender. The only sender that the LSP accepts is OP-TEE, and only from the
 * secure world. It's FF-A ID is read from the TOS_FW_CONFIG FDT.
 *
 * Service UUID. The 16 bytes of the UUID, in the order of its text form, are
 * read as two little-endian 64-bit numbers: x2 holds the first 8 bytes and x3
 * the last 8.
 *
 * Shared memory. Payloads that do not fit the registers are not passed in
 * registers but in one secure page that OP-TEE and the LSP share.It is
 * used for the request of a command and then for
 * its response, which replaces the request. The size of the buffer is always
 * passed in x5, in both directions, never in the page, and is at most the size
 * of the page. The address and the size of the page are the "qcom-ffa-shmem"
 * memory region of the SP manifest of OP-TEE, which both OP-TEE and the LSP
 * read.
 */

/* Status of the LSP (RESP2 x4). */
#define FFA_LSP_ST_SUCCESS		0
#define FFA_LSP_ST_FAILURE		1	/* the LSP could not do it */
#define FFA_LSP_ST_INVALID_PARAM	2	/* bad register value or size */
#define FFA_LSP_ST_NOT_SUPPORTED	3	/* unknown FID, UUID, command */
#define FFA_LSP_ST_DENIED		4	/* bad origin or sender */

/*
 * Number of registers, x6..x17, that a request and a response carry for the
 * service. x4 and x5 belong to the framing of the LSP.
 */
#define LSP_NUM_RSP_REGS		12U

/*
 * Handler of a service of the LSP.
 *
 * It is called for a request that passed the checks of the LSP (FF-A function,
 * origin, sender, size of the input buffer) and whose UUID is the one of the
 * service. The command and the other arguments are in x6..x17 of the context of
 * the caller.
 *
 * The input and the output buffers are in the page shared with OP-TEE.
 *
 * @handle:   Context of the caller
 * @buf:      The buffer passed by OP-TEE to the service
 * @buf_size: Size in bytes of @buf, the most that the handler may read from it
 *            or write to it. It is never 0.
 * @in_size:  Size in bytes of the input buffer, at most @buf_size;
 *            0 if the request has no buffer
 * @out_size: Receives the size in bytes of the output buffer that the handler
 *            wrote in @buf. It is 0 on entry and it must be at most
 *            @buf_size.
 * @rsp_regs: Receives the results for x6..x17 in rsp_regs[0..11]. It is zeroed
 *            on entry.
 *
 * The handler must return an LSP status code (FFA_LSP_ST_*). With a status
 * other than FFA_LSP_ST_SUCCESS it must leave *out_size and rsp_regs at 0.
 */
typedef uint64_t (*lsp_service_handler_t)(void *handle, void *buf,
					  size_t buf_size, size_t in_size,
					  size_t *out_size, uint64_t *rsp_regs);

#endif /* EL3_SPMC_LOGICAL_SP_H */
