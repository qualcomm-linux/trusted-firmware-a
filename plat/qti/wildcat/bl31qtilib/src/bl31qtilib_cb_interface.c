/*
 * Copyright (c) 2018-2020, The Linux Foundation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * Changes from Qualcomm Technologies, Inc. are provided under the following
 * license: Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <arch.h>
#include <arch_helpers.h>
#include <bl31/bl31.h>
#include <bl31/interrupt_mgmt.h>
#include <context.h>
#include <drivers/arm/gicv3.h>
#include <drivers/delay_timer.h>
#include <lib/coreboot.h>
#include <lib/el3_runtime/context_mgmt.h>
#include <lib/spinlock.h>
#include <lib/xlat_tables/xlat_tables_v2.h>

#include <platform.h>
#include <qti_interrupt_svc.h>
#include <qti_plat.h>

#include <bl31qtilib_cb_interface.h>
#include <bl31qtilib_spd_agnostic.h>
#include <qti_err_log.h>
#include <qti_sbl_shared_info.h>

#if SDEI_SUPPORT
#include <qti_errno.h>
#include <services/sdei.h>
#endif /* SDEI_SUPPORT */

/*
 * Ensure that bl31qtilib_cb_spinlock_t matches the definition of spinlock_t
 * as bl31qtilib_cb_spin_* casts bl31qtilib_cb_spinlock_t to spinlock_t
 */
_Static_assert(sizeof(bl31qtilib_cb_spinlock_t) == sizeof(spinlock_t),
	       "Mismatch of bl31qtilib_cb_spinlock_t vs spinlock_t");
_Static_assert(offsetof(bl31qtilib_cb_spinlock_t, lock) ==
		       offsetof(spinlock_t, lock),
	       "Mismatch of bl31qtilib_cb_spinlock_t vs spinlock_t");

int bl31qtilib_cb_spin_lock_init(bl31qtilib_cb_spinlock_t *lock)
{
	if (lock == NULL) {
		return 1;
	}
	memset(lock, 0, sizeof(bl31qtilib_cb_spinlock_t));
	return 0;
}

int bl31qtilib_cb_spin_lock(bl31qtilib_cb_spinlock_t *lock)
{
	if (lock == NULL) {
		return 1;
	}
	spin_lock((spinlock_t *)lock);
	return 0;
}

int bl31qtilib_cb_spin_unlock(bl31qtilib_cb_spinlock_t *lock)
{
	if (lock == NULL) {
		return 1;
	}
	spin_unlock((spinlock_t *)lock);
	return 0;
}

unsigned int bl31qtilib_cb_plat_my_core_pos(void)
{
	return plat_my_core_pos();
}

unsigned int bl31qtilib_cb_plat_core_pos_by_mpidr(uintptr_t mpidr)
{
	return (unsigned int)plat_core_pos_by_mpidr((u_register_t)mpidr);
}

unsigned int bl31qtilib_cb_plat_my_cluster_pos(void)
{
	return plat_qti_my_cluster_pos();
}

void bl31qtilib_cb_udelay(uint32_t usec)
{
	udelay(usec);
}

/*
 * Ensure that bl31qtilib_timer_ops_t matches the definition of timer_ops_t
 * as bl31qtilib_cb_timer_init casts bl31qtilib_timer_ops_t to timer_ops_t
 */
_Static_assert(sizeof(bl31qtilib_timer_ops_t) == sizeof(timer_ops_t),
	       "Mismatch of bl31qtilib_timer_ops_t vs timer_ops_t");
_Static_assert(offsetof(bl31qtilib_timer_ops_t, get_timer_value) ==
		       offsetof(timer_ops_t, get_timer_value),
	       "Mismatch of bl31qtilib_timer_ops_t vs timer_ops_t");
_Static_assert(offsetof(bl31qtilib_timer_ops_t, clk_mult) ==
		       offsetof(timer_ops_t, clk_mult),
	       "Mismatch of bl31qtilib_timer_ops_t vs timer_ops_t");
