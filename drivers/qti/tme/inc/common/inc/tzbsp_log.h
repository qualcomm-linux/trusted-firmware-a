#ifndef TZBSP_LOG_H
#define TZBSP_LOG_H

/**
@file tzbsp_log.h
@brief Trustzone Logging

Contains Macros for logging TZ debug messages. By default all logs go to the
internal ring buffer. Logs can also be dumped to JTAG based oncompile time flag
\c TZBSP_JTAG_LOGGING.

*/
/* Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * All rights reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

/*===========================================================================

	INCLUDE FILES

===========================================================================*/

#include <stdarg.h>
#include <stdint.h>
#include <stdbool.h>

#include "IxErrno.h"
/*===========================================================================

	DEFINES

===========================================================================*/


#define TZBSP_MSG_FATAL (0)
#define TZBSP_MSG_ERROR (1)
#define TZBSP_MSG_DEBUG (2)
#define TZBSP_MSG_HIGH (3)
#define TZBSP_MSG_MED (4)
#define TZBSP_MSG_LOW (5)

/**
 * Shortcut macros for error logging. These will populate both TFA ringbuffer
 * and QTEE log. These should be used for fatal error logs only, which will be
 * copied to QTEE during the error handling flow.
 */
#define TZBSP_LOG_ERR(xx_sc, ...) \
	TZBSP_LOG(TZBSP_MSG_ERROR, xx_sc, ##__VA_ARGS__)

#define TZBSP_LOG_NAME_ERR(xx_sc, ...) \
	TZBSP_LOG_NAME(TZBSP_MSG_ERROR, xx_sc, ##__VA_ARGS__)

#define TZBSP_LOG_NAME_DBG(xx_sc, ...) \
	TZBSP_LOG_NAME(TZBSP_MSG_DEBUG, xx_sc, ##__VA_ARGS__)

/**
 * Shortcut macros for fatal error logging, @see TZBSP_LOG.
 */
#define TZBSP_LOG_ERR_FATAL(xx_sc, ...) \
	TZBSP_LOG(TZBSP_MSG_FATAL, xx_sc, ##__VA_ARGS__)

#define TZBSP_LOG_NAME_ERR_FATAL(xx_sc, ...) \
	TZBSP_LOG_NAME(TZBSP_MSG_FATAL, xx_sc, ##__VA_ARGS__)

/**
 * Shortcut macro for conditinally logging an error, log is only printed if
 * error condition is present in \c pred.
 * Modified the maro to no longer take error format, but to take error codes/status codes.
 */
#define TZBSP_LOG_ERR_E(pred, xx_sc, ...)	\
	if (E_SUCCESS != (pred)) {		\
		TZBSP_LOG(TZBSP_MSG_ERROR, xx_sc, ##__VA_ARGS__); \
	}

/**
 * Modified the maro to no longer take error format, but to take error codes/status codes.
 * Shortcut macro for debug logging, @see TZBSP_LOG.
 */
#define TZBSP_LOG_DBG(xx_sc, ...) \
	TZBSP_LOG(TZBSP_MSG_DEBUG, xx_sc, ##__VA_ARGS__)

/**
 * This function is used to both TFA ringbuffer and to QTEE logs. The logs will
 * be shared with QTEE during fatal err handling path.
 *
 *
 * @param [in] pri  Prioriy of message to be logged.
 * @param [in] fmt  String describing the format of msg.
 * @param [in] ...  Variable argument list.
 */
void tzbsp_log(uint32_t pri, const char *fmt, ...);


/**
 * TFA logging functions with explicit argument counts.
 * These functions receive unpacked arguments (no format string).
 */
void tfa_log_0(uint64_t priority, uint64_t error_code);
void tfa_log_1(uint64_t priority, uint64_t error_code, uintptr_t arg1);
void tfa_log_2(uint64_t priority, uint64_t error_code, uintptr_t arg1,
	uintptr_t arg2);
void tfa_log_3(uint64_t priority, uint64_t error_code, uintptr_t arg1,
	uintptr_t arg2, uintptr_t arg3);
void tfa_log_4(uint64_t priority, uint64_t error_code, uintptr_t arg1,
	uintptr_t arg2, uintptr_t arg3, uintptr_t arg4);
void tfa_log_5(uint64_t priority, uint64_t error_code, uintptr_t arg1,
	uintptr_t arg2, uintptr_t arg3, uintptr_t arg4, uintptr_t arg5);
void tfa_log_6(uint64_t priority, uint64_t error_code, uintptr_t arg1,
	uintptr_t arg2, uintptr_t arg3, uintptr_t arg4, uintptr_t arg5,
	uintptr_t arg6);
void tfa_log_7(uint64_t priority, uint64_t error_code, uintptr_t arg1,
	uintptr_t arg2, uintptr_t arg3, uintptr_t arg4, uintptr_t arg5,
	uintptr_t arg6, uintptr_t arg7);
void tfa_log_8(uint64_t priority, uint64_t error_code, uintptr_t arg1,
	uintptr_t arg2, uintptr_t arg3, uintptr_t arg4, uintptr_t arg5,
	uintptr_t arg6, uintptr_t arg7, uintptr_t arg8);
void tfa_log_9(uint64_t priority, uint64_t error_code, uintptr_t arg1,
	uintptr_t arg2, uintptr_t arg3, uintptr_t arg4, uintptr_t arg5,
	uintptr_t arg6, uintptr_t arg7, uintptr_t arg8, uintptr_t arg9);
void tfa_log_10(uint64_t priority, uint64_t error_code, uintptr_t arg1,
	uintptr_t arg2, uintptr_t arg3, uintptr_t arg4, uintptr_t arg5,
	uintptr_t arg6, uintptr_t arg7, uintptr_t arg8, uintptr_t arg9,
	uintptr_t arg10);
void tfa_log_11(uint64_t priority, uint64_t error_code, uintptr_t arg1,
	uintptr_t arg2, uintptr_t arg3, uintptr_t arg4, uintptr_t arg5,
	uintptr_t arg6, uintptr_t arg7, uintptr_t arg8, uintptr_t arg9,
	uintptr_t arg10, uintptr_t arg11);
void tfa_log_12(uint64_t priority, uint64_t error_code, uintptr_t arg1,
	uintptr_t arg2, uintptr_t arg3, uintptr_t arg4, uintptr_t arg5,
	uintptr_t arg6, uintptr_t arg7, uintptr_t arg8, uintptr_t arg9,
	uintptr_t arg10, uintptr_t arg11, uintptr_t arg12);
void tfa_log_13(uint64_t priority, uint64_t error_code, uintptr_t arg1,
	uintptr_t arg2, uintptr_t arg3, uintptr_t arg4, uintptr_t arg5,
	uintptr_t arg6,
	uintptr_t arg7, uintptr_t arg8, uintptr_t arg9, uintptr_t arg10,
	uintptr_t arg11,
	uintptr_t arg12, uintptr_t arg13);
void tfa_log_14(uint64_t priority, uint64_t error_code, uintptr_t arg1,
	uintptr_t arg2, uintptr_t arg3, uintptr_t arg4, uintptr_t arg5,
	uintptr_t arg6, uintptr_t arg7, uintptr_t arg8, uintptr_t arg9,
	uintptr_t arg10, uintptr_t arg11, uintptr_t arg12, uintptr_t arg13,
	uintptr_t arg14);
void tfa_log_15(uint64_t priority, uint64_t error_code, uintptr_t arg1,
	uintptr_t arg2, uintptr_t arg3, uintptr_t arg4, uintptr_t arg5,
	uintptr_t arg6, uintptr_t arg7, uintptr_t arg8, uintptr_t arg9,
	uintptr_t arg10, uintptr_t arg11, uintptr_t arg12, uintptr_t arg13,
	uintptr_t arg14, uintptr_t arg15);
void tfa_log_16(uint64_t priority, uint64_t error_code, uintptr_t arg1,
	uintptr_t arg2, uintptr_t arg3, uintptr_t arg4, uintptr_t arg5,
	uintptr_t arg6, uintptr_t arg7, uintptr_t arg8, uintptr_t arg9,
	uintptr_t arg10, uintptr_t arg11, uintptr_t arg12, uintptr_t arg13,
	uintptr_t arg14, uintptr_t arg15, uintptr_t arg16);
/*void tfa_log_17(uint64_t priority, uint64_t error_code, uintptr_t arg1,
uintptr_t arg2, uintptr_t arg3, uintptr_t arg4, uintptr_t arg5, uintptr_t arg6,
uintptr_t arg7, uintptr_t arg8, uintptr_t arg9, uintptr_t arg10, uintptr_t arg11,
uintptr_t arg12, uintptr_t arg13, uintptr_t arg14, uintptr_t arg15,
uintptr_t arg16, uintptr_t arg17);
void tfa_log_18(uint64_t priority, uint64_t error_code, uintptr_t arg1,
uintptr_t arg2, uintptr_t arg3, uintptr_t arg4, uintptr_t arg5, uintptr_t arg6,
uintptr_t arg7, uintptr_t arg8, uintptr_t arg9, uintptr_t arg10, uintptr_t arg11,
uintptr_t arg12, uintptr_t arg13, uintptr_t arg14, uintptr_t arg15,
uintptr_t arg16, uintptr_t arg17, uintptr_t arg18);
void tfa_log_19(uint64_t priority, uint64_t error_code, uintptr_t arg1,
uintptr_t arg2, uintptr_t arg3, uintptr_t arg4, uintptr_t arg5, uintptr_t arg6,
uintptr_t arg7, uintptr_t arg8, uintptr_t arg9, uintptr_t arg10, uintptr_t arg11,
uintptr_t arg12, uintptr_t arg13, uintptr_t arg14, uintptr_t arg15,
uintptr_t arg16, uintptr_t arg17, uintptr_t arg18, uintptr_t arg19);
 */
/**Macro's used to count arguments passed.**/
#define _NUM_ARGS_EVALUATOR_(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11,   \
	_12, _13, _14, _15, _16, _17, _18, _19, _20, N, ...) N

#define _NUM_ARGS_COUNT_(...)						\
	_NUM_ARGS_EVALUATOR_(_0, ##__VA_ARGS__, 19, 18, 17, 16, 15, 14, 13, \
				 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)

#define _GET_MACRO_TZ_(name, n) name##n

#define _GET_MACRO_TZ(name, n) _GET_MACRO_TZ_(name, n)

#define GET_MACRO_TZ(macro, xx_prio, xx_sc, ...)			\
	_GET_MACRO_TZ(macro, _NUM_ARGS_COUNT_(__VA_ARGS__)) \
	(xx_prio, xx_sc, ##__VA_ARGS__)

/**
 * TZBSP macro's to log  messages to a ring buffer.
 * @param [in] xx_prio - Priority of the message to be logged
 * @param [in] xx_sc   - error code/status code.
 * @param [in] ...	 - Arguments, the arguments should not contain any strings.
 * Please note we support max 9 arguments to be printed.
 * The Macro  prints all the arguments in %lx format.
 */
#define TZBSP_LOG(xx_prio, xx_sc, ...) \
	GET_MACRO_TZ(TZBSP_LOG, xx_prio, xx_sc, ##__VA_ARGS__)

#define TZBSP_LOG_NAME(xx_prio, xx_sc, ...) \
	GET_MACRO_TZ(TZBSP_LOG_NAME, xx_prio, xx_sc, ##__VA_ARGS__)

#define TZBSP_LOG_NAME1(xx_prio, xx_sc, ...) \
	tzbsp_log(xx_prio, "(%x %s)", xx_sc, ##__VA_ARGS__);

#define TZBSP_LOG_NAME2(xx_prio, xx_sc, arg_str, arg0) \
	tzbsp_log(xx_prio, "(%x %s %lx)", xx_sc, arg_str, (uintptr_t)arg0);

#define TZBSP_LOG_NAME3(xx_prio, xx_sc, arg_str, arg0, arg1)		\
	tzbsp_log(xx_prio, "(%x %s %lx %lx)", xx_sc, arg_str, (uintptr_t)arg0, \
		  (uintptr_t)arg1);

#define TZBSP_LOG_NAME4(xx_prio, xx_sc, arg_str, arg0, arg1, arg2) \
	tzbsp_log(xx_prio, "(%x %s %lx %lx %lx)", xx_sc, arg_str,  \
		  (uintptr_t)arg0, (uintptr_t)arg1, (uintptr_t)arg2);

#define TZBSP_LOG_NAME5(xx_prio, xx_sc, arg_str, arg0, arg1, arg2, arg3) \
	tzbsp_log(xx_prio, "(%x %s %lx %lx %lx %lx)", xx_sc, arg_str,	\
		  (uintptr_t)arg0, (uintptr_t)arg1, (uintptr_t)arg2,	 \
		  (uintptr_t)arg3);

#define TZBSP_LOG_NAME6(xx_prio, xx_sc, arg_str, arg0, arg1, arg2, arg3, arg4) \
	tzbsp_log(xx_prio, "(%x %s %lx %lx %lx %lx %lx)", xx_sc, arg_str, \
		  (uintptr_t)arg0, (uintptr_t)arg1, (uintptr_t)arg2,	\
		  (uintptr_t)arg3, (uintptr_t)arg4);

#define TZBSP_LOG_NAME7(xx_prio, xx_sc, arg_str, arg0, arg1, arg2, arg3, arg4, \
			arg5)		\
	tzbsp_log(xx_prio, "(%x %s %lx %lx %lx %lx %lx %lx)", xx_sc, arg_str,  \
		  (uintptr_t)arg0, (uintptr_t)arg1, (uintptr_t)arg2,	\
		  (uintptr_t)arg3, (uintptr_t)arg4, (uintptr_t)arg5);

#define TZBSP_LOG_NAME8(xx_prio, xx_sc, arg_str, arg0, arg1, arg2, arg3, arg4, \
			arg5, arg6)	\
	tzbsp_log(xx_prio, "(%x %s %lx %lx %lx %lx %lx %lx %lx)", xx_sc, \
		  arg_str, (uintptr_t)arg0, (uintptr_t)arg1, (uintptr_t)arg2,  \
		  (uintptr_t)arg3, (uintptr_t)arg4, (uintptr_t)arg5,	\
		  (uintptr_t)arg6);

#define TZBSP_LOG_NAME9(xx_prio, xx_sc, arg_str, arg0, arg1, arg2, arg3, arg4, \
			arg5, arg6, arg7)	\
	tzbsp_log(xx_prio, "(%x %s %lx %lx %lx %lx %lx %lx %lx %lx)", xx_sc,   \
		  arg_str, (uintptr_t)arg0, (uintptr_t)arg1, (uintptr_t)arg2,  \
		  (uintptr_t)arg3, (uintptr_t)arg4, (uintptr_t)arg5,	\
		  (uintptr_t)arg6, (uintptr_t)arg7);

#define TZBSP_LOG_NAME10(xx_prio, xx_sc, arg_str, arg0, arg1, arg2, arg3, \
			 arg4, arg5, arg6, arg7, arg8)		\
	tzbsp_log(xx_prio, "(%x %s %lx %lx %lx %lx %lx %lx %lx %lx %lx)", \
		  xx_sc, arg_str, (uintptr_t)arg0, (uintptr_t)arg1,	   \
		  (uintptr_t)arg2, (uintptr_t)arg3, (uintptr_t)arg4,	  \
		  (uintptr_t)arg5, (uintptr_t)arg6, (uintptr_t)arg7,	  \
		  (uintptr_t)arg8);

#define TZBSP_LOG_NAME11(xx_prio, xx_sc, arg_str, arg0, arg1, arg2, arg3, \
			 arg4, arg5, arg6, arg7, arg8, arg9)		 \
	tzbsp_log(xx_prio, "(%x %s %lx %lx %lx %lx %lx %lx %lx %lx %lx %lx)", \
		  xx_sc, arg_str, (uintptr_t)arg0, (uintptr_t)arg1,	\
		  (uintptr_t)arg2, (uintptr_t)arg3, (uintptr_t)arg4,	\
		  (uintptr_t)arg5, (uintptr_t)arg6, (uintptr_t)arg7,	\
		  (uintptr_t)arg8, (uintptr_t)arg9);

#define TZBSP_LOG_NAME12(xx_prio, xx_sc, arg_str, arg0, arg1, arg2, arg3, \
			 arg4, arg5, arg6, arg7, arg8, arg9, arg10)	   \
	tzbsp_log(xx_prio,					\
		  "(%x %s %lx %lx %lx %lx %lx %lx %lx %lx %lx %lx %lx)",  \
		  xx_sc, arg_str, (uintptr_t)arg0, (uintptr_t)arg1,	   \
		  (uintptr_t)arg2, (uintptr_t)arg3, (uintptr_t)arg4,	  \
		  (uintptr_t)arg5, (uintptr_t)arg6, (uintptr_t)arg7,	  \
		  (uintptr_t)arg8, (uintptr_t)arg9, (uintptr_t)arg10);

#define TZBSP_LOG_NAME13(xx_prio, xx_sc, arg_str, arg0, arg1, arg2, arg3,  \
			 arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11) \
	tzbsp_log(xx_prio, "(%x %s %lx %lx %lx %lx %lx %lx %lx %lx %lx %lx %lx " \
		  "%lx)", xx_sc, arg_str, (uintptr_t)arg0, (uintptr_t)arg1, \
		  (uintptr_t)arg2, (uintptr_t)arg3, (uintptr_t)arg4,	   \
		  (uintptr_t)arg5, (uintptr_t)arg6, (uintptr_t)arg7,	   \
		  (uintptr_t)arg8, (uintptr_t)arg9, (uintptr_t)arg10,	  \
		  (uintptr_t)arg11);

#define TZBSP_LOG_NAME14(xx_prio, xx_sc, arg_str, arg0, arg1, arg2, arg3,  \
			 arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11, \
			 arg12)			\
	tzbsp_log(xx_prio, "(%x %s %lx %lx %lx %lx %lx %lx %lx %lx %lx %lx %lx " \
		  "%lx %lx)", xx_sc, arg_str, (uintptr_t)arg0, (uintptr_t)arg1,	\
		  (uintptr_t)arg2, (uintptr_t)arg3, (uintptr_t)arg4,	   \
		  (uintptr_t)arg5, (uintptr_t)arg6, (uintptr_t)arg7,	   \
		  (uintptr_t)arg8, (uintptr_t)arg9, (uintptr_t)arg10,	  \
		  (uintptr_t)arg11, (uintptr_t)arg12);

#define TZBSP_LOG_NAME15(xx_prio, xx_sc, arg_str, arg0, arg1, arg2, arg3,  \
			 arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11, \
			 arg12, arg13)				\
	tzbsp_log(xx_prio, "(%x %s %lx %lx %lx %lx %lx %lx %lx %lx %lx %lx %lx" \
		  "  %lx %lx %lx)",					\
		  xx_sc, arg_str, (uintptr_t)arg0, (uintptr_t)arg1,	\
		  (uintptr_t)arg2, (uintptr_t)arg3, (uintptr_t)arg4,	   \
		  (uintptr_t)arg5, (uintptr_t)arg6, (uintptr_t)arg7,	   \
		  (uintptr_t)arg8, (uintptr_t)arg9, (uintptr_t)arg10,	  \
		  (uintptr_t)arg11, (uintptr_t)arg12, (uintptr_t)arg13);

#define TZBSP_LOG_NAME16(xx_prio, xx_sc, arg_str, arg0, arg1, arg2, arg3,  \
			 arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11, \
			 arg12, arg13, arg14)				 \
	tzbsp_log(xx_prio, "(%x %s %lx %lx %lx %lx %lx %lx %lx %lx %lx %lx %lx" \
		  " %lx %lx %lx %lx)",	\
		  xx_sc, arg_str, (uintptr_t)arg0, (uintptr_t)arg1, \
		  (uintptr_t)arg2, (uintptr_t)arg3, (uintptr_t)arg4,	   \
		  (uintptr_t)arg5, (uintptr_t)arg6, (uintptr_t)arg7,	   \
		  (uintptr_t)arg8, (uintptr_t)arg9, (uintptr_t)arg10,	  \
		  (uintptr_t)arg11, (uintptr_t)arg12, (uintptr_t)arg13,	\
		  (uintptr_t)arg14);

