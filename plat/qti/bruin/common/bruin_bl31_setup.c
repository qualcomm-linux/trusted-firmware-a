/*
 * Copyright (c) 2026 Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <qti_plat.h>

void plat_qti_bl31_setup_pre(void)
{
	plat_cpuss_early_config();
}

void plat_qti_bl31_setup_post(void)
{
	plat_cpuss_late_config();
}