_Static_assert(offsetof(bl31qtilib_timer_ops_t, clk_div) ==
		       offsetof(timer_ops_t, clk_div),
	       "Mismatch of bl31qtilib_timer_ops_t vs timer_ops_t");
_Static_assert(offsetof(bl31qtilib_timer_ops_t, timeout_init_us) ==
		       offsetof(timer_ops_t, timeout_init_us),
	       "Mismatch of bl31qtilib_timer_ops_t vs timer_ops_t");
_Static_assert(offsetof(bl31qtilib_timer_ops_t, timeout_elapsed) ==
		       offsetof(timer_ops_t, timeout_elapsed),
	       "Mismatch of bl31qtilib_timer_ops_t vs timer_ops_t");

void bl31qtilib_cb_timer_init(const bl31qtilib_timer_ops_t *ops_ptr)
{
	timer_init((timer_ops_t *)ops_ptr);
}

uint64_t bl31qtilib_cb_read_cntfrq_el0(void)
{
	return read_cntfrq_el0();
}

uint64_t bl31qtilib_cb_read_cntpct_el0(void)
{
	return read_cntpct_el0();
}

unsigned int bl31qtilib_cb_plat_get_syscnt_freq2(void)
{
	return plat_get_syscnt_freq2();
}

void bl31qtilib_cb_enable_cpu_timer(void)
{
	write_cntps_ctl_el1(0x1);
}

void bl31qtilib_cb_disable_cpu_timer(void)
{
	write_cntps_ctl_el1(0x0);
}

void bl31qtilib_cb_write_cntps_cval_el1(uint64_t ticks)
{
	write_cntps_cval_el1(ticks);
}

void bl31qtilib_cb_write_cntfrq_el0(uint32_t cntfrq)
{
	write_cntfrq_el0(cntfrq);
}

int bl31qtilib_cb_mmap_dynamic_region(uintptr_t base_va, uintptr_t base_pa,
				      size_t size, uint8_t attr)
{
	unsigned int l_attr = 0;
	int ret = -1;

	switch (attr) {
	case BL31QTILIB_UNMAP:
		ret = qti_mmap_remove_dynamic_region(base_va, size);
		break;
	case BL31QTILIB_MAP_NS_RW_NC_ISH_XN:
		l_attr = MT_NS | MT_RW | MT_NON_CACHEABLE |
			 MT_SHAREABILITY_ISH | MT_EXECUTE_NEVER;
		ret = qti_mmap_add_dynamic_region(base_va, size, l_attr);
		break;
	case BL31QTILIB_MAP_S_RW_WBWA_ISH_XN:
		l_attr = MT_SECURE | MT_RW | MT_MEMORY | MT_SHAREABILITY_ISH |
			 MT_EXECUTE_NEVER;
		ret = qti_mmap_add_dynamic_region(base_va, size, l_attr);
		break;
	case BL31QTILIB_MAP_S_RW_DEVICE_ISH_XN:
		l_attr = MT_SECURE | MT_RW | MT_DEVICE | MT_SHAREABILITY_ISH |
			 MT_EXECUTE_NEVER;
		ret = qti_mmap_add_dynamic_region(base_va, size, l_attr);
		break;
	}
	return ret;
}

uintptr_t bl31qtilib_cb_get_qtee_entry_point(void)
{
	entry_point_info_t *ep_info = bl31_plat_get_next_image_ep_info(SECURE);

	if (ep_info != NULL) {
		return ep_info->pc;
	}

	WARN("QTEE entry point not provided by BL2 boot loader\n");
	return 0;
}