#define TZBSP_LOG_NAME17(xx_prio, xx_sc, arg_str, arg0, arg1, arg2, arg3, \
			 arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11, \
			 arg12, arg13, arg14, arg15)			\
	tzbsp_log(xx_prio, "(%x %s %lx %lx %lx %lx %lx %lx %lx %lx %lx %lx %lx" \
	" %lx %lx %lx %lx %lx)",	\
	 xx_sc, arg_str, (uintptr_t)arg0, (uintptr_t)arg1, (uintptr_t)arg2, \
	(uintptr_t)arg3, (uintptr_t)arg4, (uintptr_t)arg5, (uintptr_t)arg6, \
	(uintptr_t)arg7, (uintptr_t)arg8, (uintptr_t)arg9, (uintptr_t)arg10,\
	(uintptr_t)arg11, (uintptr_t)arg12, (uintptr_t)arg13,	\
	(uintptr_t)arg14, (uintptr_t)arg15);

#define TZBSP_LOG_NAME18(xx_prio, xx_sc, arg_str, arg0, arg1, arg2, arg3,  \
			 arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11, \
			 arg12, arg13, arg14, arg15, arg16)	\
	tzbsp_log(xx_prio,					\
		  "(%x %s %lx %lx %lx %lx %lx %lx %lx %lx %lx %lx %lx %lx" \
		  " %lx %lx %lx %lx %lx)",		 \
		  xx_sc, arg_str, (uintptr_t)arg0, (uintptr_t)arg1,	\
		  (uintptr_t)arg2, (uintptr_t)arg3, (uintptr_t)arg4,	   \
		  (uintptr_t)arg5, (uintptr_t)arg6, (uintptr_t)arg7,	   \
		  (uintptr_t)arg8, (uintptr_t)arg9, (uintptr_t)arg10,	  \
		  (uintptr_t)arg11, (uintptr_t)arg12, (uintptr_t)arg13,	\
		  (uintptr_t)arg14, (uintptr_t)arg15, (uintptr_t)arg16);

