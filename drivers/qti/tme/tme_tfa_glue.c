/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * TF-A implementations of the downstream helper APIs the vendored TME sources
 * expect.
 *
 * drivers/qti/tme/inc/common/ carries copies of the downstream tzbsp_log.h and
 * stringl/stringl.h headers, but those are declaration-only: the real bodies
 * are noship and live outside this tree.  The tmeintf sources call
 * TFA_LOG_ERR() / TFA_LOG_ERR_FATAL() (which expand to tfa_log_<n>()) and
 * memscpy(), so without this file BL31 fails to link.
 *
 * The tfa_log_<n>() family receives already-unpacked uintptr_t arguments and
 * no format string, so every value is printed as hex.
 *
 * Only the arities the TME sources actually call are implemented - tfa_log_0
 * through tfa_log_3 - plus memscpy().  The widest caller is TmeMessage.c's
 * TZBSP_TME_MESSAGE_ENCODE_MESSAGE_LENGTH_ERROR, which passes three arguments.
 * tzbsp_log.h declares tfa_log_0..16 and stringl.h declares memsmove(), but
 * the unused ones are deliberately omitted rather than carried here as dead
 * forwarders.
 *
 * TFA_LOG()'s arity dispatch (GET_MACRO_TFA in tzbsp_log.h) token-pastes the
 * argument count, so only the selected tfa_log_<n> is ever referenced.  A new
 * log call with four or more arguments will therefore fail to link until the
 * matching tfa_log_<n> is added below - likewise memsmove() if a caller
 * appears.
 */

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <common/debug.h>

#include <stringl/stringl.h>
#include <tzbsp_log.h>

/*
 * Emit one log line: the status/error code followed by each argument in hex.
 *
 * TZBSP priorities are inverted relative to TF-A's LOG_LEVEL_* (0 is the most
 * severe), so FATAL/ERROR map onto TF-A's ERROR level and everything else onto
 * VERBOSE.  Level filtering is done by hand here because the argument count is
 * only known at runtime, which rules out the fixed-format ERROR()/VERBOSE()
 * macros; the prefixes below match the column layout tf_log() produces.
 */
static void tme_log_emit(uint64_t priority, uint64_t error_code,
			 const uintptr_t *args, unsigned int count)
{
	unsigned int i;

	if (priority <= (uint64_t)TZBSP_MSG_ERROR) {
		if (LOG_LEVEL < LOG_LEVEL_ERROR) {
			return;
		}
		printf("ERROR:   TME sc:0x%llx",
		       (unsigned long long)error_code);
	} else {
		if (LOG_LEVEL < LOG_LEVEL_VERBOSE) {
			return;
		}
		printf("VERBOSE: TME sc:0x%llx",
		       (unsigned long long)error_code);
	}

	for (i = 0U; i < count; i++) {
		printf(" 0x%lx", (unsigned long)args[i]);
	}

	printf("\n");
}

void tfa_log_0(uint64_t priority, uint64_t error_code)
{
	tme_log_emit(priority, error_code, NULL, 0U);
}

void tfa_log_1(uint64_t priority, uint64_t error_code, uintptr_t arg1)
{
	tme_log_emit(priority, error_code, (const uintptr_t[]){arg1}, 1U);
}

void tfa_log_2(uint64_t priority, uint64_t error_code, uintptr_t arg1,
	uintptr_t arg2)
{
	tme_log_emit(priority, error_code, (const uintptr_t[]){arg1, arg2}, 2U);
}

void tfa_log_3(uint64_t priority, uint64_t error_code, uintptr_t arg1,
	uintptr_t arg2, uintptr_t arg3)
{
	tme_log_emit(priority, error_code,
		     (const uintptr_t[]){arg1, arg2, arg3}, 3U);
}

/*
 * Size-bounded copy: never writes past dst_size, and returns the number of
 * bytes actually copied so the caller can detect truncation by comparing the
 * result against src_size.
 */
size_t memscpy(void *dst, size_t dst_size, const void *src, size_t src_size)
{
	size_t copy_size = (dst_size < src_size) ? dst_size : src_size;

	(void)memcpy(dst, src, copy_size);

	return copy_size;
}