int bl31qtilib_cb_int_register_isr(uint32_t intnum, const char *int_desc,
				   void *(*fn)(void *), void *ctx,
				   uint32_t flags, bool enable)
{
	uint32_t target;
	int ret;

	(void)int_desc;

	/* Register the interrupt handler */
	ret = qti_register_intr_type_el3(intnum, fn, ctx);
	if (ret != 0) {
		ERROR("Failed to register EL3 interrupt %d, %s", intnum,
		      int_desc);
		goto bail;
	}

	/* Determine the targeting for this interrupt - either SELF or ANY */
	if ((flags & BL31QTILIB_INTR_ROUTING_MODE_SELF) != 0U) {
		target = BL31QTILIB_INTR_ROUTING_MODE_SELF;
	} else {
		target = BL31QTILIB_INTR_ROUTING_MODE_ALL;
	}

	/* Set the target CPU for the interrupt */
	ret = bl31qtilib_cb_set_int_targets(intnum, target);
	if (ret != 0) {
		ERROR("Failed to set EL3 interrupt target %d, %s, 0x%x", intnum,
		      int_desc, target);
		goto bail;
	}

	/* Enable/Disable the interrupt initially */
	if (enable) {
		qti_enable_intr_type_el3(intnum);
	} else {
		qti_disable_intr_type_el3(intnum);
	}

bail:
	return ret;
}

int bl31qtilib_cb_int_register_fatal_isr(uint32_t intnum, const char *int_desc,
					 void *(*fn)(void *), void *ctx,
					 uint32_t flags, bool enable)
{
	return bl31qtilib_spd_register_isr(intnum, int_desc, fn, ctx, flags,
					   enable);
}

void bl31qtilib_cb_int_unregister_isr(uint32_t intnum, uint32_t flags)
{
	(void)flags;
	qti_unregister_intr_type_el3(intnum);
}

int bl31qtilib_cb_int_disable_fatal_isr(uint32_t intnum)
{
	return bl31qtilib_spd_disable_isr(intnum);
}

int bl31qtilib_cb_int_disable(uint32_t intnum)
{
	qti_disable_intr_type_el3(intnum);
	return 0;
}

int bl31qtilib_cb_int_enable(uint32_t intnum)
{
	qti_enable_intr_type_el3(intnum);
	return 0;
}

int bl31qtilib_cb_set_int_targets(uint32_t intnum, uint32_t target)
{
	/*
	 * target may be:
	 * BL31QTILIB_INTR_ROUTING_MODE_ALL
	 * BL31QTILIB_INTR_ROUTING_MODE_SELF
	 *
	 * or a CPU number
	 *
	 * TFA's routing modes (irm) are:
	 * INTR_ROUTING_MODE_PE
	 * INTR_ROUTING_MODE_ANY
	 *
	 * BL31QTILIB_INTR_ROUTING_MODE_SELF or CPU num => INTR_ROUTING_MODE_PE
	 * BL31QTILIB_INTR_ROUTING_MODE_ALL => INTR_ROUTING_MODE_ANY
	 */
	uint32_t irm = 0U;
	uint32_t mpidr = 0U;
	int ret = 0;

	/*
	 * Only SPIs can have the target set, so bail with
	 * success if this interrupt is not an SPI
	 */
	if (!IS_SPI(intnum)) {
		ret = 0;
		goto bail;
	}

	if (target == BL31QTILIB_INTR_ROUTING_MODE_ALL) {
		irm = INTR_ROUTING_MODE_ANY;
		/* mpidr_el1 is ignored for this routing model */
	} else {
		irm = INTR_ROUTING_MODE_PE;

		if (target == BL31QTILIB_INTR_ROUTING_MODE_SELF) {
			mpidr = read_mpidr_el1();
		} else {
			/* Calculate the MPIDR value for a given CPU number */
			mpidr = plat_qti_logical_cpu_num_to_mpidr(target);

			if (mpidr == QTI_INVALID_MPID_ID) {
				ret = -EINVAL;
				goto bail;
			}
		}
	}

	/* Set the routing mode with TFA */
	plat_ic_set_spi_routing(intnum, irm, mpidr);

bail:
	return ret;
}

#if COREBOOT
/*
 * This function is used to check if the image is authenticated.
 * It is called from bl31_main() in bl31_entrypoint.S.
 * The function returns true if the image is authenticated, false otherwise.
 * The entrypoint is returned in the entrypoint parameter.
 * This implementation will return hardcoded address for COREBOOT enabled
 * targets.
 * This is needed because the entrypoint address is not available from the
 * bootloader in COREBOOT configurations.
 */
