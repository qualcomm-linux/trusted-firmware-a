/*
 * Copyright (c) 2024-2026, Arm Limited and Contributors. All rights reserved.
 * Copyright (c) 2022, Google LLC. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>

#include <arch_features.h>
#include <arch_helpers.h>
#include <common/debug.h>
#include <lib/el3_runtime/aarch64/context.h>
#include <lib/el3_runtime/context_mgmt.h>
#include <lib/el3_runtime/cpu_data.h>
#include <lib/el3_runtime/simd_ctx.h>
#include <lib/extensions/sve.h>
#include <plat/common/platform.h>

#if CTX_INCLUDE_FPREGS || CTX_INCLUDE_SVE_REGS

/* SIMD context managed for Secure and Normal Worlds. */
#define SIMD_CTXT_COUNT	2

#if SEPARATE_SIMD_SECTION
__section(".simd_context")
#else /* SEPARATE_SIMD_SECTION */
__section(".bss.simd_context")
#endif /* SEPARATE_SIMD_SECTION */
static simd_regs_t simd_context[SIMD_CTXT_COUNT][PLATFORM_CORE_COUNT];

#if CTX_INCLUDE_SVE_REGS

/**
 * set_up_cptr_zcr_el3_for_sme_sve_access() - Enable SVE/SME access at EL3
 * @saved_cptr: Pointer to store the original CPTR_EL3 value
 * @saved_zcr: Pointer to store the original ZCR_EL3 value
 *
 * This function configures EL3 to allow access to SVE and SME registers by:
 * 1. Saving the current CPTR_EL3 and ZCR_EL3 values for later restoration
 * 2. Disabling SVE traps (CPTR_EL3.EZ = 1) to allow SVE register access
 * 3. Disabling SME traps (CPTR_EL3.ESM = 1) if SME is present
 * 4. Configuring the SVE vector length via ZCR_EL3 to the platform-defined
 *    SVE_VECTOR_LEN
 *
 * This setup is necessary before any SVE/SME register save/restore operations
 * to prevent traps to EL3 when accessing these registers.
 */
static void set_up_cptr_zcr_el3_for_sme_sve_access(uint64_t *saved_cptr, uint64_t *saved_zcr)
{
	uint64_t cptr = read_cptr_el3();
	*saved_cptr = cptr;

	/* Disable SVE traps to allow access to Z, P, and FFR registers */
	uint64_t new_cptr = cptr | CPTR_EZ_BIT;

	if (is_feat_sme_present()) {
		/* Disable SME traps to allow access to SME registers and SVCR */
		new_cptr |= ESM_BIT;
	}

	write_cptr_el3(new_cptr);
	isb();

	/* Save current ZCR_EL3 and configure SVE vector length for EL3 */
	uint64_t zcr = read_zcr_el3();
	*saved_zcr = zcr;

	/* Set ZCR_EL3.LEN to configure maximum SVE vector length
	 * Formula: VECTOR_LEN = (LEN + 1) * 128 bits
	 */
	write_zcr_el3((SVE_VECTOR_LEN >> 7) - 1);
	isb();
}

/**
 * restore_cptr_zcr_el3() - Restore original CPTR_EL3 and ZCR_EL3 values
 * @saved_cptr: The original CPTR_EL3 value to restore
 * @saved_zcr: The original ZCR_EL3 value to restore
 *
 * This function restores the CPTR_EL3 and ZCR_EL3 registers to their original
 * values after SVE/SME register operations are complete. This re-enables any
 * traps that were previously configured and restores the original vector length
 * configuration.
 */
static void restore_cptr_zcr_el3(uint64_t saved_cptr, uint64_t saved_zcr)
{
	write_zcr_el3(saved_zcr);
	write_cptr_el3(saved_cptr);
	isb();
}

