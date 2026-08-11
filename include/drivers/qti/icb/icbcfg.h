/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Public API for the QTI ICB configuration driver.
 */

#ifndef QTI_ICBCFG_H
#define QTI_ICBCFG_H

/* Apply one-time ICB register configuration (before remap). */
void qti_icbcfg_init(void);

/* Apply post-remap ICB register configuration. */
void icbcfg_post_init(void);

#endif /* QTI_ICBCFG_H */