bool bl31qtilib_cb_is_image_authenticated(uint32_t image_id,
					  uint64_t *entrypoint)
{
	bool is_authenticated = true;
	uint64_t entry_point = 0U;

	switch (image_id) {
	case TZ_AC_CONFIG_SW_TYPE:
		entry_point = TZ_TZ_AC_CONFIG_BASE_ADDR;
		break;
	case HYP_AC_CONFIG_SW_TYPE:
		entry_point = TZ_HYP_AC_CONFIG_BASE_ADDR;
		break;
	default:
		is_authenticated = false;
		entry_point = 0U;
		break;
	}

	if ((entrypoint != NULL) && is_authenticated) {
		*entrypoint = entry_point;
	}

	return is_authenticated;
}
#else
/*
 * This function is used to check if the image is authenticated.
 * It is called from bl31_main() in bl31_entrypoint.S.
 * The function returns true if the image is authenticated, false otherwise.
 * The entrypoint is returned in the entrypoint parameter.
 * The entrypoint address is determined by the bootloader and passed to the
 * secure world via the share buffer in coldboot.
 */
bool bl31qtilib_cb_is_image_authenticated(uint32_t image_id,
					  uint64_t *entrypoint)
{
	const boot_qsee_interface *sbl_qsee_interface_ptr;
	uint32_t image_idx;

	sbl_qsee_interface_ptr = get_sbl_qsee_interface();

	/* ensure sbl_qsee_interface is populated */
	if (sbl_qsee_interface_ptr->number_images == 0) {
		return false;
	}

	/* parse to find the index for the requested image id */
	for (image_idx = 0; image_idx < sbl_qsee_interface_ptr->number_images;
	     image_idx++) {
		if (image_id ==
		    sbl_qsee_interface_ptr->boot_image_entry[image_idx]
			    .image_id) {
			break;
		}
	}

	/* If the image ID was not found, return false */
	if (image_idx == sbl_qsee_interface_ptr->number_images) {
		return false;
	}

	/* If image is authenticated, populate the entry point */
	if (entrypoint != NULL) {
		*entrypoint =
			sbl_qsee_interface_ptr->boot_image_entry[image_idx]
				.entry_point;
	}

	return true;
}
#endif /* COREBOOT */

void bl31qtilib_cb_error_fatal(int error_code)
{
	ERROR("%s: error %d\n", __func__, error_code);
	bl31qtilib_spd_error_handler(error_code);
}

void bl31qtilib_cb_set_error_fatal(int error_code)
{
	bl31qtilib_spd_set_error_fatal(error_code);
}

void bl31qtilib_cb_set_error_fatal_with_cond(int error_code,
					     bool return_to_hlos)
{
	bl31qtilib_spd_set_error_fatal_with_cond(error_code, return_to_hlos);
}

