/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * PCU/RIMPS hardware sequencer bring-up for shikra.
 *
 * The CPU, L3 and RIMPS blocks each embed a tiny opcode-sequencer FSM that
 * runs a boot-time-programmed byte "microcode" (SET1/SET0/WAIT1/WAIT0/DELI/
 * BEQ/BEQR/RETURN) to drive power-collapse and rail/PLL-off entry and exit
 * autonomously, without firmware involvement once started. This file loads
 * those opcode programs into the shared sequencer memories, programs the
 * branch targets the opcodes jump to, and enables the RIMPS opcode engine.
 *
 * At runtime, PSCI power-domain hooks only need to pick a power-state index
 * via pcu_cpu_select_mode()/pcu_l3_select_mode() and then WFI; the CPU C3
 * sequence's internal BEQR hands off to RIMPS's Gold-PLL/rail sequence
 * through dedicated hardware signals (parent_seq_busy, rail_seq_in_prgs,
 * wakeup_event, l3_seq_pwrd_up) with no further software call.
 */

#include <stdint.h>

#include <lib/mmio.h>

#include <drivers/qti/pcu/pcu.h>
#include <platform_def.h>

#include "pcu_hwio.h"

/* CPU power-collapse (C3) sequence; contains a BEQ to the short-wake path. */
static const uint8_t cpu_c3_sequence[] = {
	0xb4, 0xb1, 0xeb, 0xa7, 0x2, 0xe4, 0x54, 0xa1, 0xa2, 0x2,
	0xe5, 0xa3, 0x2, 0xe5, 0xa9, 0xa4, 0xa5, 0xb0, 0xa6, 0xaa,
	0x2, 0xe3, 0x87, 0xb7, 0xe0, 0x97, 0x5, 0xe2, 0xae, 0x2,
	0xe8, 0x8a, 0x2, 0xe3, 0x81, 0x89, 0x86, 0x90, 0x83, 0x2,
	0xe5, 0x82, 0x2, 0xe5, 0xa8, 0x2, 0x88, 0xe6, 0xa0, 0x3,
	0x85, 0x3, 0x84, 0x80, 0x8e, 0x91, 0x94, 0x20,
};

/* CPU early-wake branch target of the C3 sequence's BEQ. */
static const uint8_t cpu_c3_short_sequence[] = {
	0xad, 0xa4, 0xa5, 0x87, 0x2, 0xb7, 0x3, 0x97, 0x5, 0xae,
	0x2, 0xe8, 0xa8, 0x2, 0x88, 0xe6, 0x84, 0x85, 0x8e, 0x91,
	0x8d, 0x20,
};

/* L3 power-collapse (D4) sequence, including PLL shutdown. */
static const uint8_t l3_d4_sequence[] = {
	0x1b, 0x50, 0x2, 0x0, 0xe, 0x70, 0xc, 0x70, 0x2, 0x0,
	0xa, 0x50, 0x1e, 0x50, 0x1, 0x50, 0x7, 0x0, 0x6, 0x50,
	0x7, 0x50, 0xb, 0x50, 0x11, 0x50, 0xc, 0x50, 0x1f, 0x50,
	0x2, 0x0, 0x1f, 0x40, 0x2, 0x0, 0xd, 0x50, 0xe, 0x50,
	0x2, 0x50, 0x2, 0x0, 0x2, 0x70, 0x3, 0x50, 0x2, 0x0,
	0x2, 0x70, 0x8, 0x50, 0x1d, 0x50, 0x1a, 0x50, 0x2, 0x0,
	0x9, 0x70, 0x16, 0x50, 0x10, 0x50, 0x18, 0x50, 0x5, 0x0,
	0x5, 0x50, 0x13, 0x50, 0x2, 0x0, 0x1c, 0x50, 0x20, 0x50,
	0x2, 0x0, 0x12, 0x50, 0x2, 0x0, 0xf, 0x70, 0x17, 0x50,
	0x3f, 0x50, 0x0, 0x70, 0x3f, 0x40, 0x12, 0x40, 0x2, 0x0,
	0xf, 0x70, 0x17, 0x40, 0x19, 0x50, 0x12, 0x70, 0x19, 0x40,
	0x13, 0x40, 0x2, 0x0, 0x5, 0x40, 0xe, 0x40, 0xc, 0x40,
	0xc, 0x0, 0xd, 0x40, 0x11, 0x40, 0xb, 0x70, 0xb, 0x40,
	0x1c, 0x40, 0x18, 0x40, 0x5, 0x0, 0x16, 0x40, 0x8, 0x40,
	0x1d, 0x40, 0x3, 0x40, 0x2, 0x0, 0x2, 0x70, 0x2, 0x40,
	0x2, 0x0, 0x2, 0x70, 0x4, 0x50, 0x2, 0x0, 0x4, 0x40,
	0x3, 0x70, 0x0, 0x50, 0x3, 0x0, 0x7, 0x40, 0x10, 0x40,
	0x6, 0x40, 0x2, 0x0, 0x20, 0x40, 0x2, 0x0, 0x1, 0x40,
	0xa, 0x40, 0x1b, 0x40, 0x2, 0x0, 0xe, 0x70, 0x1e, 0x40,
	0x5, 0x0, 0x0, 0x40, 0x2, 0x0, 0x0, 0x10,
};

