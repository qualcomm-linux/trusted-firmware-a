/*
 * Copyright (c) 2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * QTI platform-specific BL auxiliary parameters (export header).
 *
 * This header defines vendor-specific bl_aux_params types/structures that can
 * be passed from the bootloader (XBL) to TF-A BL31 via X1, and parsed in BL31
 * using:
 *   bl_aux_params_parse(plat_params_from_bl2, handler)
 */

#ifndef PLAT_QTI_COMMON_PLAT_PARAMS_EXP_H
#define PLAT_QTI_COMMON_PLAT_PARAMS_EXP_H

/* EXPORT HEADER -- See include/export/README for details! -- EXPORT HEADER */

#include <stdint.h>
#include "../../../lib/bl_aux_params/bl_aux_params_exp.h"

/* QTI vendor-specific bl_aux param types */
enum bl_aux_qti_param_type {
	BL_AUX_PARAM_QTI_CPU_FEATURES = BL_AUX_PARAM_VENDOR_SPECIFIC_FIRST,
	/* Add future QTI-specific params here */
};

/* CPU feature flags for BL_AUX_PARAM_QTI_CPU_FEATURES */
#define BL_AUX_CPU_FEATURE_MTE_ENABLED  (1U << 0)
/* Bits 1-31 reserved for future CPU features (SME, SVE, MPAM, etc.) */

/* Payload for BL_AUX_PARAM_QTI_CPU_FEATURES */
struct bl_aux_param_qti_cpu_features {
	struct bl_aux_param_header h; /* h.type=..., h.next=... */
	uint32_t cpu_features;        /* Bit 0: MTE; Bits 1-31: reserved */
	uint8_t  reserved[4];         /* pad to 8-byte alignment */
};

#endif /* PLAT_QTI_COMMON_PLAT_PARAMS_EXP_H */
