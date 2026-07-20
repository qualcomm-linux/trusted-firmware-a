/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 * Copyright (c) 2018-2020, The Linux Foundation. All rights reserved.
 * Copyright (c) 2026 Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <string.h>

#include <arch.h>
#include <arch_features.h>
#include <bl31/bl31.h>
#include <common/debug.h>
#include <common/desc_image_load.h>
#include <common/tbbr/tbbr_img_def.h>
#include <drivers/arm/dcc.h>
#include <drivers/console.h>
#include <drivers/generic_delay_timer.h>
#include <drivers/qti/timer/qti_timer.h>
#include <export/plat/qti/common/plat_params_exp.h>
#include <lib/bakery_lock.h>
#include <lib/bl_aux_params/bl_aux_params.h>
#include <lib/coreboot.h>
#include <lib/mmio.h>
#include <lib/spinlock.h>
#include <plat/common/platform.h>

#include <bl31qtilib_interface.h>
#include <bl31qtilib_spd_agnostic.h>
#include <platform.h>
#include <platform_def.h>
#include <qti_interrupt_svc.h>
#include <qti_plat.h>
#include <qti_ringbuf_console.h>
#include <qti_sbl_shared_info.h>
#include <qti_uart_console.h>
#include <sysini.h>
#include <arch_helpers.h>
#include <tfa_bl31_shared_imem.h>

#if ENABLE_FEAT_RME
#include <qti_gpt.h>
#endif

#ifdef QTI_PL011_UART
#include <drivers/arm/pl011.h>
#endif /* QTI_PL011_UART */

#ifdef PLAT_QTI_SMMUV3_ENABLED
#include <qti_smmu.h>
#endif

/* Ringbuf definition */
/* For platform with TZ imem */
#ifdef TFA_IMEM_BASE
console_ringbuf_t *g_qti_bl31_ringbuf_ptr =
					(console_ringbuf_t *)TFA_BL31_RING_BUFFER_IN_TZ_IMEM_BASE;
#else
/* For platform without TZ imem, place in DDRM */
console_ringbuf_t g_qti_bl31_ringbuf;
console_ringbuf_t *g_qti_bl31_ringbuf_ptr = &g_qti_bl31_ringbuf;
#endif /* TFA_IMEM_BASE */

/* Sysini related flags */
static int cpuss_sysini_done __section(".tzfw_coherent_mem");
static int cluster_sysini_done[PLAT_CLUSTER_COUNT]
	__section(".tzfw_coherent_mem");

/*
 * The macro ``DEFINE_BAKERY_LOCK`` allocates locks in section `bakery_lock`
 */
#if !HW_ASSISTED_COHERENCY
DEFINE_BAKERY_LOCK(cluster_sysini_lock[PLAT_CLUSTER_COUNT]);
#else
static spinlock_t cluster_sysini_lock[PLAT_CLUSTER_COUNT]
	__section(".tzfw_coherent_mem");
#endif

static boot_qsee_interface *sbl_qsee_interface;

#if defined(SPD_qteed) || defined(QTEE_BL31_SHARED_DATA)
static uintptr_t g_qsee_iface;
#endif /* SPD_qteed || QTEE_BL31_SHARED_DATA */

/*
 * Placeholder variables for copying the arguments that have been passed to
 * BL31 from BL2.
 */
static entry_point_info_t bl33_image_ep_info;

static entry_point_info_t bl32_image_ep_info;

#if ENABLE_RMM
/* RMM entry point info; populated from BL2 bl_params_t. */
static entry_point_info_t rmm_image_ep_info;
#endif /* ENABLE_RMM */

/*
 * Variable to hold counter frequency for the CPU's generic timer. In this
 * platform coreboot image configure counter frequency for boot core before
 * reaching TF-A.
 */
static uint64_t g_qti_cpu_cntfrq;

/* XBL-latched MTE enable flag (latched from bl_aux_params handler) */
static bool g_qti_mte_enabled;
#if !COREBOOT && !defined(QTI_PL011_UART) && defined(QTI_UART_CONSOLE)
static console_t g_qti_console_uart;
#endif
static console_t qti_console_ringbuf;

bool qti_is_mte_enabled(void)
{
	return g_qti_mte_enabled;
}

