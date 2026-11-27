/*
 * Copyright (c) 2026, Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * DCC (Data Capture and Compare) Configuration for Shikra
 *
 * Ported from TZ.XF.5.36-00138-SHIKRA-1:
 * trustzone_images/core/systemdebug/sysdbg/qsee/src/target/shikra/sysdbg_dcc_cfg.c
 */

#include <stdint.h>
#include <stddef.h>
#include <sys/cdefs.h>
#include <lib/utils_def.h>
#include <drivers/qti/sysdbg/sysdbg_dcc.h>

/*===========================================================================
 * DCC Memory Region Layout
 *===========================================================================
 */

#define KB(x)                         ((x) * 1024U)
#define DCC_LL_CODE_REGION_BASE       (416U)
#define DCC_LL_CODE_REGION_SIZE       (KB(0.5))
#define DCC_LL_DATA_REGION_BASE       (1200U)
#define DCC_LL_DATA_REGION_SIZE       (KB(23))
#define DCC_LL_SRAM_REGION_END        (KB(24))
#define DCC_LL_PC_BYTE_OFFSET         (32U)
#define DCC_LL5_CODE_REGION_BASE      (0U)
#define DCC_LL5_DATA_REGION_BASE      (320U)

/*===========================================================================
 * DCC Linked List Sequences
 *===========================================================================
 */

/*
 * DCC Linked List Sequence 0 - Scandump Trigger
 * Captures CPU power control, clock enables, and APSS scan dump data
 */
