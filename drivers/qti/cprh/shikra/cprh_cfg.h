/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CPRH_CFG_H
#define CPRH_CFG_H

#include "../cprh_internal.h"

/* Number of CPR controllers (rails) on shikra: APC0 only. */
#define CPRH_MAX_CONTROLLERS		1U

/* Number of CPR threads (clock domains) per controller. */
#define CPRH_MAX_THREADS		3U

/* CPR refclk frequency in kHz. */
#define CPRH_REFCLK_RATE_KHZ		19200U

extern cprh_controller_context *cprh_context[CPRH_MAX_CONTROLLERS];
extern cprh_saw_context *saw_context[CPRH_MAX_CONTROLLERS];

void cprh_init_config(void);

#endif /* CPRH_CFG_H */
