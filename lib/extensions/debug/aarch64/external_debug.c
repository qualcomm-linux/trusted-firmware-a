/*
 * Copyright (c) 2025, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <arch_features.h>
#include <arch_helpers.h>
#include <lib/el3_runtime/context_mgmt.h>
#include <lib/extensions/trf.h>

/*
 * MDCR_EL3.EDAD (bit 20):
 *
 * When FEAT_RME is implemented:
 * 	External Debug Access Disable. Together with MDCR_EL3.EDADE, controls access to breakpoint
 *	registers, watchpoint registers, and OSLAR_EL1 by an external debugger.
 *
 * When FEAT_Debugv8p4 is implemented:
 *	External Debug Access Disable. Controls Non-secure access to breakpoint registers, watchpoint registers,
 *	and OSLAR_EL1 by an external debugger.
 *	0b0: No accesses from an external debugger to the debug registers are prohibited by this control.
 *	0b1: Non-secure accesses from an external debugger to the affected debug registers are prohibited.
 * When FEAT_Debugv8p2 is implemented:
 *	External Debug Access Disable. Controls access to breakpoint registers, watchpoint registers, and
 *	OSLAR_EL1 by an external debugger.
 *	0b0: No accesses from an external debugger to the debug registers are prohibited by this control.
 *	0b1: If the IMPLEMENTATION DEFINED authentication interface function
 *	     ExternalSecureInvasiveDebugEnabled() returns FALSE, then accesses from an external debugger
 *	     to the affected debug registers are prohibited.
 * Otherwise:
 *	External Debug Access Disable. Controls access to breakpoint registers, watchpoint registers, and optionally
 *	OSLAR_EL1 by an external debugger.
 *	0b0: No accesses from an external debugger to the debug registers are prohibited by this control.
 *	0b1: If the IMPLEMENTATION DEFINED authentication interface function
 *	     ExternalSecureInvasiveDebugEnabled() returns FALSE, then accesses from an external debugger
 *	     to the affected debug registers are prohibited.
 */

void disable_external_debug_access(cpu_context_t *ctx)
{
	el3_state_t *state = get_el3state_ctx(ctx);
	u_register_t mdcr_el3 = read_ctx_reg(state, CTX_MDCR_EL3);

	mdcr_el3 |= MDCR_EDAD_BIT;
	write_ctx_reg(state, CTX_MDCR_EL3, mdcr_el3);
}

void enable_external_debug_access(cpu_context_t *ctx)
{
	el3_state_t *state = get_el3state_ctx(ctx);
	u_register_t mdcr_el3 = read_ctx_reg(state, CTX_MDCR_EL3);

	mdcr_el3 &= ~MDCR_EDAD_BIT;
	write_ctx_reg(state, CTX_MDCR_EL3, mdcr_el3);
}

