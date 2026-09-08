/*
 * Copyright (c) 2026 Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <stdint.h>

#include <drivers/qti/pdc/pdc_internal.h>

/*============================================================================
 *                              GLOBAL VARIABLES
 *===========================================================================*/
/* Fixed interrupts in the PDC */
struct pdc_interrupt_mapping g_pdc_interrupt_mapping[] =
{
	/* Bit 0 */
	{{ TRIGGER_RISING_EDGE, PDC_DRV2 }, 777}, /* spmi_protocol_irq[1] */
	{{ TRIGGER_RISING_EDGE, PDC_DRV2 }, 778}, /* ee0_apps_hlos_spmi_periph_irq_0 */
	{{ TRIGGER_RISING_EDGE, PDC_DRV2 }, 779}, /* ee1_apps_trustzone_spmi_periph_irq_0 */
	{{ TRIGGER_RISING_EDGE, PDC_DRV2 }, 780}, /* spare_bit_3 */

	/* Bit 4 */
	{{ TRIGGER_RISING_EDGE, PDC_DRV2 }, 781}, /* spare_bit_4 */
	{{ TRIGGER_RISING_EDGE, PDC_DRV0 }, 782}, /* aop_wdog_expired_irq */
	{{ TRIGGER_RISING_EDGE, PDC_DRV0 }, 782}, /* aop_wdog_expired_irq */
	{{ TRIGGER_RISING_EDGE, PDC_DRV2 }, 783}, /* o_sailss_to_md_irq[9] */

	/* Bit 8 */
	{{ TRIGGER_RISING_EDGE, PDC_DRV2 }, 784}, /* usb2_dmse */
	{{ TRIGGER_RISING_EDGE, PDC_DRV2 }, 785}, /* usb2_dpse */
	{{ TRIGGER_RISING_EDGE, PDC_DRV2 }, 786}, /* not-connected */
	{{ TRIGGER_RISING_EDGE, PDC_DRV2 }, 787}, /* not-connected */

	/* Bit 12 */
	{{ TRIGGER_RISING_EDGE, PDC_DRV2 }, 788}, /* u_eud_eud_int_mx[1] */
	{{ TRIGGER_RISING_EDGE, PDC_DRV2 }, 789}, /* u_eud_eud_p0_dmse_int_mx */
	{{ TRIGGER_RISING_EDGE, PDC_DRV2 }, 790}, /* u_eud_eud_p0_dpse_int_mx */
	{{ TRIGGER_RISING_EDGE, PDC_DRV2 }, 791}, /* u_eud_eud_p0_dpse_int_mx */

	/* Bit 16 */
	{{ TRIGGER_RISING_EDGE, PDC_DRV2 }, 792}, /* u_eud_eud_p0_dmse_int_mx */
	{{ TRIGGER_RISING_EDGE, PDC_DRV2 }, 793}, /* not-connected */
	{{ TRIGGER_RISING_EDGE, PDC_DRV2 }, 794}, /* u_cm_usb3_dp_con_qmp_usb3_lfps_rxterm_irq */
	{{ TRIGGER_RISING_EDGE, PDC_DRV2 }, 795}, /* aoss_all_xpu_malicious_summ_intr */

	/* Bit 20 */
	{{ TRIGGER_RISING_EDGE, PDC_DRV2 }, 796}, /* rpmh_wake_combined */
	{{ TRIGGER_RISING_EDGE, PDC_DRV2 }, 797}, /* tsens_0C_summ_int */
	{{ TRIGGER_RISING_EDGE, PDC_DRV2 }, 798}, /* spare_bit_21 */
	{{ TRIGGER_RISING_EDGE, PDC_DRV2 }, 799}, /* pdc_apps_epcb_timeout_summary_irq */

	/* Bit 24 */
	{{ TRIGGER_RISING_EDGE, PDC_DRV2 }, 800}, /* spmi_protocol_irq[0] */
	{{ TRIGGER_RISING_EDGE, PDC_DRV2 }, 801}, /* not-connected */
	{{ TRIGGER_RISING_EDGE, PDC_DRV2 }, 802}, /* not-connected */
	{{ TRIGGER_RISING_EDGE, PDC_DRV2 }, 803}, /* tsense0_upper_lower_intr */

	/* Bit 28 */
	{{ TRIGGER_RISING_EDGE, PDC_DRV2 }, 804}, /* tsense1_upper_lower_intr */
	{{ TRIGGER_RISING_EDGE, PDC_DRV2 }, 805}, /* tsense2_upper_lower_intr */
	{{ TRIGGER_RISING_EDGE, PDC_DRV2 }, 806}, /* tsense3_upper_lower_intr */
	{{ TRIGGER_RISING_EDGE, PDC_DRV2 }, 807}, /* tsense4_upper_lower_intr */

