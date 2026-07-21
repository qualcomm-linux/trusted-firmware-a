/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 * Copyright (c) 2018-2020, The Linux Foundation. All rights reserved.
 * Portions copyright (c) 2026, Qualcomm Technologies, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>

#include <arch_helpers.h>
#include <common/debug.h>
#include <common/par.h>
#include <lib/mmio.h>
#include <lib/smccc.h>
#include <lib/utils_def.h>
#include <lib/xlat_tables/xlat_tables_v2.h>
#include <services/arm_arch_svc.h>

#include <bl31qtilib_interface.h>
#include <platform_def.h>
#include <qti_plat.h>

#if ENABLE_FEAT_RME
#include <qti_gpt.h>
#endif

/*
 * Table of regions for various BL stages to map using the MMU.
 * This doesn't include TZRAM as the 'mem_layout' argument passed to
 * qti_configure_mmu_elx() will give the available subset of that.
 */
const mmap_region_t plat_qti_mmap[] = {
	MAP_REGION_FLAT(QTI_DEVICE_BASE, QTI_DEVICE_SIZE,
			MT_DEVICE | MT_RW | EL3_PAS),
#ifdef QTI_GIC_SEPARATE_MAPPING
	MAP_REGION_FLAT(QTI_GICD_BASE, BASE_GIC_SIZE,
			MT_DEVICE | MT_RW | MT_SECURE),
#endif /* QTI_GIC_SEPARATE_MAPPING */
	MAP_REGION_FLAT(QTI_TME_FUSE_CONTROLLER_BASE,
			QTI_TME_FUSE_CONTROLLER_LENGTH,
			MT_DEVICE | MT_RO | EL3_PAS),
	MAP_REGION_FLAT(QTI_SEC_PRNG_BASE, QTI_PRNG_LENGTH,
			MT_DEVICE | MT_RO | EL3_PAS),
	MAP_REGION_FLAT(QTI_AOP_CMD_DB_BASE, QTI_AOP_CMD_DB_SIZE,
			MT_NS | MT_RO | MT_EXECUTE_NEVER),
	MAP_REGION_FLAT(TFA_SHARED_MEMORY_BASE, TFA_SHARED_MEMORY_SIZE,
			MT_MEMORY | MT_SECURE | MT_RW | MT_EXECUTE_NEVER),
#if ENABLE_FEAT_RME
#ifdef QTI_L0_GPT_BASE
	/* GPT L0 table memory. */
	MAP_REGION_FLAT(QTI_L0_GPT_BASE, QTI_L0_GPT_SIZE,
			MT_MEMORY | MT_RW | EL3_PAS),
#endif /* QTI_L0_GPT_BASE */
#ifdef QTI_L1_GPT_BASE
	/* GPT L1 tables memory. */
	MAP_REGION_FLAT(QTI_L1_GPT_BASE, QTI_L1_GPT_SIZE,
			MT_MEMORY | MT_RW | EL3_PAS),
#endif /* QTI_L1_GPT_BASE */
#endif /* ENABLE_FEAT_RME */
#if ENABLE_RMM
	/* EL3 <-> RMM shared buffer. */
	MAP_REGION_FLAT(QTI_RMM_SHARED_BASE, QTI_RMM_SHARED_SIZE,
			MT_MEMORY | MT_RW | EL3_PAS),
#endif /* ENABLE_RMM */
	{ 0 }
};

CASSERT(ARRAY_SIZE(plat_qti_mmap) <= MAX_MMAP_REGIONS,
	assert_max_mmap_regions);

int qti_ns_va_to_pa(uintptr_t va, unsigned int client_mode, uintptr_t *pa_out)
{
	sysreg_t par;
	u_register_t scr_el3;

	assert((client_mode == MODE_EL2) || (client_mode == MODE_EL1));
	assert(pa_out != NULL);

	/* Force NS/Normal-World context so the ATS uses NS stage-2 tables. */
	scr_el3 = read_scr_el3();
	write_scr_el3((scr_el3 | SCR_NS_BIT) & ~SCR_NSE_BIT);
	isb();

	/* Issue the appropriate ATS instruction for the caller's EL. */
	if (client_mode == MODE_EL2) {
		ats1e2r(va);        /* stage-1 EL2 read */
	} else {
		AT(ats12e1r, va);   /* stage-1+2 EL1 read */
	}
	isb();

	/* Capture PAR_EL1 before restoring SCR_EL3. */
	par = read_par_el1();
	write_scr_el3(scr_el3);
	isb();

	/* PAR_EL1.F (bit 0) set means the translation faulted. */
	if ((par & PAR_F_MASK) != 0) {
		return -1;
	}

	*pa_out = (uintptr_t)get_par_el1_pa(par);
	return 0;
}

bool qti_is_overlap_atf_rg(unsigned long long addr, size_t size)
{
	if (addr > addr + size ||
	    (BL31_BASE < addr + size && BL31_LIMIT > addr)) {
		return true;
	}
	return false;
}

