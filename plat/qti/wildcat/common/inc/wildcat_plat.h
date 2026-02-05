/*
 * Copyright (c) 2026 Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef WILDCAT_PLAT_H
#define WILDCAT_PLAT_H

#include <stdint.h>

#include <lib/coreboot.h>

#include <platform_def.h>
#include <qti_plat.h>
#include <qti_ringbuf_console.h>

#define QTI_INVALID_CLUSTER_ID	((uint32_t)-1)
#define QTI_INVALID_CPU_ID	((uint32_t)-1)
#define QTI_INVALID_MPID_ID	((uint32_t)-1)

#if COREBOOT
#define QTI_BL31_EARLY_UART_BASE_ADDR	coreboot_serial.baseaddr
#elif defined(QTI_UART_CONSOLE)
#define QTI_BL31_EARLY_UART_BASE_ADDR	UART_BASE_ADDR
#else
#define QTI_BL31_EARLY_UART_BASE_ADDR	0U
#endif

#if USE_COHERENT_MEM
#define QTI_BL31_COHERENT_RAM_BASE	BL_COHERENT_RAM_BASE
#define QTI_BL31_COHERENT_RAM_END	BL_COHERENT_RAM_END
#else
#define QTI_BL31_COHERENT_RAM_BASE	0U
#define QTI_BL31_COHERENT_RAM_END	0U
#endif

struct console_ringbuf *plat_qti_get_bl31_ringbuf(void);

int qti_ns_va_to_pa(uintptr_t va, unsigned int client_mode,
		    uintptr_t *pa_out);

uint32_t plat_qti_get_cluster_id_from_logical_cpu_num(uint32_t cpu_num);
uint32_t plat_qti_get_core_id_from_logical_cpu_num(uint32_t cpu_num);
uint32_t plat_qti_logical_cpu_num_to_mpidr(uint32_t cpu_num);

#endif /* WILDCAT_PLAT_H */