static const uint32_t __aligned(16) dcc_ll_sequence_0[] = {
	/* STRM &GCC_QDSS_DAP_CBCR &HWIO_GCC_QDSS_DAP_CBCR_CLK_ENABLE_BMSK 0x1 */
	0x0141902,
	0xc0008101,
	0x80000001,
	0x1,
	0x1,
	/* STRM GCC_APSS_QDSS_APB_CBCR &HWIO_GCC_APSS_QDSS_APB_CBCR_CLK_ENABLE_BMSK 0x1 */
	0x0142B17,
	0xc0008101,
	0x80000001,
	0x1,
	0x1,
	/* STRM &HWIO_GCC_APCS_CLOCK_BRANCH_ENA_VOTE_ADDR &HWIO_GCC_APCS_CLOCK_BRANCH_ENA_VOTE_CPUSS_GNOC_CLK_ENA_BMSK &HWIO_GCC_APCS_CLOCK_BRANCH_ENA_VOTE_CPUSS_GNOC_CLK_ENA_BMSK */
	0x0147900,
	0xc0008101,
	0x80000001,
	0x400000,
	0x400000,
	/* STRM &HWIO_GCC_APCS_CLOCK_BRANCH_ENA_VOTE_ADDR &HWIO_GCC_APCS_CLOCK_BRANCH_ENA_VOTE_CPUSS_AHB_CLK_ENA_BMSK &HWIO_GCC_APCS_CLOCK_BRANCH_ENA_VOTE_CPUSS_AHB_CLK_ENA_BMSK */
	0x0147900,
	0xc0008101,
	0x80000001,
	0x200000,
	0x200000,
	/* STRM &HWIO_GCC_APCS_VOTE_QDSS_APB_CLK_ADDR &HWIO_GCC_APCS_VOTE_QDSS_APB_CLK_CLK_ENABLE_BMSK 0x1 */
	0x0147901,
	0xc0008101,
	0x80000001,
	0x1,
	0x1,
	/* STRM &HWIO_GCC_APCS_CLOCK_BRANCH_ENA_VOTE_ADDR &HWIO_GCC_APCS_CLOCK_BRANCH_ENA_VOTE_QDSS_CFG_AHB_CLK_ENA_BMSK 0x1 */
	0x0147900,
	0xc0008101,
	0x80000001,
	0x4,
	0x1,
	/* LOOP #1 */
	0x40010000,
	/* LOOP #2 */
	0x40010000,
	/* LOOP #3 */
	0x40010000,
	/* LOOP #4 */
	0x40010000,
	/* STRM &HWIO_APSS_ALIAS_0_CPU_SEQ_FORCE_PWR_CTL_VAL_ADDR &BMSK(31) &BMSK(31) */
	0x0F80002,
	0xc0008100,
	0x80000001,
	0x80000000,
	0x80000000,
	/* STRM &HWIO_APSS_ALIAS_0_CPU_SEQ_FORCE_PWR_CTL_VAL_ADDR &BMSK(2) 0x0 */
	0x0F80002,
	0xc0008100,
	0x80000001,
	0x4,
	0x0,
	/* STRM &HWIO_APSS_ALIAS_0_CPU_SEQ_FORCE_PWR_CTL_EN_ADDR &BMSK(2) &BMSK(2) */
	0x0F80001,
	0xc0008103,
	0x80000001,
	0x4,
	0x4,
	/* LOOP #5 */
	0x401F0000,
	/* STRM &HWIO_APSS_ALIAS_0_CPU_SEQ_FORCE_PWR_CTL_VAL_ADDR &BMSK(8) &BMSK(8) */
	0x0F80002,
	0xc0008100,
	0x80000001,
	0x100,
	0x100,
	/* STRM &HWIO_APSS_ALIAS_0_CPU_SEQ_FORCE_PWR_CTL_EN_ADDR &BMSK(8) &BMSK(8) */
	0x0F80001,
	0xc0008103,
	0x80000001,
	0x100,
	0x100,
	/* LOOP #6 */
	0x401F0000,
	/* STRM &HWIO_APSS_ALIAS_0_CPU_SEQ_FORCE_PWR_CTL_VAL_ADDR &BMSK(8) 0x0 */
	0x0F80002,
	0xc0008100,
	0x80000001,
	0x100,
	0x0,
	/* STRM &HWIO_APSS_L3_SEQ_FORCE_PWR_CTL_VAL_ADDR &BMSK(2) 0x0 */
	0x0F88002,
	0xc0008100,
	0x80000001,
	0x4,
	0x0,
	/* STRM &HWIO_APSS_L3_SEQ_FORCE_PWR_CTL_EN_ADDR &BMSK(2) &BMSK(2) */
	0x0F88001,
	0xc0008103,
	0x80000001,
	0x4,
	0x4,
	/* LOOP #7 */
	0x400F0000,
	/* STRM &HWIO_APSS_L3_SEQ_FORCE_PWR_CTL_VAL_ADDR &BMSK(4) &BMSK(4) */
	0x0F88002,
	0xc0008100,
	0x80000001,
	0x10,
	0x10,
	/* STRM &HWIO_APSS_L3_SEQ_FORCE_PWR_CTL_EN_ADDR &BMSK(4)  &BMSK(4) */
	0x0F88001,
	0xc0008103,
	0x80000001,
	0x10,
	0x10,
	/* LOOP #8 */
	0x400F0000,
	/* STRM &HWIO_APSS_L3_SEQ_FORCE_PWR_CTL_VAL_ADDR &BMSK(4) 0x0 */
	0x0F88002,
	0xc0008100,
	0x80000001,
	0x10,
	0x0,
	/* STR &IMEM_MARKER_ADDR_OFFS(1) &IMEM_MARKER_WRITE(11) */
	0x10c11eb0,
	0xc0000101,
	0xa636364,
	/* STR &HWIO_APSS_CL_QCHANNEL_FSM_EN_CTL_ADDR 0x155 */
	0x10F88005,
	0xc0000101,
	0x155,
	/* STR &APB:HWIO_QDSS_APSS_APSS_APB_SDC_CONTROL_ADDR 0x10 */
	0x30987000,
	0xc0000100,
	0x00000010,
	/* STR &APB:HWIO_QDSS_APSS_APSS_CTRL_0_EN_ADDR  0x1 */
	0x30987000,
	0xc0000102,
	0x00000100,
	/* STR &APB:HWIO_QDSS_APSS_APSS_CTRL_1_EN_ADDR  0x1 */
	0x30987000,
	0xc0000103,
	0x00000100,
	/* STR &APB:HWIO_QDSS_APSS_APSS_CTRL_2_EN_ADDR  0x1 */
	0x30987001,
	0xc0000100,
	0x00000100,
	/* STR &APB:HWIO_QDSS_APSS_APSS_APB_SDC_CONTROL_ADDR 0x30 */
	0x30987000,
	0xc0000100,
	0x00800010,
	/* STR &APB:HWIO_QDSS_APSS_APSS_SCAN_DUMP_MODE_ADDR  0x100 */
	0x30987000,
	0xc0000101,
	0x00000001,
	/* STR &APB:HWIO_QDSS_APSS_APSS_APB_SDC_CONTROL_ADDR 0x30 */
	0x30987000,
	0xc0000100,
	0x00800000,
	/* STR &APB:HWIO_QDSS_APSS_APSS_APB_SDC_CONTROL_DELAY 0x1 */
	0x30987001,
	0xc0000103,
	0x00000001,
	/* STR &IMEM_MARKER_ADDR_OFFS(1) &IMEM_MARKER_WRITE(11) */
	0x10c11eb0,
	0xc0000101,
	0x0b636364,
	/* LD &HWIO_QDSS_APSS_APSS_SCAN_REG_APB_ADDR &HWIO_QDSS_APSS_APSS_SCAN_REG_APB_ADDR */
	0x20987002,
	0xc0008100,
	/* LOOP 256 */
	0x4FFF0002,
	/* LD &HWIO_QDSS_APSS_APSS_SCAN_REG_APB_ADDR &HWIO_QDSS_APSS_APSS_SCAN_REG_APB_ADDR */
	0x20987002,
	0xc0008100,
	/* LOOP 256 */
	0x44510002,
	/* STR &IMEM_MARKER_ADDR_OFFS(1) &IMEM_MARKER_WRITE(11) */
	0x10c11eb0,
	0xc0000101,
	0x0c636364,
	/* STR &APB:HWIO_QDSS_APSS_APSS_SCAN_DUMP_MODE_ADDR  0x100 */
	0x30987000,
	0xc0000101,
	0x00000000,
	/* LD APSS_BOOTFSM_STS */
	0x00f88008,
	0xc0008101,
	/* LD GCC_RESET_FSM_STATUS */
	0x00149002,
	0xc0008102,
	/* LD APSS_IPM_CBCR */
	0x00faa009,
	0xc0008101,
	/* LD APSS_IPM_GFMUX_CFG */
	0x00faa008,
	0xc0008103,
	/* STR &APB:HWIO_QDSS_APSS_APSS_CTRL_0_EN_ADDR  0x1 */
	0x30987000,
	0xc0000102,
	0x00000001,
	/* STR &APB:HWIO_QDSS_APSS_APSS_CTRL_1_EN_ADDR  0x1 */
	0x30987000,
	0xc0000103,
	0x00000001,
	/* STR &APB:HWIO_QDSS_APSS_APSS_APB_SDC_CONTROL_ADDR 0x0 */
	0x30987000,
	0xc0000100,
	0x00000000,
	/* STR &APB:HWIO_QDSS_APSS_APSS_CTRL_2_EN_ADDR  0x1 */
	0x30987001,
	0xc0000100,
	0x00000001,
	/* STR &APB:HWIO_QDSS_APSS_APSS_BOOT_TRIGGER_ADDR 0x1 */
	0x30987001,
	0xc0000102,
	0x00000001,
	/* STRM &HWIO_APSS_L3_SEQ_FORCE_PWR_CTL_VAL_ADDR &BMSK(28) &BMSK(28) */
	0x00f88002,
	0xc0008100,
	0x80000001,
	0x10000000,
	0x10000000,
	/* LOOP &DELAY_US(80) */
	0x40010000,
	/* STRM &HWIO_APSS_L3_SEQ_FORCE_PWR_CTL_EN_ADDR &BMSK(28)  &BMSK(28) */
	0x00f88001,
	0xc0008103,
	0x80000001,
	0x10000000,
	0x10000000,
	/* LD APSS_BOOTFSM_STS */
	0x00f88008,
	0xc0008101,
	/* LD AOSS_CC_RESET_FSM_STATUS */
	0x00149002,
	0xc0008102,
	/* LD APSS_IPM_CBCR */
	0x00faa009,
	0xc0008101,
	/* LD APSS_IPM_GFMUX_CFG */
	0x00faa008,
	0xc0008103,
	/* STR &IMEM_MARKER_ADDR_OFFS(1) &IMEM_MARKER_WRITE(13) */
	0x10c11eb0,
	0xc0000101,
	0x0d636364,
	/* END */
	0xc0000000,
};