/*
 * plat_qti_my_cluster_pos() - Return the cluster index of the calling CPU.
 *
 * In Armv8, MPIDR_EL1[24] is 0 and the cluster id is MPIDR_EL1[15:8].
 * In Armv8.1 and later, MPIDR_EL1[24] is 1 and the cluster id is
 * MPIDR_EL1[23:15].
 */
unsigned int plat_qti_my_cluster_pos(void)
{
	unsigned int mpidr;
	unsigned int cluster_id;

	mpidr = read_mpidr_el1();
	if ((mpidr & MPIDR_MT_MASK) == 0U) {
		cluster_id = (mpidr >> MPIDR_AFF1_SHIFT) & MPIDR_AFFLVL_MASK;
	} else {
		cluster_id = (mpidr >> MPIDR_AFF2_SHIFT) & MPIDR_AFFLVL_MASK;
	}

	assert(cluster_id < PLAT_CLUSTER_COUNT);
	return cluster_id;
}

/*
 * Set up the page tables for the generic and platform-specific memory regions.
 * The extents of the generic memory regions are specified by the function
 * arguments and consist of:
 * - Trusted SRAM seen by the BL image;
 * - Code section;
 * - Read-only data section;
 * - Coherent memory region, if applicable.
 */
#if USE_COHERENT_MEM
void qti_setup_page_tables(uintptr_t total_base, size_t total_size,
			   uintptr_t code_start, uintptr_t code_limit,
			   uintptr_t rodata_start,
			   uintptr_t rodata_limit,
			   uintptr_t coherent_ram_start,
			   uintptr_t coherent_ram_limit)
#else
void qti_setup_page_tables(uintptr_t total_base, size_t total_size,
			   uintptr_t code_start, uintptr_t code_limit,
			   uintptr_t rodata_start, uintptr_t rodata_limit)
#endif
{
	/*
	 * Map the Trusted SRAM with appropriate memory attributes.
	 * Subsequent mappings will adjust the attributes for specific regions.
	 */
	VERBOSE("Trusted SRAM seen by this BL image: %p - %p\n",
		(void *)total_base, (void *)(total_base + total_size));
	mmap_add_region(total_base, total_base, total_size,
			MT_MEMORY | MT_RW | EL3_PAS);

	/* Re-map the code section. */
	VERBOSE("Code region: %p - %p\n", (void *)code_start,
		(void *)code_limit);
	mmap_add_region(code_start, code_start, code_limit - code_start,
			MT_CODE | EL3_PAS);

	/* Re-map the read-only data section. */
	VERBOSE("Read-only data region: %p - %p\n", (void *)rodata_start,
		(void *)rodata_limit);
	mmap_add_region(rodata_start, rodata_start,
			rodata_limit - rodata_start, MT_RO_DATA | EL3_PAS);

#if USE_COHERENT_MEM
	/* Re-map the coherent memory region */
	if ((coherent_ram_start != 0) && (coherent_ram_limit != 0)) {
		mmap_add_region(coherent_ram_start, coherent_ram_start,
				coherent_ram_limit - coherent_ram_start,
				MT_DEVICE | MT_RW | EL3_PAS);
	}
#endif /* USE_COHERENT_MEM */

	/* Now (re-)map the platform-specific memory regions. */
	mmap_add(plat_qti_mmap);

	/* Create the page tables to reflect the above mappings. */
	init_xlat_tables();
}

static inline void qti_align_mem_region(uintptr_t addr, size_t size,
					uintptr_t *aligned_addr,
					size_t *aligned_size)
{
	*aligned_addr = round_down(addr, PAGE_SIZE);
	*aligned_size = round_up(addr - *aligned_addr + size, PAGE_SIZE);
}

int qti_mmap_add_dynamic_region(uintptr_t base_pa, size_t size,
				unsigned int attr)
{
	uintptr_t aligned_pa;
	size_t aligned_size;

	qti_align_mem_region(base_pa, size, &aligned_pa, &aligned_size);

	if (qti_is_overlap_atf_rg(base_pa, size)) {
		/* Memory shouldn't overlap with TF-A range. */
		return -EPERM;
	}

	return mmap_add_dynamic_region(aligned_pa, aligned_pa, aligned_size,
				       attr);
}

int qti_mmap_remove_dynamic_region(uintptr_t base_va, size_t size)
{
	qti_align_mem_region(base_va, size, &base_va, &size);
	return mmap_remove_dynamic_region(base_va, size);
}

/*
 * This function returns soc version which mainly consist of below fields
 *
 * soc_version[30:24] = JEP-106 continuation code for the SiP
 * soc_version[23:16] = JEP-106 identification code with parity bit for the SiP
 * soc_version[0:15]  = Implementation defined SoC ID
 */
int32_t plat_get_soc_version(void)
{
	/*
	 * soc_version will be 0 if no associated chip id could be found, or if
	 * called before Chipinfo is initialized
	 */
	uint32_t soc_version = (bl31qtilib_get_chip_id() & SOC_ID_IMPL_DEF_MASK);
	uint32_t jep106az_code =
		(JEDEC_QTI_BKID << QTI_SOC_CONTINUATION_SHIFT) |
		(JEDEC_QTI_MFID << QTI_SOC_IDENTIFICATION_SHIFT);

	return (int32_t)(jep106az_code | soc_version);
}

