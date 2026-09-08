/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>

#include <cpu_config.h>

/*
 * Clock-domain to CPU-mask mapping - unused on Nord. qti_pwr_domain_on_finish
 * (plat/qti/wildcat/common/wildcat_pm.c) does NOT call cpucp_clkdom_init():
 * Nord's 3 symmetric Oryon clusters are brought up entirely by the native
 * NCC_ARCH sequence in wildcat_pm.c, with no CPUCP clock-domain SCMI
 * handshake in EL3. This table exists only to satisfy cpucp.c's link-time
 * reference; all domains are pre-marked "already initialised" below so the
 * (unreachable on Nord) clkdom_init path is inert either way.
 */
struct clkdom_cpumask clkdom_cpumasks[CD_MAX] = {
	{ CD_L3,       0x00U },
	{ CD_SILVER,   0x00U },
	{ CD_GOLD,     0x00U },
	{ CD_GOLDPLUS, 0x00U },
};

unsigned int clkdom_init_status[CD_MAX] = {
	1U, 1U, 1U, 1U
};
