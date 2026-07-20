/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef QTI_TIMER_H
#define QTI_TIMER_H

/*
 * Initialize the QTI secure QTimer hardware.
 * Configures frame security access, CNTFRQ, and NCC sleep timers.
 * Must be called on every boot (cold and warm) from EL3.
 */
void qti_timer_init(void);

/*
 * Initialize the generic delay timer using the QTI QTimer as the tick source.
 * Must be called after qti_timer_init().
 */
void qti_delay_timer_init(void);

#endif /* QTI_TIMER_H */
