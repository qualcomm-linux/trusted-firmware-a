/*
 * Copyright (c) 2026, Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * DCC Virtualization Header for Shikra (TFA Port)
 *
 * Target-specific defines for DCC address space virtualization
 * Target-specific DCC HW block addressing parameters
 *
 * Ported from TZ.XF.5.36-00138-SHIKRA-1:
 * trustzone_images/core/systemdebug/sysdbg/inc/target/shikra/sysdbg_dcc_virt.h
 *
 * This file follows TZ convention exactly:
 * - All addresses derived from HWIO (single source of truth)
 * - No hardcoded values
 * - Minimal, clean definitions
 */

#ifndef __SYSDBG_DCC_VIRT_H
#define __SYSDBG_DCC_VIRT_H

#include <drivers/qti/sysdbg/shikra/sysdbg_hwio.h>

/*===========================================================================
 * DCC Base Address Definitions (Derived from HWIO)
 *===========================================================================
 */

/**
 * DCC_RAM_REG_BASE - DCC RAM register base address
 * Derived from HWIO: QDSS_QDSS_DCC_DCC_RAM_REG_BASE
 * = QDSS_SOC_DBG_BASE (0x08000000) + 0x00080000 = 0x08080000
 */
#define DCC_RAM_REG_BASE                QDSS_QDSS_DCC_DCC_RAM_REG_BASE

/**
 * DCC_APU_REG_BASE - DCC APU (Access Protection Unit) register base
 * Derived from HWIO: QDSS_QDSS_DCC_DCC_APU_REG_BASE
 * = QDSS_SOC_DBG_BASE (0x08000000) + 0x000E0000 = 0x080E0000
 */
#define DCC_APU_REG_BASE                QDSS_QDSS_DCC_DCC_APU_REG_BASE

/**
 * DCC_SRAM_DUMMY_ADDR - Dummy address for SRAM write tests
 * Offset 0x7E00 from RAM base (used for validation/test operations)
 */
#define DCC_SRAM_DUMMY_ADDR             (DCC_RAM_REG_BASE + 0x7E00)

/**
 * DCC_ADDR_SPACE_START - Start of DCC address space
 */
#define DCC_ADDR_SPACE_START            (DCC_RAM_REG_BASE)

/**
 * DCC_ADDR_SPACE_END - End of DCC address space
 * Covers 512KB range from RAM base
 */
#define DCC_ADDR_SPACE_END              (DCC_ADDR_SPACE_START + 0x7FFFF)

/**
 * DCC_LIST_COUNT - Number of DCC linked lists supported by hardware
 * Derived from HWIO: Max index + 1
 */
#define DCC_LIST_COUNT                  (HWIO_QDSS_QDSS_DCC_DCC_LL_CONFIGm_MAXm + 1)

/**
 * DCC_CFG_SPACE_START - Start of DCC configuration register space
 * Derived from HWIO: QDSS_QDSS_DCC_DCC_CFG_REG_BASE
 */
#define DCC_CFG_SPACE_START             (QDSS_QDSS_DCC_DCC_CFG_REG_BASE)

/**
 * DCC_CFG_SPACE_END - End of DCC configuration register space
 * 4KB configuration space
 */
#define DCC_CFG_SPACE_END               (QDSS_QDSS_DCC_DCC_CFG_REG_BASE + 0xFFF)

/*===========================================================================
 * Legacy TFA Compatibility Aliases
 * (These maintain compatibility with existing TFA HAL code)
 *===========================================================================
 */

/**
 * DCC_SRAM_BASE - Alias for DCC_RAM_REG_BASE
 * Used by hal_dcc_program_list() for SRAM writes
 */
#define DCC_SRAM_BASE                   DCC_RAM_REG_BASE
#define DCC_SRAM_BASE_PHYS              DCC_RAM_REG_BASE

/**
 * DCC_CFG_BASE - Alias for configuration base
 * Used by DCC HAL for register access
 */
#define DCC_CFG_BASE                    QDSS_QDSS_DCC_DCC_CFG_REG_BASE
#define DCC_CFG_BASE_PHYS               QDSS_QDSS_DCC_DCC_CFG_REG_BASE

/**
 * DCC_SRAM_SIZE - DCC SRAM size (32KB)
 */
#define DCC_SRAM_SIZE                   0x00008000U

/**
 * DCC_ETFETB_SIZE - ETB/ETF buffer size
 * Note: ETFETB_BASE is defined in sysdbg_hwio.h
 */
#define DCC_ETFETB_SIZE                 (1 << 16)  /* 64KB */

/**
 * ETFETB_BASE - ETB/ETF base for ETB sink mode (if not using SRAM)
 * Note: Already defined in sysdbg_hwio.h, no redefinition needed
 */

/*===========================================================================
 * DCC Register HWIO Macro Aliases
 *
 * The HAL uses short-form macros (HWIO_DCC_*) but sysdbg_hwio.h defines
 * long-form macros (HWIO_QDSS_QDSS_DCC_DCC_*). These aliases map short to long.
 *===========================================================================
 */

/* DCC_LL_BASEm - Linked List Base Address Register */
#define HWIO_DCC_LL_BASEm_ADDR(m)       HWIO_QDSS_QDSS_DCC_DCC_LL_BASEm_ADDR(m)
#define HWIO_DCC_LL_BASEm_MAXm          HWIO_QDSS_QDSS_DCC_DCC_LL_BASEm_MAXm
#define HWIO_DCC_LL_BASEm_ADDRESS_BMSK  HWIO_QDSS_QDSS_DCC_DCC_LL_BASEm_ADDRESS_BMSK
#define HWIO_DCC_LL_BASEm_ADDRESS_SHFT  HWIO_QDSS_QDSS_DCC_DCC_LL_BASEm_ADDRESS_SHFT

