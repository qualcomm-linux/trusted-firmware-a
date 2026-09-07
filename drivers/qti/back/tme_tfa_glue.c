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

void tfa_log_4(uint64_t priority, uint64_t error_code, uintptr_t arg1,
	uintptr_t arg2, uintptr_t arg3, uintptr_t arg4)
{
	tme_log_emit(priority, error_code,
		     (const uintptr_t[]){arg1, arg2, arg3, arg4}, 4U);
}

void tfa_log_5(uint64_t priority, uint64_t error_code, uintptr_t arg1,
	uintptr_t arg2, uintptr_t arg3, uintptr_t arg4, uintptr_t arg5)
{
	tme_log_emit(priority, error_code,
		     (const uintptr_t[]){arg1, arg2, arg3, arg4, arg5}, 5U);
}

void tfa_log_6(uint64_t priority, uint64_t error_code, uintptr_t arg1,
	uintptr_t arg2, uintptr_t arg3, uintptr_t arg4, uintptr_t arg5,
	uintptr_t arg6)
{
	tme_log_emit(priority, error_code,
		     (const uintptr_t[]){arg1, arg2, arg3, arg4, arg5, arg6},
		     6U);
}

void tfa_log_7(uint64_t priority, uint64_t error_code, uintptr_t arg1,
	uintptr_t arg2, uintptr_t arg3, uintptr_t arg4, uintptr_t arg5,
	uintptr_t arg6, uintptr_t arg7)
{
	tme_log_emit(priority, error_code,
		     (const uintptr_t[]){arg1, arg2, arg3, arg4, arg5, arg6,
					 arg7}, 7U);
}

void tfa_log_8(uint64_t priority, uint64_t error_code, uintptr_t arg1,
	uintptr_t arg2, uintptr_t arg3, uintptr_t arg4, uintptr_t arg5,
	uintptr_t arg6, uintptr_t arg7, uintptr_t arg8)
{
	tme_log_emit(priority, error_code,
		     (const uintptr_t[]){arg1, arg2, arg3, arg4, arg5, arg6,
					 arg7, arg8}, 8U);
}

void tfa_log_9(uint64_t priority, uint64_t error_code, uintptr_t arg1,
	uintptr_t arg2, uintptr_t arg3, uintptr_t arg4, uintptr_t arg5,
	uintptr_t arg6, uintptr_t arg7, uintptr_t arg8, uintptr_t arg9)
{
	tme_log_emit(priority, error_code,
		     (const uintptr_t[]){arg1, arg2, arg3, arg4, arg5, arg6,
					 arg7, arg8, arg9}, 9U);
}

void tfa_log_10(uint64_t priority, uint64_t error_code, uintptr_t arg1,
	uintptr_t arg2, uintptr_t arg3, uintptr_t arg4, uintptr_t arg5,
	uintptr_t arg6, uintptr_t arg7, uintptr_t arg8, uintptr_t arg9,
	uintptr_t arg10)
{
	tme_log_emit(priority, error_code,
		     (const uintptr_t[]){arg1, arg2, arg3, arg4, arg5, arg6,
					 arg7, arg8, arg9, arg10}, 10U);
}

void tfa_log_11(uint64_t priority, uint64_t error_code, uintptr_t arg1,
	uintptr_t arg2, uintptr_t arg3, uintptr_t arg4, uintptr_t arg5,
	uintptr_t arg6, uintptr_t arg7, uintptr_t arg8, uintptr_t arg9,
	uintptr_t arg10, uintptr_t arg11)
{
	tme_log_emit(priority, error_code,
		     (const uintptr_t[]){arg1, arg2, arg3, arg4, arg5, arg6,
					 arg7, arg8, arg9, arg10, arg11}, 11U);
}

void tfa_log_12(uint64_t priority, uint64_t error_code, uintptr_t arg1,
	uintptr_t arg2, uintptr_t arg3, uintptr_t arg4, uintptr_t arg5,
	uintptr_t arg6, uintptr_t arg7, uintptr_t arg8, uintptr_t arg9,
	uintptr_t arg10, uintptr_t arg11, uintptr_t arg12)
{
	tme_log_emit(priority, error_code,
		     (const uintptr_t[]){arg1, arg2, arg3, arg4, arg5, arg6,
					 arg7, arg8, arg9, arg10, arg11, arg12},
		     12U);
}

void tfa_log_13(uint64_t priority, uint64_t error_code, uintptr_t arg1,
	uintptr_t arg2, uintptr_t arg3, uintptr_t arg4, uintptr_t arg5,
	uintptr_t arg6,
	uintptr_t arg7, uintptr_t arg8, uintptr_t arg9, uintptr_t arg10,
	uintptr_t arg11,
	uintptr_t arg12, uintptr_t arg13)
{
	tme_log_emit(priority, error_code,
		     (const uintptr_t[]){arg1, arg2, arg3, arg4, arg5, arg6,
					 arg7, arg8, arg9, arg10, arg11, arg12,
					 arg13}, 13U);
}

void tfa_log_14(uint64_t priority, uint64_t error_code, uintptr_t arg1,
	uintptr_t arg2, uintptr_t arg3, uintptr_t arg4, uintptr_t arg5,
	uintptr_t arg6, uintptr_t arg7, uintptr_t arg8, uintptr_t arg9,
	uintptr_t arg10, uintptr_t arg11, uintptr_t arg12, uintptr_t arg13,
	uintptr_t arg14)
{
	tme_log_emit(priority, error_code,
		     (const uintptr_t[]){arg1, arg2, arg3, arg4, arg5, arg6,
					 arg7, arg8, arg9, arg10, arg11, arg12,
					 arg13, arg14}, 14U);
}

void tfa_log_15(uint64_t priority, uint64_t error_code, uintptr_t arg1,
	uintptr_t arg2, uintptr_t arg3, uintptr_t arg4, uintptr_t arg5,
	uintptr_t arg6, uintptr_t arg7, uintptr_t arg8, uintptr_t arg9,
	uintptr_t arg10, uintptr_t arg11, uintptr_t arg12, uintptr_t arg13,
	uintptr_t arg14, uintptr_t arg15)
{
	tme_log_emit(priority, error_code,
		     (const uintptr_t[]){arg1, arg2, arg3, arg4, arg5, arg6,
					 arg7, arg8, arg9, arg10, arg11, arg12,
					 arg13, arg14, arg15}, 15U);
}

void tfa_log_16(uint64_t priority, uint64_t error_code, uintptr_t arg1,
	uintptr_t arg2, uintptr_t arg3, uintptr_t arg4, uintptr_t arg5,
	uintptr_t arg6, uintptr_t arg7, uintptr_t arg8, uintptr_t arg9,
	uintptr_t arg10, uintptr_t arg11, uintptr_t arg12, uintptr_t arg13,
	uintptr_t arg14, uintptr_t arg15, uintptr_t arg16)
{
	tme_log_emit(priority, error_code,
		     (const uintptr_t[]){arg1, arg2, arg3, arg4, arg5, arg6,
					 arg7, arg8, arg9, arg10, arg11, arg12,
					 arg13, arg14, arg15, arg16}, 16U);
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

/* As memscpy(), but tolerates overlapping source and destination. */
size_t memsmove(void *dst, size_t dst_size, const void *src, size_t src_size)
{
	size_t copy_size = (dst_size < src_size) ? dst_size : src_size;

	(void)memmove(dst, src, copy_size);

	return copy_size;
}
