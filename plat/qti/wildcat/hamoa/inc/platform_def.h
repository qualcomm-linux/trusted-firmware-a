/*
 * Copyright (c) 2018, Arm Limited and Contributors. All rights reserved.
 * Copyright (c) 2018-2021, The Linux Foundation. All rights reserved.
 * Portions copyright (c) 2026, Qualcomm Technologies, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLATFORM_DEF_H
#define PLATFORM_DEF_H

/* Enable the dynamic translation tables library. */
#define PLAT_XLAT_TABLES_DYNAMIC 1

#include <common_def.h>

#include <qti_board_def.h>
#include <qti_platform_pm.h>

/*----------------------------------------------------------------------------*/

/******************************************************************************
 * Required platform porting definitions common to all ARM standard platforms
 *****************************************************************************/

/*
 * Platform specific page table and MMU setup constants.
 */
#define MAX_MMAP_REGIONS (PLAT_QTI_MMAP_ENTRIES)

/*
 * Platform specific page table and MMU setup constants
 */
#define MAX_XLAT_TABLES 16

#define PLAT_PHY_ADDR_SPACE_SIZE (1ULL << 36)
#define PLAT_VIRT_ADDR_SPACE_SIZE (1ULL << 36)

#define ARM_CACHE_WRITEBACK_SHIFT 6

/*
 * Some data must be aligned on the biggest cache line size in the platform.
 * This is known only to the platform as it might have a combination of
 * integrated and external caches.
 */
#define CACHE_WRITEBACK_GRANULE (1 << ARM_CACHE_WRITEBACK_SHIFT)

/*
 * One cache line needed for bakery locks on ARM platforms
 */
#define PLAT_PERCPU_BAKERY_LOCK_SIZE (1 * CACHE_WRITEBACK_GRANULE)

/*****************************************************************************/
/* Memory mapped Generic timer interfaces  */
/*****************************************************************************/

/*----------------------------------------------------------------------------*/
/* GIC-700 constants */
/*----------------------------------------------------------------------------*/
#define BASE_GICD_BASE 0x17000000
#define BASE_GICR_BASE 0x17080000
#define BASE_GICC_BASE 0x0
#define BASE_GICH_BASE 0x0
#define BASE_GICV_BASE 0x0

#define QTI_GICD_BASE BASE_GICD_BASE
#define QTI_GICR_BASE BASE_GICR_BASE
#define QTI_GICC_BASE BASE_GICC_BASE

/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/* UART related constants. */
/*----------------------------------------------------------------------------*/
/* BASE ADDRESS OF DIFFERENT REGISTER SPACES IN HW */
#define UART_BASE_ADDR 0x00894000 /* QUPV3_2_SE5 */
#define GENI4_CFG 0x0
#define GENI4_IMAGE_REGS 0x100
#define GENI4_DATA 0x600

/* COMMON STATUS/CONFIGURATION REGISTERS AND MASKS */
#define GENI_STATUS_REG (GENI4_CFG + 0x00000040)
#define GENI_STATUS_M_GENI_CMD_ACTIVE_MASK (0x1)
#define UART_TX_TRANS_LEN_REG (GENI4_IMAGE_REGS + 0x00000170)
/* MASTER/TX ENGINE REGISTERS */
#define GENI_M_CMD0_REG (GENI4_DATA + 0x00000000)
/* FIFO, STATUS REGISTERS AND MASKS */
#define GENI_TX_FIFOn_REG (GENI4_DATA + 0x00000100)

#define GENI_M_CMD_TX (0x08000000)

/*----------------------------------------------------------------------------*/
/* Device address space for mapping. Excluding starting 1MB */
/*----------------------------------------------------------------------------*/
#define QTI_DEVICE_BASE 0x100000
#define QTI_DEVICE_SIZE (0x80000000 - QTI_DEVICE_BASE)

/* Memory Mapped Peripheral(MMP) address ranges used for dynamic mapping. */
/*----------------------------------------------------------------------------*/
/* Devices for re-mapping as read-only (QTI_DEVICE cover all as RW) */
/*----------------------------------------------------------------------------*/

/*
 * Used single one (FUSE_CONTROLLER) to cover others to reduce XLAT table usage
 *
 * #define QTI_QFPROM_RAW_BASE          0x221C0000 QFPROM_RAW
 * #define QTI_QFPROM_RAW_LENGTH        0x00002000
 *
 * #define QTI_SEC_JTAG_BASE            0x221C4000
 *                                      FUSE_CONTROLLER_SW_RANGE0
 * #define QTI_SEC_JTAG_LENGTH          0x00001000
 *
 * #define QTI_QFPROM_CORR_BASE         0x221C2000 QFPROM_CORR
 * #define QTI_QFPROM_CORR_LENGTH       0x00002000
 *
 * #define QTI_SECURITY_CONTROL_BASE    0x221C8000
 *                                      FUSE_CONTROLLER_SW_RANGE4
 * #define QTI_SECURITY_CONTROL_LENGTH  0x00002000
 *                                      + FUSE_CONTROLLER_SW_RANGE5
 */
