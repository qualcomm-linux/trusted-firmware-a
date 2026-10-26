/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * TF-A clock driver for ipq52xx: TZ clock-branch-ena votes for the blocks
 * secure world depends on and the BOOT_IMEM teardown sequence.
 */

#include <stdint.h>

#include <arch_helpers.h>
#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <drivers/qti/clock/ipq52xx/clock_init.h>
#include <lib/mmio.h>

#include "clock_hwio.h"

/*
 * Vote to keep the clock branches enabled for the hardware blocks secure
 * world depends on, so HLOS cannot inadvertently turn them off.
 */
static void qti_clock_vote_branches(void)
{
	mmio_setbits_32(GCC_APCS_TZ_CLOCK_BRANCH_ENA_VOTE,
			 GCC_APCS_TZ_CLOCK_BRANCH_ENA_VOTE_MPM_AHB_CLK_ENA_BMSK);
	mmio_setbits_32(GCC_APCS_TZ_CLOCK_BRANCH_ENA_VOTE,
			 GCC_APCS_TZ_CLOCK_BRANCH_ENA_VOTE_IMEM_AXI_CLK_ENA_BMSK);
	mmio_setbits_32(GCC_APCS_TZ_CLOCK_BRANCH_ENA_VOTE,
			 GCC_APCS_TZ_CLOCK_BRANCH_ENA_VOTE_MSG_RAM_AHB_CLK_ENA_BMSK);
	mmio_setbits_32(GCC_APCS_TZ_CLOCK_BRANCH_ENA_VOTE,
			 GCC_APCS_TZ_CLOCK_BRANCH_ENA_VOTE_TLMM_CLK_ENA_BMSK);
	mmio_setbits_32(GCC_APCS_TZ_CLOCK_BRANCH_ENA_VOTE,
			 GCC_APCS_TZ_CLOCK_BRANCH_ENA_VOTE_TLMM_AHB_CLK_ENA_BMSK);
	mmio_setbits_32(GCC_APCS_TZ_CLOCK_BRANCH_ENA_VOTE,
			 GCC_APCS_TZ_CLOCK_BRANCH_ENA_VOTE_GEMNOC_APSS_CLK_ENA_BMSK);
	mmio_setbits_32(GCC_APCS_TZ_CLOCK_BRANCH_ENA_VOTE,
			 GCC_APCS_TZ_CLOCK_BRANCH_ENA_VOTE_CNOC_APSS_AHB_CLK_ENA_BMSK);
	mmio_setbits_32(GCC_APCS_TZ_CLOCK_BRANCH_ENA_VOTE,
			 GCC_APCS_TZ_CLOCK_BRANCH_ENA_VOTE_APSS_AXI_CLK_ENA_BMSK);
	mmio_setbits_32(GCC_APCS_TZ_CLOCK_BRANCH_ENA_VOTE,
			 GCC_APCS_TZ_CLOCK_BRANCH_ENA_VOTE_APSS_AHB_CLK_ENA_BMSK);
	mmio_setbits_32(GCC_APCS_TZ_CLOCK_BRANCH_ENA_VOTE,
			 GCC_APCS_TZ_CLOCK_BRANCH_ENA_VOTE_QDSS_DAP_CLK_ENA_BMSK);
	mmio_setbits_32(GCC_APCS_TZ_CLOCK_BRANCH_ENA_VOTE,
			 GCC_APCS_TZ_CLOCK_BRANCH_ENA_VOTE_QRNG_AHB_CLK_ENA_BMSK);
}

/*
 * BOOT_IMEM is no longer required for system boot. This is a sticky bit:
 * once 1 has been written, its value can only be returned to 0 by
 * asserting the tcsr_fp_alt_ares signal.
 */
static void qti_clock_disable_boot_imem(void)
{
	uint32_t imem_ack;

	/*
	 * GCC_PCNOC_AHB_CBCR, GCC_PCNOC_XO_DCD_CBCR, GCC_CNOC_AXI_CBCR and
	 * TCSR_GCC_CLK_MUX_SEL are required to be enabled to read
	 * TCSR_BOOT_IMEM_DISABLE_ACK. GCC_PCNOC_AHB_CBCR and GCC_CNOC_AXI_CBCR
	 * are default enabled with reset value 0x1, so enabling the other two
	 * alone.
	 */
	mmio_setbits_32(GCC_PCNOC_XO_DCD_CBCR, GCC_CBCR_CLK_ENABLE_BMSK);
	mmio_setbits_32(TCSR_GCC_CLK_MUX_SEL,
			 TCSR_GCC_CLK_MUX_SEL_CLK_MUX_SEL_BMSK);

	/*
	 * Flush the BOOT_IMEM contents before disabling it, to avoid future
	 * inconsistency, e.g. trying to flush after disabling BOOT_IMEM.
	 */
	dcsw_op_all(DCCISW);

	mmio_setbits_32(TCSR_BOOT_IMEM_DISABLE,
			 TCSR_BOOT_IMEM_DISABLE_BOOT_IMEM_DISABLE_BMSK);
	udelay(1);

	/* Read the ack bit to ensure the disable is reflected. */
	imem_ack = mmio_read_32(TCSR_BOOT_IMEM_DISABLE_ACK) &
		   TCSR_BOOT_IMEM_DISABLE_ACK_BOOT_IMEM_DISABLE_ACK_BMSK;

	if (imem_ack == 0U) {
		ERROR("Clock: BOOT IMEM disable failed\n");
	}
}

void qti_clock_init(void)
{
	qti_clock_vote_branches();
	qti_clock_disable_boot_imem();
}
