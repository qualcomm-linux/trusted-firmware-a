/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * CPRH (CPR hardened) public API.
 *
 * CPRH is the closed-loop AVS controller: it programs the per-virtual-corner
 * target quotient and voltage computed by VPP into hardware, then hands
 * the rail voltage over to the SAW4 AVS block for autonomous DCVS tracking.
 * cprh_init() must run after vpp_init() (consumes vpp_get_plan()) and after
 * the CPU PLLs are up (CPR measurement assumes the clock domains it
 * monitors are running).
 */

#ifndef CPRH_H
#define CPRH_H

/* cprh_init - configure and enable the closed-loop AVS controller(s). */
void cprh_init(void);

#endif /* CPRH_H */