/* RIMPS Silver-PLL off sequence. */
static const uint8_t rimps_silver_sequence[] = {
	0xb7, 0xa9, 0xb8, 0x2, 0xf3, 0xb9, 0xb3, 0xb2, 0xb4, 0xb6,
	0xba, 0x3, 0xf2, 0x5, 0xf4, 0x9a, 0x96, 0x92, 0xc, 0x94,
	0xf1, 0x93, 0x99, 0x98, 0x2, 0xd3, 0x0, 0x89, 0x97, 0x20,
};

/* RIMPS Gold-PLL off sequence; BEQR hands off to the rail sequence below. */
static const uint8_t rimps_gold_pll_sequence[] = {
	0xb7, 0xa9, 0xb9, 0xb3, 0xb2, 0xb4, 0xb6, 0x61, 0x96, 0x92,
	0xc, 0x94, 0xf1, 0x93, 0x99, 0x89, 0x97, 0x20,
};

/* RIMPS Gold-rail off sequence; branch target of the sequence above. */
static const uint8_t rimps_gold_rail_sequence[] = {
	0xb8, 0x2, 0xf3, 0xbd, 0xbc, 0x2, 0xba, 0x3, 0xf2, 0x5,
	0xf4, 0x9a, 0x9c, 0x2, 0x9d, 0x5, 0x98, 0x2, 0xd3, 0x0,
	0x0, 0x20,
};

/* RIMPS L3-PLL off sequence. */
static const uint8_t rimps_l3_sequence[] = {
	0xb7, 0xba, 0xf2, 0x5, 0xf4, 0x9a, 0x97, 0x20,
};

/* Start addresses resolved while loading the sequences into their memories. */
static uint32_t cpu_c3_start_addr;
static uint32_t cpu_c3_short_start_addr;
static uint32_t l3_d4_start_addr;

static void pcu_write_seq(void (*write_word)(uint32_t idx, uint8_t val),
			   uint32_t *word_idx, const uint8_t *seq, size_t len)
{
	size_t i;

	for (i = 0U; i < len; i++) {
		write_word(*word_idx, seq[i]);
		(*word_idx)++;
	}
}

static void apss_seq_mem_write(uint32_t idx, uint8_t val)
{
	mmio_write_32(APSS_SEQ_COMMON_INSTR_n(idx), val);
}

static void rimpsfast_seq_mem_write(uint32_t idx, uint8_t val)
{
	mmio_write_32(RIMPSFAST_SEQ_MEM_n(idx), val);
}

/*
 * Loads the CPU power-collapse/short-wake sequence, then the L3
 * power-collapse sequence, into the shared APSS sequencer memory.
 * All 4 cores share this one copy; each core's CPU_SEQ_CTL just points at
 * the same start address.
 */
