/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CPRH_HAL_H
#define CPRH_HAL_H

#include <stdint.h>

#include "../cprh_internal.h"

void cprh_hal_create_controller(cprh_controller_context *controller_context);

void cprh_hal_create_thread(cprh_controller_context *controller_context,
			     cprh_thread_context *thread_context);

uint32_t cprh_hal_get_target(cprh_controller_context *controller_context,
			      uint8_t cprh_thread, uint8_t ro);

void cprh_hal_config_target(cprh_controller_context *controller_context,
			     uint8_t cprh_thread, uint8_t ro, uint32_t target);

void cprh_hal_config_gcnt(cprh_controller_context *controller_context,
			   uint8_t ro, uint32_t gcnt);

void cprh_hal_config_virtual_corner(cprh_controller_context *controller_context,
				     uint8_t cprh_thread,
				     cprh_virtual_corner_context *vc_context);

uint32_t cprh_hal_get_base_voltage(cprh_controller_context *controller_context);

void cprh_hal_enable_dcvsh(cprh_controller_context *controller_context,
			    uint8_t enable);

void cprh_hal_apm_threshold_set(cprh_controller_context *controller_context,
				 uint16_t apm_threshold);

void cprh_hal_apm_enable(cprh_controller_context *controller_context,
			  uint8_t enable);

void cprh_hal_enable_measurement(cprh_controller_context *controller_context,
				  uint8_t enable);

#endif /* CPRH_HAL_H */