/*
 * This function returns soc revision in below format
 *
 *   soc_revision[0:30] = SOC revision of specific SOC
 *	   [15:8] = Major Revision
 *	   [7:0]  = Minor Revision
 */
int32_t plat_get_soc_revision(void)
{
	return bl31qtilib_get_soc_revision() & SOC_ID_REV_MASK;
}

/*****************************************************************************
 * plat_is_smccc_feature_available() - This function checks whether SMCCC
 * feature is available for the platform or not.
 * @fid: SMCCC function id
 *
 * Return SMC_ARCH_CALL_SUCCESS if SMCCC feature is available and
 * SMC_ARCH_CALL_NOT_SUPPORTED otherwise.
 *****************************************************************************/
int32_t plat_is_smccc_feature_available(u_register_t fid)
{
	switch (fid) {
	case SMCCC_ARCH_SOC_ID:
#if !defined(QTI_NO_SMCC_ARCH_SOC_ID)
		return SMC_ARCH_CALL_SUCCESS;
#else
		/* Intentional fall through */
#endif
	default:
		return SMC_ARCH_CALL_NOT_SUPPORTED;
	}
}

#if PLAT_RUNTIME_DEBUG_CFG

bool plat_trace_enabled(int security_state)
{
	switch (security_state) {
	case SECURE:
		return bl31qtilib_is_invasive_debug_enabled(SECURE) ||
			bl31qtilib_is_non_invasive_debug_enabled(SECURE);
	case NON_SECURE:
		return bl31qtilib_is_non_invasive_debug_enabled(NON_SECURE);
	default:
		return false;
	}
}

bool plat_external_debug_access_enabled(void)
{
	return bl31qtilib_is_invasive_debug_enabled(NON_SECURE);
}

bool plat_perfmon_enabled(int security_state)
{
	switch (security_state) {
	case SECURE:
		return bl31qtilib_is_non_invasive_debug_enabled(SECURE);
	case NON_SECURE:
		return bl31qtilib_is_non_invasive_debug_enabled(SECURE);
	default:
		return false;
	}
}

#endif /* PLAT_RUNTIME_DEBUG_CFG */

#if ENABLE_FEAT_RME
/*
 * Initialize GPT tables and enable Granule Protection Checks.
 *
 * Called from bl31_plat_arch_setup() after MMU is enabled. This is the
 * QTI equivalent of arm_gpt_setup() + gpt_runtime_init() combined, since
 * QTI BL31 is responsible for the full GPT lifecycle.
 *
 * Platform-specific parameters are obtained via plat_qti_get_gpt_info(),
 * which every platform that sets ENABLE_FEAT_RME=1 must implement.
 */
void qti_gpt_setup(void)
{
	const qti_gpt_info_t *gpt_info = plat_qti_get_gpt_info();

	if (gpt_info == NULL) {
		ERROR("GPT: plat_qti_get_gpt_info() returned NULL\n");
		panic();
	}

	/* Step 0: initialize entire protected space to GPT_GPI_ANY. */
	INFO("GPT: pps=0x%x l0_base=0x%lx l0_size=0x%lx\n",
	     (unsigned int)gpt_info->pps,
	     (unsigned long)gpt_info->l0_base,
	     (unsigned long)gpt_info->l0_size);
	if (gpt_init_l0_tables(gpt_info->pps, gpt_info->l0_base,
			       gpt_info->l0_size) < 0) {
		ERROR("GPT: gpt_init_l0_tables() failed\n");
		panic();
	}

	/* Step 1: carve PAS regions into L1 tables when provided. */
	if ((gpt_info->pas_region_base != NULL) &&
	    (gpt_info->pas_region_count > 0U)) {
		if (gpt_init_pas_l1_tables(gpt_info->pgs,
					   gpt_info->l1_base,
					   gpt_info->l1_size,
					   gpt_info->pas_region_base,
					   gpt_info->pas_region_count) < 0) {
			ERROR("GPT: gpt_init_pas_l1_tables() failed\n");
			panic();
		}
	}

	/* Step 2: write GPCCR_EL3 and enable GPC hardware enforcement. */
	INFO("GPT: Enabling Granule Protection Checks\n");
	if (gpt_enable() < 0) {
		ERROR("GPT: gpt_enable() failed\n");
		panic();
	}

	/*
	 * Step 3: initialize runtime structures so the Granule Transition
	 * Service can locate the tables via GPTBR_EL3/GPCCR_EL3.
	 */
	INFO("GPT: bitlock_base=0x%lx bitlock_size=0x%lx\n",
	     (unsigned long)gpt_info->bitlock_base,
	     (unsigned long)gpt_info->bitlock_size);
	if (gpt_runtime_init(gpt_info->bitlock_base,
			     gpt_info->bitlock_size) < 0) {
		ERROR("GPT: gpt_runtime_init() failed\n");
		panic();
	}
}
#endif /* ENABLE_FEAT_RME */
