/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * PCU/RIMPS hardware sequencer: CPU and L3 power-collapse opcode engines,
 * plus the RIMPS cluster-PLL/rail sequencer they hand off to on deep idle.
 */

#ifndef QTI_PCU_H
#define QTI_PCU_H

/* HW power-state indices selectable via pcu_cpu_select_mode()/pcu_l3_select_mode(). */
#define PCU_PWR_IDX_RUN			(-1)
#define PCU_PWR_IDX_PWR_COLLAPSE	4U

/*
 * One-time boot init: loads the CPU/L3/RIMPS opcode sequences into their
 * sequencer memories, programs branch targets and LPM counters, and enables
 * the RIMPS opcode sequencer (top-level enable + MUC clock domain).
 */
void pcu_sequencer_init(void);

/* Select a core's PCU low-power mode, or PCU_PWR_IDX_RUN to disable. */
void pcu_cpu_select_mode(unsigned int core_pos, int pwr_idx);

/* Select the L3/cluster PCU low-power mode, or PCU_PWR_IDX_RUN to disable. */
void pcu_l3_select_mode(int pwr_idx);

#endif /* QTI_PCU_H */
