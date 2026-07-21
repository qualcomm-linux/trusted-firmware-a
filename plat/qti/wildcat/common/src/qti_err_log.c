/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdbool.h>
#include <stdint.h>

#include <common/debug.h>
#include <lib/spinlock.h>

#include <bl31qtilib_interface.h>
#include <qti_err_log.h>

qti_err_log_t g_qti_err_log;

void qti_err_finalize_recording(void)
{
	uint32_t rb_pos_now = (uint32_t)g_qti_bl31_ringbuf_ptr->wo_cbuf.head;
	uint32_t rb_sz = (uint32_t)g_qti_bl31_ringbuf_ptr->conf.buf_size;
	uint8_t *rb_buf = g_qti_bl31_ringbuf_ptr->wo_cbuf.buf;
	uint32_t before_idx =
		(uint32_t)(g_qti_err_log.rb_pos_before - rb_buf);
	uint32_t len = (rb_pos_now - before_idx + rb_sz) % rb_sz;

	if (len > 0U) {
		uint32_t idx = g_qti_err_log.write_idx %
			       QTI_ERR_LOG_MAX_REFS;

		g_qti_err_log.refs[idx].addr = g_qti_err_log.rb_pos_before;
		g_qti_err_log.refs[idx].len = len;
		g_qti_err_log.total_written += len;
		g_qti_err_log.total_written_at[idx] =
			g_qti_err_log.total_written;
		g_qti_err_log.write_idx++;
		g_qti_err_log.total++;
	}

	g_qti_err_log.is_recording = false;
}

/*
 * Strong override of the weak plat_log_get_prefix() from
 * plat/common/plat_log_common.c. Called once per log entry by tf_log()
 * before vprintf writes the entry to the console.
 */
const char *plat_log_get_prefix(unsigned int log_level)
{
	static const char * const plat_prefix_str[] = {
		"ERROR:   ",
		"NOTICE:  ",
		"WARNING: ",
		"INFO:    ",
		"VERBOSE: "
	};
	unsigned int level;
	bool use_lock = bl31qtilib_is_cold_boot_done();

	if (use_lock) {
		spin_lock(&g_qti_err_log.lock);
	}

	if (g_qti_err_log.is_recording) {
		qti_err_finalize_recording();
	}

	if (log_level == LOG_LEVEL_ERROR) {
		g_qti_err_log.rb_pos_before =
			&g_qti_bl31_ringbuf_ptr->wo_cbuf.buf[
				g_qti_bl31_ringbuf_ptr->wo_cbuf.head];
		g_qti_err_log.is_recording = true;
	}

	if (use_lock) {
		spin_unlock(&g_qti_err_log.lock);
	}

	if (log_level < LOG_LEVEL_ERROR) {
		level = LOG_LEVEL_ERROR;
	} else if (log_level > LOG_LEVEL_VERBOSE) {
		level = LOG_LEVEL_VERBOSE;
	} else {
		level = log_level;
	}

	return plat_prefix_str[(level / 10U) - 1U];
}