static void pcu_load_cpu_l3_sequences(void)
{
	uint32_t word_idx = 0U;

	cpu_c3_start_addr = word_idx;
	pcu_write_seq(apss_seq_mem_write, &word_idx, cpu_c3_sequence,
		      sizeof(cpu_c3_sequence));

	cpu_c3_short_start_addr = word_idx;
	pcu_write_seq(apss_seq_mem_write, &word_idx, cpu_c3_short_sequence,
		      sizeof(cpu_c3_short_sequence));

	l3_d4_start_addr = word_idx;
	pcu_write_seq(apss_seq_mem_write, &word_idx, l3_d4_sequence,
		      sizeof(l3_d4_sequence));
}

/*
 * Loads the RIMPS Silver/Gold-PLL/Gold-rail/L3 sequences into the RIMPS
 * opcode-sequencer memory, and programs each clock domain's sequence start
 * address plus Gold's PLL-off -> rail-off branch target.
 */
static void pcu_load_rimps_sequences(void)
{
	uint32_t word_idx = 0U;
	uint32_t silver_start_addr;
	uint32_t gold_pll_start_addr;
	uint32_t gold_rail_start_addr;
	uint32_t l3_start_addr;

	silver_start_addr = word_idx;
	pcu_write_seq(rimpsfast_seq_mem_write, &word_idx, rimps_silver_sequence,
		      sizeof(rimps_silver_sequence));

	gold_pll_start_addr = word_idx;
	pcu_write_seq(rimpsfast_seq_mem_write, &word_idx, rimps_gold_pll_sequence,
		      sizeof(rimps_gold_pll_sequence));

	gold_rail_start_addr = word_idx;
	pcu_write_seq(rimpsfast_seq_mem_write, &word_idx, rimps_gold_rail_sequence,
		      sizeof(rimps_gold_rail_sequence));

	l3_start_addr = word_idx;
	pcu_write_seq(rimpsfast_seq_mem_write, &word_idx, rimps_l3_sequence,
		      sizeof(rimps_l3_sequence));

	mmio_clrsetbits_32(RIMPSSLOW_SEQ_CTRL1(RIMPSSLOW_CLKDOM0_BASE),
			   RIMPSSLOW_SEQ_CTRL1_PLL_SEQ_START_ADDR_BMSK,
			   l3_start_addr << RIMPSSLOW_SEQ_CTRL1_PLL_SEQ_START_ADDR_SHFT);
	mmio_clrsetbits_32(RIMPSSLOW_SEQ_CTRL1(RIMPSSLOW_CLKDOM1_BASE),
			   RIMPSSLOW_SEQ_CTRL1_PLL_SEQ_START_ADDR_BMSK,
			   silver_start_addr << RIMPSSLOW_SEQ_CTRL1_PLL_SEQ_START_ADDR_SHFT);
	mmio_clrsetbits_32(RIMPSSLOW_SEQ_CTRL1(RIMPSSLOW_CLKDOM2_BASE),
			   RIMPSSLOW_SEQ_CTRL1_PLL_SEQ_START_ADDR_BMSK,
			   gold_pll_start_addr << RIMPSSLOW_SEQ_CTRL1_PLL_SEQ_START_ADDR_SHFT);
	mmio_write_32(RIMPSSLOW_CLKDOM2_SEQ_BR_CTRL_n(RIMPSSLOW_SEQ_BR_CTRL_BRANCH_IDX),
		      gold_rail_start_addr);
}

/* Programs the per-core branch target the CPU C3 BEQ jumps to on early wake. */
static void pcu_program_cpu_branch_addrs(void)
{
	unsigned int core;

	for (core = 0U; core < PLATFORM_CORE_COUNT; core++) {
		mmio_write_32(CPU_SEQ_BRANCH_START_ADDR_n(core, CPU_SEQ_BRANCH_C3_SHORT_IDX),
			      cpu_c3_short_start_addr);
	}
}

/* LPM residency counters, needed by the sequencer to size its wake-up window. */
static void pcu_program_lpm_counters(void)
{
	unsigned int core;

	for (core = 0U; core < PLATFORM_CORE_COUNT; core++) {
		mmio_write_32(CPU_SEQ_LPM_COUNTER_CFG(core), 0xcU);
		mmio_write_32(APSS_CPU_LPM_RESIDENCY_CNTR_CFG_n(core), 0x14U);
	}

	mmio_write_32(L3_SEQ_LPM_COUNTER_CFG, 0x80000008U);
	mmio_write_32(APSS_CL_LPM_RESIDENCY_CNTR_CFG, 0x4U);
}

