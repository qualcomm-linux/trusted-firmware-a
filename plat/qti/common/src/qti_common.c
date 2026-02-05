/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 * Copyright (c) 2018-2020, The Linux Foundation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>

#include <common/debug.h>
#include <lib/smccc.h>
#include <lib/xlat_tables/xlat_tables_v2.h>
#include <services/arm_arch_svc.h>

#include <drivers/qti/chipinfo/chipinfo.h>
#include <platform_def.h>
#include <qti_plat.h>

/*
 * Table of regions for various BL stages to map using the MMU.
 * This doesn't include TZRAM as the 'mem_layout' argument passed to
 * qti_configure_mmu_elx() will give the available subset of that,
 */

const mmap_region_t plat_qti_mmap[] = {
	MAP_REGION_FLAT(QTI_DEVICE_BASE, QTI_DEVICE_SIZE,
			MT_DEVICE | MT_RW | EL3_PAS),
#ifdef QTI_TME_FUSE_CONTROLLER_BASE
	MAP_REGION_FLAT(QTI_TME_FUSE_CONTROLLER_BASE,
			QTI_TME_FUSE_CONTROLLER_LENGTH,
			MT_DEVICE | MT_RO | EL3_PAS),
#endif
#if defined(QTI_SEC_PRNG_BASE) && defined(QTI_PRNG_LENGTH)
	MAP_REGION_FLAT(QTI_SEC_PRNG_BASE, QTI_PRNG_LENGTH,
			MT_DEVICE | MT_RO | EL3_PAS),
#endif
#ifdef QTI_AOP_CMD_DB_BASE
	MAP_REGION_FLAT(QTI_AOP_CMD_DB_BASE, QTI_AOP_CMD_DB_SIZE,
			MT_NS | MT_RO | MT_EXECUTE_NEVER),
#endif
#ifdef QTI_SMEM_BASE
	MAP_REGION_FLAT(QTI_SMEM_BASE, QTI_SMEM_SIZE,
			MT_NON_CACHEABLE | MT_RW | MT_EXECUTE_NEVER),
#endif
#ifdef TFA_SHARED_MEMORY_BASE
	MAP_REGION_FLAT(TFA_SHARED_MEMORY_BASE, TFA_SHARED_MEMORY_SIZE,
			MT_MEMORY | MT_SECURE | MT_RW | MT_EXECUTE_NEVER),
#endif
	{0}
};

CASSERT(ARRAY_SIZE(plat_qti_mmap) <= MAX_MMAP_REGIONS, assert_max_mmap_regions);

static const struct {
	uintptr_t start;
	uintptr_t end;
} sec_regions[] = {
	{ BL31_BASE, BL31_LIMIT },
#if defined(BL32_BASE) && defined(BL32_LIMIT)
/* TEE region */
	{ BL32_BASE, BL32_LIMIT },
/* TA region */
#if defined(QTI_PIMEM_BASE) && defined(QTI_PIMEM_LIMIT)
	{ QTI_PIMEM_BASE, QTI_PIMEM_LIMIT },
#endif
#endif
#if defined(BL2_BASE) && defined(BL2_LIMIT)
	{ BL2_BASE, BL2_LIMIT },
#endif
};

static bool qti_is_overlap_secure_rg(uintptr_t addr, size_t size)
{
	uintptr_t end = addr + size;

	if (addr > end)
		return true;

	for (size_t i = 0; i < ARRAY_SIZE(sec_regions); i++) {
		if (addr < sec_regions[i].end && end > sec_regions[i].start) {
			return true;
		}
	}

	return false;
}

/*
 *  unsigned int plat_qti_my_cluster_pos(void)
 *  definition to get the cluster index of the calling CPU.
 *  - In ARM v8   (MPIDR_EL1[24]=0)
 *    ClusterId = MPIDR_EL1[15:8]
 *  - In ARM v8.1 & Later version (MPIDR_EL1[24]=1)
 *    ClusterId = MPIDR_EL1[23:15]
 */
unsigned int plat_qti_my_cluster_pos(void)
{
	unsigned int mpidr, cluster_id;

	mpidr = read_mpidr_el1();
	if ((mpidr & MPIDR_MT_MASK) == 0) {	/* MT not supported */
		cluster_id = (mpidr >> MPIDR_AFF1_SHIFT) & MPIDR_AFFLVL_MASK;
	} else {		/* MT supported */
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
void qti_setup_page_tables(uintptr_t total_base, size_t total_size,
			   uintptr_t code_start, uintptr_t code_limit,
			   uintptr_t rodata_start, uintptr_t rodata_limit,
			   uintptr_t coherent_ram_start,
			   uintptr_t coherent_ram_limit)
{
	/*
	 * Map the Trusted SRAM with appropriate memory attributes.
	 * Subsequent mappings will adjust the attributes for specific regions.
	 */
	VERBOSE("Trusted SRAM seen by this BL image: %p - %p\n",
		(void *)total_base, (void *)(total_base + total_size));
	mmap_add_region(total_base, total_base,
			total_size, MT_MEMORY | MT_RW | EL3_PAS);

	/* Re-map the code section */
	VERBOSE("Code region: %p - %p\n",
		(void *)code_start, (void *)code_limit);
	mmap_add_region(code_start, code_start,
			code_limit - code_start, MT_CODE | EL3_PAS);

	/* Re-map the read-only data section */
	VERBOSE("Read-only data region: %p - %p\n",
		(void *)rodata_start, (void *)rodata_limit);
	mmap_add_region(rodata_start, rodata_start,
			rodata_limit - rodata_start, MT_RO_DATA | EL3_PAS);

	if ((coherent_ram_start != 0U) && (coherent_ram_limit != 0U)) {
		mmap_add_region(coherent_ram_start, coherent_ram_start,
				coherent_ram_limit - coherent_ram_start,
				MT_DEVICE | MT_RW | EL3_PAS);
	}

	/* Now (re-)map the platform-specific memory regions */
	mmap_add(plat_qti_mmap);

	/* Create the page tables to reflect the above mappings */
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

	if (qti_is_overlap_secure_rg(base_pa, size)) {
		/* Memory shouldn't overlap with secure ranges. */
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
	uint32_t soc_version =
		(chipinfo_get_chip_id() & SOC_ID_IMPL_DEF_MASK);
	uint32_t jep106az_code = (JEDEC_QTI_BKID << QTI_SOC_CONTINUATION_SHIFT)
			 | (JEDEC_QTI_MFID << QTI_SOC_IDENTIFICATION_SHIFT);

	return (int32_t)(jep106az_code | (soc_version));
}

/*
 * This function returns soc revision in below format
 *
 *   soc_revision[0:30] = SOC revision of specific SOC
 */
int32_t plat_get_soc_revision(void)
{
	uint32_t revision = chipinfo_get_chip_version();
	uint32_t major = (revision >> 16) & U(0xFF);
	uint32_t minor = revision & U(0xFF);

	return (int32_t)((major << 8) | minor);
}

/*****************************************************************************
 * plat_is_smccc_feature_available() - This function checks whether SMCCC feature
 *                                  is availabile for the platform or not.
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
		__fallthrough;
#endif
	default:
		return SMC_ARCH_CALL_NOT_SUPPORTED;
	}
}
