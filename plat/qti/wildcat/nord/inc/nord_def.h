/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef NORD_DEF_H
#define NORD_DEF_H

#include <common_def.h>

#include <qti_board_def.h>
#include <nord_ints.h>

/*----------------------------------------------------------------------------*/
/*
 * MPIDR_PRIMARY_CPU
 * The core_affinity_val is represented by [7:0] and cluster_affinity_val is
 * represented by [15:8] while the rest bits are ignored
 */
/*----------------------------------------------------------------------------*/
#define MPIDR_PRIMARY_CPU		0x0000
/*----------------------------------------------------------------------------*/

#define QTI_PWR_LVL0			MPIDR_AFFLVL0
#define QTI_PWR_LVL1			MPIDR_AFFLVL1
#define QTI_PWR_LVL2			MPIDR_AFFLVL2
#define QTI_PWR_LVL3			MPIDR_AFFLVL3

/*
 *  Macros for local power states encoded by State-ID field
 *  within the power-state parameter.
 */
/* Local power state for power domains in Run state. */
#define QTI_LOCAL_STATE_RUN		0
/*
 * Local power state for clock-gating. Valid only for CPU and not cluster power
 * domains
 */
#define QTI_LOCAL_STATE_STB		1
/*
 * Local power state for retention. Valid for CPU and cluster power
 * domains
 */
#define QTI_LOCAL_STATE_RET		2
/*
 * Local power state for OFF/power down. Valid for CPU, cluster, RSC and PDC
 * power domains
 */
#define QTI_LOCAL_STATE_OFF		3
/*
 * Local power state for DEEPOFF/power rail down. Valid for CPU, cluster and RSC
 * power domains
 */
#define QTI_LOCAL_STATE_DEEPOFF		4

/*
 * This macro defines the deepest retention state possible. A higher state
 * id will represent an invalid or a power down state.
 */
#define PLAT_MAX_RET_STATE		QTI_LOCAL_STATE_RET

/*
 * This macro defines the deepest power down states possible. Any state ID
 * higher than this is invalid.
 */
#define PLAT_MAX_OFF_STATE		QTI_LOCAL_STATE_DEEPOFF

/******************************************************************************
 * Required platform porting definitions common to all ARM standard platforms
 *****************************************************************************/

/*
 * Platform specific page table and MMU setup constants.
 */
#define MAX_MMAP_REGIONS		(PLAT_QTI_MMAP_ENTRIES)

/*
 * Number of translation tables for the dynamic xlat library. The Wildcat
 * qti_board_def.h does not define this (unlike the hoya common one), so it is
 * provided here at the platform layer.
 */
#define MAX_XLAT_TABLES			12

/*
 * Platform system counter frequency (19.2 MHz), consumed by
 * plat_get_syscnt_freq2() / wildcat_bl31_setup.c.
 */
#define PLAT_SYSCNT_FREQ		UL(19200000)

#define PLAT_PHY_ADDR_SPACE_SIZE	(1ull << 36)
#define PLAT_VIRT_ADDR_SPACE_SIZE	(1ull << 36)

#define ARM_CACHE_WRITEBACK_SHIFT	6

/*
 * Some data must be aligned on the biggest cache line size in the platform.
 * This is known only to the platform as it might have a combination of
 * integrated and external caches.
 */
#define CACHE_WRITEBACK_GRANULE		(1 << ARM_CACHE_WRITEBACK_SHIFT)

/*
 * One cache line needed for bakery locks on ARM platforms
 */
#define PLAT_PERCPU_BAKERY_LOCK_SIZE	(1 * CACHE_WRITEBACK_GRANULE)

/*----------------------------------------------------------------------------*/
/* PSCI power domain topology definitions */
/*----------------------------------------------------------------------------*/
/* One domain each to represent RSC and PDC level */
#define PLAT_PDC_COUNT			1
#define PLAT_RSC_COUNT			1

/*
 * Nord has three CPU clusters of six Oryon-1 cores each (18 cores total),
 * matching the Nord device tree (three affinity-level-2 groups, cores 0..5 in
 * each).
 */
#define PLAT_CLUSTER_COUNT		3

