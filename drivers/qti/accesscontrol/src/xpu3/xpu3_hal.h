/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef XPU3_HAL_H
#define XPU3_HAL_H

#include <stdint.h>

#include <xpu3.h>

/*
 * XPU3 register programming.
 *
 * Everything that touches an XPU3 instance's registers lives behind this
 * interface; the driver in xpu3.c decides which instances and resource groups
 * to program and calls in here to do it. Each function takes the instance it
 * operates on, and those that act on resource groups take either a specific
 * group number or XPU_RG_ALL.
 */

/* Read IDR0.XPUTYPE. Compare against XPU_TYPE_MPU. */
uint8_t xpu3_hal_get_xpu_type(struct xpu_instance *xpu);

/* Number of resource groups the instance implements, from IDR0.NRG. */
uint32_t xpu3_hal_get_idr0_nrg(struct xpu_instance *xpu);

/*
 * Enable the instance and its two QAD ports, and unmask their error reporting.
 * Idempotent: returns early if this driver already brought the instance up, or
 * if an earlier boot stage left GCR0 programmed.
 */
void xpu3_hal_enable_domain(struct xpu_instance *xpu);

/* Program the address range of each matching resource group. MPU only. */
void xpu3_hal_program_mpu_partitions(struct xpu_instance *xpu, uint32_t rg_num);

/* Assign each matching resource group, and the unmapped region, to its owner. */
void xpu3_hal_set_region_ownership(struct xpu_instance *xpu, uint32_t rg_num);

/*
 * Apply the read and write permissions the configuration tables carry, for each
 * matching resource group and for the unmapped region. MPU only.
 */
void xpu3_hal_set_mpu_permissions(struct xpu_instance *xpu, uint32_t rg_num);

/*
 * Apply permissions supplied at runtime rather than from the tables, used when a
 * region is assigned or released. MPU only.
 */
void xpu3_hal_set_mpu_dynamic_permissions(struct xpu_instance *xpu,
					  uint32_t rg_num, uint32_t perm_r,
					  uint32_t perm_w);

#endif /* XPU3_HAL_H */