/* DCC_FD_BASEm - Fetch Data Base Address Register */
#define HWIO_DCC_FD_BASEm_ADDR(m)       HWIO_QDSS_QDSS_DCC_DCC_FD_BASEm_ADDR(m)
#define HWIO_DCC_FD_BASEm_ADDRESS_BMSK  HWIO_QDSS_QDSS_DCC_DCC_FD_BASEm_ADDRESS_BMSK
#define HWIO_DCC_FD_BASEm_ADDRESS_SHFT  HWIO_QDSS_QDSS_DCC_DCC_FD_BASEm_ADDRESS_SHFT

/* DCC_LL_TIMEOUTm - Linked List Timeout Register */
#define HWIO_DCC_LL_TIMEOUTm_ADDR(m)    HWIO_QDSS_QDSS_DCC_DCC_LL_TIMEOUTm_ADDR(m)
#define HWIO_DCC_LL_TIMEOUTm_VALUE_BMSK HWIO_QDSS_QDSS_DCC_DCC_LL_TIMEOUTm_VALUE_BMSK
#define HWIO_DCC_LL_TIMEOUTm_VALUE_SHFT HWIO_QDSS_QDSS_DCC_DCC_LL_TIMEOUTm_VALUE_SHFT

/* DCC_LL_CONFIGm - Linked List Configuration Register */
#define HWIO_DCC_LL_CONFIGm_ADDR(m)             HWIO_QDSS_QDSS_DCC_DCC_LL_CONFIGm_ADDR(m)
#define HWIO_DCC_LL_CONFIGm_SINK_BMSK           HWIO_QDSS_QDSS_DCC_DCC_LL_CONFIGm_SINK_BMSK
#define HWIO_DCC_LL_CONFIGm_SINK_SHFT           HWIO_QDSS_QDSS_DCC_DCC_LL_CONFIGm_SINK_SHFT
#define HWIO_DCC_LL_CONFIGm_FUNC_BMSK           HWIO_QDSS_QDSS_DCC_DCC_LL_CONFIGm_FUNC_BMSK
#define HWIO_DCC_LL_CONFIGm_FUNC_SHFT           HWIO_QDSS_QDSS_DCC_DCC_LL_CONFIGm_FUNC_SHFT
#define HWIO_DCC_LL_CONFIGm_HW_TRIGGER_EN_BMSK  HWIO_QDSS_QDSS_DCC_DCC_LL_CONFIGm_HW_TRIGGER_EN_BMSK
#define HWIO_DCC_LL_CONFIGm_HW_TRIGGER_EN_SHFT  HWIO_QDSS_QDSS_DCC_DCC_LL_CONFIGm_HW_TRIGGER_EN_SHFT

/* DCC_LL_LOCKm - Linked List Lock Register */
#define HWIO_DCC_LL_LOCKm_ADDR(m)       HWIO_QDSS_QDSS_DCC_DCC_LL_LOCKm_ADDR(m)
#define HWIO_DCC_LL_LOCKm_ACTIVE_BMSK   HWIO_QDSS_QDSS_DCC_DCC_LL_LOCKm_ACTIVE_BMSK
#define HWIO_DCC_LL_LOCKm_S_BIT_BMSK    HWIO_QDSS_QDSS_DCC_DCC_LL_LOCKm_S_BIT_BMSK

/* DCC_TRANS_TIMEOUTm - Transaction Timeout Register (optional, may not exist on all chips) */
#ifdef HWIO_QDSS_QDSS_DCC_DCC_TRANS_TIMEOUTm_ADDR
#define HWIO_DCC_TRANS_TIMEOUTm_ADDR(m)    HWIO_QDSS_QDSS_DCC_DCC_TRANS_TIMEOUTm_ADDR(m)
#define HWIO_DCC_TRANS_TIMEOUTm_VALUE_BMSK HWIO_QDSS_QDSS_DCC_DCC_TRANS_TIMEOUTm_VALUE_BMSK
#define HWIO_DCC_TRANS_TIMEOUTm_VALUE_SHFT HWIO_QDSS_QDSS_DCC_DCC_TRANS_TIMEOUTm_VALUE_SHFT
#endif

/*===========================================================================
 * Helper Macros (TFA additions for HAL)
 *===========================================================================
 */

/**
 * WORD_INDEX - Convert byte address to word index (divide by 4)
 * DCC hardware uses word indices for LL_BASE and FD_BASE registers
 */
#ifndef WORD_INDEX
#define WORD_INDEX(byte_addr)           ((byte_addr) >> 2)
#endif

/**
 * BYTE_INDEX - Convert word index to byte address (multiply by 4)
 */
#ifndef BYTE_INDEX
#define BYTE_INDEX(word_idx)            ((word_idx) << 2)
#endif

/**
 * ALIGN_ADDR - Align address to specified alignment (must be power of 2)
 * DCC requires all LL and FD offsets to be 16-byte aligned
 */
#ifndef ALIGN_ADDR
#define ALIGN_ADDR(addr, align)         (((addr) + (align) - 1U) & (~((align) - 1U)))
#endif

#endif /* __SYSDBG_DCC_VIRT_H */
