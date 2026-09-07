/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef QTI_QGIC_H
#define QTI_QGIC_H

#include <stdint.h>

#include <lib/utils_def.h>

/*
 * Single-argument ISR type. Both platform families are normalised to this
 * signature by their respective implementation file (qgic_intr_svc.c on hoya,
 * qgic_intr_el3.c on wildcat). Ported drivers use this API exclusively and
 * never include the family-specific interrupt-service headers.
 */
typedef void *(*qti_isr_t)(void *ctx);

int qti_gic_register_isr(uint32_t intr, qti_isr_t fn, void *ctx);
void qti_gic_unregister_isr(uint32_t intr);
void qti_gic_enable_intr(uint32_t intr);
void qti_gic_disable_intr(uint32_t intr);
/* irm: GICV3_IRM_PE (route to mpidr) or GICV3_IRM_ANY */
void qti_gic_set_spi_routing(uint32_t intr, unsigned int irm,
			     u_register_t mpidr);

#endif /* QTI_QGIC_H */
