/*
 * Copyright (c) 2026 Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <string.h>

#include <arch.h>
#include <arch_features.h>
#include <arch_helpers.h>
#include <bl31/bl31.h>
#include <common/debug.h>
#include <common/desc_image_load.h>
#include <drivers/arm/dcc.h>
#include <drivers/arm/gicv3.h>
#include <drivers/console.h>
#include <drivers/generic_delay_timer.h>
#include <drivers/qti/chipinfo/chipinfo.h>
#include <drivers/qti/smem/smem.h>
#include <lib/bl_aux_params/bl_aux_params.h>
#include <lib/coreboot.h>
#include <lib/mmio.h>
#include <plat/common/platform.h>
#include <platform.h>

#include <bl31qtilib_interface.h>

void __dead2 bl31qtilib_panic(void);
#include <platform_def.h>
#include <qti_interrupt_svc.h>
#include <qti_ringbuf_console.h>
#include <qti_uart_console.h>
#include <tfa_bl31_shared_imem.h>
#include <wildcat_plat.h>
#include <wildcat_sbl_shared_info.h>
#include <wildcat_xbl_params.h>

static struct boot_tfabl31_interface *sbl_tfabl31_interface;

void __dead2 qti_panic(void)
{
	bl31qtilib_panic();
}

/*
 * Placeholder variables for copying the arguments that have been passed to
 * BL31 from BL2.
 */
static entry_point_info_t bl33_image_ep_info;

static entry_point_info_t bl32_image_ep_info;

/*
 * Variable to hold counter frequency for the CPU's generic timer. In this
 * platform coreboot image configure counter frequency for boot core before
 * reaching TF-A.
 */
static unsigned int g_qti_cpu_cntfrq;

/* XBL-latched MTE enable flag (latched from bl_aux_params handler) */
static bool g_qti_mte_enabled;
static console_t g_qti_console_uart;
static console_t qti_console_ringbuf;

static bool qti_aux_param_handler(struct bl_aux_param_header *param)
{
	INFO("%s: type=0x%lx\n", __func__, (uint64_t)param->type);

	switch ((enum bl_aux_qti_param_type)param->type) {
	case BL_AUX_PARAM_QTI_CPU_FEATURES: {
		const struct bl_aux_param_qti_cpu_features *p =
			(const struct bl_aux_param_qti_cpu_features *)param;

		INFO("CPU features param: features=0x%x cpu_mte2=%u\n",
		     p->cpu_features,
		     (unsigned int)is_feat_mte2_supported());

		if ((p->cpu_features & BL_AUX_CPU_FEATURE_MTE_ENABLED) &&
		    is_feat_mte2_supported()) {
			g_qti_mte_enabled = true;
		}

		return true;
	}
	default:
		INFO("Unknown param type, skipping\n");
		return false;
	}
}

void qti_el3_sys_regs_init(void)
{
	write_icc_sre_el3(read_icc_sre_el3() |
			  (ICC_SRE_SRE_BIT | ICC_SRE_EN_BIT));

	/* PMHE is the only writable field in the legacy 0xCC40 value. */
	write_icc_ctlr_el3(ICC_CTLR_EL3_PMHE_BIT);

	write_icc_sre_el1(read_icc_sre_el1() | ICC_SRE_SRE_BIT);

	write_icc_igrpen0_el1(IGRPEN1_EL1_ENABLE_G0_BIT);
}

void plat_qti_cpu_boot_setup(void)
{
	qti_el3_sys_regs_init();
}

void bl31_early_platform_setup(u_register_t from_bl2,
			       u_register_t plat_params_from_bl2)
{
	uintptr_t uart_base;
	struct console_ringbuf *ringbuf;

	g_qti_cpu_cntfrq = PLAT_SYSCNT_FREQ;

	g_qti_mte_enabled = false;
	bl_aux_params_parse(plat_params_from_bl2, qti_aux_param_handler);

	uart_base = QTI_BL31_EARLY_UART_BASE_ADDR;
	if (uart_base != 0U) {
		qti_console_uart_register(&g_qti_console_uart, uart_base);
	}

	ringbuf = plat_qti_get_bl31_ringbuf();
	qti_console_ringbuf_init(ringbuf);

	qti_console_ringbuf_register(&qti_console_ringbuf, ringbuf);
	qti_console_ringbuf.flags |= CONSOLE_FLAG_RUNTIME;

	INFO("MTE latched=%u (xbl_param=0x%lx)\n",
	     (unsigned int)g_qti_mte_enabled,
	     (u_register_t)plat_params_from_bl2);

	*(uint64_t *)TFA_BL31_SHARED_IMEM_RING_LOG_BASE =
					(uint64_t)ringbuf;

	/*
	 * Tell BL31 where the non-trusted software image
	 * is located and the entry state information
	 */
	bl31_params_parse_helper(from_bl2, &bl32_image_ep_info,
				 &bl33_image_ep_info);
}

