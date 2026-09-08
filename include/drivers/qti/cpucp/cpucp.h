/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef QTI_CPUCP_H
#define QTI_CPUCP_H

/*
 * Lean CPUCP host interface for TF-A. Only the clock-domain enable path that
 * is required during secondary-core cold boot is implemented natively. The
 * full CPUCP firmware-load / DCVS machinery still lives in qtiseclib.
 */

/* Request to enable the clock domain owning the calling core on cold boot. */
void cpucp_clkdom_init(void);

/*
 * Notify CPUCP that the given core (identified by its NCC core/cluster index,
 * matching the mpidr core/cluster fields) has just powered on or off, via an
 * SCMI Power Domain Management POWER_STATE_SET command, which Nord's native
 * PSCI path does not otherwise send.
 */
void cpucp_notify_core_power_on(unsigned int core, unsigned int cluster);
void cpucp_notify_core_power_off(unsigned int core, unsigned int cluster);

#endif /* QTI_CPUCP_H */