/*
 * bl_aux_params handler for QTI vendor-specific parameters.
 *
 * Called once per node in the bl_aux_param linked list passed in X1 by XBL.
 * Returns true if the node type was handled, false to skip (unknown types are
 * skipped gracefully by bl_aux_params_parse).
 */
static bool qti_aux_param_handler(struct bl_aux_param_header *param)
{
	INFO("%s: type=0x%lx\n", __func__, (unsigned long)param->type);

	switch ((enum bl_aux_qti_param_type)param->type) {
	case BL_AUX_PARAM_QTI_CPU_FEATURES: {
		const struct bl_aux_param_qti_cpu_features *p =
			(const struct bl_aux_param_qti_cpu_features *)param;

		INFO("CPU features param: features=0x%x cpu_mte2=%u\n",
		     p->cpu_features,
		     (unsigned int)is_feat_mte2_supported());

		/* Check MTE feature bit */
		if ((p->cpu_features & BL_AUX_CPU_FEATURE_MTE_ENABLED) &&
		    is_feat_mte2_supported()) {
			g_qti_mte_enabled = true;
			INFO("MTE ENABLED: Setting g_qti_mte_enabled=true\n");
		} else {
			INFO("MTE NOT enabled: features_bit=%u mte2_support=%u\n",
			     (unsigned int)((p->cpu_features &
					     BL_AUX_CPU_FEATURE_MTE_ENABLED) != 0),
			     (unsigned int)is_feat_mte2_supported());
		}

		INFO("Final: mte=%u\n", (unsigned int)g_qti_mte_enabled);

		/* Future: Add handling for other CPU features (SME, SVE, etc.) */
		return true;
	}
	default:
		INFO("Unknown param type, skipping\n");
		return false;
	}
}

/*
 * Platform implementation of plat_mte_enabled() declared in
 * include/lib/extensions/mte.h. Returns the XBL-latched MTE decision
 * that was parsed from the bl_aux_param list passed in X1 at
 * bl31_early_platform_setup().
 */
#if ENABLE_FEAT_MTE2_PLATFORM_CONTROL
bool plat_mte_enabled(void)
{
	return qti_is_mte_enabled();
}
#endif /* ENABLE_FEAT_MTE2_PLATFORM_CONTROL */

void qti_cpuss_poll_sysini_reset(void) /* NEEDSWORK */
{
	/* can be implemented in bl31qtilib */
}

void qti_el3_sys_regs_init(void) /* NEEDSWORK */
{
	/* can be implemented in bl31qtilib */
	/* Enable MPAM if it is supported */
	/*
	 * Enables the system register interface for interrupt management for
	 * El3 and El1 (gic v3)
	 */
	/* EL3 SRE Setting */
	write_icc_sre_el3(0x9U | read_icc_sre_el3());

	/* Set PMHE & IDbits to 24 bits */
	write_icc_ctlr_el3(0xCC40);

	/* EL1 SRE Setting */
	write_icc_sre_el1(0x1U | read_icc_sre_el1());

	/* PC DEBUG:: Setting ICC_IGRPEN0_EL1 to 1 */
	write_icc_igrpen0_el1(1);

	/* Clear SCTLR_EL2 */
}
/***************************************************************************
 * This function invokes cpuss and cluster sysini. It is expected
 * that sysini is executed before MMUs are enabled.
 **************************************************************************/