void bl31_early_platform_setup2(u_register_t arg0, u_register_t arg1,
				u_register_t arg2, u_register_t arg3)
{
	(void)arg2;
	(void)arg3;

	bl31_early_platform_setup(arg0, arg1);
}

void bl31_plat_arch_setup(void)
{
	qti_setup_page_tables(BL31_START, BL31_END - BL31_START, BL_CODE_BASE,
			      BL_CODE_END, BL_RO_DATA_BASE, BL_RO_DATA_END,
			      QTI_BL31_COHERENT_RAM_BASE,
			      QTI_BL31_COHERENT_RAM_END);
	enable_mmu_el3(0);
}

static const char qti_build_variant[] = "BL31 variant: bl31";

extern char QC_IMAGE_VERSION_STRING_AUTO_UPDATED[];
extern char IMAGE_VARIANT_STRING_AUTO_UPDATED[];
extern char OEM_IMAGE_VERSION_STRING_AUTO_UPDATED[];
extern char OEM_IMAGE_UUID_STRING_AUTO_UPDATED[];
extern char OEM_HOST_TIMESTAMP_STRING_AUTO_UPDATED[];

void bl31_platform_setup(void)
{
	INFO("Starting %s - %s\n", qti_build_variant, bl31qtilib_build_variant);
	INFO("QC Image Version %s\n", QC_IMAGE_VERSION_STRING_AUTO_UPDATED);
	INFO("Image Variant %s\n", IMAGE_VARIANT_STRING_AUTO_UPDATED);
	INFO("OEM Image Version %s\n", OEM_IMAGE_VERSION_STRING_AUTO_UPDATED);
	INFO("OEM Image UUID %s\n", OEM_IMAGE_UUID_STRING_AUTO_UPDATED);
	INFO("OEM Host timestamp %s\n", OEM_HOST_TIMESTAMP_STRING_AUTO_UPDATED);
	bl31qtilib_set_boot_cpu_num(plat_my_core_pos());

	bl31qtilib_bl31_platform_early_setup();
	qti_smem_init();

	plat_qti_gic_driver_init();
	plat_qti_gic_init();

	if (qti_chipinfo_init() != CHIPINFO_SUCCESS) {
		WARN("ChipInfo initialization error\n");
	}

	qti_interrupt_svc_init(bl32_image_ep_info.pc != 0UL);

	bl31qtilib_bl31_platform_setup();
}

entry_point_info_t *bl31_plat_get_next_image_ep_info(uint32_t type)
{
	if (type == SECURE) {
		if (bl32_image_ep_info.pc != 0UL) {
			assert((bl32_image_ep_info.h.type == PARAM_BL_PARAMS) ||
			       (bl32_image_ep_info.h.type == PARAM_EP));
			assert((bl32_image_ep_info.h.attr & EP_SECURITY_MASK) ==
			       SECURE);
			return &bl32_image_ep_info;
		}
	} else if (bl33_image_ep_info.pc != 0UL) {
		assert((bl33_image_ep_info.h.type == PARAM_BL_PARAMS) ||
		       (bl33_image_ep_info.h.type == PARAM_EP));
		assert((bl33_image_ep_info.h.attr & EP_SECURITY_MASK) ==
		       NON_SECURE);
		return &bl33_image_ep_info;
	}

	return NULL;
}

unsigned int plat_get_syscnt_freq2(void)
{
	assert(g_qti_cpu_cntfrq != 0);
	return g_qti_cpu_cntfrq;
}

const struct boot_tfabl31_interface *get_sbl_tfabl31_interface(void)
{
	assert(sbl_tfabl31_interface != NULL);
	return sbl_tfabl31_interface;
}

void bl31_plat_runtime_setup(void)
{
	bl31qtilib_bl31_platform_post_coldboot_setup();
}

#if PLAT_RUNTIME_DEBUG_CFG

bool plat_trace_enabled(int security_state)
{
	switch (security_state) {
	case SECURE:
		return bl31qtilib_is_invasive_debug_enabled(SECURE) ||
			bl31qtilib_is_non_invasive_debug_enabled(SECURE);
	case NON_SECURE:
		return bl31qtilib_is_non_invasive_debug_enabled(NON_SECURE);
	default:
		return false;
	}
}

bool plat_external_debug_access_enabled(void)
{
	return bl31qtilib_is_invasive_debug_enabled(NON_SECURE);
}

bool plat_perfmon_enabled(int security_state)
{
	switch (security_state) {
	case SECURE:
		return bl31qtilib_is_non_invasive_debug_enabled(SECURE);
	case NON_SECURE:
		return bl31qtilib_is_non_invasive_debug_enabled(SECURE);
	default:
		return false;
	}
}

#endif /* PLAT_RUNTIME_DEBUG_CFG */