	/* Bit 32 */
	{{ TRIGGER_RISING_EDGE, PDC_DRV2 }, 808}, /* tsense5_upper_lower_intr */
	{{ TRIGGER_RISING_EDGE, PDC_DRV2 }, 809}, /* tsense6_upper_lower_intr */
	{{ TRIGGER_RISING_EDGE, PDC_DRV2 }, 810}, /* tsense7_upper_lower_intr */
	{{ TRIGGER_RISING_EDGE, PDC_DRV2 }, 811}, /* not-connected */

	/* Bit 36 */
	{{ TRIGGER_RISING_EDGE, PDC_DRV2 }, 812}, /* not-connected */
	{{ TRIGGER_RISING_EDGE, PDC_DRV2 }, 813}, /* not-connected */
	{{ TRIGGER_RISING_EDGE, PDC_DRV2 }, 814}, /* not-connected */
	{{ TRIGGER_RISING_EDGE, PDC_DRV2 }, 815}, /* not-connected */

	/* Bit 40 */
	{{ TRIGGER_RISING_EDGE, PDC_DRV2 }, 816}, /* not-connected */
	{{ TRIGGER_RISING_EDGE, PDC_DRV2 }, 817}, /* not-connected */
	{{ TRIGGER_RISING_EDGE, PDC_DRV2 }, 818}, /* u_cm_usb3_dp_con_qmp_usb3_lfps_rxterm_irq */
	{{ TRIGGER_RISING_EDGE, PDC_DRV2 }, 819}, /* not-connected */

	/* Bit 44 */
	{{ TRIGGER_RISING_EDGE, PDC_DRV2 }, 582}, /* gp_irq_hvm[7] */
	{{ TRIGGER_RISING_EDGE, PDC_DRV2 }, 583}, /* gp_irq_hvm[8] */
	{{ TRIGGER_RISING_EDGE, PDC_DRV2 }, 584}, /* gp_irq_hvm[9] */
	{{ TRIGGER_RISING_EDGE, PDC_DRV2 }, 594}, /* gp_irq_hvm[19] */

	/* Bit 48 */
	{{ TRIGGER_RISING_EDGE, PDC_DRV2 }, 600}, /* gp_irq_hvm[25] */
	{{ TRIGGER_RISING_EDGE, PDC_DRV2 }, 601}, /* gp_irq_hvm[26] */
	{{ TRIGGER_RISING_EDGE, PDC_DRV2 }, 602}, /* gp_irq_hvm[27] */
	{{ TRIGGER_RISING_EDGE, PDC_DRV2 }, 603}, /* gp_irq_hvm[28] */

	/* Bit 52 */
	{{ TRIGGER_RISING_EDGE, PDC_DRV2 }, 604}, /* gp_irq_hvm[29] */
	{{ TRIGGER_RISING_EDGE, PDC_DRV2 }, 605}, /* gp_irq_hvm[30] */
	{{ TRIGGER_RISING_EDGE, PDC_DRV2 }, 645}, /* gp_irq_hvm[35] */
	{{ TRIGGER_RISING_EDGE, PDC_DRV2 }, 646}, /* gp_irq_hvm[36] */

	/* Bit 56 */
	{{ TRIGGER_RISING_EDGE, PDC_DRV2 }, 647}, /* gp_irq_hvm[37] */
	{{ TRIGGER_RISING_EDGE, PDC_DRV2 }, 648}, /* gp_irq_hvm[38] */
	{{ TRIGGER_RISING_EDGE, PDC_DRV2 }, 649}, /* gp_irq_hvm[39] */
	{{ TRIGGER_RISING_EDGE, PDC_DRV2 }, 655}, /* gp_irq_hvm[45] */

	/* Bit 60 */
	{{ TRIGGER_RISING_EDGE, PDC_DRV2 }, 656}, /* gp_irq_hvm[46] */
	{{ TRIGGER_RISING_EDGE, PDC_DRV2 }, 657}, /* gp_irq_hvm[47] */
	{{ TRIGGER_RISING_EDGE, PDC_DRV2 }, 658}, /* gp_irq_hvm[48] */
	{{ TRIGGER_RISING_EDGE, PDC_DRV2 }, 659}, /* gp_irq_hvm[49] */

	/* Bit 64 */
	{{ TRIGGER_RISING_EDGE, PDC_DRV2 }, 664}, /* gp_irq_hvm[54] */
	{{ TRIGGER_RISING_EDGE, PDC_DRV2 }, 665}, /* gp_irq_hvm[55] */
	{{ TRIGGER_RISING_EDGE, PDC_DRV2 }, 666}, /* gp_irq_hvm[56] */
	{{ TRIGGER_RISING_EDGE, PDC_DRV2 }, 667}, /* gp_irq_hvm[57] */

	/* Bit 68 */
	{{ TRIGGER_RISING_EDGE, PDC_DRV2 }, 668}, /* gp_irq_hvm[58] */
	{{ TRIGGER_RISING_EDGE, PDC_DRV2 }, 669}, /* gp_irq_hvm[59] */
	{{ TRIGGER_RISING_EDGE, PDC_DRV2 }, 670}, /* gp_irq_hvm[60] */
	{{ TRIGGER_RISING_EDGE, PDC_DRV2 }, 671}, /* gp_irq_hvm[61] */

