/*
 * Copyright (c) 2026 Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLAT_LD_S
#define PLAT_LD_S

/*
 * QUALCOMM OVERRIDE: DATA_SECTION
 *
 * Purpose: Add Qualcomm-specific .lock.CACHED_MUTEX section
 *
 * Qualcomm modification:
 *   + Added: *(.lock.CACHED_MUTEX) section for cached mutex support
 *
 * IMPORTANT: If upstream DATA_SECTION changes, this override must be updated
 * to incorporate those changes while preserving the .lock.CACHED_MUTEX
 * addition.
 */
#ifdef DATA_SECTION
#undef DATA_SECTION
#endif

#define DATA_SECTION					\
	.data . : ALIGN(DATA_ALIGN) {			\
		__DATA_START__ = .;			\
		*(SORT_BY_ALIGNMENT(.data*))		\
		*(.lock.CACHED_MUTEX)			\
		__DATA_END__ = .;			\
	}


/*
 * QUALCOMM OVERRIDE: RELA_SECTION
 *
 * Purpose: Change alignment from STRUCT_ALIGN to PAGE_SIZE for image hashing
 *
 * Qualcomm modification:
 *   - Changed: ALIGN(STRUCT_ALIGN) → ALIGN(PAGE_SIZE)
 *   - Reason: All bl31 image segments must be 4KB-aligned for image hashing
 *
 * IMPORTANT: If upstream RELA_SECTION changes (e.g., new sections added),
 * this override must be updated while preserving PAGE_SIZE alignment.
 */
#ifdef RELA_SECTION
#undef RELA_SECTION
#endif

#define RELA_SECTION					\
	RELA_DYN_NAME : ALIGN(PAGE_SIZE) {		\
		__RELA_START__ = .;			\
		RELOC_SECTIONS_PATTERN			\
		__RELA_END__ = .;			\
	}

/*
 * QUALCOMM OVERRIDE: STACK_SECTION
 *
 * Purpose: Add PAGE_SIZE alignment for image hashing requirements
 *
 *
 * Qualcomm modification:
 *   - Changed: .stacks (NOLOAD) : { →
 *     .stacks (NOLOAD) : ALIGN(PAGE_SIZE) {
 *   - Reason: All bl31 image segments must be 4KB-aligned for image hashing
 *
 * IMPORTANT: If upstream STACK_SECTION changes (e.g., new sections, different
 * conditional logic), this override must be updated while preserving PAGE_SIZE
 * alignment.
 */
#ifdef STACK_SECTION
#undef STACK_SECTION
#endif
#if !(defined(IMAGE_BL31) && RECLAIM_INIT_CODE)
#define STACK_SECTION					\
	.stacks (NOLOAD) : ALIGN(PAGE_SIZE) {		\
		__STACKS_START__ = .;			\
		*(.tzfw_normal_stacks)			\
		__STACKS_END__ = .;			\
	}
#endif

#endif /* PLAT_LD_S */
