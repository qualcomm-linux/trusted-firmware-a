/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <lib/spinlock.h>

#include <sysdbg_lock.h>

/*
 * No-op, matching src_repo: "For all existing TFA targets SDI runs on
 * only single core so we don't need spinlock api" (CR4390719 tracks
 * moving to a HW mutex where needed). Ported as-is, not fixed.
 */
void sysdbg_lock_obtain(spinlock_t *lock)
{
	(void)lock;
}

void sysdbg_lock_release(spinlock_t *lock)
{
	(void)lock;
}
