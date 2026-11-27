/*
 * Copyright (c) 2016-2018, 2024-2026, Qualcomm Innovation Center, Inc.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Ported from TZ.XF.5.29.1-00158-KODIAKAAAAANAAZT-1:
 * trustzone_images/core/systemdebug/sysdbg/qsee/src/target/lemans/sysdbg_dcc_hal_lemans.c
 */

#include <string.h>
#include <stdint.h>
#include <lib/mmio.h>
#include <common/debug.h>
#include <drivers/qti/sysdbg/sysdbg_dcc_hal.h>
#include <sysdbg_dcc_virt.h>

/**
 * @file sysdbg_dcc_hal.c
 * @brief DCC Hardware Abstraction Layer Implementation for Shikra
 *
 * This module defines target-specific HAL for DCC hardware.
 *
 * Ported from TZ Lemans reference implementation.
 */

/*===========================================================================
 * HWIO Wrapper Macros
 * These macros provide compatibility with TrustZone HWIO layer
 *===========================================================================
 */

/* Read indexed register with mask */
#define HWIO_INMI(reg, index, mask) \
	(mmio_read_32(HWIO_##reg##_ADDR(index)) & (mask))

/* Read indexed register */
#define HWIO_INI(reg, index) \
	mmio_read_32(HWIO_##reg##_ADDR(index))

/* Write indexed register with mask (read-modify-write) */
#define HWIO_OUTMI(reg, index, mask, val) \
	do { \
		uint32_t _reg_val = mmio_read_32(HWIO_##reg##_ADDR(index)); \
		_reg_val &= ~(mask); \
		_reg_val |= ((val) & (mask)); \
		mmio_write_32(HWIO_##reg##_ADDR(index), _reg_val); \
	} while (0)

/* Write indexed register directly */
#define HWIO_OUTI(reg, index, val) \
	mmio_write_32(HWIO_##reg##_ADDR(index), (val))

/*===========================================================================
 * Local Definitions
 *===========================================================================
 */

/* Max number of lists supported by HW */
#define DCC_LL_COUNT      (HWIO_DCC_LL_BASEm_MAXm + 1)
#define DCC_SRAM_ADDRMASK 0x00007FFFU  /* For 32KB SRAM (Shikra) */

/*===========================================================================
 * Function: hal_dcc_get_max_lists
 * Reference: sysdbg_dcc_hal_lemans.c:56
 *===========================================================================
 */
uint32_t hal_dcc_get_max_lists(void)
{
	return DCC_LL_COUNT;
}

/*===========================================================================
 * Function: hal_dcc_config_list_base
 * Reference: sysdbg_dcc_hal_lemans.c:86
 *===========================================================================
 */
void hal_dcc_config_list_base(uint32_t list, uint32_t base)
{
	/* Program the start address of the linked list */
	HWIO_OUTMI(DCC_LL_BASEm, list,
		   HWIO_DCC_LL_BASEm_ADDRESS_BMSK,
		   ((base + WORD_INDEX(DCC_SRAM_BASE)) << HWIO_DCC_LL_BASEm_ADDRESS_SHFT));
}

/*===========================================================================
 * Function: hal_dcc_config_data_base
 * Reference: sysdbg_dcc_hal_lemans.c:115
 *===========================================================================
 */
void hal_dcc_config_data_base(uint32_t list, uint32_t base)
{
	/* Program the address to put the fetched data in */
	HWIO_OUTMI(DCC_FD_BASEm, list,
		   HWIO_DCC_FD_BASEm_ADDRESS_BMSK,
		   ((base + WORD_INDEX(DCC_SRAM_BASE)) << HWIO_DCC_FD_BASEm_ADDRESS_SHFT));
}

/*===========================================================================
 * Function: hal_dcc_config_timeout
 * Reference: sysdbg_dcc_hal_lemans.c:144
 *===========================================================================
 */
void hal_dcc_config_timeout(uint32_t list, uint32_t timeout)
{
	HWIO_OUTMI(DCC_LL_TIMEOUTm, list,
		   HWIO_DCC_LL_TIMEOUTm_VALUE_BMSK,
		   (timeout << HWIO_DCC_LL_TIMEOUTm_VALUE_SHFT));
}

/*===========================================================================
 * Function: hal_dcc_config_trans_timeout
 * Reference: sysdbg_dcc_hal_lemans.c:172
 *===========================================================================
 */
void hal_dcc_config_trans_timeout(uint32_t list, uint32_t timeout)
{
#ifdef HWIO_DCC_TRANS_TIMEOUTm_ADDR
	HWIO_OUTMI(DCC_TRANS_TIMEOUTm, list,
		   HWIO_DCC_TRANS_TIMEOUTm_VALUE_BMSK,
		   (timeout << HWIO_DCC_TRANS_TIMEOUTm_VALUE_SHFT));
#else
	(void)list;
	(void)timeout;
#endif
}

/*===========================================================================
 * Function: hal_dcc_config_sink
 * Reference: sysdbg_dcc_hal_lemans.c:204
 *===========================================================================
 */
void hal_dcc_config_sink(uint32_t list, uint32_t sink)
{
	HWIO_OUTMI(DCC_LL_CONFIGm, list,
		   HWIO_DCC_LL_CONFIGm_SINK_BMSK,
		   (sink << HWIO_DCC_LL_CONFIGm_SINK_SHFT));
}

/*===========================================================================
 * Function: hal_dcc_config_trigger
 * Reference: sysdbg_dcc_hal_lemans.c:234
 *===========================================================================
 */
void hal_dcc_config_trigger(uint32_t list, hal_dcc_trig_type trigger)
{
	if (trigger == HAL_DCC_HW_TRIGGER) {
		HWIO_OUTMI(DCC_LL_CONFIGm, list,
			   HWIO_DCC_LL_CONFIGm_HW_TRIGGER_EN_BMSK,
			   (0x1 << HWIO_DCC_LL_CONFIGm_HW_TRIGGER_EN_SHFT));
	}
}

/*===========================================================================
 * Function: hal_dcc_config_func
 * Reference: sysdbg_dcc_hal_lemans.c:265
 *===========================================================================
 */
void hal_dcc_config_func(uint32_t list, uint32_t func)
{
	HWIO_OUTMI(DCC_LL_CONFIGm, list,
		   HWIO_DCC_LL_CONFIGm_FUNC_BMSK,
		   (func << HWIO_DCC_LL_CONFIGm_FUNC_SHFT));
}

/*===========================================================================
 * Function: hal_dcc_lock_list
 * Reference: sysdbg_dcc_hal_lemans.c:293
 *===========================================================================
 */
void hal_dcc_lock_list(uint32_t list)
{
	HWIO_OUTI(DCC_LL_LOCKm, list,
		  (HWIO_DCC_LL_LOCKm_ACTIVE_BMSK | HWIO_DCC_LL_LOCKm_S_BIT_BMSK));
}

/*===========================================================================
 * Function: hal_dcc_lock_list_ns
 * Reference: sysdbg_dcc_hal_lemans.c:321
 *===========================================================================
 */
void hal_dcc_lock_list_ns(uint32_t list)
{
	HWIO_OUTI(DCC_LL_LOCKm, list, HWIO_DCC_LL_LOCKm_ACTIVE_BMSK);
}

/*===========================================================================
 * Function: hal_dcc_unlock_list
 * Reference: sysdbg_dcc_hal_lemans.c:349
 *===========================================================================
 */
void hal_dcc_unlock_list(uint32_t list)
{
	HWIO_OUTMI(DCC_LL_LOCKm, list, HWIO_DCC_LL_LOCKm_ACTIVE_BMSK, 0);
}

/*===========================================================================
 * Function: hal_dcc_is_list_locked
 * Reference: sysdbg_dcc_hal_lemans.c:377
 *===========================================================================
 */
bool hal_dcc_is_list_locked(uint32_t list)
{
	return (HWIO_INMI(DCC_LL_LOCKm, list, HWIO_DCC_LL_LOCKm_ACTIVE_BMSK) != 0);
}

/*===========================================================================
 * Function: hal_dcc_get_sink
 * Reference: sysdbg_dcc_hal_lemans.c:407
 *===========================================================================
 */
hal_dcc_sink_type hal_dcc_get_sink(uint32_t list)
{
	return ((hal_dcc_sink_type)HWIO_INMI(DCC_LL_CONFIGm, list,
					      HWIO_DCC_LL_CONFIGm_SINK_BMSK));
}

/*===========================================================================
 * Function: hal_dcc_get_database
 * Reference: sysdbg_dcc_hal_lemans.c:435
 *===========================================================================
 */
uint32_t hal_dcc_get_database(uint32_t list)
{
	return ((BYTE_INDEX((HWIO_INMI(DCC_FD_BASEm, list,
				       HWIO_DCC_FD_BASEm_ADDRESS_BMSK))) +
		 DCC_SRAM_SIZE) & (DCC_SRAM_ADDRMASK));
}

/*===========================================================================
 * Function: hal_dcc_config_list
 * Reference: sysdbg_dcc_hal_lemans.c:464
 *===========================================================================
 */
bool hal_dcc_config_list(uint32_t idx, hal_dcc_list_params_type *params)
{
	bool success = false;

	if (params) {
		/* Lock the list */
		hal_dcc_lock_list(idx);

		if (!hal_dcc_is_list_locked(idx))
			return success;

		/* Configure all params of the list */
		hal_dcc_config_sink(idx, params->esink);

		hal_dcc_config_timeout(idx, params->ntimeout);

		hal_dcc_config_trans_timeout(idx, params->ntranstimeout);

		hal_dcc_config_data_base(idx, WORD_INDEX(ALIGN_ADDR(params->noffset, 16)));

		hal_dcc_config_func(idx, params->efunc);

		hal_dcc_config_trigger(idx, params->etrigger);

		success = true;
	}

	return success;
}

/*===========================================================================
 * Function: hal_dcc_get_sinkbase
 * Reference: sysdbg_dcc_hal_lemans.c:520
 *===========================================================================
 */
uint64_t hal_dcc_get_sinkbase(hal_dcc_sink_type sink)
{
	switch (sink) {
	case HAL_DCC_SINK_SRAM:
		return DCC_SRAM_BASE;

	case HAL_DCC_SINK_ETB:
		return ETFETB_BASE;

	default:
		return 0;
	}
}

/*===========================================================================
 * Function: hal_dcc_init_sink
 * Reference: sysdbg_dcc_hal_lemans.c:562
 *===========================================================================
 */
bool hal_dcc_init_sink(hal_dcc_sink_type sink)
{
	/* Get the base address for this sink */
	static bool init;

	if (!init) {
		volatile uint64_t *base = (uint64_t *)((uintptr_t)(hal_dcc_get_sinkbase(sink)));

		if (base) {
			for (uint32_t count = DCC_SRAM_SIZE; count > 0; count -= sizeof(uint64_t))
				*(base++) = 0L;
		}

		init = true;
	}

	return init;
}

/*===========================================================================
 * Function: hal_dcc_program_list
 * Reference: sysdbg_dcc_hal_lemans.c:607
 *===========================================================================
 */
bool hal_dcc_program_list(void *list, uint32_t list_idx, uint32_t offset, uint32_t size)
{
	bool success = false;

	if ((offset & DCC_SRAM_SIZE) || (offset >= DCC_ETFETB_SIZE))
		return success;

	uint32_t aligned_offset = ALIGN_ADDR(offset, 16);

	/* Program the actual linked list based on the sink */
	hal_dcc_sink_type sink = hal_dcc_get_sink(list_idx);

	if (sink == HAL_DCC_SINK_SRAM) {
		if (offset & DCC_SRAM_SIZE)
			return success;

		/* Use 4-byte (word) writes directly to DCC SRAM device memory.
		 * memscpy does byte-by-byte copy which requires a TLB mapping for
		 * the QDSS DCC RAM region and can cause a crash on device memory.
		 * DCC SRAM requires word-aligned 32-bit accesses.
		 */
		volatile uint32_t *dst = (volatile uint32_t *)(uintptr_t)(aligned_offset + DCC_SRAM_BASE);
		const uint32_t *src = (const uint32_t *)list;
		uint32_t word_count = size / sizeof(uint32_t);

		for (uint32_t i = 0; i < word_count; i++)
			dst[i] = src[i];
	} else if (sink == HAL_DCC_SINK_ETB) {
		if (offset & DCC_ETFETB_SIZE)
			return success;

		/* Use 4-byte (word) writes directly to ETB device memory.
		 * memscpy does byte-by-byte copy which requires a TLB mapping for
		 * the QDSS ETB region and can cause a crash on device memory.
		 * ETB requires word-aligned 32-bit accesses.
		 */
		volatile uint32_t *dst = (volatile uint32_t *)(uintptr_t)(aligned_offset + ETFETB_BASE);
		const uint32_t *src = (const uint32_t *)list;
		uint32_t word_count = size / sizeof(uint32_t);

		for (uint32_t i = 0; i < word_count; i++)
			dst[i] = src[i];
	}

	/* Now point the memory to the list */
	hal_dcc_config_list_base(list_idx, WORD_INDEX(ALIGN_ADDR(offset, 16)));

	success = true;

	return success;
}
