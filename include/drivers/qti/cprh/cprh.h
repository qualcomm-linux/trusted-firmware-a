/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef QTI_CPRH_H
#define QTI_CPRH_H

#ifdef QTI_CPRH_ENABLED
void qti_cprh_init(void);
#else
static inline void qti_cprh_init(void)
{
}
#endif

#endif /* QTI_CPRH_H */
