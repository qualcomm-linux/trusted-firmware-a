/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 * Copyright (c) 2018-2020, The Linux Foundation. All rights reserved.
 * Changes from Qualcomm Technologies, Inc. are provided under the following license:
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <arch_helpers.h>
#include <common/debug.h>
#include <common/desc_image_load.h>
#include <drivers/console.h>
#include <drivers/generic_delay_timer.h>
#include <drivers/qti/clock/ipq52xx/clock_init.h>
#include <drivers/qti/qtimer/qtimer.h>
#include <drivers/qti/sec_core/sec_core.h>
#include <drivers/qti/watchdog/apss_watchdog.h>

#include <platform.h>
#include <platform_def.h>
#include <qti_plat.h>
#include <qti_uart_console.h>
#ifdef TFA_BL31_RING_LOG_BASE
#include <qti_ringbuf_console.h>

/* Ring buffer lives in IMEM_RW, reserved by plat.ld.S */
extern uint8_t __QTI_RING_LOG_START__[];

CASSERT(sizeof(struct console_ringbuf) <= QTI_IMEM_RING_LOG_SIZE,
	assert_ringbuf_fits_imem_rw);

static struct console_ringbuf *const g_qti_bl31_ringbuf =
	(struct console_ringbuf *)__QTI_RING_LOG_START__;
static console_t g_qti_console_ringbuf;
#endif

/* Variable to hold QTI UART configuration */
static console_t g_qti_console_uart;

/*
 * Placeholder variables for copying the BL32 and Bl33 arguments that have been
 * passed to BL31 from BL2.
 */
static entry_point_info_t bl31_image_ep_info;
static entry_point_info_t bl32_image_ep_info;
static entry_point_info_t bl33_image_ep_info;

/*
 * Variable to hold counter frequency for the CPU's generic timer. In this
 * platform coreboot image configure counter frequency for boot core before
 * reaching TF-A.
 */
static uint64_t g_qti_cpu_cntfrq;

/*
 * Variable to hold bl31 cold boot status. Default value 0x0 means yet to boot.
 * Any other value means cold booted.
 */
uint32_t g_qti_bl31_cold_booted;

/*******************************************************************************
 * Helper to extract BL31 entry point info from arg0 passed to BL31
 ******************************************************************************/

static void bl31_parse_ep_info(u_register_t param)
{
	bl_params_node_t *node;
	bl_params_t *v2 = (void *)(uintptr_t)param;

	if (v2 == NULL) {
		ERROR("Invalid parameter passed to bl31 ep info parsing\n");
		return;
	}

	assert(v2->h.version == PARAM_VERSION_2);
	assert(v2->h.type == PARAM_BL_PARAMS);

	for (node = v2->head; node != NULL; node = node->next_params_info) {
		if (node->image_id == BL31_IMAGE_ID) {
			if (node->ep_info == NULL) {
				ERROR("BL31 entry point info is NULL\n");
				return;
			}
			bl31_image_ep_info = *node->ep_info;
			return; /* Successfully found and copied */
		}
	}
}

/*******************************************************************************
 * Perform any BL31 early platform setup common to ARM standard platforms.
 * Here is an opportunity to copy parameters passed by the calling EL (S-EL1
 * in BL2 & S-EL3 in BL1) before they are lost (potentially). This needs to be
 * done before the MMU is initialized so that the memory layout can be used
 * while creating page tables. BL2 has flushed this information to memory, so
 * we are guaranteed to pick up good data.
 ******************************************************************************/