uint32_t bl31qtilib_cb_dispatch_sdei_event(void)
{
#if SDEI_SUPPORT
	uint32_t ret = 0;
	cpu_context_t *ns_ctx;
	el3_state_t *ns_el3_state;
	u_register_t saved_scr_el3;
	u_register_t saved_spsr_el3;
	u_register_t modified_scr, modified_spsr;

	/* Get the Non-Secure context that will be restored during dispatch */
	cm_el1_sysregs_context_save(SECURE);
	ns_ctx = cm_get_context(NON_SECURE);
	assert(ns_ctx != NULL);

	ns_el3_state = get_el3state_ctx(ns_ctx);

	/* Save the original SCR_EL3 and SPSR_EL3 from the NS context */
	saved_scr_el3 = read_ctx_reg(ns_el3_state, CTX_SCR_EL3);
	saved_spsr_el3 = read_ctx_reg(ns_el3_state, CTX_SPSR_EL3);

	/* We want to make sure that during the event dispatch, the NS side is not
	 * interrupted due to pending secure interrupts (e.g. pending NOCs). Those
	 * will be handled after the dispatch has completed.
	 */

	/* Modify SCR_EL3 in the context: Clear FIQ bit to route FIQ to NS */
	modified_scr = saved_scr_el3 & ~SCR_FIQ_BIT;
	write_ctx_reg(ns_el3_state, CTX_SCR_EL3, modified_scr);

	/* Modify SPSR_EL3 in the context: Set DAIF.F to mask FIQ at EL2 */
	modified_spsr = saved_spsr_el3 | (DAIF_FIQ_BIT << SPSR_DAIF_SHIFT);
	write_ctx_reg(ns_el3_state, CTX_SPSR_EL3, modified_spsr);

	isb();

	/* Dispatch the SDEI event
	 * This will restore the NS context (including our modified SCR_EL3 and
	 * SPSR_EL3) and ERET to NSEL2 with FIQ routing disabled and FIQ masked
	 */
	ret = sdei_dispatch_event(SDEI_FATAL_ERROR);

	/* Restore the original SCR_EL3 and SPSR_EL3 values in the NS context */
	write_ctx_reg(ns_el3_state, CTX_SCR_EL3, saved_scr_el3);
	write_ctx_reg(ns_el3_state, CTX_SPSR_EL3, saved_spsr_el3);

	if (ret == 0) {
		/* We are back from the dispatch (EVENT_COMPLETE or
		 * COMPLETE_AND_RESUME)
		 * The NS context is already restored by sdei_dispatch_event()
		 */

		/* Restore and resume Secure context
		 * This function was called from Secure world, so we need to
		 * return to it
		 */
		cm_el1_sysregs_context_restore(SECURE);
		cm_set_next_eret_context(SECURE);
	}

	return ret;
#else
	ERROR("SDEI not enabled\n");
	panic();
#endif /* SDEI_SUPPORT */
}

uint32_t bl31qtilib_cb_copy_err_logs(uint8_t *dst, size_t dst_size,
				     size_t *out_bytes)
{
	uint8_t *rb_buf;
	uint32_t rb_sz;
	uint32_t count;
	uint32_t start;
	size_t bytes_written = 0;

	spin_lock(&g_qti_err_log.lock);

	/* Make sure the latest ERROR log has been registered for dumping. */
	if (g_qti_err_log.is_recording) {
		qti_err_finalize_recording();
	}

	rb_buf = g_qti_bl31_ringbuf_ptr->wo_cbuf.buf;
	rb_sz = (uint32_t)g_qti_bl31_ringbuf_ptr->conf.buf_size;

	/*
	 * The total might be higher than the maximum number, in which case old
	 * entries have been overwritten.
	 */
	count = (g_qti_err_log.total < QTI_ERR_LOG_MAX_REFS) ?
		g_qti_err_log.total : QTI_ERR_LOG_MAX_REFS;

	/*
	 * write_idx points to the next slot to be written, so the oldest entry
	 * among the last count entries is count steps behind it.
	 */
	start = (g_qti_err_log.write_idx - count + QTI_ERR_LOG_MAX_REFS) %
		QTI_ERR_LOG_MAX_REFS;

	for (uint32_t i = 0; i < count; i++) {
		uint32_t slot = (start + i) % QTI_ERR_LOG_MAX_REFS;
		const qti_err_log_ref_t *ref = &g_qti_err_log.refs[slot];
		uint32_t first_part;

		/*
		 * Skip entries whose bytes have been overwritten by the
		 * circular ring buffer.
		 */
		if ((g_qti_err_log.total_written -
		     g_qti_err_log.total_written_at[slot]) > rb_sz) {
			continue;
		}

		if ((bytes_written + ref->len) > dst_size) {
			break;
		}

		first_part = (uint32_t)(rb_buf + rb_sz - ref->addr);
		if (first_part >= ref->len) {
			memcpy(dst + bytes_written, ref->addr, ref->len);
		} else {
			memcpy(dst + bytes_written, ref->addr, first_part);
			memcpy(dst + bytes_written + first_part, rb_buf,
			       ref->len - first_part);
		}

		bytes_written += ref->len;
	}

	spin_unlock(&g_qti_err_log.lock);

	*out_bytes = bytes_written;

	return 0;
}

int bl31qtilib_cb_ns_va_to_pa(uintptr_t va, unsigned int client_mode,
			      uintptr_t *pa_out)
{
	return qti_ns_va_to_pa(va, client_mode, pa_out);
}
