/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * RPMh lifecycle interface.
 */

#ifndef QTI_RPMH_H
#define QTI_RPMH_H

#ifdef QTI_RPMH_ENABLED
void rpmh_client_init(void);
void rpmh_client_deinit(void);
#else
static inline void rpmh_client_init(void) {}
static inline void rpmh_client_deinit(void) {}
#endif

#endif /* QTI_RPMH_H */
