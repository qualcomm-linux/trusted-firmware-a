/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SYSDBG_CRUMBS_H
#define SYSDBG_CRUMBS_H

/* Offsets within the shared-IMEM region (SYSDBG_SHARED_IMEM_BASE, nord/sysdbg_target_config.h). */
#define SYSDBG_DUMPTABLE_OFFSET		(0x010)
#define SYSDBG_LOGBUFPTR_OFFSET		(0xAF8)
#define SYSDBG_COLDBOOTST_OFFSET	(0xB14)
#define SYSDBG_PASS1COMPL_OFFSET	(SYSDBG_COLDBOOTST_OFFSET)

#define SYSDBG_PASS1_SUCCESS_COOKIE	0xDEADD00D
#define SYSTEM_DEBUG_IMAGE_ID		0x000005D1

#endif /* SYSDBG_CRUMBS_H */