static bool is_ffr_access_safe(void)
{
	bool sme_supported = is_feat_sme_present();
	bool is_ffr_access_safe = false;

	/* Access to FFR is safe if SME is not supported, if the PE is not in
	 * streaming mode, or if in streaming mode with SME_FA64 enabled.
	 */
	if (sme_supported) {
		bool is_streaming_mode = (read_svcr() & SVCR_SM_BIT) != 0U;
		if (is_streaming_mode) {
			bool sme_fa64_implemented =
				(read_id_aa64smfr0_el1() & ID_AA64SMFR0_EL1_SME_FA64_BIT) != 0U;
			bool sme_fa64_enabled = (read_smcr_el3() & SMCR_ELX_FA64_BIT) != 0U;
			is_ffr_access_safe = sme_fa64_implemented && sme_fa64_enabled;
		} else {
			is_ffr_access_safe = true;
		}
	} else {
		is_ffr_access_safe = true;
	}

	return is_ffr_access_safe;
}
#endif /* CTX_INCLUDE_SVE_REGS */

void simd_ctx_save(uint32_t security_state, bool hint_sve)
{
	simd_regs_t *regs;

	if (security_state != NON_SECURE && security_state != SECURE) {
		ERROR("Unsupported security state specified for SIMD context: %u\n",
		      security_state);
		panic();
	}

	regs = &simd_context[security_state][plat_my_core_pos()];

	disable_fpregs_traps_el3();
#if CTX_INCLUDE_SVE_REGS
	regs->hint = hint_sve;

	if (hint_sve) {
		/*
		 * Hint bit denoting absence of SVE live state. Hence, only
		 * save FP context.
		 */
		fpregs_context_save(regs);
	} else {
		/*
		 * SVE state is live. This covers:
		 * - SVE only (no SME) and SVE is being used
		 * - Both SVE and SME, not in streaming mode, SVE is being used
		 * - SME with streaming mode enabled (Streaming SVE)
		 */
		uint64_t saved_cptr_ctx, saved_zcr_ctx;

		set_up_cptr_zcr_el3_for_sme_sve_access(&saved_cptr_ctx, &saved_zcr_ctx);


		/* Save Z and P registers in current mode */
		sve_vectors_context_save(regs);

		/* We don't save/restore ZA context due to memory size constraints,
		instead we assume secure world does not use ZA state. */

		/* FFR must be saved after saving the predicates (P0-P15) because
		 * reading the FFR necessarily uses a predicate register, clobbering
		 * its value.
		 */
		if (is_ffr_access_safe()) {
			save_ffr(regs);
		}

		restore_cptr_zcr_el3(saved_cptr_ctx, saved_zcr_ctx);
	}
#elif CTX_INCLUDE_FPREGS
	fpregs_context_save(regs);
#endif
	enable_fpregs_traps_el3();
}

void simd_ctx_restore(uint32_t security_state)
{
	simd_regs_t *regs;

	if (security_state != NON_SECURE && security_state != SECURE) {
		ERROR("Unsupported security state specified for SIMD context: %u\n",
		      security_state);
		panic();
	}

	regs = &simd_context[security_state][plat_my_core_pos()];

	disable_fpregs_traps_el3();
#if CTX_INCLUDE_SVE_REGS
	if (regs->hint) {
		fpregs_context_restore(regs);
	} else {
		uint64_t saved_cptr_ctx, saved_zcr_ctx;

		set_up_cptr_zcr_el3_for_sme_sve_access(&saved_cptr_ctx, &saved_zcr_ctx);

		/* Restore FFR if accessible. If not accessible (streaming mode without FA64),
		 * FFR was already reset to a known state (all 1s) when entering streaming mode.
		 * FFR must be restored before restoring the predicates (P0-P15) because
		 * writing the FFR necessarily uses a predicate register, clobbering its value.
		 */
		if (is_ffr_access_safe()) {
			restore_ffr(regs);
		}

		/* Restore Z and P registers */
		sve_vectors_context_restore(regs);

		restore_cptr_zcr_el3(saved_cptr_ctx, saved_zcr_ctx);
	}
#elif CTX_INCLUDE_FPREGS
	fpregs_context_restore(regs);
#endif
	enable_fpregs_traps_el3();
}
#endif /* CTX_INCLUDE_FPREGS || CTX_INCLUDE_SVE_REGS */
