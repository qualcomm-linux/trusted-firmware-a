/*
 * Copyright (c) 2018-2021, The Linux Foundation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef QTI_CPU_H
#define QTI_CPU_H

/* KRYO-4xx Gold MIDR */
#define QTI_KRYO4_GOLD_MIDR	0x517F804D

/* KRYO-4xx Silver MIDR */
#define QTI_KRYO4_SILVER_MIDR	0x517F805D

/* KRYO-6xx Gold MIDR */
#define QTI_KRYO6_GOLD_MIDR	0x412FD410

/* KRYO-6xx Silver MIDR */
#define QTI_KRYO6_SILVER_MIDR	0x412FD050

/*
 * plat_qti_cpu_boot_cluster_reset - NCC boot-core CL4 sleep-state workaround.
 *
 * Implemented in plat/qti/cpu/ncc/src/bl31_cpu_setup.c.
 * Called once from bl31_early_platform_setup() on the boot core only.
 */

void plat_qti_cpu_boot_cluster_reset(void);
#endif /* QTI_CPU_H */
