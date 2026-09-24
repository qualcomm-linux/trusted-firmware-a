/*
 * Copyright (c) 2026, Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>
#include <common/debug.h>
#include <stdbool.h>
#include <lib/mmio.h>
#include <lib/xlat_tables/xlat_tables_v2.h>
#include <platform_def.h>
#include <qti_plat.h>
#include <drivers/qti/fuseprov/fuseprov.h>
#include <drivers/qti/fuseprov/fuseprov_mrc_cfg.h>
#include <drivers/qti/pmic/pm_pon.h>
#include <drivers/qti/fuseprov/fuseprov_port_tme.h>

static __dead2 void qti_fuseprov_trigger_reset(bool warm_reset)
{

	NOTICE("Fuseprov-Reset: Configuring the Reset\n");
	pm_app_ps_hold_cfg(warm_reset ? RESET_TYPE_WARM_RESET :
				   RESET_TYPE_HARD_RESET);
	NOTICE("Fuseprov-Reset: Configured the Reset\n");

	qti_platform_psci_system_reset();

	/* Deasserting PS_HOLD starts the reset selected in the PMIC. */
	NOTICE("Fuseprov-Reset: Writing the Register for PS Hold to Low\n");
	mmio_write_32(QTI_PS_HOLD_REG, 0U);
	NOTICE("Fuseprov-Reset: Written the Register for PS Hold to Low\n");

	/* The reset is asynchronous; do not continue execution if delayed. */
	while (true) wfi();
}

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

#if defined(QTI_ARB_TEST)
/*
 * ANTIROLLBACK1 is a legacy HWIO register token.  TF-A uses the absolute
 * TME/QFPROM address for fuse reads, so its value is zero here and the
 * address supplied by the platform is retained by the OR expression below.
 */
#define QFPROM_RAW_ANTIROLLBACK_ROW10_LSB	0x360C0318
#define QFPROM_RAW_ANTIROLLBACK_ROW10_MSB	0x360C031C

void qti_arb_read_test(const fuseprov_transport_t *transport)
{
	uint32_t fuse_data[2] = {0};
	uint32_t fuse_data_msb[2] = {0};
	fuseprov_err_t ret;

	INFO("ARB Test: reading QFPROM_RAW_ANTIROLLBACK_ROW10  (addr:0x%X)\n",
	     QFPROM_RAW_ANTIROLLBACK_ROW10_LSB );

	ret = fuseprov_row_read(transport, QFPROM_RAW_ANTIROLLBACK_ROW10_LSB ,
				FUSEPROV_ADDR_CORR, fuse_data);
	if (ret != FUSEPROV_OK) {
		INFO("ARB Test: QFPROM_RAW_ANTIROLLBACK_ROW10_LSB  read failed ret=%d\n",
		     ret);
		return;
	}

	ret = fuseprov_row_read(transport, QFPROM_RAW_ANTIROLLBACK_ROW10_MSB ,
				FUSEPROV_ADDR_CORR, fuse_data_msb);
	if (ret != FUSEPROV_OK) {
		INFO("ARB Test: QFPROM_RAW_ANTIROLLBACK_ROW10_MSB  read failed ret=%d\n",
		     ret);
		return;
	}

	INFO("ARB Test: QFPROM_RAW_ANTIROLLBACK_ROW10_LSB  PASS fuseData:0x%08X%08X\n",
	     fuse_data_msb[0], fuse_data[0]);
}
#endif /* QTI_ARB_TEST */

/* Blow fuses and trigger reset
 *
 * This function locates the SEC.DAT buffer, calls the fuseprov driver to parse
 * and blow fuses via the TME transport, and triggers a device reset on completion
 * or error.
 *
 * @param secdat_buffer: Pointer to SEC.DAT buffer
 * @param secdat_len: Length of SEC.DAT buffer
 *
 * @return Returns success when provisioning completes; a reset is only
 *         performed when at least one fuse row was programmed.
 */
int qti_fuseprov_blow_fuses_and_reset(uint8_t *secdat_buffer,
				      size_t secdat_len)
{
	fuseprov_error_etype ret;
	const fuseprov_transport_t *transport;
	bool did_program = false;

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
					     secdat_buffer,
					     secdat_len,
					     &did_program);
	if (ret != FUSEPROV_SUCCESS && ret != FUSEPROV_SECDAT_LOCK_BLOWN) {
		ERROR("Fuseprov: Fuse provisioning failed with error %d\n", ret);
		return ret;
	}

	NOTICE("Fuseprov: Fuse provisioning complete, reset deferred\n");

	/* TODO: Re-enable when the FuseProv reset dependency is available. */
	/* qti_fuseprov_trigger_reset(false); */
	return FUSEPROV_SUCCESS;
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
 * @return: 0 if provisioning found nothing to do or the SEC.DAT was already
 *          locked; -1 if the sec.elf region could not be located or mapped;
 *          the fuseprov_error_etype value on a fuse-blow failure. A reset is
 *          triggered only after successful fuse programming.
 */
int qti_fuseprov_init(void)
{
	fuseprov_error_etype ret;
	const fuseprov_transport_t *transport;
	bool did_program = false;
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
					MT_RW_DATA | MT_SECURE) != 0) {
		ERROR("Fuseprov: failed to map sec.elf buffer\n");
		return -1;
	}

	transport = fuseprov_port_tme_get();

#if defined(QTI_FUSEPROV_TEST)
	qti_fuseprov_read_test(transport);
#endif

	ret = fuseprov_blow_fuses_sec_elf_v3(transport, (uint8_t *)secelf_pa,
					     secelf_len, &did_program);
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

	if (ret == FUSEPROV_SUCCESS && did_program) {
    	qti_fuseprov_trigger_reset(false);
		return FUSEPROV_SUCCESS;
	}

	if (ret == FUSEPROV_SUCCESS)
		NOTICE("Fuseprov: no fuse programming required, reset skipped\n");

	if (ret == FUSEPROV_SECDAT_LOCK_BLOWN)
		return FUSEPROV_SUCCESS;

	return (int)ret;
}
