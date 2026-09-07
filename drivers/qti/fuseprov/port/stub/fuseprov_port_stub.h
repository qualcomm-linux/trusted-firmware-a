/*
 * Copyright (c) 2026, Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef FUSEPROV_PORT_STUB_H
#define FUSEPROV_PORT_STUB_H

#include <stdint.h>
#include <drivers/qti/fuseprov/fuseprov_transport.h>

/* Get the stub transport instance for off-target testing */
const fuseprov_transport_t *fuseprov_port_stub_get(void);

/* Reset stub storage for testing */
void fuseprov_port_stub_reset(void);

/* Get fuse value from stub storage (for testing) */
void fuseprov_port_stub_get_fuse(uint32_t addr, uint32_t out[2]);

#endif /* FUSEPROV_PORT_STUB_H */
