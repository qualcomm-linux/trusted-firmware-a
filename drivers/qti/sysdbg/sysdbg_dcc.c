/*
 * Copyright (c) 2016-2018, 2024-2026, Qualcomm Innovation Center, Inc.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Ported from TZ.XF.5.29.1-00158-KODIAKAAAAANAAZT-1:
 * trustzone_images/core/systemdebug/sysdbg/qsee/src/sysdbg_dcc.c
 */

#include <string.h>
#include <common/debug.h>
#include <lib/utils_def.h>
#include <drivers/qti/sysdbg/sysdbg_dcc.h>
#include <drivers/qti/sysdbg/sysdbg_dcc_hal.h>
#include <drivers/qti/sysdbg/sysdbg_main.h>

/**
 * @file sysdbg_dcc.c
 * @brief DCC Driver Implementation
 *
 * This module defines all interfaces exposed by the DCC driver.
 *
 * Reference: trustzone_images/core/systemdebug/sysdbg/qsee/src/sysdbg_dcc.c
 */

/*===========================================================================
 * Static Function Declarations
 *===========================================================================
 */

static bool sysdbg_dcc_target_init(void);

/*===========================================================================
 * Function: sysdbg_dcc_target_init
 * Reference: sysdbg_dcc.c:30
 *===========================================================================
 */
static bool sysdbg_dcc_target_init(void)
{
	uint32_t index = 0;
	hal_dcc_list_params_type params = {0};
	bool success = false;

	INFO("DCC: Target initialization started\n");

	/* All clocks and necessary init at the system level is done already at the top level */
	const dcc_config_type *dcc_config = dcc_get_config();

	INFO("DCC: Configuration retrieved\n");

	/* Get the default configuration */
	const sysdbg_config_t *sysdbg_config = sysdbg_get_config();

	/* Get the size */
	const uint32_t count = dcc_get_ll_cnt();

	INFO("DCC: Found %u linked list(s) to configure\n", count);

	if (dcc_config->dcc_en) {
		INFO("DCC: DCC is enabled, configuring linked lists\n");

		for (index = 0; index < count; index++) {
			INFO("DCC: Processing linked list %u/%u\n", index + 1, count);

			/* Get all the linked list configuration */
			const dcc_ll_config_type *dcc_ll_config = dcc_get_ll_config(index);

			/* DCC enabled but null data is a failure scenario */
			if (!dcc_ll_config) {
				ERROR("DCC: Failed to get linked list config for index %u\n", index);
				return false;
			}

			/* Linked list index obtain */
			uint32_t idx = dcc_ll_config->ll_idx;

			INFO("DCC: Configuring list index %u\n", idx);

			if (dcc_ll_config->ll_trigger == DCC_TRIGGER_IHM_MHM_SCANDUMP) {
				if (sysdbg_config->mhm_scandumpen) {
					INFO("DCC: Unlocking list %u for MHM scandump\n", idx);
					hal_dcc_unlock_list(idx);
				} else {
					INFO("DCC: Skipping list %u (MHM scandump disabled)\n", idx);
					continue;
				}
			}

			/* Check the index */
			if (idx > hal_dcc_get_max_lists())
				ERROR("DCC List index invalid: idx=%u\n", idx);

			/* Configure the LL to be active. If already active, that's a failure. */
			if (hal_dcc_is_list_locked(idx))
				ERROR("DCC List Locked: idx=%u\n", idx);

			/* Enable timeout */
			if (dcc_ll_config->ll_timeout_en) {
				params.ntimeout = dcc_ll_config->ll_timeout;
				INFO("DCC: List %u timeout enabled: %u\n", idx, params.ntimeout);
			}

			/* Enable per-transaction timeout */
			if (dcc_ll_config->llt_timeout_en) {
				params.ntranstimeout = dcc_ll_config->llt_timeout;
				INFO("DCC: List %u transaction timeout enabled: %u\n", idx, params.ntranstimeout);
			}

			/* Configure the trigger and enables = 0 : DCC_SRAM 1 : DCC_TPDM */
			if (dcc_config->sink == DCC_SINK_ETB) {
				params.esink = HAL_DCC_SINK_ETB;
				INFO("DCC: Using ETB sink for list %u\n", idx);
			} else {
				params.esink = HAL_DCC_SINK_SRAM;
				INFO("DCC: Using SRAM sink for list %u\n", idx);
			}

			/* Initialize the memory */
			INFO("DCC: Initializing sink for list %u\n", idx);
			if (!hal_dcc_init_sink(params.esink)) {
				ERROR("DCC: Failed to initialize sink for list %u\n", idx);
				return success;
			}

			/* Choose the trigger */
			switch (dcc_ll_config->ll_trigger) {
			case DCC_TRIGGER_SCANDUMP:
			case DCC_TRIGGER_IHM_MHM_SCANDUMP:
			case DCC_TRIGGER_DDRAUX:
			case DCC_TRIGGER_APSSWDOGBITE:
			case DCC_TRIGGER_WINDDOWN:
				params.etrigger = HAL_DCC_HW_TRIGGER;
				INFO("DCC: List %u using HW trigger (type=%u)\n", idx, dcc_ll_config->ll_trigger);
				break;

			case DCC_TRIGGER_CTI:
				params.etrigger = HAL_DCC_CTI_TRIGGER;
				INFO("DCC: List %u using CTI trigger\n", idx);
				break;

			default:
				INFO("DCC: List %u using default trigger\n", idx);
				break;
			}

			/* Program the address to put the fetched data in */
			params.noffset = dcc_ll_config->ll_fd_offset;
			params.efunc = HAL_DCC_FUNC_REG_CAPTURE;

			/* Configure all the list params */
			INFO("DCC: Configuring list %u parameters\n", idx);
			if (!hal_dcc_config_list(idx, &params)) {
				ERROR("DCC: Failed to configure list %u parameters\n", idx);
				return success;
			}

			/* Program the actual linked list */
			INFO("DCC: Programming list %u (offset=0x%x, size=%u)\n",
			     idx, dcc_ll_config->ll_offset, dcc_ll_config->ll_size);
			if (!hal_dcc_program_list((void *)dcc_ll_config->ll, idx,
						  dcc_ll_config->ll_offset, dcc_ll_config->ll_size)) {
				ERROR("DCC: Failed to program list %u\n", idx);
				return success;
			}

			INFO("DCC: List %u configured successfully\n", idx);
		}
		/* Done. Init */
		success = true;
		NOTICE("DCC: All linked lists configured successfully\n");
	} else {
		/* Not fatal at the moment if DCC is disabled for boot processor. Add log here */
		INFO("DCC: DCC is disabled, skipping configuration\n");
		success = true;
	}

	return success;
}


/*===========================================================================
 * Function: sysdbg_dcc_init
 * Reference: sysdbg_dcc.c:291
 *===========================================================================
 */
bool sysdbg_dcc_init(void)
{
	bool status = false;
	/* Grab OS Lock here */

	INFO("DCC: Starting DCC hardware initialization\n");

	/* Perform DCC Target HW Init */
	status = sysdbg_dcc_target_init();

	if (status) {
		NOTICE("DCC: Hardware initialization successful\n");
	} else {
		ERROR("DCC: Hardware initialization failed\n");
	}

	/* Relinquish Lock */

	return status;
}
