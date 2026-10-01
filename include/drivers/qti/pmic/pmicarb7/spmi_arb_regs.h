/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SPMI_ARB_REGS_H
#define SPMI_ARB_REGS_H

#include <lib/utils_def.h>

/*
 * PMIC ARB7 (v7p3) register layout. Offsets below are relative to
 * SPMI_ARB_BASE (include/drivers/qti/pmic/<chipset>/spmi_platform.h).
 */

#define PMIC_ARB_CORE_REG_BASE_OFFS		0x1400000UL
#define PMIC_ARB_CORE_REGISTERS_REG_BASE_OFFS	0x1500000UL
#define SPMI_CFG_REG_BASE_OFFS			0x142D000UL

/* Per-channel peripheral identity (SID + PPID) and IRQ owner. */
#define HWIO_PMIC_ARB_REG_ADDRp_ADDR(base, p) \
	((base) + PMIC_ARB_CORE_REG_BASE_OFFS + 0x2000UL + (0x4UL * (p)))
#define HWIO_PMIC_ARB_REG_ADDRp_IRQ_OWN_BMSK	0x1000000UL
#define HWIO_PMIC_ARB_REG_ADDRp_IRQ_OWN_SHFT	24U
#define HWIO_PMIC_ARB_REG_ADDRp_SID_BMSK	0xf0000UL
#define HWIO_PMIC_ARB_REG_ADDRp_SID_SHFT	16U
#define HWIO_PMIC_ARB_REG_ADDRp_ADDRESS_BMSK	0xff00UL
#define HWIO_PMIC_ARB_REG_ADDRp_ADDRESS_SHFT	8U

/* Per-channel command/status/data registers used to issue a transaction. */
#define HWIO_PMIC_ARB_CHNLn_CMD_ADDR(base, n) \
	((base) + PMIC_ARB_CORE_REGISTERS_REG_BASE_OFFS + (0x1000UL * (n)))
#define HWIO_PMIC_ARB_CHNLn_STATUS_ADDR(base, n) \
	((base) + PMIC_ARB_CORE_REGISTERS_REG_BASE_OFFS + 0x8UL + (0x1000UL * (n)))
#define HWIO_PMIC_ARB_CHNLn_WDATA0_ADDR(base, n) \
	((base) + PMIC_ARB_CORE_REGISTERS_REG_BASE_OFFS + 0x10UL + (0x1000UL * (n)))
#define HWIO_PMIC_ARB_CHNLn_RDATA0_ADDR(base, n) \
	((base) + PMIC_ARB_CORE_REGISTERS_REG_BASE_OFFS + 0x18UL + (0x1000UL * (n)))

#define HWIO_PMIC_ARB_CHNLn_CMD_OPCODE_SHFT		27U
#define HWIO_PMIC_ARB_CHNLn_CMD_PRIORITY_SHFT		26U
#define HWIO_PMIC_ARB_CHNLn_CMD_ADDRESS_OFFSET_BMSK	0xff0UL
#define HWIO_PMIC_ARB_CHNLn_CMD_ADDRESS_OFFSET_SHFT	4U
#define HWIO_PMIC_ARB_CHNLn_CMD_BYTE_CNT_BMSK		0x7UL

#define HWIO_PMIC_ARB_CHNLn_STATUS_DROPPED_BMSK	0x8UL
#define HWIO_PMIC_ARB_CHNLn_STATUS_DENIED_BMSK		0x4UL
#define HWIO_PMIC_ARB_CHNLn_STATUS_FAILURE_BMSK	0x2UL
#define HWIO_PMIC_ARB_CHNLn_STATUS_DONE_BMSK		0x1UL

/* Opcodes for this ARB version's extended-register read/write transactions. */
#define PMIC_ARB_CMD_EXTENDED_REG_WRITE_LONG	0x0U
#define PMIC_ARB_CMD_EXTENDED_REG_READ_LONG	0x1U

/* Peripheral-to-owner table: which EE-id (owner) a periph is assigned to. */
#define HWIO_SPMI_PERIPHm_2OWNER_TABLE_REG_ADDR(base, m) \
	((base) + SPMI_CFG_REG_BASE_OFFS + (0x4UL * (m)))
#define HWIO_SPMI_PERIPHm_2OWNER_TABLE_REG_PERIPH2OWNER_BMSK	0xfUL
#define HWIO_SPMI_PERIPHm_2OWNER_TABLE_REG_PERIPH2OWNER_SHFT	0U

/*
 * TZ's raw EE-id value for owner/EE-ID-aware channel resolution — NOT the
 * SpmiBusCfg_OwnerMask bitmask. Shared across every chipset on this ARB
 * version (all pmicarb7 SpmiBlock.c configs use SPMI_BARE_OWNER_NUMBER = 1).
 */
#define TZ_OWNER_ID				1U

/* 1us poll interval; SPMI_TIMEOUT_USEC downstream is 400us. */
#define SPMI_ARB_TIMEOUT_ITER			400U

#endif /* SPMI_ARB_REGS_H */
