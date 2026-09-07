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

#ifndef BL31QTILIB_CB_INTERFACE_H
#define BL31QTILIB_CB_INTERFACE_H

#include <bl31qtilib_defs.h>

int bl31qtilib_cb_spin_lock_init(bl31qtilib_cb_spinlock_t *lock);
int bl31qtilib_cb_spin_lock(bl31qtilib_cb_spinlock_t *lock);
int bl31qtilib_cb_spin_unlock(bl31qtilib_cb_spinlock_t *lock);
unsigned int bl31qtilib_cb_plat_my_core_pos(void);
unsigned int bl31qtilib_cb_plat_core_pos_by_mpidr(uintptr_t mpidr);
unsigned int bl31qtilib_cb_plat_my_cluster_pos(void);

void bl31qtilib_cb_udelay(uint32_t usec);
void bl31qtilib_cb_timer_init(const bl31qtilib_timer_ops_t *ops_ptr);
uint64_t bl31qtilib_cb_read_cntfrq_el0(void);
uint64_t bl31qtilib_cb_read_cntpct_el0(void);
void bl31qtilib_cb_enable_cpu_timer(void);
void bl31qtilib_cb_disable_cpu_timer(void);
void bl31qtilib_cb_write_cntps_cval_el1(uint64_t ticks);
void bl31qtilib_cb_write_cntfrq_el0(uint32_t cntfrq);
unsigned int bl31qtilib_cb_plat_get_syscnt_freq2(void);
int bl31qtilib_cb_mmap_dynamic_region(uintptr_t base_va, uintptr_t base_pa,
				      size_t size, uint8_t attr);
uintptr_t bl31qtilib_cb_get_qtee_entry_point(void);

/*
 * Installs an ISR function for a Group 0 interrupt.
 *
 * @param [in] intnum   Interrupt number the ISR is registered for.
 * @param [in] int_desc Character string description of the interrupt (unused)
 * @param [in] fn       The ISR function itself.
 * @param [in] ctx      The context passed to ISR.
 * @param [in] flags    Combination of \c BL31QTILIB_INTR_* flags.
 * @param [in] enable   Initial interrupt enable/disable status on the
 *                      interrupt controller.
 *
 * @return Zero on success, error code otherwise.
 *
 */
int bl31qtilib_cb_int_register_isr(uint32_t intnum, const char *int_desc,
				   void *(*fn)(void *), void *ctx,
				   uint32_t flags, bool enable);

/*
 * Install an ISR function for a fatal error. This might be registered with
 * Secure Group 1 in QTEE if qteed SPD is present, or in Group 0 with TFA.
 *
 * @param [in] intnum   Interrupt number the ISR is registered for.
 * @param [in] int_desc Character string description of the interrupt
 * @param [in] fn       The ISR function itself.
 * @param [in] ctx      The context passed to ISR.
 * @param [in] flags    Combination of \c BL31QTILIB_INTR_* flags.
 * @param [in] enable   Initial interrupt enable/disable status on the
 *                      interrupt controller.
 *
 * @return Zero on success, error code otherwise.
 *
 */
int bl31qtilib_cb_int_register_fatal_isr(uint32_t intnum, const char *int_desc,
					 void *(*fn)(void *), void *ctx,
					 uint32_t flags, bool enable);

/*
 * Uninstalls an ISR function. If no installed ISR is found for the interrupt
 * number, then it is not an error and is ignored silently.
 *
 * @param [in] intnum   Interrupt number the ISR is registered for.
 * @param [in] flags    Combination of \c BL31QTILIB_INTR_* flags (unused)
 *
 */
void bl31qtilib_cb_int_unregister_isr(uint32_t intnum, uint32_t flags);

/*
 * Disables the given interrupt on the interrupt controller.
 *
 * @param [in] intnum   The interrupt to be disabled.
 *
 * @return 0 if successful, Error code otherwise.
 *
 */
int bl31qtilib_cb_int_disable(uint32_t intnum);

