/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 * Copyright (c) 2018-2020, The Linux Foundation. All rights reserved.
 * Copyright (c) 2026 Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef QTI_PLAT_H
#define QTI_PLAT_H

#include <stdint.h>

#include <common/bl_common.h>
#include <common/interrupt_props.h>
#include <lib/cassert.h>
#include <lib/el3_runtime/cpu_data.h>
#include <lib/xlat_tables/xlat_tables_v2.h>

#define QTI_INVALID_CLUSTER_ID  ((uint32_t)-1)
#define QTI_INVALID_CPU_ID      ((uint32_t)-1)
#define QTI_INVALID_MPID_ID     ((uint32_t)-1)

/*
 * Utility functions common to QTI platforms
 */

/**
 * qti_ns_va_to_pa
 *
 * Translate a non-secure virtual address to a physical address.
 *
 * Temporarily asserts SCR_EL3.NS and issues an ATS instruction (ATS1E2R for
 * EL2 clients, ATS12E1R for EL1 clients) to perform the address translation,
 * then reads the result from PAR_EL1.
 *
 * @param[in]  va           NS virtual address to translate.
 * @param[in]  client_mode  Exception level of the NS caller; must be MODE_EL1
 *                          or MODE_EL2.
 * @param[out] pa_out       Receives the translated physical address on success.
 * @return 0 on success, -1 if the translation faulted (PAR_EL1.F set).
 */
int qti_ns_va_to_pa(uintptr_t va, unsigned int client_mode, uintptr_t *pa_out);

int qti_mmap_add_dynamic_region(uintptr_t base_pa, size_t size,
				unsigned int attr);
int qti_mmap_remove_dynamic_region(uintptr_t base_va, size_t size);

/*
 * Utility functions common to ARM standard platforms.
 * TODO: The below CR is created to handle page table
 * with and without coherent memory.
 * https://orbit/CR/4444422
 */
#if USE_COHERENT_MEM
void qti_setup_page_tables(uintptr_t total_base, size_t total_size,
			   uintptr_t code_start, uintptr_t code_limit,
			   uintptr_t rodata_start, uintptr_t rodata_limit,
			   uintptr_t coherent_ram_start,
			   uintptr_t coherent_ram_limit);
#else
void qti_setup_page_tables(uintptr_t total_base, size_t total_size,
			   uintptr_t code_start, uintptr_t code_limit,
			   uintptr_t rodata_start, uintptr_t rodata_limit);
#endif

/*
 * Mandatory functions required in ARM standard platforms
 */
void plat_qti_gic_driver_init(void);
void plat_qti_gic_init(void);
void plat_qti_gic_cpuif_enable(void);
void plat_qti_gic_cpuif_disable(void);
void plat_qti_gic_pcpu_init(void);
const interrupt_prop_t *plat_qti_get_interrupt_props(unsigned int *num_props);
void plat_qti_invoke_unhandled_isr(uint32_t id, void *handle);

/*
 * Optional functions required in ARM standard platforms
 */
unsigned int plat_qti_core_pos_by_mpidr(u_register_t mpidr);
unsigned int plat_qti_my_cluster_pos(void);

void gic_set_spi_routing(unsigned int id, unsigned int irm, u_register_t mpidr);

void qti_pmic_prepare_reset(void);
void qti_pmic_prepare_shutdown(void);

/* Optional functions required in ARM standard platforms */
unsigned int plat_qcom_core_pos_by_mpidr(u_register_t mpidr);
unsigned int find_cluster_id(void);
unsigned int find_cluster_id_by_mpidr(u_register_t mpidr);

/**
 *  @brief - Returns cluster id for logical cpu number
 *
 *  @param - cpu number
 *  @return - cluster id if found successful, otherwise return error
 */
uint32_t plat_qti_get_cluster_id_from_logical_cpu_num(uint32_t cpu_num);

/**
 *  @brief - Returns core id for logical cpu number
 *
 *  @param - cpu number
 *  @return - core id if found successful, otherwise return error
 */
uint32_t plat_qti_get_core_id_from_logical_cpu_num(uint32_t cpu_num);

/**
 *  @brief - Returns mpir val of logical cpu number
 *
 *  @param - cpu number
 *  @return - mpidr if found successful, otherwise return error
 */
uint32_t plat_qti_logical_cpu_num_to_mpidr(uint32_t cpu_num);

typedef struct chip_id_info {
	uint16_t jtag_id;
	uint16_t chipinfo_id;
} chip_id_info_t;

#endif /* QTI_PLAT_H */