/* No. of cores in each cluster */
#define PLAT_CLUSTER0_CORE_COUNT	6
#define PLAT_CLUSTER1_CORE_COUNT	6
#define PLAT_CLUSTER2_CORE_COUNT	6

/*
 * Cores in cluster 0. The wildcat MPIDR->linear-index helper
 * (plat_qti_core_pos_by_mpidr) uses PLAT_CORE_COUNT_PER_CLUSTER as the stride
 * to space cluster indices apart, deriving the cluster id from MPIDR AFF2.
 * Nord clusters are uniform (6 cores), so this also equals the per-cluster max.
 */
#define PLAT_CORE_COUNT_PER_CLUSTER	6
#define PLAT_MAX_CPUS_PER_CLUSTER	6

#define PLATFORM_CORE_COUNT		(PLAT_CLUSTER0_CORE_COUNT + \
					 PLAT_CLUSTER1_CORE_COUNT + \
					 PLAT_CLUSTER2_CORE_COUNT)

#define PLAT_NUM_PWR_DOMAINS		(PLAT_PDC_COUNT + \
					 PLAT_RSC_COUNT + \
					 PLAT_CLUSTER_COUNT + \
					 PLATFORM_CORE_COUNT)

#define PLAT_MAX_PWR_LVL		3

/*****************************************************************************/
/* Memory mapped I/O  */
/*****************************************************************************/

/*----------------------------------------------------------------------------*/
/* GIC-700 constants (APSS_GIC700_GICD_APSS_REG_BASE = 0x17000000)            */
/*----------------------------------------------------------------------------*/
#define BASE_GICD_BASE			0x17000000
#define BASE_GICR_BASE			0x17080000
#define BASE_GICC_BASE			0x0
#define BASE_GICH_BASE			0x0
#define BASE_GICV_BASE			0x0

#define QTI_GICD_BASE			BASE_GICD_BASE
#define QTI_GICR_BASE			BASE_GICR_BASE
#define QTI_GICC_BASE			BASE_GICC_BASE

/*----------------------------------------------------------------------------*/
/* UART related constants. (QUPV3_2 SE1)                                      */
/*----------------------------------------------------------------------------*/
#define PLAT_QTI_UART_BASE			0x884000
#define UART_BASE_ADDR				PLAT_QTI_UART_BASE
/* BASE ADDRESS OF DIFFERENT REGISTER SPACES IN HW */
#define GENI4_CFG				0x0
#define GENI4_IMAGE_REGS			0x100
#define GENI4_DATA				0x600

/* COMMON STATUS/CONFIGURATION REGISTERS AND MASKS */
#define GENI_STATUS_REG				(GENI4_CFG + 0x00000040)
#define GENI_STATUS_M_GENI_CMD_ACTIVE_MASK	(0x1)
#define UART_TX_TRANS_LEN_REG			(GENI4_IMAGE_REGS + 0x00000170)
/* MASTER/TX ENGINE REGISTERS */
#define GENI_M_CMD0_REG				(GENI4_DATA + 0x00000000)
/* FIFO, STATUS REGISTERS AND MASKS */
#define GENI_TX_FIFOn_REG			(GENI4_DATA + 0x00000100)

#define GENI_M_CMD_TX				(0x08000000)

/*----------------------------------------------------------------------------*/
/* Peripherals base addresses */
/*----------------------------------------------------------------------------*/
/* RNG_CM_CM_SOC_QRNG_CM */
#define QTI_SEC_PRNG_BASE			0x010C0000
#define QTI_PRNG_LENGTH				0x00001000

/*
 * TME fuse controller (read-only, secure device). Wildcat common maps this in
 * qti_setup_coherent_page_tables(). Nord QFPROM raw fuse region.
 */
#define QTI_TME_FUSE_CONTROLLER_BASE		0x360C0000
#define QTI_TME_FUSE_CONTROLLER_LENGTH		0x00008000

/*----------------------------------------------------------------------------*/
/* Device address space for mapping. Excluding starting 4K */
/*----------------------------------------------------------------------------*/
#define QTI_DEVICE_BASE				0x1000
#define QTI_DEVICE_SIZE				(0x1C000000 - QTI_DEVICE_BASE)