/*
 * Disables the given interrupt on the interrupt controller for a fatal error.
 * If qteed is enabled, then it will disable it in QTEE.
 *
 * @param [in] intnum   The interrupt to be disabled. NB: if qteed is enabled,
 *                      this parameter will be ignored and the interrupt being
 *                      handled will be disabled.
 *
 * @return 0 if successful, Error code otherwise.
 *
 */
int bl31qtilib_cb_int_disable_fatal_isr(uint32_t intnum);

/*
 * Enables the given interrupt on the interrupt controller.
 *
 * @param [in] intnum   The interrupt to be enabled.
 *
 * @return 0 if successful, Error code otherwise.
 *
 */
int bl31qtilib_cb_int_enable(uint32_t intnum);

/*
 * @brief Configure target list for an interrupt.
 *
 * @param[in]      intnum        - Interrupt number
 * @param[in]      target        - Target CPU, can be:
 * BL31QTILIB_INTR_ROUTING_MODE_ALL
 * BL31QTILIB_INTR_ROUTING_MODE_SELF
 * or a valid logical CPU number
 *
 * @return                         0 if successful, otherwise return
 * Error code.
 */
int bl31qtilib_cb_set_int_targets(uint32_t intnum, uint32_t target);

/*
 * @brief API to check if image is authenticated, if so
 * return true and update entry point else return false
 *
 * @param[in]            image_id          - sw_id
 * @param[in/out]        entrypoint        - entrypoint of image that is
 * requested
 *
 * @return               true if image is authenticated, otherwise false
 */
bool bl31qtilib_cb_is_image_authenticated(uint32_t image_id,
					  uint64_t *entrypoint);

/*
 * @brief API to report a fatal error. BL31 will forward the error to QTEE
 * which then will prepare for crashdump.
 *
 * This function does not return.
 *
 * @param[in]		errorCode	Fatal error reason
 *
 */
void bl31qtilib_cb_error_fatal(int error_code);

/*
 * @brief API to report a fatal error within an ISR. BL31 will forward the error
 * to QTEE only if qteed is enabled, and QTEE will prepare for crashdump.
 *
 * This function does not return.
 *
 * @param[in]		errorCode	Fatal error reason
 *
 */
void bl31qtilib_cb_set_error_fatal(int error_code);

/*
 * @brief API to report a fatal error within an ISR. BL31 will forward the error
 * to QTEE only if qteed is enabled, and QTEE will prepare for crashdump. The
 * condition allows QTEE to go back to HLOS to collect additional dumps.
 *
 * This function does not return.
 *
 * @param[in]		errorCode	Fatal error reason
 * @param[in]		return_to_hlos	Whether to return to HLOS or not
 *
 */
void bl31qtilib_cb_set_error_fatal_with_cond(int error_code,
					     bool return_to_hlos);

/*
 * @brief API to dispatch a fatal error SDEI event
 *
 */
uint32_t bl31qtilib_cb_dispatch_sdei_event(void);

/*
 * bl31qtilib_cb_copy_err_logs
 *
 * Copies ERROR-level log segments into dst as a raw concatenated string.
 *
 * @param[in]  dst        Destination buffer.
 * @param[in]  dst_size   Available bytes.
 * @param[out] out_bytes  Total bytes written.
 * @return 0 on success, error code otherwise.
 */
uint32_t bl31qtilib_cb_copy_err_logs(uint8_t *dst, size_t dst_size,
				     size_t *out_bytes);

/*
 * bl31qtilib_cb_ns_va_to_pa
 *
 * Translates a Non-Secure virtual address to its physical address using the
 * hardware AT instruction for the specified exception level.
 *
 * @param[in]  va           Non-Secure virtual address to translate.
 * @param[in]  client_mode  Exception level of the caller.
 * @param[out] pa_out       Resolved physical address.
 * @return 0 on success, error code otherwise.
 */
int bl31qtilib_cb_ns_va_to_pa(uintptr_t va, unsigned int client_mode,
			      uintptr_t *pa_out);

#endif /* BL31QTILIB_CB_INTERFACE_H */