void plat_qti_cpu_boot_setup(void)
{
	unsigned int cluster_id;

	/*
	 * One-Time Synchronization of CPUCP and APSS required before CPUSS
	 * sysini
	 */
	qti_cpuss_poll_sysini_reset();
	cluster_id = find_cluster_id();

	/* Clear all values if present */
	memset(cluster_sysini_lock, 0, sizeof(cluster_sysini_lock));

	/* CPUSS sysini - execute only once */
	if (!cpuss_sysini_done) {
#ifndef ROLAS_VP
		cpuss_aarch64_por_sysini(1, (uintptr_t)NULL);
#endif
		cpuss_sysini_done = 1;
	}
#if !HW_ASSISTED_COHERENCY
	bakery_lock_get(&cluster_sysini_lock[cluster_id]);
#else
#ifndef DISABLE_SPINLOCK
	spin_lock(&cluster_sysini_lock[cluster_id]);
#endif /* DISABLE_SPINLOCK */
#endif

	/* Cluster sysini - execute once per cluster */
	/* Note: Hoya SOCs have single FCM cluster */
	if (cluster_sysini_done[cluster_id] == 0) {
#ifndef ROLAS_VP
		cluster_aarch64_sysini(
			((SYSINI_CLUSTER_POWER_UP
			  << SYSINI_CLUSTER_POWER_SHIFT) &
			 SYSINI_CLUSTER_POWER_MASK) |
				((cluster_id << SYSINI_CLUSTER_ID_SHIFT) &
				 SYSINI_CLUSTER_ID_MASK),
			(uintptr_t)NULL);
#endif
		/* (SYSINI_CLUSTER_POWER_UP,(uintptr_t)NULL); */
		cluster_sysini_done[cluster_id] = 1;
	}

#if !HW_ASSISTED_COHERENCY
	bakery_lock_release(&cluster_sysini_lock[cluster_id]);
#else
#ifndef DISABLE_SPINLOCK
	spin_unlock(&cluster_sysini_lock[cluster_id]);
#endif /* DISABLE_SPINLOCK */
#endif

	/* Initialize system registers that can only be done in EL3 */
	qti_el3_sys_regs_init();
}

#if ENABLE_RMM
/*
 * Scan bl_params_t from BL2 for RMM_IMAGE_ID and populate rmm_image_ep_info.
 * If not found, rmm_image_ep_info.pc stays 0 and RMMD handles it gracefully.
 */
static void qti_bl31_parse_rmm_ep_info(u_register_t from_bl2)
{
	const bl_params_t *params = (const bl_params_t *)from_bl2;
	const bl_params_node_t *node;

	if (params == NULL) {
		return;
	}

	for (node = params->head; node != NULL; node = node->next_params_info) {
		if (node->image_id == RMM_IMAGE_ID) {
			if (node->ep_info != NULL) {
				rmm_image_ep_info = *(node->ep_info);
				INFO("BL31: RMM ep_info found: pc=0x%lx\n",
				     (unsigned long)rmm_image_ep_info.pc);
			}
			break;
		}
	}
}
#endif /* ENABLE_RMM */

#ifdef QTEE_BL31_SHARED_DATA
static void *get_sbl_qsee_interface_from_bl2(u_register_t from_bl2)
{
	const bl_params_t *params = (const bl_params_t *)from_bl2;
	const bl_params_node_t *node;

	if (params == NULL) {
		return NULL;
	}

	assert(params->h.type == PARAM_BL_PARAMS);
	assert(params->h.version >= PARAM_VERSION_1);

	for (node = params->head; node != NULL; node = node->next_params_info) {
		if (node->image_id == SP_PKG1_ID) {
			if ((node->ep_info == NULL) ||
			    (node->ep_info->args.arg0 == 0)) {
				return NULL;
			}

			return (void *)(uintptr_t)node->ep_info->args.arg0;
		}
	}

	return NULL;
}
#endif /* QTEE_BL31_SHARED_DATA */

/*******************************************************************************
 * Perform any BL31 early platform setup common to ARM standard platforms.
 * Here is an opportunity to copy parameters passed by the calling EL (S-EL1
 * in BL2 & S-EL3 in BL1) before they are lost (potentially). This needs to be
 * done before the MMU is initialized so that the memory layout can be used
 * while creating page tables. BL2 has flushed this information to memory, so
 * we are guaranteed to pick up good data.
 ******************************************************************************/
