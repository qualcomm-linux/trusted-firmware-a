/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <plat/common/platform.h>
#include <smccc_helpers.h>

#include <bl31qtilib_cb_interface.h>
#include <bl31qtilib_spd_agnostic.h>

void bl31qtilib_init_sbl_qsee_interface(void);
void *qteed_copy_obj_to_shared_data(uint32_t object_id, void *ptr, size_t size);

uintptr_t bl31qtilib_spd_smc_handler(uint32_t smc_fid, u_register_t x1,
				      u_register_t x2, u_register_t x3,
				      u_register_t x4, void *cookie,
				      void *handle, u_register_t flags)
{
	(void)smc_fid;
	(void)x1;
	(void)x2;
	(void)x3;
	(void)x4;
	(void)cookie;
	(void)flags;

	SMC_RET1(handle, BL31QTILIB_SPD_NOT_SUPPORTED);
}

bool bl31qtilib_spd_is_available(void)
{
	return false;
}

void *bl31qtilib_spd_share_object(uint32_t object_id, void *ptr, size_t size)
{
	(void)object_id;
	(void)ptr;
	(void)size;

	return NULL;
}

/*
 * Some BL31 QTI library variants are built with SPD_qteed and keep hard
 * references to these QTEED handoff symbols. In SPD-agnostic builds, there is
 * no secure payload to receive shared objects, so keep the handoff disabled.
 */
void bl31qtilib_init_sbl_qsee_interface(void)
{
}

void *qteed_copy_obj_to_shared_data(uint32_t object_id, void *ptr, size_t size)
{
	(void)object_id;
	(void)ptr;
	(void)size;

	return NULL;
}

int bl31qtilib_spd_register_isr(uint32_t intnum, const char *int_desc,
				void *(*fn)(void *), void *ctx,
				uint32_t flags, bool enable)
{
	return bl31qtilib_cb_int_register_isr(intnum, int_desc, fn, ctx, flags,
					      enable);
}

int bl31qtilib_spd_disable_isr(uint32_t intnum)
{
	return bl31qtilib_cb_int_disable(intnum);
}

void bl31qtilib_spd_store_intr_context(uint32_t intid, uint32_t flags,
				       void *handle, void *cookie)
{
	(void)intid;
	(void)flags;
	(void)handle;
	(void)cookie;
}

__dead2 void bl31qtilib_spd_plat_error_handler(int error)
{
	(void)error;

	panic();
}

__dead2 void bl31qtilib_spd_error_handler(int error)
{
	/* Call the default plat_error_handler if QTEED is not present */
	plat_error_handler(error);
}

void bl31qtilib_spd_set_error_fatal(int error)
{
	(void)error;

	panic();
}

void bl31qtilib_spd_set_error_fatal_with_cond(int error, bool return_to_hlos)
{
	(void)error;
	(void)return_to_hlos;

	panic();
}

bool bl31qtilib_spd_owns_interrupts(void)
{
	return false;
}

uintptr_t bl31qtilib_spd_get_qsee_iface(uintptr_t bl32_arg0)
{
	(void)bl32_arg0;
	return 0;
}
