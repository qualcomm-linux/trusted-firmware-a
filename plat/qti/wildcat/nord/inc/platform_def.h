/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
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

/*
 * Platform specific page table and MMU setup constants.
 */
#define MAX_MMAP_REGIONS (PLAT_QTI_MMAP_ENTRIES)

/*
 * Platform specific page table and MMU setup constants
 */
#define MAX_XLAT_TABLES 16

#define PLAT_PHY_ADDR_SPACE_SIZE (1ull << 36)
#define PLAT_VIRT_ADDR_SPACE_SIZE (1ull << 36)

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
/* UART related constants. */
/*----------------------------------------------------------------------------*/
#define UART_BASE_ADDR 0x00884000 /* QUPV3_2_SE1 */
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

/*----------------------------------------------------------------------------*/
/* Devices for re-mapping as read-only (QTI_DEVICE cover all as RW) */
/*----------------------------------------------------------------------------*/

#define QTI_TME_FUSE_CONTROLLER_BASE    0x360C0000
#define QTI_TME_FUSE_CONTROLLER_LENGTH  0x20000   /* (128 KB) */

#define QTI_SEC_PRNG_BASE            0x010C0000 /* RNG_CM_CM_SOC_QRNG_CM */
#define QTI_PRNG_LENGTH              0x00001000

/*----------------------------------------------------------------------------*/
/* BL31 specific defines. */
/*----------------------------------------------------------------------------*/
#define TFA_SHARED_MEMORY_BASE	0xBC180000
#define TFA_SHARED_MEMORY_SIZE	0x0
#define BL31_BASE (TFA_SHARED_MEMORY_BASE + TFA_SHARED_MEMORY_SIZE)
#define BL31_SIZE 0x000A6000	/* 664 KB */

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
#define QTI_AOP_CMD_DB_BASE 0x87148000
#define QTI_AOP_CMD_DB_SIZE 0x00020000

/*----------------------------------------------------------------------------*/
/* SMEM base address                                                          */
/*----------------------------------------------------------------------------*/
#define QTI_SMEM_BASE ULL(0x89B00000)
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

/* Number of boot image entries supported for the platform */
#define BOOT_IMAGES_NUM_ENTRIES 25

#endif /* PLATFORM_DEF_H */
