/*
 * Copyright (c) 2018-2020, 2025. The Linux Foundation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * Changes from Qualcomm Technologies, Inc. are provided under the following
 * license:
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef BL31QTILIB_DEFS_H
#define BL31QTILIB_DEFS_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <platform_def.h>

typedef uint32_t tzbsp_err_fatal_e;

typedef struct bl31qtilib_cb_spinlock {
	volatile uint32_t lock;
} bl31qtilib_cb_spinlock_t;

typedef struct bl31qtilib_timer_ops_t {
	uint32_t (*get_timer_value)(void);
	uint32_t clk_mult;
	uint32_t clk_div;
	uint64_t (*timeout_init_us)(uint32_t usec);
	bool (*timeout_elapsed)(uint64_t cnt);
} bl31qtilib_timer_ops_t;

#define BL31QTILIB_UNMAP 0		    /* UNMAP */
#define BL31QTILIB_MAP_NS_RW_NC_ISH_XN 1    /* DEFAULT */
#define BL31QTILIB_MAP_S_RW_WBWA_ISH_XN 2   /* PRIVATE */
#define BL31QTILIB_MAP_S_RW_DEVICE_ISH_XN 3 /* SHARED */

/*
 * Interrupt routing modes - must align with QTEE and
 * is statically asserted in int_svc.c
 */
#define BL31QTILIB_INTR_ROUTING_MODE_ALL   0x03000000U
#define BL31QTILIB_INTR_ROUTING_MODE_SELF  0x02000000U

/*
 * Define the current maskings for PSCI needs
 */
#define BL31QTILIB_PSCI_FIELD_MASK 0xFF
#define BL31QTILIB_PSCI_FIELD_WIDTH 8

/*
 * Syscalls marked with RSP flag will be passed a ptr to
 * this struct.
 */
typedef struct smc_rsp_s {
	uintptr_t rsp[6]; /* Values to return to HLOS or TEE */
	/*
	 * Indication to post an smc after the current smc has been handled
	 * and return to tee.
	 */
	uintptr_t return_to_tee;
} smc_rsp_t;

#endif /* BL31QTILIB_DEFS_H */
