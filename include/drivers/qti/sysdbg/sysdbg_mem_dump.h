/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SYSDBG_MEM_DUMP_H
#define SYSDBG_MEM_DUMP_H

#include <stdbool.h>
#include <stdint.h>

#include <sysdbg.h>
#include <sysdbg_target_config.h>

#define CACHE_DUMP_FORMAT_VERSION	0x11

/* CPU register-dump context, 64-bit EL. */
typedef struct {
	uint64_t x0, x1, x2, x3, x4, x5, x6, x7, x8, x9;
	uint64_t x10, x11, x12, x13, x14, x15, x16, x17, x18, x19;
	uint64_t x20, x21, x22, x23, x24, x25, x26, x27, x28, x29, x30;
	uint64_t pc;
	uint64_t currentEL;
	uint64_t sp_el3, elr_el3, spsr_el3;
	uint64_t sp_el2, elr_el2, spsr_el2;
	uint64_t sp_el1, elr_el1, spsr_el1;
	uint64_t sp_el0;
	uint64_t __reserved1, __reserved2, __reserved3, __reserved4;
	uint64_t __reserved5, __reserved6, __reserved7, __reserved8;
} sysdbg_cpu64_ctxt_regs_type;

typedef struct {
	sysdbg_cpu64_ctxt_regs_type cpu64_ctxt;
} sysdbg_cpu_ctxt_regs_type;

typedef struct {
	uint64_t regs[0x64];
} cpu_ctxt_buf_type;

bool SysdbgDumpTableInit(void);
bool SysdbgMemoryDumpInit(void);

#endif /* SYSDBG_MEM_DUMP_H */