/*
 * DCC Linked List Sequence 1 - Winddown Trigger
 * Captures CPU PC registers and critical clock/power state during winddown
 */
static const uint32_t __aligned(16) dcc_ll_sequence_1[] = {
	/* LOOP #1 */
	0x40010000,
	/* LOOP #2 */
	0x40010000,
	/* LOOP #3 */
	0x40010000,
	/* STRM &GCC_QDSS_DAP_CBCR &HWIO_GCC_QDSS_DAP_CBCR_CLK_ENABLE_BMSK 0x1 */
	0x141902,
	0xc0008101,
	0x80000001,
	0x1,
	0x1,
	/* STRM GCC_APSS_QDSS_APB_CBCR &HWIO_GCC_APSS_QDSS_APB_CBCR_CLK_ENABLE_BMSK 0x1 */
	0x142B17,
	0xc0008101,
	0x80000001,
	0x1,
	0x1,
	/* STRM &HWIO_GCC_APB_CBCR_ADDR &HWIO_GCC_APB_CBCR_CLK_ENABLE_BMSK 0x1 */
	0x141903,
	0xc0008101,
	0x80000001,
	0x1,
	0x1,
	/* STRM &HWIO_GCC_APSS_QDSS_APB_CBCR_ADDR &HWIO_GCC_APSS_QDSS_APB_CBCR_CLK_ENABLE_BMSK 0x1 */
	0x142B17,
	0xc0008101,
	0x80000001,
	0x1,
	0x1,
	/* STRM &HWIO_GCC_APCS_VOTE_QDSS_APB_CLK_ADDR &HWIO_GCC_APCS_VOTE_QDSS_APB_CLK_CLK_ENABLE_BMSK 0x1 */
	0x147901,
	0xc0008101,
	0x80000001,
	0x1,
	0x1,
	/* STRM &HWIO_GCC_APCS_CLOCK_BRANCH_ENA_VOTE_ADDR &HWIO_GCC_APCS_CLOCK_BRANCH_ENA_VOTE_IMEM_AXI_CLK_ENA_BMSK &HWIO_GCC_APCS_TZ_CLOCK_BRANCH_ENA_VOTE_IMEM_AXI_CLK_ENA_BMSK */
	0x147900,
	0xc0008101,
	0x80000001,
	0x1600005,
	0x1600005,
	/* STRM &HWIO_GCC_APCS_CLOCK_BRANCH_ENA_VOTE_ADDR &HWIO_GCC_APCS_CLOCK_BRANCH_ENA_VOTE_CPUSS_GNOC_CLK_ENA_BMSK &HWIO_GCC_APCS_CLOCK_BRANCH_ENA_VOTE_CPUSS_GNOC_CLK_ENA_BMSK */
	0x147900,
	0xc0008101,
	0x80000001,
	0x400000,
	0x400000,
	/* STRM &HWIO_GCC_APCS_CLOCK_BRANCH_ENA_VOTE_ADDR &HWIO_GCC_APCS_CLOCK_BRANCH_ENA_VOTE_CPUSS_AHB_CLK_ENA_BMSK &HWIO_GCC_APCS_CLOCK_BRANCH_ENA_VOTE_CPUSS_AHB_CLK_ENA_BMSK */
	0x147900,
	0xc0008101,
	0x80000001,
	0x200000,
	0x200000,
	/* LOOP #4 */
	0x40010000,
	/* LOOP #5 */
	0x40010000,
	/* LD &CORE0_PC_LO */
	0x20703020,
	0xc0008100,
	/* LD &CORE0_PC_HI */
	0x20703020,
	0xc0008103,
	/* LD &CORE1_PC_LO */
	0x20713020,
	0xc0008100,
	/* LD &CORE1_PC_HI */
	0x20713020,
	0xc0008103,
	/* LD &CORE2_PC_LO */
	0x20723020,
	0xc0008100,
	/* LD &CORE2_PC_HI */
	0x20723020,
	0xc0008103,
	/* LD &CORE3_PC_LO */
	0x20733020,
	0xc0008100,
	/* LD &CORE3_PC_HI */
	0x20733020,
	0xc0008103,
	/* END */
	0xc0000000,
};