void bl31_early_platform_setup(u_register_t from_bl2,
			       u_register_t plat_params_from_bl2)
{
	g_qti_cpu_cntfrq = PLAT_SYSCNT_FREQ;

	/*
	 * Dynamic CPU feature enablement handoff (XBL -> TF-A BL31)
	 *
	 * X1 (plat_params_from_bl2) points to the head of a bl_aux_param linked
	 * list built by XBL. qti_aux_param_handler() handles the
	 * BL_AUX_PARAM_QTI_CPU_FEATURES node and latches feature flags.
	 *
	 * g_qti_mte_enabled is initialised to false here; the handler sets it
	 * to true only when XBL requests MTE AND the CPU supports MTE2.
	 *
	 * The latched value is consumed in context_mgmt to set/clear SCR_EL3.ATA.
	 * The actual LCP/tag-region programming is done entirely in XBL before
	 * this point.
	 */
	g_qti_mte_enabled = false;
	bl_aux_params_parse(plat_params_from_bl2, qti_aux_param_handler);

#if COREBOOT
	if (coreboot_serial.baseaddr != 0) {
		static console_t g_qti_console_uart;

		qti_console_uart_register(&g_qti_console_uart,
					  coreboot_serial.baseaddr);
	}
#elif defined(QTI_PL011_UART)
	/* Use PL011 UART (standard ARM UART). */
	static console_t g_qti_console_uart;
	console_pl011_register(PL011_UART_BASE, PL011_UART_CLK_IN_HZ,
			       PL011_BAUDRATE, &g_qti_console_uart);
#elif defined(QTI_UART_CONSOLE)
	qti_console_uart_register(&g_qti_console_uart, UART_BASE_ADDR);
#endif

	/* Initialize the ringbuf object and buffer memory */
	qti_console_ringbuf_init(g_qti_bl31_ringbuf_ptr);

	qti_console_ringbuf_register(&qti_console_ringbuf, g_qti_bl31_ringbuf_ptr);
	qti_console_ringbuf.flags |= CONSOLE_FLAG_RUNTIME;

	/* Log AFTER console is ready so the line is not dropped */
	INFO("MTE latched=%u (xbl_param=0x%lx)\n",
	     (unsigned int)g_qti_mte_enabled,
	     (unsigned long)plat_params_from_bl2);

#ifndef ROLAS_VP
	/* Write the location of the ring buffer to shared imem */
	*(uint64_t *)TFA_BL31_SHARED_IMEM_RING_LOG_BASE =
					(uint64_t)(g_qti_bl31_ringbuf_ptr);
#endif

	/*
	 * Tell BL31 where the non-trusted software image
	 * is located and the entry state information
	 */
	bl31_params_parse_helper(from_bl2, &bl32_image_ep_info,
				 &bl33_image_ep_info);

#if defined(SPD_qteed)
	g_qsee_iface = (uintptr_t)bl32_image_ep_info.args.arg0;
#endif /* SPD_qteed */

#if ENABLE_RMM
	qti_bl31_parse_rmm_ep_info(from_bl2);
#endif /* ENABLE_RMM */

#if defined(QTEE_BL31_SHARED_DATA)
	g_qsee_iface = (uintptr_t)get_sbl_qsee_interface_from_bl2(from_bl2);
	/*
	 * Once the BL2 changes to pass sbl_qsee_interface via SP_PKG1_ID
	 * are completed and have landed in a meta this fallback will
	 * be removed. Tracked in CR: https://orbit/CR/4522407
	 */
	if (g_qsee_iface == 0) {
		g_qsee_iface = (uintptr_t)bl32_image_ep_info.args.arg0;
	}
	if (g_qsee_iface == 0) {
		INFO("Failed to find the boot qsee interface in bl2 params\n");
		panic();
	}
#endif /* QTEE_BL31_SHARED_DATA */

#if defined(SPD_qteed) || defined(QTEE_BL31_SHARED_DATA)
	if (g_qsee_iface != 0) {
#else
	if (bl32_image_ep_info.args.arg0 != 0) {
#endif
		sbl_qsee_interface = (boot_qsee_interface *)
			bl31qtilib_spd_share_object(
				BL31QTILIB_SPD_BOOT_QSEE_INTERFACE,
#if defined(SPD_qteed) || defined(QTEE_BL31_SHARED_DATA)
				(void *)g_qsee_iface,
#else
				(void *)bl32_image_ep_info.args.arg0,
#endif
				sizeof(boot_qsee_interface));
		if (sbl_qsee_interface != NULL) {
#if !defined(QTEE_BL31_SHARED_DATA)
			bl32_image_ep_info.args.arg0 =
				(uintptr_t)sbl_qsee_interface;
#endif
		}
	}
}

void bl31_early_platform_setup2(u_register_t arg0, u_register_t arg1,
				u_register_t arg2, u_register_t arg3)
{
	bl31_early_platform_setup(arg0, arg1);
}

/*******************************************************************************
 * Perform the very early platform specific architectural setup here. At the
 * moment this only initializes the mmu in a quick and dirty way.
 ******************************************************************************/
void bl31_plat_arch_setup(void)
{
	/*
	 * Any Api's that need atomic instruction need to be invoked post mmu
	 * enablement, as some platform has issue to use atomic
	 * instructions pre mmu enablement.
	 */
#if USE_COHERENT_MEM
	qti_setup_page_tables(BL31_START, BL31_END - BL31_START, BL_CODE_BASE,
			      BL_CODE_END, BL_RO_DATA_BASE, BL_RO_DATA_END,
			      BL_COHERENT_RAM_BASE, BL_COHERENT_RAM_END);
#else
	qti_setup_page_tables(BL31_START, BL31_END - BL31_START, BL_CODE_BASE,
			      BL_CODE_END, BL_RO_DATA_BASE, BL_RO_DATA_END);
#endif
	enable_mmu_el3(0);
#if ENABLE_FEAT_RME
	qti_gpt_setup();
#endif
}

/*******************************************************************************
 * Perform any BL31 platform setup common to ARM standard platforms
 ******************************************************************************/
#ifdef QTI_BL31_WITH_TEST
static const char qti_build_variant[] = "BL31 variant: bl31_with_test";
#else
static const char qti_build_variant[] = "BL31 variant: bl31";
#endif

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

#ifdef BOOT_IMEM_BASE
	/* Clean and Invalidate boot imem to ensure no dirty lines remain */
	flush_dcache_range(BOOT_IMEM_BASE, BOOT_IMEM_SIZE);
#endif

	/* Initialize QTI secure QTimer and generic delay timer */
	qti_timer_init();
	qti_delay_timer_init();

	/* Initialize the GIC driver, CPU and distributor interfaces */
	plat_qti_gic_driver_init();
	plat_qti_gic_init();
#ifndef ROLAS_VP
	/**
	 * Initialize the EL3 interrupt service and
	 * registers EL3 common interrupt handler
	 */
	qti_interrupt_svc_init();
#endif

#ifdef PLAT_QTI_SMMUV3_ENABLED
	/* Initialize SMMUv3 instances. */
	qti_smmu_init();
#endif

	bl31qtilib_bl31_platform_setup();
}