#define TZBSP_LOG0(xx_prio, xx_sc, ...) tzbsp_log(xx_prio, "(%x)", xx_sc);

#define TZBSP_LOG1(xx_prio, xx_sc, ...) \
	tzbsp_log(xx_prio, "(%x %lx)", xx_sc, ##__VA_ARGS__);

#define TZBSP_LOG2(xx_prio, xx_sc, ...) \
	tzbsp_log(xx_prio, "(%x %lx %lx)", xx_sc, ##__VA_ARGS__);

#define TZBSP_LOG3(xx_prio, xx_sc, ...) \
	tzbsp_log(xx_prio, "(%x %lx %lx %lx)", xx_sc, ##__VA_ARGS__);

#define TZBSP_LOG4(xx_prio, xx_sc, ...) \
	tzbsp_log(xx_prio, "(%x %lx %lx %lx %lx)", xx_sc, ##__VA_ARGS__);

#define TZBSP_LOG5(xx_prio, xx_sc, ...) \
	tzbsp_log(xx_prio, "(%x %lx %lx %lx %lx %lx)", xx_sc, ##__VA_ARGS__);

#define TZBSP_LOG6(xx_prio, xx_sc, ...)		\
	tzbsp_log(xx_prio, "(%x %lx %lx %lx %lx %lx %lx)", xx_sc, \
		  ##__VA_ARGS__);

#define TZBSP_LOG7(xx_prio, xx_sc, ...)	 \
	tzbsp_log(xx_prio, "(%x %lx %lx %lx %lx %lx %lx %lx)", xx_sc, \
		  ##__VA_ARGS__);

#define TZBSP_LOG8(xx_prio, xx_sc, ...)	 \
	tzbsp_log(xx_prio, "(%x %lx %lx %lx %lx %lx %lx %lx %lx)", xx_sc, \
		  ##__VA_ARGS__);

#define TZBSP_LOG9(xx_prio, xx_sc, ...)		\
	tzbsp_log(xx_prio, "(%x %lx %lx %lx %lx %lx %lx %lx %lx %lx)", xx_sc, \
		  ##__VA_ARGS__);

#define TZBSP_LOG10(xx_prio, xx_sc, ...)	\
	tzbsp_log(xx_prio, "(%x %lx %lx %lx %lx %lx %lx %lx %lx %lx %lx)", \
		  xx_sc, ##__VA_ARGS__);

#define TZBSP_LOG11(xx_prio, xx_sc, ...)	 \
	tzbsp_log(xx_prio, "(%x %lx %lx %lx %lx %lx %lx %lx %lx %lx %lx %lx)", \
		  xx_sc, ##__VA_ARGS__);

#define TZBSP_LOG12(xx_prio, xx_sc, ...)	\
	tzbsp_log(xx_prio,			\
		  "(%x %lx %lx %lx %lx %lx %lx %lx %lx %lx %lx %lx %lx)", \
		  xx_sc, ##__VA_ARGS__);

#define TZBSP_LOG13(xx_prio, xx_sc, ...)	\
	tzbsp_log(xx_prio,	\
		  "(%x %lx %lx %lx %lx %lx %lx %lx %lx %lx %lx %lx %lx %lx)", \
		  xx_sc, ##__VA_ARGS__);

#define TZBSP_LOG14(xx_prio, xx_sc, ...)	\
	tzbsp_log(xx_prio,	\
		"(%x %lx %lx %lx %lx %lx %lx %lx %lx %lx %lx %lx %lx %lx %lx)", \
		xx_sc, ##__VA_ARGS__);

#define TZBSP_LOG15(xx_prio, xx_sc, ...)	\
	tzbsp_log(xx_prio,		\
		"(%x %lx %lx %lx %lx %lx %lx %lx %lx %lx %lx %lx %lx %lx %lx %lx)", \
		xx_sc, ##__VA_ARGS__);

#define TZBSP_LOG16(xx_prio, xx_sc, ...)		\
	tzbsp_log(xx_prio,						\
		"(%x %lx %lx %lx %lx %lx %lx %lx %lx %lx %lx %lx %lx %lx %lx %lx %lx)", \
		xx_sc, ##__VA_ARGS__);

#define TZBSP_LOG17(xx_prio, xx_sc, ...)	\
	tzbsp_log(xx_prio,						\
		"(%x %lx %lx %lx %lx %lx %lx %lx %lx %lx %lx %lx %lx %lx %lx %lx %lx %lx)", \
		xx_sc, ##__VA_ARGS__);

#define TZBSP_LOG18(xx_prio, xx_sc, ...)	\
	tzbsp_log(xx_prio,						\
		"(%x %lx %lx %lx %lx %lx %lx %lx %lx %lx %lx %lx %lx %lx %lx %lx %lx %lx %lx)", \
		xx_sc, ##__VA_ARGS__);

#define TZBSP_LOG19(xx_prio, xx_sc, ...)		\
	tzbsp_log(xx_prio,						\
		"(%x %lx %lx %lx %lx %lx %lx %lx %lx %lx %lx %lx %lx %lx %lx %lx %lx %lx %lx %lx)", \
		xx_sc, ##__VA_ARGS__);


/**
 * TFA_LOG macros to log  messages to a ring buffer in TFA.
 * @param [in] xx_prio - Priority of the message to be logged
 * @param [in] xx_sc   - error code/status code.
 * @param [in] ...	 - Arguments.
 * Please note we support max 12 arguments to be printed. Additional arguments
 * must be enabled by adding the corresponding helper function.
 * The Macro  prints all the arguments in %lx format.

 * Developers should use TFA_LOG_* macros to abstract the priority.
 */

#define _GET_MACRO_TFA_(name, n) name##n

#define _GET_MACRO_TFA(name, n) _GET_MACRO_TFA_(name, n)

#define GET_MACRO_TFA(macro, xx_prio, xx_sc, ...)		 \
	_GET_MACRO_TFA(macro, _NUM_ARGS_COUNT_(__VA_ARGS__)) \
	(xx_prio, xx_sc, ##__VA_ARGS__)

#define TFA_LOG_ERR(xx_sc, ...) \
	TFA_LOG(TZBSP_MSG_ERROR, xx_sc, ##__VA_ARGS__)

#define TFA_LOG_DBG(xx_sc, ...) \
	TFA_LOG(TZBSP_MSG_DEBUG, xx_sc, ##__VA_ARGS__)

#define TFA_LOG_NAME_ERR(xx_sc, ...) \
	TFA_LOG(TZBSP_MSG_ERROR, xx_sc, ##__VA_ARGS__)

#define TFA_LOG_NAME_DBG(xx_sc, ...) \
	TFA_LOG(TZBSP_MSG_DEBUG, xx_sc, ##__VA_ARGS__)

#define TFA_LOG_ERR_FATAL(xx_sc, ...) \
	TFA_LOG(TZBSP_MSG_FATAL, xx_sc, ##__VA_ARGS__)

#define TFA_LOG_NAME_ERR_FATAL(xx_sc, ...) \
	TFA_LOG(TZBSP_MSG_FATAL, xx_sc, ##__VA_ARGS__)

#define TFA_LOG(xx_prio, xx_sc, ...) \
	GET_MACRO_TFA(TFA_LOG, xx_prio, xx_sc, ##__VA_ARGS__)

#define TFA_LOG0(xx_prio, xx_sc) \
	tfa_log_0(xx_prio, xx_sc)

#define TFA_LOG1(xx_prio, xx_sc, arg0) \
	tfa_log_1(xx_prio, xx_sc, (uintptr_t)(arg0))

#define TFA_LOG2(xx_prio, xx_sc, arg0, arg1) \
	tfa_log_2(xx_prio, xx_sc, (uintptr_t)(arg0), (uintptr_t)(arg1))

#define TFA_LOG3(xx_prio, xx_sc, arg0, arg1, arg2) \
	tfa_log_3(xx_prio, xx_sc, (uintptr_t)(arg0), (uintptr_t)(arg1), \
	(uintptr_t)(arg2))

#define TFA_LOG4(xx_prio, xx_sc, arg0, arg1, arg2, arg3) \
	tfa_log_4(xx_prio, xx_sc, (uintptr_t)(arg0), (uintptr_t)(arg1), \
	(uintptr_t)(arg2), (uintptr_t)(arg3))

#define TFA_LOG5(xx_prio, xx_sc, arg0, arg1, arg2, arg3, arg4) \
	tfa_log_5(xx_prio, xx_sc, (uintptr_t)(arg0), (uintptr_t)(arg1), \
	(uintptr_t)(arg2), (uintptr_t)(arg3), (uintptr_t)(arg4))

#define TFA_LOG6(xx_prio, xx_sc, arg0, arg1, arg2, arg3, arg4, arg5) \
	tfa_log_6(xx_prio, xx_sc, (uintptr_t)(arg0), (uintptr_t)(arg1), \
	(uintptr_t)(arg2), (uintptr_t)(arg3), (uintptr_t)(arg4), (uintptr_t)(arg5))

#define TFA_LOG7(xx_prio, xx_sc, arg0, arg1, arg2, arg3, arg4, arg5, arg6) \
	tfa_log_7(xx_prio, xx_sc, (uintptr_t)(arg0), (uintptr_t)(arg1), \
	(uintptr_t)(arg2), (uintptr_t)(arg3), (uintptr_t)(arg4), (uintptr_t)(arg5), \
	(uintptr_t)(arg6))

#define TFA_LOG8(xx_prio, xx_sc, arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7) \
	tfa_log_8(xx_prio, xx_sc, (uintptr_t)(arg0), (uintptr_t)(arg1), \
	(uintptr_t)(arg2), (uintptr_t)(arg3), (uintptr_t)(arg4), (uintptr_t)(arg5), \
	(uintptr_t)(arg6), (uintptr_t)(arg7))

#define TFA_LOG9(xx_prio, xx_sc, arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8) \
	tfa_log_9(xx_prio, xx_sc, (uintptr_t)(arg0), (uintptr_t)(arg1), \
	(uintptr_t)(arg2), (uintptr_t)(arg3), (uintptr_t)(arg4), (uintptr_t)(arg5), \
	(uintptr_t)(arg6), (uintptr_t)(arg7), (uintptr_t)(arg8))

#define TFA_LOG10(xx_prio, xx_sc, arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, \
	arg8, arg9) \
	tfa_log_10(xx_prio, xx_sc, (uintptr_t)(arg0), (uintptr_t)(arg1), \
	(uintptr_t)(arg2), (uintptr_t)(arg3), (uintptr_t)(arg4), (uintptr_t)(arg5), \
	(uintptr_t)(arg6), (uintptr_t)(arg7), (uintptr_t)(arg8), (uintptr_t)(arg9))

#define TFA_LOG11(xx_prio, xx_sc, arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, \
	arg8, arg9, arg10) \
	tfa_log_11(xx_prio, xx_sc, (uintptr_t)(arg0), (uintptr_t)(arg1), \
	(uintptr_t)(arg2), (uintptr_t)(arg3), (uintptr_t)(arg4), (uintptr_t)(arg5), \
	(uintptr_t)(arg6), (uintptr_t)(arg7), (uintptr_t)(arg8), (uintptr_t)(arg9), \
	(uintptr_t)(arg10))

#define TFA_LOG12(xx_prio, xx_sc, arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, \
	arg8, arg9, arg10, arg11) \
	tfa_log_12(xx_prio, xx_sc, (uintptr_t)(arg0), (uintptr_t)(arg1), \
	(uintptr_t)(arg2), (uintptr_t)(arg3), (uintptr_t)(arg4), (uintptr_t)(arg5), \
	(uintptr_t)(arg6), (uintptr_t)(arg7), (uintptr_t)(arg8), (uintptr_t)(arg9), \
	(uintptr_t)(arg10), (uintptr_t)(arg11))

#define TFA_LOG13(xx_prio, xx_sc, arg0, arg1, arg2, arg3, arg4, arg5, arg6, \
	arg7, arg8, arg9, arg10, arg11, arg12) \
	tfa_log_13(xx_prio, xx_sc, (uintptr_t)(arg0), (uintptr_t)(arg1), \
	(uintptr_t)(arg2), (uintptr_t)(arg3), (uintptr_t)(arg4), (uintptr_t)(arg5), \
	(uintptr_t)(arg6), (uintptr_t)(arg7), (uintptr_t)(arg8), (uintptr_t)(arg9), \
	(uintptr_t)(arg10), (uintptr_t)(arg11), (uintptr_t)(arg12))

#define TFA_LOG14(xx_prio, xx_sc, arg0, arg1, arg2, arg3, arg4, arg5, arg6, \
	arg7, arg8, arg9, arg10, arg11, arg12, arg13) \
	tfa_log_14(xx_prio, xx_sc, (uintptr_t)(arg0), (uintptr_t)(arg1), \
	(uintptr_t)(arg2), (uintptr_t)(arg3), (uintptr_t)(arg4), (uintptr_t)(arg5), \
	(uintptr_t)(arg6), (uintptr_t)(arg7), (uintptr_t)(arg8), (uintptr_t)(arg9), \
	(uintptr_t)(arg10), (uintptr_t)(arg11), (uintptr_t)(arg12), (uintptr_t)(arg13))

#define TFA_LOG15(xx_prio, xx_sc, arg0, arg1, arg2, arg3, arg4, arg5, arg6, \
	arg7, arg8, arg9, arg10, arg11, arg12, arg13, arg14) \
	tfa_log_15(xx_prio, xx_sc, (uintptr_t)(arg0), (uintptr_t)(arg1), \
	(uintptr_t)(arg2), (uintptr_t)(arg3), (uintptr_t)(arg4), (uintptr_t)(arg5), \
	(uintptr_t)(arg6), (uintptr_t)(arg7), (uintptr_t)(arg8), (uintptr_t)(arg9), \
	(uintptr_t)(arg10), (uintptr_t)(arg11), (uintptr_t)(arg12), (uintptr_t)(arg13), \
	(uintptr_t)(arg14))

#define TFA_LOG16(xx_prio, xx_sc, arg0, arg1, arg2, arg3, arg4, arg5, arg6, \
	arg7, arg8, arg9, arg10, arg11, arg12, arg13, arg14, arg15) \
	tfa_log_16(xx_prio, xx_sc, (uintptr_t)(arg0), (uintptr_t)(arg1), \
	(uintptr_t)(arg2), (uintptr_t)(arg3), (uintptr_t)(arg4), (uintptr_t)(arg5), \
	(uintptr_t)(arg6), (uintptr_t)(arg7), (uintptr_t)(arg8), (uintptr_t)(arg9), \
	(uintptr_t)(arg10), (uintptr_t)(arg11), (uintptr_t)(arg12), (uintptr_t)(arg13), \
	(uintptr_t)(arg14), (uintptr_t)(arg15))

/*
#define TFA_LOG17(xx_prio, xx_sc, arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11, arg12, arg13, arg14, arg15, arg16) \
	tfa_log_17(xx_prio, xx_sc, (uintptr_t)(arg0), (uintptr_t)(arg1), (uintptr_t)(arg2), (uintptr_t)(arg3), (uintptr_t)(arg4), (uintptr_t)(arg5), (uintptr_t)(arg6), (uintptr_t)(arg7), (uintptr_t)(arg8), (uintptr_t)(arg9), (uintptr_t)(arg10), (uintptr_t)(arg11), (uintptr_t)(arg12), (uintptr_t)(arg13), (uintptr_t)(arg14), (uintptr_t)(arg15), (uintptr_t)(arg16))

#define TFA_LOG18(xx_prio, xx_sc, arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11, arg12, arg13, arg14, arg15, arg16, arg17) \
	tfa_log_18(xx_prio, xx_sc, (uintptr_t)(arg0), (uintptr_t)(arg1), (uintptr_t)(arg2), (uintptr_t)(arg3), (uintptr_t)(arg4), (uintptr_t)(arg5), (uintptr_t)(arg6), (uintptr_t)(arg7), (uintptr_t)(arg8), (uintptr_t)(arg9), (uintptr_t)(arg10), (uintptr_t)(arg11), (uintptr_t)(arg12), (uintptr_t)(arg13), (uintptr_t)(arg14), (uintptr_t)(arg15), (uintptr_t)(arg16), (uintptr_t)(arg17))

#define TFA_LOG19(xx_prio, xx_sc, arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11, arg12, arg13, arg14, arg15, arg16, arg17, arg18) \
	tfa_log_19(xx_prio, xx_sc, (uintptr_t)(arg0), (uintptr_t)(arg1), (uintptr_t)(arg2), (uintptr_t)(arg3), (uintptr_t)(arg4), (uintptr_t)(arg5), (uintptr_t)(arg6), (uintptr_t)(arg7), (uintptr_t)(arg8), (uintptr_t)(arg9), (uintptr_t)(arg10), (uintptr_t)(arg11), (uintptr_t)(arg12), (uintptr_t)(arg13), (uintptr_t)(arg14), (uintptr_t)(arg15), (uintptr_t)(arg16), (uintptr_t)(arg17), (uintptr_t)(arg18))
 */



#include "tme_log_ids.h"

#endif /* TZBSP_LOG_H */
