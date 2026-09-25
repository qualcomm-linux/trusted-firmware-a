/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef QTI_SECBOOT_H
#define QTI_SECBOOT_H

#include <drivers/qti/fuseprov/fuseprov_port_tme.h>

/* Provision the authenticated SEC.DAT buffer through TME. */
int qti_fuseprov_init(void);

#if defined(QTI_ARB_TEST)
/* Read the anti-rollback rows through the supplied transport. */
void qti_arb_read_test(const fuseprov_transport_t *transport);
#endif /* QTI_ARB_TEST */

/* Update the boot-image anti-rollback fuse version through TME. */
int qti_secboot_update_rollback_fuse_version(void);

/* Update anti-rollback state before starting SEC.DAT provisioning. */
void qti_secboot_post_milestone_setup(void);

#endif /* QTI_SECBOOT_H */