	/* Bit 72 */
	{{ TRIGGER_RISING_EDGE, PDC_DRV2 }, 672}, /* gp_irq_hvm[62] */
	{{ TRIGGER_RISING_EDGE, PDC_DRV2 }, 749}, /* gp_irq_hvm[63] */
	{{ TRIGGER_RISING_EDGE, PDC_DRV2 }, 750}, /* gp_irq_hvm[64] */
	{{ TRIGGER_RISING_EDGE, PDC_DRV2 }, 751}, /* gp_irq_hvm[65] */

	/* Bit 76 */
	{{ TRIGGER_RISING_EDGE, PDC_DRV2 }, 752}, /* gp_irq_hvm[66] */
	{{ TRIGGER_RISING_EDGE, PDC_DRV2 }, 753}, /* gp_irq_hvm[67] */
	{{ TRIGGER_RISING_EDGE, PDC_DRV2 }, 754}, /* gp_irq_hvm[68] */
	{{ TRIGGER_RISING_EDGE, PDC_DRV2 }, 755}, /* gp_irq_hvm[69] */

	/* Bit 80 */
	{{ TRIGGER_RISING_EDGE, PDC_DRV2 }, 756}, /* gp_irq_hvm[70] */
	{{ TRIGGER_RISING_EDGE, PDC_DRV2 }, 757}, /* gp_irq_hvm[71] */
	{{ TRIGGER_RISING_EDGE, PDC_DRV2 }, 758}, /* gp_irq_hvm[72] */
	{{ TRIGGER_RISING_EDGE, PDC_DRV2 }, 759}, /* gp_irq_hvm[73] */

	/* Bit 84 */
	{{ TRIGGER_RISING_EDGE, PDC_DRV2 }, 760}, /* gp_irq_hvm[74] */
	{{ TRIGGER_RISING_EDGE, PDC_DRV2 }, 283}, /* gp_irq_hvm[75] */
	{{ TRIGGER_RISING_EDGE, PDC_DRV2 }, 284}, /* gp_irq_hvm[76] */
	{{ TRIGGER_RISING_EDGE, PDC_DRV2 }, 285}, /* gp_irq_hvm[77] */

	/* Bit 88 */
	{{ TRIGGER_RISING_EDGE, PDC_DRV2 }, 286}, /* gp_irq_hvm[78] */
	{{ TRIGGER_RISING_EDGE, PDC_DRV2 }, 287}, /* gp_irq_hvm[79] */
	{{ TRIGGER_RISING_EDGE, PDC_DRV2 }, 828}, /* gp_irq_hvm[80] */
	{{ TRIGGER_RISING_EDGE, PDC_DRV2 }, 829}, /* gp_irq_hvm[81] */

	/* Bit 92 */
	{{ TRIGGER_RISING_EDGE, PDC_DRV2 }, 830}, /* gp_irq_hvm[82] */
	{{ TRIGGER_RISING_EDGE, PDC_DRV2 }, 831}, /* gp_irq_hvm[83] */
	{{ TRIGGER_RISING_EDGE, PDC_DRV2 }, 832}, /* gp_irq_hvm[84] */
	{{ TRIGGER_RISING_EDGE, PDC_DRV2 }, 833}, /* gp_irq_hvm[85] */

	/* Bit 96 */
	{{ TRIGGER_RISING_EDGE, PDC_DRV2 }, 834}, /* gp_irq_hvm[86] */
	{{ TRIGGER_RISING_EDGE, PDC_DRV2 }, 835}, /* gp_irq_hvm[87] */
	{{ TRIGGER_RISING_EDGE, PDC_DRV2 }, 836}, /* gp_irq_hvm[88] */
	{{ TRIGGER_RISING_EDGE, PDC_DRV2 }, 837}, /* gp_irq_hvm[89] */

	/* Bit 100 */
	{{ TRIGGER_RISING_EDGE, PDC_DRV2 }, 838}, /* gp_irq_hvm[90] */
	{{ TRIGGER_RISING_EDGE, PDC_DRV2 }, 839}, /* gp_irq_hvm[91] */
	{{ TRIGGER_RISING_EDGE, PDC_DRV2 }, 840}, /* gp_irq_hvm[92] */
	{{ TRIGGER_RISING_EDGE, PDC_DRV2 }, 841}, /* gp_irq_hvm[93] */

	/* Bit 104 */
	{{ TRIGGER_RISING_EDGE, PDC_DRV2 }, 842}, /* gp_irq_hvm[94] */
	{{ TRIGGER_RISING_EDGE, PDC_DRV2 }, 843}, /* gp_irq_hvm[95] */
};

/* Size of the table */
const uint32_t g_pdc_interrupt_table_size = sizeof(g_pdc_interrupt_mapping) / sizeof(g_pdc_interrupt_mapping[0]);
