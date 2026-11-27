/*
 * Copyright (c) 2026, Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SYSDBG_TARGET_H
#define SYSDBG_TARGET_H

#include <stdbool.h>

/*
 * Chipset contract. The common SDI code (sysdbg_main.c, sysdbg_log.c, sysdbg_dcc*.c)
 * is chipset independent. Each chipset directory (drivers/qti/sysdbg/<CHIPSET>/)
 * provides:
 *
 *   sysdbg_get_config()        SDI configuration (reset debug enables, timeouts)
 *   sysdbg_target_info_update()   cold boot: read chip info
 *   sysdbg_target_enable()       cold boot: arm the reset debug feature
 *   sysdbg_target_reset_soc()     crash path: end of pass 1, trigger the reset
 *   sysdbg_wait_for_reset()       crash path: secondary core wait loop
 *   sysdbg_save_shared_data()  crash path: take the data handed over by the entry code
 *   sysdbg_get_shared_imem_base()  base of the shared IMEM used for the SDI cookies
 *   sysdbg_reset_check()       early reset path: is this a debug reset
 *   sysdbg_entry_handler()     early reset path: SDI crash entry
 *
 * and a sysdbg_target_config.h with the chipset constants (CPU count, clocks,
 * shared IMEM base) and a sysdbg_chip.mk listing its sources and option defaults.
 */
bool sysdbg_target_info_update(void);
bool sysdbg_target_enable(void);

#endif /* SYSDBG_TARGET_H */