/*
 * SAW4 SPM sequencer config for the Silver rail, coordinated with the CPU C3
 * opcode sequence's silver_apc_clamp_en/silver_apm_comp_pc steps.
 */
static void pcu_program_saw4(void)
{
	mmio_write_32(SILVER_SAW4_SRM_CTL, 0x3U);
	mmio_clrsetbits_32(SILVER_SAW4_CFG, SILVER_SAW4_CFG_CLK_DIV_BMSK,
			   0x14U << SILVER_SAW4_CFG_CLK_DIV_SHFT);
	mmio_write_32(SILVER_SAW4_SRM_PMIC_DATA_n(0U), 0x03030080U);
	mmio_write_32(SILVER_SAW4_SRM_PMIC_DATA_n(1U), 0x00030000U);
}

/*
 * Enables the RIMPS opcode sequencer: top-level block enable, the MUC clock
 * domain that gates it, and the RIMPS_EN bit in each clock domain. This does
 * NOT load or start the separate CPUCP RISC-V firmware used for OSM/DCVS.
 */
static void pcu_enable_rimps(void)
{
	mmio_setbits_32(RIMPSTOP_GLOBAL_ENABLE, RIMPSTOP_GLOBAL_ENABLE_BMSK);

	mmio_clrsetbits_32(RIMPSFAST_RIMPS_MUC_CLK_CTRL,
			   RIMPSFAST_MUC_CLK_CTRL_CPU_PCU_SEQ_HALT_BMSK,
			   0U);
	mmio_setbits_32(RIMPSFAST_RIMPS_MUC_CLK_CTRL,
			RIMPSFAST_MUC_CLK_CTRL_CORE_SS_CLK_EN_BMSK |
			RIMPSFAST_MUC_CLK_CTRL_CORE_CLK_EN_BMSK);

	mmio_setbits_32(RIMPSSLOW_RIMPS_ENABLE(RIMPSSLOW_CLKDOM0_BASE),
			RIMPSSLOW_RIMPS_ENABLE_RIMPS_EN_BMSK);
	mmio_setbits_32(RIMPSSLOW_RIMPS_ENABLE(RIMPSSLOW_CLKDOM1_BASE),
			RIMPSSLOW_RIMPS_ENABLE_RIMPS_EN_BMSK);
	mmio_setbits_32(RIMPSSLOW_RIMPS_ENABLE(RIMPSSLOW_CLKDOM2_BASE),
			RIMPSSLOW_RIMPS_ENABLE_RIMPS_EN_BMSK);
}

void pcu_sequencer_init(void)
{
	pcu_load_cpu_l3_sequences();
	pcu_load_rimps_sequences();
	pcu_program_cpu_branch_addrs();
	pcu_program_lpm_counters();
	pcu_program_saw4();
	pcu_enable_rimps();
}

static void pcu_select_mode(uintptr_t seq_ctl_reg, int pwr_idx, uint32_t start_addr)
{
	if (pwr_idx < 0) {
		mmio_write_32(seq_ctl_reg, 0U);
		return;
	}

	mmio_write_32(seq_ctl_reg,
		      (start_addr << CPU_SEQ_CTL_SEQ_START_ADDR_SHFT) |
		      (((uint32_t)pwr_idx << CPU_SEQ_CTL_SEQ_PWR_STATE_IDX_SHFT) &
		       CPU_SEQ_CTL_SEQ_PWR_STATE_IDX_BMSK) |
		      CPU_SEQ_CTL_SEQ_EN_BMSK);
}

void pcu_cpu_select_mode(unsigned int core_pos, int pwr_idx)
{
	pcu_select_mode(CPU_SEQ_CTL(core_pos), pwr_idx, cpu_c3_start_addr);
}

void pcu_l3_select_mode(int pwr_idx)
{
	pcu_select_mode(L3_SEQ_CTL, pwr_idx, l3_d4_start_addr);
}
