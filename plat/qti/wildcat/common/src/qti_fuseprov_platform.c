/*
 * Copyright (c) 2026, Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>
#include <common/debug.h>
#include <lib/xlat_tables/xlat_tables_v2.h>
#include <qti_plat.h>
#include <drivers/qti/fuseprov/fuseprov.h>
#include <drivers/qti/fuseprov/fuseprov_mrc_cfg.h>
#include <drivers/qti/fuseprov/fuseprov_port_tme.h>

#include <bl31qtilib_interface.h>

#if defined(QTI_FUSEPROV_TEST)
/*
 * Boot-time self-test: read the QFPROM_CORR_OEM_CONFIG_ROW1_MSB register
 * through the fuseprov transport abstraction (the same path
 * qti_fuseprov_init() uses to blow fuses) to confirm that this init flow is
 * actually being reached and exercised at boot.  Read-only -- never panics,
 * a failure is logged only.
 */
#define QFPROM_RAW_OEM_CONFIG_ROW1_MSB 	0x360C0164

static void qti_fuseprov_read_test(const fuseprov_transport_t *transport)
{
	uint32_t fuse_data[2] = {0};
	fuseprov_err_t ret;

	INFO("Fuseprov test: reading QFPROM_RAW_OEM_CONFIG_ROW1_MSB  (addr:0x%X)\n",
	     QFPROM_RAW_OEM_CONFIG_ROW1_MSB );

	ret = fuseprov_row_read(transport, QFPROM_RAW_OEM_CONFIG_ROW1_MSB ,
				FUSEPROV_ADDR_CORR, fuse_data);
	if (ret != FUSEPROV_OK) {
		INFO("Fuseprov test: QFPROM_RAW_OEM_CONFIG_ROW1_MSB  read failed ret=%d\n",
		     ret);
		return;
	}

	INFO("Fuseprov test: QFPROM_RAW_OEM_CONFIG_ROW1_MSB  PASS fuseData:0x%08X%08X\n",
	     fuse_data[1], fuse_data[0]);
}
#endif /* QTI_FUSEPROV_TEST */

/* Blow fuses and trigger reset
 *
 * This function locates the SEC.DAT buffer, calls the fuseprov driver to parse
 * and blow fuses via the TME transport, and triggers a device reset on completion
 * or error.
 *
 * @param secdat_buffer: Pointer to SEC.DAT buffer
 * @param secdat_len: Length of SEC.DAT buffer
 *
 * @return Does not return on success (device resets); returns error code on failure
 */
int qti_fuseprov_blow_fuses_and_reset(const uint8_t *secdat_buffer,
				      size_t secdat_len)
{
	fuseprov_error_etype ret;
	const fuseprov_transport_t *transport;

	if (secdat_buffer == NULL || secdat_len == 0) {
		ERROR("Fuseprov: Invalid SEC.DAT buffer\n");
		return -1;
	}

	NOTICE("Fuseprov: Starting fuse provisioning and reset sequence\n");

	/* Get the TME transport for fuse read/write operations */
	transport = fuseprov_port_tme_get();
	if (transport == NULL) {
		ERROR("Fuseprov: Failed to get TME transport\n");
		return -1;
	}

	/* Parse SEC.DAT and blow fuses via transport abstraction */
	ret = fuseprov_blow_fuses_sec_elf_v3(transport,
					     (uint8_t *)secdat_buffer,
					     secdat_len);
	if (ret != FUSEPROV_SUCCESS && ret != FUSEPROV_SECDAT_LOCK_BLOWN) {
		ERROR("Fuseprov: Fuse provisioning failed with error %d\n", ret);
		return ret;
	}

	NOTICE("Fuseprov: Fuse provisioning complete, triggering system reset\n");

	bl31qtilib_psci_system_reset();

	/* Stay in WFI until the system resets */
	while (1) {
		wfi();
	}
}

/* Ask TME where it authenticated sec.elf during boot, then parse and blow
 * fuses from that buffer.
 *
 * This is the TF-A counterpart of the Zephyr fuseprov_init() boot hook: it
 * performs the same "self-locate the buffer via TME, then provision" work.
 * Unlike the Zephyr version it is not registered against any boot-time init
 * framework -- TF-A has none -- so it is exposed here for a caller to invoke
 * once one is chosen.
 *
 * @return: 0 if provisioning ran (successfully, with nothing to do, or
 *          already locked); -1 if the sec.elf region could not be located
 *          or mapped; the fuseprov_error_etype value on a fuse-blow failure
 */
int qti_fuseprov_init(void)
{
	fuseprov_error_etype ret;
	const fuseprov_transport_t *transport;
	uint32_t secelf_len;
	uintptr_t secelf_pa;

	secelf_pa = 0x87452000;
	secelf_len = 4096;

	if (secelf_pa == 0 || secelf_len == 0 ||
	    secelf_len > FUSEPROV_SECDAT_BUFFER_SIZE) {
		ERROR("Fuseprov: sec.elf region out of bounds (0x%lx, %u bytes)\n",
		      (unsigned long)secelf_pa, secelf_len);
		return -1;
	}

	/* secelf_pa is a DDR physical address; it is not part of any static
	 * MMU region, so map it before use.
	 */
	if (qti_mmap_add_dynamic_region(secelf_pa, secelf_len,
					MT_RO_DATA | MT_SECURE) != 0) {
		ERROR("Fuseprov: failed to map sec.elf buffer\n");
		return -1;
	}

	transport = fuseprov_port_tme_get();

#if defined(QTI_FUSEPROV_TEST)
	qti_fuseprov_read_test(transport);
#endif

	ret = fuseprov_blow_fuses_sec_elf_v3(transport, (uint8_t *)secelf_pa,
					     secelf_len);

#if defined(QTI_FUSEPROV_TEST)
	qti_fuseprov_read_test(transport);
#endif

	switch (ret) {
	case FUSEPROV_SUCCESS:
		NOTICE("Fuseprov: fuse provisioning complete\n");
		break;
	case FUSEPROV_SECDAT_LOCK_BLOWN:
		NOTICE("Fuseprov: fuse provisioning skipped, write permission disabled\n");
		break;
	case FUSEPROV_SECDAT_MAGIC_MISMATCH:
	case FUSEPROV_SECDAT_DEFAULT_NOFUSES:
		NOTICE("Fuseprov: no fuses to blow\n");
		break;
	default:
		ERROR("Fuseprov: fuse blow failed with error %d\n", ret);
		break;
	}

	if (qti_mmap_remove_dynamic_region(secelf_pa, secelf_len) != 0)
		ERROR("Fuseprov: failed to unmap sec.elf buffer\n");

	if (ret == FUSEPROV_SUCCESS || ret == FUSEPROV_SECDAT_LOCK_BLOWN)
		return 0;

	return (int)ret;
}
