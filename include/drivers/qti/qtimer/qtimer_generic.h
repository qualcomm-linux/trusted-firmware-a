/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef QTIMER_GENERIC_H
#define QTIMER_GENERIC_H

#include <stdbool.h>
#include <stdint.h>

/*
 * Internal hooks between the family-agnostic core driver (qtimer.c) and the
 * per-family platform timer service / NCC extension. Not part of the public
 * qtimer.h API and not for use outside the qtimer driver itself.
 */

/* Implemented once per platform family in plat/qti/<family>/common/src. */
void qti_qtimer_platform_init(void);

/* Implemented in qtimer_ncc.c; compiled in only under QTI_USE_NCC_QTIMER. */
void qti_qtimer_ncc_init(void);

/*
 * Generic one-shot timer identifiers dispatched by qtimer.c. TID_CP15 and
 * TID_QTMR (frame 2) exist on every target; the FR3/FR4/FR5 tids are only
 * considered valid on NCC targets, as enforced by
 * qti_qtimer_is_tid_valid().
 */
typedef enum {
	QTI_TID_CP15 = 0,
	QTI_TID_QTMR = 1,
	QTI_TID_QTMR_FR3 = 2,
	QTI_TID_QTMR_FR4 = 3,
	QTI_TID_QTMR_FR5 = 4,
	QTI_TID_MAX,
} qti_qtimer_id_t;

/* Single-argument ISR type shared with the qgic shim (see qgic.h). */
typedef void *(*qti_qtimer_isr_t)(void *ctx);

/*
 * Generic dispatch API (qtimer.c). Frame register layout (CNTP_CTL/CNTPCT/
 * CNTP_CVAL) is common ARM generic-timer-frame geometry, not chipset
 * specific, so only frame base addresses, offsets, and interrupt IDs come
 * from qtimer_defs.h.
 */
bool qti_qtimer_is_tid_valid(qti_qtimer_id_t tid);
int qti_qtimer_one_shot_start(qti_qtimer_id_t tid, uint64_t timeout_us);
int qti_qtimer_stop(qti_qtimer_id_t tid);
uint32_t qti_qtimer_get_secure_int_id(qti_qtimer_id_t tid);
int qti_qtimer_install_isr(qti_qtimer_id_t tid, qti_qtimer_isr_t fn, void *ctx);
int qti_qtimer_enable_int(qti_qtimer_id_t tid);
int qti_qtimer_disable_int(qti_qtimer_id_t tid);

#endif /* QTIMER_GENERIC_H */