/*----------------------------------------------------------------------------*/
/* AOSS registers (MPM_POWER_STATE_HOLD_REG_BASE) */
/*----------------------------------------------------------------------------*/
#define QTI_PS_HOLD_REG				0x0C264000
/*
 * MPM_PS_HOLD_MASK (QTI_PS_HOLD_REG + 0x4): bit1 TME_WDOG_EXPIRED, bit2
 * PDC_WDOG_EXPIRED, bit0 SECONDARY_DIE_PS_HOLD_ARES (IPCAT-verified, chip
 * 781). Boot-tested (2026-07-06): with TME_WDOG_ENABLE disabled directly
 * (see bl31_platform_setup) TME_WDOG_EXPIRED is STILL observed set at
 * cold-boot BL31 entry - the direct disable races the boundary condition
 * that latches TME's fatal/expired status, it does not prevent it. Masking
 * bit1 here is what actually stops the ~12s PS_HOLD reset (A/B verified:
 * removing this mask reproduces the reset every time; restoring it removes
 * it every time). bit2 (PDC_WDOG_EXPIRED) reads 0 at this point on this
 * board/config - included defensively since it gates through the same
 * register and costs nothing to mask.
 */
#define QTI_MPM_PS_HOLD_MASK			(QTI_PS_HOLD_REG + 0x4U)
#define QTI_MPM_PS_HOLD_MASK_TME_WDOG_EXPIRED	U(0x2)
#define QTI_MPM_PS_HOLD_MASK_PDC_WDOG_EXPIRED	U(0x4)
/*----------------------------------------------------------------------------*/
/* AOP CMD DB address space for mapping (ipcat_ddr.xml AOP_CMD_DB_P)          */
/*----------------------------------------------------------------------------*/
#define QTI_AOP_CMD_DB_BASE			0x87148000
#define QTI_AOP_CMD_DB_SIZE			0x00020000
/*----------------------------------------------------------------------------*/
/* SMEM region populated by the boot firmware (XBL). Consumed by the chipinfo */
/* driver for the SoC id. Base matches the MinPlatform PcdSmemBaseAddress.    */
/*----------------------------------------------------------------------------*/
#define QTI_SMEM_BASE				0x89B00000
#define QTI_SMEM_SIZE				0x00200000
/*----------------------------------------------------------------------------*/
/* SOC hw version register (TCSR 0x01F40000 + 0x88000) */
/*----------------------------------------------------------------------------*/
#define QTI_SOC_VERSION_MASK			U(0xFFFF)
#define QTI_SOC_REVISION_REG			0x01FC8000
#define QTI_SOC_REVISION_MASK			U(0xFFFF)
/*----------------------------------------------------------------------------*/
/* LC PON register offsets */
/*----------------------------------------------------------------------------*/
#define PON_PS_HOLD_RESET_CTL			0x852
#define PON_PS_HOLD_RESET_CTL2			0x853
/*----------------------------------------------------------------------------*/
/* APSS HM registers */
/*----------------------------------------------------------------------------*/
#define QTI_APSS_HM_BASE			0x17000000
#define QTI_APSS_HM_SIZE			0x00d99000
/*----------------------------------------------------------------------------*/
/* AOSS registers */
/*----------------------------------------------------------------------------*/
#define QTI_AOSS_BASE				0x0b000000
#define QTI_AOSS_SIZE				0x04000000
/*----------------------------------------------------------------------------*/
/* CORE_TOP_CSR */
/*----------------------------------------------------------------------------*/
#define QTI_CORE_TOP_CSR_BASE			0x01f00000
#define QTI_CORE_TOP_CSR_BASE_SIZE		0x00100000
/*----------------------------------------------------------------------------*/
/* QTIMER registers (APSS_PRIMARY_QTMR_BASE)                                  */
/*----------------------------------------------------------------------------*/
#define QTI_QTIMER_BASE				0x17810000

/*----------------------------------------------------------------------------*/
/* TF-A shared memory region.                                                 */
/* OCIMEM scratch (ipcat_ddr.xml OCIMEM 0x85100000).                          */
/*----------------------------------------------------------------------------*/
#define TFA_SHARED_MEMORY_BASE			0x85100000
#define TFA_SHARED_MEMORY_SIZE			0x00001000
/*----------------------------------------------------------------------------*/
#endif /* NORD_DEF_H */
