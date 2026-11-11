/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SYSDBG_LOCK_H
#define SYSDBG_LOCK_H

#include <lib/spinlock.h>

void sysdbg_lock_obtain(spinlock_t *lock);
void sysdbg_lock_release(spinlock_t *lock);

#endif /* SYSDBG_LOCK_H */
