/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Interconnect bus arbiter (ICB) interface used by the QTI clock driver.
 * The BSP describes bandwidth votes through these aliases; the platform
 * ICB back-end (drivers/qti/icb/uarb) provides icbuarb_create_client()
 * and icbuarb_issue_request().
 */

#ifndef CLOCK_ICBUARB_H
#define CLOCK_ICBUARB_H

#include <drivers/qti/icb/icbid.h>
#include <drivers/qti/icb/icbuarb.h>

typedef enum icbid_master	icb_master_id;
typedef enum icbid_slave	icb_slave_id;

#endif /* CLOCK_ICBUARB_H */