#define QTI_TME_FUSE_CONTROLLER_BASE	0x221C0000
#define QTI_TME_FUSE_CONTROLLER_LENGTH	0x10000 /* (64 KB) */

#define QTI_SEC_PRNG_BASE		0x010C0000 /* RNG_CM_CM_SOC_QRNG_CM */
#define QTI_PRNG_LENGTH		0x00002000 /* + RNG_EE1_EE1_SOC_QRNG_SUB */

/*******************************************************************************
 * BL31 specific defines.
 ******************************************************************************/
#define TFA_SHARED_MEMORY_BASE  0xD856A000
/*
 * NB: this value must be kept in sync with the TZ one
 * (TFA_SHARED_DATA_SECTION_SIZE).
 */
#define TFA_SHARED_MEMORY_SIZE 0x00002000

#define BL31_BASE (TFA_SHARED_MEMORY_BASE + TFA_SHARED_MEMORY_SIZE)
#define BL31_SIZE 0x000A6000	/* 664 KB */

/*
 * Put BL31 at DDR as per memory map. BL31_BASE is calculated using the
 * current BL31 debug size plus a little space for growth.
 */
#define BL31_LIMIT (BL31_BASE + BL31_SIZE)

/*----------------------------------------------------------------------------*/
/* SHARED_IMEM address space for mapping */
/*----------------------------------------------------------------------------*/
#define SHARED_IMEM_BASE 0x146AA000
#define SHARED_IMEM_SIZE 0x00001000

/*----------------------------------------------------------------------------*/
/* TFA address space for mapping */
/*----------------------------------------------------------------------------*/
#define TFA_IMEM_BASE 0x1468D000
/* Ring buffer + TFA Reserved */
#define TFA_IMEM_SIZE 0x00002000

/*----------------------------------------------------------------------------*/
/* Shared IMEM space for QSEE/TZ */
/*----------------------------------------------------------------------------*/
/*
 * ==========================
 * 100K starting 0x14680000
 * ==========================
 * The address layout below appears in TF-A, TZ and SECLIB, and
 * the three should be kept in sync.
 * Currently the TF-A logs are set placed at the start of the
 * QSEE_TZ_IMEM region
 * TZ: securemsm/platform/config/cfg/hamoa/tz_self_configured.xml
 * SECLIB: SecPkg/Library/SecTargetLib/chipsets/hamoa/seclib_target.h
 * TFA: tfa_images/tfa_upstream/plat/qti/v2/hamoa/inc/platform_def.h
 *
 * Start Address    Page Size       Region
 * =======================================
 * 14680000         52K             Reserved
 * 1468D000         4K              TFA ringbuffer
 * 1468E000         4K              TFA Reserved
 * ... other shared data
 */
#define TFA_BL31_RING_BUFFER_IN_TZ_IMEM_BASE            (TFA_IMEM_BASE)

/*----------------------------------------------------------------------------*/
/* AOP CMD DB  address space for mapping */
/*----------------------------------------------------------------------------*/
#define QTI_AOP_CMD_DB_BASE 0x81C60000
#define QTI_AOP_CMD_DB_SIZE 0x00020000
/*----------------------------------------------------------------------------*/
/* SMEM base address                                                          */
/*----------------------------------------------------------------------------*/
#define QTI_SMEM_BASE ULL(0xFFE00000)
#define QTI_SMEM_SIZE ULL(0x00200000)
/*----------------------------------------------------------------------------*/
/* LC PON register offsets */
/*----------------------------------------------------------------------------*/
#define PON_PS_HOLD_RESET_CTL 0x852
#define PON_PS_HOLD_RESET_CTL2 0x853

/*
 * The Qualcomm QGIC2 implementation seems to have PIDR0-4 and PIDR4-7
 * erroneously swapped for some reason. PIDR2 is actually at 0xFD8.
 * Override the address in <drivers/arm/gicv2.h> to avoid a failing assert().
 */
#define GICD_PIDR2_GICV2 U(0xFFE8)

/* Timer */
#define PLAT_SYSCNT_FREQ 19200000

/* Boot IMEM region */
#define BOOT_IMEM_BASE 0x14800000
#define BOOT_IMEM_SIZE 0x00400000

/*
 * Number if boot image entries supported for the platform
 * Change to this size will impact boot_qsee_interface shared structure,
 * which is shared accross BOOT, TFA and QTEE
 */
#define BOOT_IMAGES_NUM_ENTRIES 25

/*
 * TZ AC config and HYP AC config SW ids and base address
 */
#define TZ_AC_CONFIG_SW_TYPE                       0x96
#define HYP_AC_CONFIG_SW_TYPE                      0x97
#define TZ_TZ_AC_CONFIG_BASE_ADDR                  0xD8019000
#define TZ_HYP_AC_CONFIG_BASE_ADDR                 0xD8000000

#endif /* PLATFORM_DEF_H */
