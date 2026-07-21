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

#define SMC_SET_RSP(rsp_ptr, rsp0, rsp1, rsp2, rsp3, rsp4, rsp5)	\
	do {								\
		if ((rsp_ptr) == NULL) {				\
			break;						\
		}							\
		((smc_rsp_t *)(rsp_ptr))->rsp[0] = (uintptr_t)(rsp0);	\
		((smc_rsp_t *)(rsp_ptr))->rsp[1] = (uintptr_t)(rsp1);	\
		((smc_rsp_t *)(rsp_ptr))->rsp[2] = (uintptr_t)(rsp2);	\
		((smc_rsp_t *)(rsp_ptr))->rsp[3] = (uintptr_t)(rsp3);	\
		((smc_rsp_t *)(rsp_ptr))->rsp[4] = (uintptr_t)(rsp4);	\
		((smc_rsp_t *)(rsp_ptr))->rsp[5] = (uintptr_t)(rsp5);	\
	} while (0U)

/*
 * Helper macros for setting smc_rsp_t*
 */
#define SMC_SET_RSP1(rsp_ptr, rsp0)					\
	SMC_SET_RSP(rsp_ptr, rsp0, 0U, 0U, 0U, 0U, 0U)
#define SMC_SET_RSP2(rsp_ptr, rsp0, rsp1)				\
	SMC_SET_RSP(rsp_ptr, rsp0, rsp1, 0U, 0U, 0U, 0U)
#define SMC_SET_RSP3(rsp_ptr, rsp0, rsp1, rsp2)			\
	SMC_SET_RSP(rsp_ptr, rsp0, rsp1, rsp2, 0U, 0U, 0U)
#define SMC_SET_RSP4(rsp_ptr, rsp0, rsp1, rsp2, rsp3)		\
	SMC_SET_RSP(rsp_ptr, rsp0, rsp1, rsp2, rsp3, 0U, 0U)
#define SMC_SET_RSP5(rsp_ptr, rsp0, rsp1, rsp2, rsp3, rsp4)	\
	SMC_SET_RSP(rsp_ptr, rsp0, rsp1, rsp2, rsp3, rsp4, 0U)
#define SMC_SET_RSP6(rsp_ptr, rsp0, rsp1, rsp2, rsp3, rsp4, rsp5) \
	SMC_SET_RSP(rsp_ptr, rsp0, rsp1, rsp2, rsp3, rsp4, rsp5)

#endif /* BL31QTILIB_DEFS_H */