static void bl31_early_platform_setup(u_register_t from_bl2)
{
	g_qti_cpu_cntfrq = read_cntfrq_el0();

	qti_console_uart_register(&g_qti_console_uart, PLAT_QTI_UART_BASE);
	console_set_scope(&g_qti_console_uart, CONSOLE_FLAG_RUNTIME |
			  CONSOLE_FLAG_BOOT | CONSOLE_FLAG_CRASH);

#ifdef TFA_BL31_RING_LOG_BASE
	qti_console_ringbuf_init(g_qti_bl31_ringbuf);
	qti_console_ringbuf_register(&g_qti_console_ringbuf,
				     g_qti_bl31_ringbuf);
	console_set_scope(&g_qti_console_ringbuf, CONSOLE_FLAG_RUNTIME |
			  CONSOLE_FLAG_BOOT | CONSOLE_FLAG_CRASH);

	/* Publish the ring buffer location in the platform pointer slot. */
	*(uint64_t *)TFA_BL31_RING_LOG_BASE = (uint64_t)(g_qti_bl31_ringbuf);
#endif

	/*
	 * Tell BL31 where the non-trusted software image
	 * is located and the entry state information
	 */
	bl31_params_parse_helper(from_bl2, &bl32_image_ep_info, &bl33_image_ep_info);

	bl31_parse_ep_info(from_bl2);
}

void bl31_early_platform_setup2(u_register_t arg0, u_register_t arg1,
				u_register_t arg2, u_register_t arg3)
{
	bl31_early_platform_setup(arg0);
}

/*******************************************************************************
 * Perform the very early platform specific architectural setup here. At the
 * moment this only initializes the mmu in a quick and dirty way.
 ******************************************************************************/
void bl31_plat_arch_setup(void)
{
	qti_setup_page_tables(
			      BL31_START,
			      BL31_END-BL31_START,
			      BL_CODE_BASE,
			      BL_CODE_END,
			      BL_RO_DATA_BASE,
			      BL_RO_DATA_END
			     );
	enable_mmu_el3(0);
}

/*******************************************************************************
 * Perform any BL31 platform setup common to ARM standard platforms
 ******************************************************************************/
void bl31_platform_setup(void)
{
	generic_delay_timer_init();
	/*
	 * Vote to keep the clock branches secure world depends on enabled,
	 * and tear down BOOT_IMEM, before anything else touches the APSS/
	 * GEMNOC/CNOC buses those votes protect.
	 */
	qti_clock_init();
	/* Initialize the GIC driver, CPU and distributor interfaces */
	plat_qti_gic_driver_init();
	plat_qti_gic_init();
	qti_qtimer_init();
	qti_apss_watchdog_init();
	/* Lock down secure-only access to the APSS per-CPU/cluster registers */
	qti_sec_core_init();
	/* set boot state to cold boot complete. */
	g_qti_bl31_cold_booted = 0x1;
}

/*******************************************************************************
 * Return a pointer to the 'entry_point_info' structure of the next image for the
 * security state specified. BL33 corresponds to the non-secure image type
 * while BL32 corresponds to the secure image type. A NULL pointer is returned
 * if the image does not exist.
 ******************************************************************************/
entry_point_info_t *bl31_plat_get_next_image_ep_info(uint32_t type)
{
	entry_point_info_t *ep;

	assert(sec_state_is_valid(type) != 0);
	ep = (type == SECURE) ? &bl32_image_ep_info : &bl33_image_ep_info;

	return ep->pc ? ep : NULL;
}

/*******************************************************************************
 * Return a pointer to the 'entry_point_info' structure of the bl31 image.
 * Platform specific code can parse it for any args extraction.
 ******************************************************************************/
entry_point_info_t *bl31_plat_get_bl31_image_ep_info(void)
{
	return &bl31_image_ep_info;
}

/*******************************************************************************
 * This function is used by the architecture setup code to retrieve the counter
 * frequency for the CPU's generic timer. This value will be programmed into the
 * CNTFRQ_EL0 register. In Arm standard platforms, it returns the base frequency
 * of the system counter, which is retrieved from the first entry in the
 * frequency modes table. This will be used later in warm boot (psci_arch_setup)
 * of CPUs to set when CPU frequency.
 ******************************************************************************/
unsigned int plat_get_syscnt_freq2(void)
{
	assert(g_qti_cpu_cntfrq != 0);
	return g_qti_cpu_cntfrq;
}
