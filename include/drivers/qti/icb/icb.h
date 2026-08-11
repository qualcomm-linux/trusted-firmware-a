/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Top-level init for the QTI ICB drivers (micro-arbiter, NoC error
 * handler, configuration).
 */

#ifndef QTI_ICB_H
#define QTI_ICB_H

/*
 * Initialise all QTI ICB drivers. Call once from BL31 platform setup; this
 * is the only ICB entry point bl31_platform_setup() should invoke.
 */
#ifdef QTI_ICB_ENABLED
void qti_icb_init(void);
#else
static inline void qti_icb_init(void) {}
#endif

#endif /* QTI_ICB_H */
