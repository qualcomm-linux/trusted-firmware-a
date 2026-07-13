/*
 * Copyright (c) ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef PLAT_LD_S
#define PLAT_LD_S

/* Validate required platform definitions */
#ifndef TFA_SHARED_MEMORY_BASE
#error "TFA_SHARED_MEMORY_BASE must be defined in platform_def.h"
#endif

#ifndef TFA_SHARED_MEMORY_SIZE
#error "TFA_SHARED_MEMORY_SIZE must be defined in platform_def.h"
#endif

/* Validate memory region is properly aligned and sized */
#if (TFA_SHARED_MEMORY_BASE & (PAGE_SIZE - 1)) != 0
#error "TFA_SHARED_MEMORY_BASE must be page-aligned"
#endif

MEMORY {
	SHARED_DATA_QTEE_SECTION (RW) : ORIGIN = TFA_SHARED_MEMORY_BASE,
		LENGTH = TFA_SHARED_MEMORY_SIZE
}

SECTIONS {

	.shared_data_qtee : ALIGN(PAGE_SIZE) {	\
		KEEP(*(.shared_data_qtee)) \
		. = ALIGN(PAGE_SIZE); \
	} > SHARED_DATA_QTEE_SECTION

}

/*
 * ============================================================================
 * QUALCOMM OVERRIDE SECTIONS
 * ============================================================================
 *
 * The following sections override definitions from upstream bl_common.ld.h
 * to add Qualcomm-specific modifications required for our platform.
 *
 * UPSTREAM SYNCHRONIZATION:
 * The build system automatically validates that these overrides stay in sync
 * with upstream definitions. If upstream changes are detected, the build will
 * fail with instructions to update this file to make sure that our
 * modifications are on top of the latest upstream changes.
 *
 * ADDING A NEW OVERRIDE:
 * If you need to add a new section override, follow these steps:
 *
 * 1. Add the override definition below following the same pattern as existing
 *    overrides (include comprehensive comments explaining the modification)
 *
 * 2. Update the validation script to track the new macro:
 *    - Edit: trustzone_images/tfa_upstream/qti_noship/validate_upstream_sync.py
 *    - Add the macro name to the TRACKED_MACROS list
 *    - Example:
 *      TRACKED_MACROS = ["DATA_SECTION", "RELA_SECTION",
 *                        "STACK_SECTION", "YOUR_NEW_SECTION"]
 *
 * 3. Update the baseline file:
 *    - Run: cd trustzone_images/tfa_upstream/qti_noship
 *    - Run: python3 validate_upstream_sync.py --update-baseline
 *    - (The validation date is automatically set to the current date)
 *
 * 4. Test the validation:
 *    - Run: python3 validate_upstream_sync.py
 *    - Should output: "✓ All tracked macro definitions match baseline"
 *
 * 5. Commit all changes together:
 *    - plat.ld.S (this file with new override)
 *    - validate_upstream_sync.py (updated TRACKED_MACROS)
 *    - bl_common.ld.h.ref (updated baseline)
 *
 * ============================================================================
 */

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
