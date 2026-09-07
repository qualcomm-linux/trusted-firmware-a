/*
 * Copyright (c) 2026, Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef FUSEPROV_PORT_TME_H
#define FUSEPROV_PORT_TME_H

#include <drivers/qti/fuseprov/fuseprov_transport.h>

/* Get the TME transport instance for Wildcat/Nord platforms */
const fuseprov_transport_t *fuseprov_port_tme_get(void);

#endif /* FUSEPROV_PORT_TME_H */
