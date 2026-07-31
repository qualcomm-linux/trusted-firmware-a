/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>

#include <cpu_config.h>

/*
 * Clock-domain to CPU-mask mapping for lemans (qcs9075).
 *
 * The qcs9075 APSS exposes eight CPUs split across two DSU clusters: CPU0-CPU3
 * belong to DSU0 (silver clock domain) and CPU4-CPU7 belong to DSU1 (gold
 * clock domain), matching the PSCI node topology. The goldplus domain is not
 * populated on the EVK.
 */
#define LEMANS_SILVER_CORES_CPUMASK	0x0fU
#define LEMANS_GOLD_CORES_CPUMASK	0xf0U
#define LEMANS_GOLDPLUS_CORES_CPUMASK	0x00U

struct clkdom_cpumask clkdom_cpumasks[CD_MAX] = {
	{ CD_L3,       0x00U                          },
	{ CD_SILVER,   LEMANS_SILVER_CORES_CPUMASK    },
	{ CD_GOLD,     LEMANS_GOLD_CORES_CPUMASK      },
	{ CD_GOLDPLUS, LEMANS_GOLDPLUS_CORES_CPUMASK  },
};

/*
 * On lemans the CPU cluster clock domains (OSM) are managed outside EL3: the
 * boot firmware brings up the boot (silver) cluster and the kernel EPSS/OSM
 * driver drives every cluster's frequency at runtime. Issuing the CPUCP clock
 * domain enable SCMI from EL3 is therefore unnecessary, and when CPUCP does not
 * service it each core of the domain stalls on the SCMI timeout before coming
 * online. Flag all populated domains as already initialised so secondary
 * bringup never blocks on that SCMI. The goldplus domain is not populated.
 */
unsigned int clkdom_init_status[CD_MAX] = {
	1U, 1U, 1U, 0U
};
