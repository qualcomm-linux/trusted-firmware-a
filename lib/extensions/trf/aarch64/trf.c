/*
 * Copyright (c) 2021-2025, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <arch_features.h>
#include <arch_helpers.h>
#include <lib/extensions/trf.h>

void trf_enable(cpu_context_t *ctx)
{
	el3_state_t *state = get_el3state_ctx(ctx);
	u_register_t mdcr_el3_val = read_ctx_reg(state, CTX_MDCR_EL3);

	/*
	 * MDCR_EL3.TTRF = b0
	 * Allow access of trace filter control registers from NS-EL2
	 * and NS-EL1 when NS-EL2 is implemented but not used
	 *
	 * MDCR_EL3.RLTE = b0
	 * Trace prohibited in Realm state, unless overridden by the
	 * IMPLEMENTATION DEFINED authentication interface.
	 */
	mdcr_el3_val &= ~(MDCR_TTRF_BIT | MDCR_RLTE_BIT);
	write_ctx_reg(state, CTX_MDCR_EL3, mdcr_el3_val);
}

void trf_disable(cpu_context_t *ctx)
{
	el3_state_t *state = get_el3state_ctx(ctx);
	u_register_t mdcr_el3_val = read_ctx_reg(state, CTX_MDCR_EL3);

	/*
	 * MDCR_EL3.TTRF = b1
	 * Trap access of trace filter control registers from NS-EL2
	 * and NS-EL1 to EL3
	 */
	mdcr_el3_val |= MDCR_TTRF_BIT;
	write_ctx_reg(state, CTX_MDCR_EL3, mdcr_el3_val);
}

void trf_init_el2_unused(void)
{
	/*
	 * MDCR_EL2.TTRF: Set to zero so that access to Trace
	 *  Filter Control register TRFCR_EL1 at EL1 is not
	 *  trapped to EL2. This bit is RES0 in versions of
	 *  the architecture earlier than ARMv8.4.
	 *
	 */
	write_mdcr_el2(read_mdcr_el2() & ~MDCR_EL2_TTRF);
}

/*
 * MDCR_EL3.STE:
 * When FEAT_TRF is implemented and Secure state is implemented:
 *     Secure Trace enable. Enables tracing in Secure state.
 * Otherwise:
 *     Reserved, RES0.
 */
void secure_trace_enable(cpu_context_t *ctx)
{
	el3_state_t *state = get_el3state_ctx(ctx);
	u_register_t mdcr_el3_val = read_ctx_reg(state, CTX_MDCR_EL3);

	/*
	 * MDCR_EL3.STE (bit 18): Secure Trace enable
	 * = b1 : Trace in Secure state is not affected by this field.
	 */
	mdcr_el3_val |= MDCR_STE_BIT;
	write_ctx_reg(state, CTX_MDCR_EL3, mdcr_el3_val);
}

void secure_trace_disable(cpu_context_t *ctx)
{
	el3_state_t *state = get_el3state_ctx(ctx);
	u_register_t mdcr_el3_val = read_ctx_reg(state, CTX_MDCR_EL3);

	/*
	 * MDCR_EL3.STE (bit 18): Secure Trace enable
	 * = b0 : Trace prohibited in Secure state unless overridden
	 * by the IMPLEMENTATION DEFINED authentication interface.
	 */
	mdcr_el3_val &= ~MDCR_STE_BIT;
	write_ctx_reg(state, CTX_MDCR_EL3, mdcr_el3_val);
}
