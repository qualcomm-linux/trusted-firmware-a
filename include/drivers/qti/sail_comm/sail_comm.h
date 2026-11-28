/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef QTI_SAIL_COMM_H
#define QTI_SAIL_COMM_H

#ifdef QTI_SAIL_COMM

/* Notify a companion SAIL die before the platform powers off or resets. */
void qti_sail_comm_system_off(void);
void qti_sail_comm_system_reset(void);
void qti_sail_comm_system_warm_reset(void);

#else

static inline void qti_sail_comm_system_off(void) {}
static inline void qti_sail_comm_system_reset(void) {}
static inline void qti_sail_comm_system_warm_reset(void) {}

#endif /* QTI_SAIL_COMM */

#endif /* QTI_SAIL_COMM_H */
