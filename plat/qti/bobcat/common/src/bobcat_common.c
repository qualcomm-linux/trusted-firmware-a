/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 * Copyright (c) 2018-2020, The Linux Foundation. All rights reserved.
 * Changes from Qualcomm Technologies, Inc. are provided under the following license:
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stddef.h>
#include <stdint.h>

#include <common/debug.h>
#include <lib/mmio.h>
#include <lib/smccc.h>
#include <lib/xlat_tables/xlat_tables_v2.h>
#include <services/arm_arch_svc.h>
#include <platform_def.h>
#include <qti_map_chipinfo.h>
#include <qti_plat.h>

/*
 * Platform-specific regions to map using the MMU. The BL31 image itself
 * (code, read-only data and RW data) is mapped separately by
 * qti_setup_page_tables() before this table is added.
 */

const mmap_region_t plat_qti_mmap[] = {
	MAP_REGION_FLAT(QTI_DEVICE_BASE, QTI_DEVICE_SIZE,
			MT_DEVICE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(QTI_IMEM_RING_LOG_BASE, QTI_IMEM_RING_LOG_SIZE,
			MT_DEVICE | MT_RW | MT_SECURE),
	{0}
};

CASSERT(ARRAY_SIZE(plat_qti_mmap) <= MAX_MMAP_REGIONS, assert_max_mmap_regions);

/*
 * Set up the page tables for the generic and platform-specific memory regions.
 * The extents of the generic memory regions are specified by the function
 * arguments and consist of:
 * - Trusted SRAM seen by the BL image;
 * - Code section;
 * - Read-only data section;
 * - Coherent memory region, if applicable.
 */
void qti_setup_page_tables(
			   uintptr_t total_base,
			   size_t total_size,
			   uintptr_t code_start,
			   uintptr_t code_limit,
			   uintptr_t rodata_start,
			   uintptr_t rodata_limit
			  )
{
	/*
	 * Map the Trusted SRAM with appropriate memory attributes.
	 * Subsequent mappings will adjust the attributes for specific regions.
	 */
	VERBOSE("Trusted SRAM seen by this BL image: %p - %p\n",
		(void *)total_base, (void *)(total_base + total_size));
	mmap_add_region(total_base, total_base,
			total_size, MT_MEMORY | MT_RW | MT_SECURE);

	/* Re-map the code section */
	VERBOSE("Code region: %p - %p\n",
		(void *)code_start, (void *)code_limit);
	mmap_add_region(code_start, code_start,
			code_limit - code_start, MT_CODE | MT_SECURE);

	/* Re-map the read-only data section */
	VERBOSE("Read-only data region: %p - %p\n",
		(void *)rodata_start, (void *)rodata_limit);
	mmap_add_region(rodata_start, rodata_start,
			rodata_limit - rodata_start, MT_RO_DATA | MT_SECURE);

	/* Now (re-)map the platform-specific memory regions */
	mmap_add(plat_qti_mmap);

	/* Create the page tables to reflect the above mappings */
	init_xlat_tables();
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
	size_t i;
	/* Variant other than in mapped g_map_jtag_chipinfo_id variable will have
	 * default chipinfo id as 0xFFFF
	 */
	uint32_t soc_version = (QTI_DEFAULT_CHIPINFO_ID & QTI_SOC_VERSION_MASK);
	uint32_t jep106az_code = (JEDEC_QTI_BKID << QTI_SOC_CONTINUATION_SHIFT)
			 | (JEDEC_QTI_MFID << QTI_SOC_IDENTIFICATION_SHIFT);
	uint32_t jtag_id = mmio_read_32(QTI_JTAG_ID_REG);
	uint32_t jtag_id_val = (jtag_id >> QTI_JTAG_ID_SHIFT)
			 & QTI_SOC_VERSION_MASK;

	for (i = 0U; i < ARRAY_SIZE(g_map_jtag_chipinfo_id); i++) {
		if (g_map_jtag_chipinfo_id[i].jtag_id == jtag_id_val) {
			soc_version = g_map_jtag_chipinfo_id[i].chipinfo_id &
				      QTI_SOC_VERSION_MASK;
			break;
		}
	}
	return (int32_t)(jep106az_code | (soc_version));
}

/*
 * This function returns soc revision in below format
 *
 *   soc_revision[0:30] = SOC revision of specific SOC
 */
int32_t plat_get_soc_revision(void)
{
	return (int32_t)(mmio_read_32(QTI_SOC_REVISION_REG) &
			 QTI_SOC_REVISION_MASK);
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
		return SMC_ARCH_CALL_SUCCESS;
	default:
		return SMC_ARCH_CALL_NOT_SUPPORTED;
	}
}