/*===========================================================================
 * DCC Configuration Structures
 *===========================================================================
 */

/*
 * Global DCC configuration
 * Controls overall DCC behavior including memory size, sink type, and PC capture
 */
static const dcc_config_type dcc_config_data = {
	.dcc_en = 1,                        /* DCC enabled */
	.dcc_mem_size = KB(24),             /* 24KB SRAM */
	.sink = DCC_SINK_SRAM,              /* Sink to internal SRAM */
	.pc_trigger = DCC_TRIGGER_WINDDOWN, /* PC capture on winddown */
	.pc_offset = DCC_LL_PC_BYTE_OFFSET, /* PC storage offset */
	.pc_size = sizeof(uint64_t),        /* 64-bit PC values */
	.pc_count = 8,                      /* 8 PC samples */
};

/*
 * Linked list configurations
 * Defines two DCC linked lists:
 * [0] - Scandump trigger: Comprehensive APSS/QDSS state capture
 * [1] - Winddown trigger: CPU PC registers and critical state
 */
static const dcc_ll_config_type dcc_ll_config[] = {
	[0] = {
		.ll_idx = 0,                        /* Linked list 0 */
		.ll_timeout = 0xFFF,                /* Timeout value */
		.ll_timeout_en = 1,                 /* Timeout enabled */
		.llt_timeout_en = 0,                /* Per-transaction timeout disabled */
		.ll_offset = DCC_LL_CODE_REGION_BASE,     /* Code region start */
		.ll_fd_offset = DCC_LL_DATA_REGION_BASE,  /* Data region start */
		.ll_fd_size = DCC_LL_DATA_REGION_SIZE,    /* Data region size */
		.ll_trigger = DCC_TRIGGER_SCANDUMP,       /* Scandump trigger */
		.ll = (void *)dcc_ll_sequence_0,          /* Sequence pointer */
		.ll_size = sizeof(dcc_ll_sequence_0),     /* Sequence size */
	},
	[1] = {
		.ll_idx = 5,                        /* Linked list 5 */
		.ll_timeout = 0xFFF,                /* Timeout value */
		.ll_timeout_en = 1,                 /* Timeout enabled */
		.llt_timeout_en = 0,                /* Per-transaction timeout disabled */
		.ll_offset = DCC_LL5_CODE_REGION_BASE,    /* Code region start */
		.ll_fd_offset = DCC_LL5_DATA_REGION_BASE, /* Data region start */
		.ll_fd_size = 0,                          /* No separate data region */
		.ll_trigger = DCC_TRIGGER_WINDDOWN,       /* Winddown trigger */
		.ll = (void *)dcc_ll_sequence_1,          /* Sequence pointer */
		.ll_size = sizeof(dcc_ll_sequence_1),     /* Sequence size */
	},
};

/*===========================================================================
 * DCC API Functions
 *===========================================================================
 */

const dcc_config_type *dcc_get_config(void)
{
	return &dcc_config_data;
}

const dcc_ll_config_type *dcc_get_ll_config(uint32_t idx)
{
	if (idx < ARRAY_SIZE(dcc_ll_config)) {
		return &dcc_ll_config[idx];
	}
	return NULL;
}

uint32_t dcc_get_ll_cnt(void)
{
	return ARRAY_SIZE(dcc_ll_config);
}
