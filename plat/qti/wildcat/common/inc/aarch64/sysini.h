#ifndef __SYSINI_H__
#define __SYSINI_H__

#define SYSINI_POWER_DOWN		0
#define SYSINI_POWER_UP			1

#define SYSINI_CLUSTER_POWER_DOWN	0
#define SYSINI_CLUSTER_POWER_UP		1

#define SYSINI_CLUSTER_POWER_MASK                       0x00FF
#define SYSINI_CLUSTER_POWER_SHIFT                      0
#define SYSINI_CLUSTER_ID_MASK                          0xFF00
#define SYSINI_CLUSTER_ID_SHIFT                         8

#ifndef KRYO_SYSINI_CONFIG_ID
#define KRYO_SYSINI_CONFIG_ID 69
#endif

#ifndef __ASSEMBLY__

#include <stdint.h>

/*
 * cluster_aarch64_sysini(x0, x1) - Cluster/DSU SYSINI for AArch64.
 *
 * Inputs:
 *   arg0 - flags:
 *     [7:0]   - Power up/down; 1 = power up, 0 = power down.
 *     [15:8]  - Cluster0/1/2/...; 2 = Cluster2, 1 = Cluster1,
 *               0 = Cluster0.
 *     [63:16] - General purpose flags.
 *   arg1 - aux_data pointer:
 *     Pointer to auxiliary data structure.
 *
 * Return: None.
 */
void cluster_aarch64_sysini(unsigned long flags, uintptr_t aux_data);

/*
 * cpuss_aarch64_por_sysini(x0, x1) - CPUSS POR SYSINI for AArch64.
 *
 * Inputs:
 *   arg0 - flags:
 *     [7:0]  - Power up/down; 1 = power up, 0 = power down.
 *     [63:8] - General purpose flags.
 *   arg1 - aux_data pointer:
 *     Pointer to auxiliary data structure.
 *
 * Return: None.
 */
extern void cpuss_aarch64_por_sysini(unsigned long flags,
				     uintptr_t aux_data);

/*
 * cpu_aarch64_sysini(x0, x1) - CPU SYSINI for AArch64.
 *
 * Inputs:
 *   arg0 - flags:
 *     [7:0]   - Power up/down; 1 = power up, 0 = power down.
 *     [15:8]  - Disable C2; 0 = enable C2.
 *     [63:16] - General purpose bits.
 *   arg1 - aux_data pointer.
 *
 * Return: None.
 *
 * Description:
 *   Initialize implementation-defined CPU registers and Arm errata
 *   workarounds. CPU-specific functions are called based on the MIDR value.
 */
extern void cpu_aarch64_sysini(unsigned long flags, uintptr_t aux_data);

/*
 * phoenix_gen1_aarch64_sysini(x0, x1).
 *
 * Inputs:
 *   x0:
 *     [7:0]   - Power up/down; 1 = power up, 0 = power down.
 *     [15:8]  - Disable C2; 0 = enable C2.
 *     [63:16] - General purpose bits.
 *   x1 - Aux data pointer.
 *
 * Description:
 *   Initialize Phoenix Gen1 implementation-defined CPU registers and Arm
 *   errata workarounds.
 */
extern void phoenix_gen1_aarch64_sysini(unsigned long flags,
					uintptr_t aux_data);

/*
 * klein_aarch64_sysini(x0, x1).
 *
 * Inputs:
 *   x0:
 *     [7:0]   - Power up/down; 1 = power up, 0 = power down.
 *     [15:8]  - Disable C2; 0 = enable C2.
 *     [63:16] - General purpose bits.
 *   x1 - Aux data pointer.
 *
 * Description:
 *   Initialize Klein implementation-defined CPU registers and Arm errata
 *   workarounds.
 */
extern void klein_aarch64_sysini(unsigned long flags, uintptr_t aux_data);

