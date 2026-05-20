/*
 * Copyright (c) 2025, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef DEBUG_CFG_H
#define DEBUG_CFG_H

#include <stdbool.h>
#include <common/ep_info.h>

#if PLAT_RUNTIME_DEBUG_CFG

/*
 * Query whether trace is permitted for the given security state.
 *
 * @security_state: SECURE or NON_SECURE
 * @return true if tracing is permitted, false otherwise.
 */
bool plat_trace_enabled(int security_state);

/*
 * Query whether Non-secure invasive (external halting) debug access is enabled.
 *
 * @return true if external debug access is permitted, false otherwise.
 */
bool plat_external_debug_access_enabled();

/*
 * Query whether performance monitor access is permitted for the given
 * security state. Controls MDCR_EL3 bits for SECURE and PMUSERENR_EL0
 * for NON_SECURE.
 *
 * @security_state: SECURE or NON_SECURE
 * @return true if performance monitor access is permitted, false otherwise.
 */
bool plat_perfmon_enabled(int security_state);

#else /* PLAT_RUNTIME_DEBUG_CFG=0 */


static inline bool plat_trace_enabled(int security_state)
{
	switch (security_state) {
		case SECURE:
			return false;
		case NON_SECURE:
			return true;
		default:
			return false;
	}
}

static inline bool plat_external_debug_access_enabled()
{
	return true;
}

static inline bool plat_perfmon_enabled(int security_state)
{
	switch (security_state) {
		case SECURE:
			return false;
		case NON_SECURE:
			return true;
		default:
			return false;
	}
}

#endif /* PLAT_RUNTIME_DEBUG_CFG */

#endif /* DEBUG_CFG_H */
