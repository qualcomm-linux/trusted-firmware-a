/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Public interface for the QTI ATF clock driver.
 *
 * Scoped to the lemans (qcs9075) boot use case: clock-group bring-up and
 * teardown. The generic by-name clock-control, DFS and RPM-resource APIs from
 * the original vendor interface are not provided for this target.
 */

#ifndef CLOCK_H
#define CLOCK_H

/* Logical groups of related clocks. */
enum clock_group_type {
	CLOCK_GROUP_ABT,
	CLOCK_GROUP_BUS,
	CLOCK_GROUP_INIT,
	CLOCK_GROUP_QDSS,
	CLOCK_GROUP_EUD,
	CLOCK_GROUP_INIT_SSC,
	CLOCK_GROUP_INIT_GPU,
	CLOCK_GROUP_INIT_CAMERA,
	CLOCK_GROUP_INIT_DISPLAY,
	CLOCK_GROUP_INIT_AUDIO,
	CLOCK_GROUP_VSENSE_PRIMARY,
	CLOCK_GROUP_VSENSE_SECONDARY,
	CLOCK_GROUP_INIT_VIDEO,
	CLOCK_GROUP_INIT_DISPLAY_1,
	CLOCK_GROUP_INIT_PCIE,
	CLOCK_GROUP_INIT_NSP,
	CLOCK_GROUP_INIT_MODEM,

	CLOCK_GROUP_TOTAL
};

/*
 * Enable clocks necessary for TZ initialization. Must be called before any
 * other clock API.
 */
#ifdef QTI_CLOCK_ENABLED
void qti_clock_init(void);
#else
static inline void qti_clock_init(void) {}
#endif

/* Disable clocks that were only needed during TZ initialization. */
#ifdef QTI_CLOCK_ENABLED
void qti_clock_init_done(void);
#else
static inline void qti_clock_init_done(void) {}
#endif

/* Enable all clocks in a group, plus any required power domains. */
int clock_enable_clock_group(enum clock_group_type group);

/* Disable all clocks in a group, plus any required power domains. */
int clock_disable_clock_group(enum clock_group_type group);

#endif /* CLOCK_H */
