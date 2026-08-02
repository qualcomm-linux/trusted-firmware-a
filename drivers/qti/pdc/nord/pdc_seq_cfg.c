/*
 * Copyright (c) 2026 Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <drivers/qti/pdc/pdc_seq.h>

#include "pdc_grp0_branch.h"

static uint8_t apps_branches[PDC_SEQ_BR_ADDR_REG_COUNT] = { 0 };
static uint32_t apps_delays[PDC_SEQ_DELAY_REG_COUNT] = { 0 };

static struct pdc_seq_cfg apps_pdc_cfg = {
	PDC_SEQ_ARRAY(apps_branches),
	PDC_SEQ_ARRAY(apps_delays),
};

static struct pdc_seq_mode apps_pdc_modes[] = {
	{ NULL, 0, 1, 0, PDC_BRANCH_SLEEP_TCS2 | PDC_BRANCH_NO_AOSS_SLEEP },
	{ NULL, 0, 2, 0, PDC_BRANCH_SLEEP_TCS0 | PDC_BRANCH_NO_AOSS_SLEEP },
	{ NULL, 0, 3, 0, PDC_BRANCH_SLEEP_TCS2 },
	{ NULL, 0, 4, 0, PDC_BRANCH_SLEEP_TCS0 },
	{ NULL, 0, 5, 0, PDC_BRANCH_SLEEP_TCS1 },
};

static struct pdc_seq pdc_seq_instance = {
	PDC_SEQ_APPS,
	&apps_pdc_cfg,
	PDC_SEQ_ARRAY(apps_pdc_modes),
	0x200000U,	/* PDC offset from AOSS base */
	0x1900000U,	/* RSC offset from SS base   */
};

struct pdc_seq *g_pdc_seqs = &pdc_seq_instance;
uint32_t g_pdc_seq_count = 1U;