/*******************************************************************************
 * Return a pointer to the 'entry_point_info' structure of the next image for
 * the security state specified. BL33 corresponds to the non-secure image type
 * while BL32 corresponds to the secure image type. A NULL pointer is returned
 * if the image does not exist.
 ******************************************************************************/
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
#if ENABLE_RMM
	} else if (type == REALM) {
		if (rmm_image_ep_info.pc != 0UL) {
			assert((rmm_image_ep_info.h.type == PARAM_BL_PARAMS) ||
			       (rmm_image_ep_info.h.type == PARAM_EP));
			assert((rmm_image_ep_info.h.attr & EP_SECURITY_MASK) ==
			       REALM);
			return &rmm_image_ep_info;
		}
		return NULL;
#endif /* ENABLE_RMM */
	} else if (bl33_image_ep_info.pc != 0UL) {
		assert((bl33_image_ep_info.h.type == PARAM_BL_PARAMS) ||
		       (bl33_image_ep_info.h.type == PARAM_EP));
		assert((bl33_image_ep_info.h.attr & EP_SECURITY_MASK) ==
		       NON_SECURE);
		return &bl33_image_ep_info;
	}

	return NULL;
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

/*******************************************************************************
 * Returns a pointer to the sbl_qsee_interface structure.
 * This structure contains information about images
 * authenticated by boot loader and its entry point
 * The returned pointer should be treated as read-only.
 ******************************************************************************/
const boot_qsee_interface *get_sbl_qsee_interface(void)
{
	assert(sbl_qsee_interface != NULL);
	return sbl_qsee_interface;
}

/*******************************************************************************
 * Perform any platform specific runtime setup prior to cold boot exit
 * from BL31
 ******************************************************************************/
void bl31_plat_runtime_setup(void)
{
	INFO("start: platform specific runtime setup\n");
	bl31qtilib_bl31_platform_post_coldboot_setup();
	/* set boot state to cold boot complete. */
	bl31qtilib_set_cold_boot_done();
	INFO("end: platform specific runtime setup\n");
}

#ifdef SPD_spmd
/*
 * Platform handler for Group0 secure interrupt.
 */
int plat_spmd_handle_group0_interrupt(uint32_t intid)
{
	/* NEEDSWORK */
	(void)intid;
	return -1;
}
#endif /* SPD_spmd */
