/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef QTI_TIMER_H
#define QTI_TIMER_H

#include <stdbool.h>
#include <drivers/qti/timer/tzbsp_timer.h>

/*
 * Register platform interrupt callbacks and cold-boot state before calling
 * qti_timer_init(). Must be called once per boot from EL3 plat code.
 *
 * plat_ops:           Platform interrupt operation callbacks.
 * needs_frame_config: true on cold boot or quick boot (frame security
 *                     registers must be (re-)programmed); false on warm boot.
 */
void qti_timer_plat_register(const timer_plat_ops_t *plat_ops,
			      bool needs_frame_config);

/*
 * Initialize the QTI secure QTimer hardware.
 * Must be called after qti_timer_plat_register().
 */
void qti_timer_init(void);

/*
 * Initialize the generic delay timer using the QTI QTimer as the tick source.
 * Must be called after qti_timer_init().
 */
void qti_delay_timer_init(void);

#endif /* QTI_TIMER_H */
