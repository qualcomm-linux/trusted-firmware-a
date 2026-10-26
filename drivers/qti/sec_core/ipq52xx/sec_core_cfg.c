/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stddef.h>
#include <stdint.h>

#include <drivers/qti/sec_core/sec_core.h>
#include <lib/utils_def.h>

#include <sec_core_defs.h>

#define APCS_ALIAS_APC_SECURE(n)					\
	{ APCS_ALIAS0_APC_SECURE_ADDR +				\
		  ((n) * APCS_ALIASn_APC_SECURE_OFFSET_TO_NEXT),	\
	  APCS_ALIAS0_APC_SECURE_RMSK }

#define APCS_ALIAS_SAW4_SECURE(n)					\
	{ APCS_ALIAS0_SAW4_SECURE_ADDR +				\
		  ((n) * APCS_ALIASn_SAW4_SECURE_OFFSET_TO_NEXT),	\
	  APCS_ALIAS0_SAW4_SECURE_RMSK }

const struct sec_core_cfg_s qti_sec_core_cfg[] = {
	{APCS_WDT_TMR1_WDOG_SECURE_ADDR, APCS_WDT_TMR1_WDOG_SECURE_RMSK},
	APCS_ALIAS_APC_SECURE(0),
	APCS_ALIAS_APC_SECURE(1),
	APCS_ALIAS_APC_SECURE(2),
	APCS_ALIAS_APC_SECURE(3),
	APCS_ALIAS_SAW4_SECURE(0),
	APCS_ALIAS_SAW4_SECURE(1),
	APCS_ALIAS_SAW4_SECURE(2),
	APCS_ALIAS_SAW4_SECURE(3),
	{APCS_ALIAS0_GLB_SECURE_ADDR, APCS_ALIAS0_GLB_SECURE_RMSK},
	{APCS_CLUS0_L2_SAW4_SECURE_ADDR, APCS_CLUS0_L2_SAW4_SECURE_RMSK},
	/*
	 * Vendor clears only the GLB_CTL bit here (read-modify-write).
	 * RMSK for this register is 0x1, i.e. GLB_CTL is the only bit
	 * that exists, so an unconditional write of 0x0 is equivalent.
	 */
	{APCS_ALIAS0_CFG_SECURE_ADDR, 0x0},
	{APCS_C0_PLL_SECURE_ADDR, APCS_C0_PLL_SECURE_RMSK},
};

const size_t qti_sec_core_cfg_count = ARRAY_SIZE(qti_sec_core_cfg);

const uintptr_t qti_sec_core_rvbaraddr_lo = RVBARADDR_LO_ADDR;
const uintptr_t qti_sec_core_rvbaraddr_hi = RVBARADDR_HI_ADDR;
