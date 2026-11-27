/*
 * Copyright (c) 2026, Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SYSDBG_H
#define SYSDBG_H

/* Return codes */
#define SYSDBG_SUCCESS	0
#define SYSDBG_ERROR	-1

/* System Debug Image (SDI) entry points used by the platform code. */

/*
 * system_debug_main() - SDI cold boot initialization.
 * Returns SYSDBG_SUCCESS (0) on success, SYSDBG_ERROR (-1) on failure.
 */
int system_debug_main(void);

/*
 * sysdbg_reset_check() - called early from plat_reset_handler.
 * Returns non-zero if this reset is an SDI debug (crash) reset.
 */
unsigned int sysdbg_reset_check(void);

/*
 * sysdbg_entry_handler() - SDI crash-reset entry; does not return.
 */
void sysdbg_entry_handler(void);

#endif /* SYSDBG_H */
