/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * RIMPS: Silver/Gold/L3 CPU PLL bring-up plus the full OSM/DCVS runtime
 * (CPUCP/RIMPS firmware load, VPP-derived LUT/ACD/AVG/BCL programming,
 * APM/MEMACC init, and enabling all three OSM domains).
 */

#ifndef QTI_RIMPS_H
#define QTI_RIMPS_H

/*
 * One-time boot init: enables the Silver, Gold and L3 CPU PLLs and switches
 * each domain's mux onto its PLL at a safe cold-boot frequency. Must run
 * before pcu_sequencer_init(), since RIMPS's opcode sequences assume the
 * PLLs are already locked and outputting.
 */
void rimps_cpu_pll_init(void);

/*
 * Full OSM/DCVS bring-up: loads the CPUCP firmware, derives each domain's
 * LUT/ACD/AVG/BCL/APM/MEMACC configuration from the VPP voltage plan, and
 * enables the L3, Silver and Gold OSM domains with an initial perf-state
 * request. Must run after rimps_cpu_pll_init() and vpp_init(), and
 * before cprh_init() (whose closed-loop AVS controller assumes OSM is
 * already enabled and voting).
 */
void rimps_dcvs_init(void);

#endif /* QTI_RIMPS_H */