/*
 * matterhorn_aarch64_sysini(x0, x1).
 *
 * Inputs:
 *   x0:
 *     [7:0]   - Power up/down; 1 = power up, 0 = power down.
 *     [15:8]  - Disable C2; 0 = enable C2.
 *     [63:16] - General purpose bits.
 *   x1 - Aux data pointer.
 *
 * Description:
 *   Initialize Matterhorn implementation-defined CPU registers and Arm errata
 *   workarounds.
 */
extern void matterhorn_aarch64_sysini(unsigned long flags, uintptr_t aux_data);

/*
 * matterhorn_elp_aarch64_sysini(x0, x1).
 *
 * Inputs:
 *   x0:
 *     [7:0]   - Power up/down; 1 = power up, 0 = power down.
 *     [15:8]  - Disable C2; 0 = enable C2.
 *     [63:16] - General purpose bits.
 *   x1 - Aux data pointer.
 *
 * Description:
 *   Initialize Matterhorn ELP implementation-defined CPU registers and Arm
 *   errata workarounds.
 */
extern void matterhorn_elp_aarch64_sysini(unsigned long flags,
					  uintptr_t aux_data);

/*
 * cpu_amu_enable()
 *
 * Inputs: None.
 * Return: None.
 *
 * Description:
 *   Enable counting for the AMU counters.
 */
extern void cpu_amu_enable(void);

/*
 * cpu_amu_disable()
 *
 * Inputs: None.
 * Return: None.
 *
 * Description:
 *   Disable counting for the AMU counters.
 */
extern void cpu_amu_disable(void);

/*
 * kryo_sysini_wa()
 *
 * Inputs:
 *   pf_value - PF value.
 *   config   - Configuration value.
 * Return: None.
 *
 * Description:
 *   Perform PF config writes for silver cores only.
 */
extern void kryo_sysini_wa(unsigned int pf_value, unsigned int config);

/*
 * kryo_sysini_rcc()
 *
 * Inputs:
 *   smc_id - SMC ID.
 * Return: None.
 *
 * Description:
 *   Dynamically control system register bits/configuration.
 */
extern void kryo_sysini_rcc(unsigned int smc_id);

/*
 * cpu_prefetch_tgt_enable()
 *
 * Inputs: None.
 * Return: None.
 *
 * Description:
 *   Dynamically enable the prefetch target feature.
 */
extern void cpu_prefetch_tgt_enable(void);

/*
 * cpu_prefetch_tgt_disable()
 *
 * Inputs: None.
 * Return: None.
 *
 * Description:
 *   Dynamically disable the prefetch target feature.
 */
extern void cpu_prefetch_tgt_disable(void);

/*
 * kryo_sysini_lcc()
 *
 * Inputs:
 *   config - Configuration information:
 *     [31:24] - Config type.
 *     [23:y]  - Reserved.
 *     [y:16]  - Config subtype.
 *     [15:x]  - Reserved.
 *     [x:0]   - Config data.
 * Return: None.
 *
 * Description:
 *   Perform dynamic configuration control based on inputs from the local
 *   software stack.
 */
#define A55_PREFETCH_CONFIG   0x00000000
#define A55_PREFETCH_CACHE_L1 0x10000
#define A55_PREFETCH_CACHE_L3 0x30000
extern void kryo_sysini_lcc(unsigned int config);

/*
 * sysini_get_cpu_logical_num()
 *
 * Inputs: None.
 * Return: Current CPU logical number.
 *
 * Description:
 *   Return the current CPU logical number.
 */
extern uint32_t sysini_get_cpu_logical_num(void);

/*
 * sysini_get_mpidr_cpu_num()
 *
 * Inputs:
 *   mpidr - MPIDR.
 * Return: CPU logical number for the MPIDR.
 *
 * Description:
 *   Return the CPU logical number of the given MPIDR.
 */
extern uint32_t sysini_get_mpidr_cpu_num(uint32_t mpidr);

/*
 * sysini_get_cpu_cluster_num()
 *
 * Inputs: None.
 * Return: Current CPU cluster number.
 *
 * Description:
 *   Return the current CPU cluster number.
 */
extern uint32_t sysini_get_cpu_cluster_num(void);

#endif /* __ASSEMBLY__ */

#endif /* __SYSINI_H__ */
