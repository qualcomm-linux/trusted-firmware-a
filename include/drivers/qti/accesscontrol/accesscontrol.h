/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef ACCESSCONTROL_H
#define ACCESSCONTROL_H

#include <stdint.h>

#define AC_PERM_X 0x1
#define AC_PERM_W 0x2
#define AC_PERM_R 0x4

enum virtual_machine_id {
	AC_VM_NONE    = 0,
	AC_VM_TZ      = 1,
	AC_VM_RPM     = 2,
	AC_VM_HLOS    = 3,
	AC_VM_MSS_MSA = 15,
	AC_VM_MSS_NAV = 43,
	AC_VM_LAST    = 127,
	AC_VM_MAX     = 0x7FFFFFFF,
};

typedef struct qti_accesscontrol_mem {
	uint64_t mem_addr;
	uint64_t mem_size;
} qti_accesscontrol_mem_t;

typedef struct qti_accesscontrol_perm {
	uint32_t dst_vm;
	uint32_t dst_vm_perm;
	uint64_t ctx;
	uint32_t ctx_size;
} qti_accesscontrol_perm_t;

uint64_t qti_accesscontrol_mem_assign(const qti_accesscontrol_mem_t *mem_info,
				      uint32_t mem_len,
				      const uint32_t *src, uint32_t src_len,
				      const qti_accesscontrol_perm_t *dst,
				      uint32_t dst_len);
void qti_accesscontrol_init(void);

#endif /* ACCESSCONTROL_H */
