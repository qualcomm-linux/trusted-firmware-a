/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef QTI_ERR_LOG_H
#define QTI_ERR_LOG_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <lib/spinlock.h>

#define QTI_ERR_LOG_MAX_REFS	16U

typedef struct qti_err_log_ref {
	/* Direct pointer into rb_buffer at log entry start. */
	uint8_t *addr;
	/* Byte count of this log entry. */
	uint32_t len;
} qti_err_log_ref_t;

typedef struct qti_err_log {
	/* Circular array of captured ERROR log entry references. */
	qti_err_log_ref_t refs[QTI_ERR_LOG_MAX_REFS];
	/* Parallel array used by the stale check. */
	uint64_t total_written_at[QTI_ERR_LOG_MAX_REFS];
	/* Index of the next slot to write. */
	uint32_t write_idx;
	/* Total ERROR entries captured since boot. */
	uint32_t total;
	/* Count of bytes written to the ringbuffer via ERROR entries. */
	uint64_t total_written;
	/* Ringbuffer position at the start of the entry being recorded. */
	uint8_t *rb_pos_before;
	/* True while an ERROR entry is in flight. */
	bool is_recording;
	/* Protects all fields above. */
	spinlock_t lock;
} qti_err_log_t;

/*
 * Finalize the in-progress ERROR entry recording. Must be called with
 * g_qti_err_log.lock held and g_qti_err_log.is_recording == true.
 */
void qti_err_finalize_recording(void);

extern qti_err_log_t g_qti_err_log;

#endif /* QTI_ERR_LOG_H */
