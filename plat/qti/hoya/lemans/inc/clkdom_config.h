/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef QTI_CPUCP_LEMANS_CLKDOM_CONFIG_H
#define QTI_CPUCP_LEMANS_CLKDOM_CONFIG_H

#define LEMANS_CL0_GOLD_CORES_CPUMASK	0x0fU
#define LEMANS_CL1_GOLD_CORES_CPUMASK	0xf0U

/*
 * Clock domains for lemans (qcs9075). The APSS is organised as two Gold DSU
 * clusters, each with its own L3 and CPU (Gold) clock domain.
 */
enum plat_clock_domain_id {
	CD_CL0_L3,
	CD_CL0_GOLD,
	CD_CL1_L3,
	CD_CL1_GOLD,
	CD_MAX
};

struct clkdom_parent_map {
	unsigned int core_mask;
	enum plat_clock_domain_id cpu_id;
	enum plat_clock_domain_id l3_id;
};

struct clkdom_parent_map plat_clkdom_parent_maps[] = {
	{ LEMANS_CL0_GOLD_CORES_CPUMASK, CD_CL0_GOLD, CD_CL0_L3 },
	{ LEMANS_CL1_GOLD_CORES_CPUMASK, CD_CL1_GOLD, CD_CL1_L3 },
};

unsigned int plat_clkdom_init_status[CD_MAX] = {
	0U, 0U, 1U, 0U
};
#endif /* QTI_CPUCP_LEMANS_CLKDOM_CONFIG_H */
