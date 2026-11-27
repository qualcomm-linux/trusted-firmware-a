/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SHIKRA_PLAT_LD_S
#define SHIKRA_PLAT_LD_S

#include <platform_def.h>

/*
 * Internal memory reserved for EL3 code that must remain executable
 * when external memory is unavailable. The platform memory range is
 * defined by PLAT_IMEM_BASE and PLAT_IMEM_SIZE.
 */
MEMORY {
	IMEM (rwx) : ORIGIN = PLAT_IMEM_BASE,
	             LENGTH = PLAT_IMEM_SIZE
}

/*
 * EL3 entry and reset-critical code that must remain executable from
 * internal memory.
 *
 * Keep address-sensitive dependencies together in this region. Some
 * AArch64 PC-relative instructions have limited reach, so separating
 * dependent sections across distant memory regions can result in
 * relocation failures or linker-generated veneers.
 *
 * Only sections required during the internal-memory execution window
 * should be added here.
 */
SECTIONS
{
	.text_imem ORIGIN(IMEM) : {
		__BL31_IMEM_TEXT_START__ = .;

		/*
		 * EL3 cold and warm entry points. Keeping both entry points in
		 * the same region preserves their common entry dependencies
		 * while allowing the warm-reset path to execute without relying
		 * on external memory.
		 */
		KEEP(*bl31_entrypoint.o(.text.asm.bl31_entrypoint))
		KEEP(*bl31_entrypoint.o(.text.asm.bl31_warm_entrypoint))

		/*
		 * EL3 exception vectors and locally required exception helpers.
		 * These sections are kept with the entry code because the entry
		 * path installs runtime_exceptions in VBAR_EL3.
		 */
		KEEP(*runtime_exceptions.o(.vectors))
		KEEP(*runtime_exceptions.o(.text.asm.imp_def_el3_handler))

		/*
		 * Platform reset helpers. Keeping the assembly text from this
		 * object in internal memory ensures that reset handling and its
		 * local helper routines remain colocated.
		 */
		KEEP(*bruin_helpers.o(.text.asm.plat_reset_handler))

		. = ALIGN(8);
		__BL31_IMEM_TEXT_END__ = .;

		ASSERT(__BL31_IMEM_TEXT_END__ <= (ORIGIN(IMEM) + LENGTH(IMEM)),
		       ".text_imem exceeds the reserved IMEM region");
	} >IMEM AT>IMEM
}

#endif /* SHIKRA_PLAT_LD_S */
