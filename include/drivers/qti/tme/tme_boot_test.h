/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef QTI_TME_BOOT_TEST_H
#define QTI_TME_BOOT_TEST_H

/*
 * Boot-time TMECOM self-tests (drivers/qti/tme/tme_boot_test.c).  The bodies
 * only exist when QTI_USE_TMECOM and QTI_TMECOM_TEST are both defined -
 * keep call sites behind the same two guards, since these declarations are
 * visible unconditionally but the definitions are not.
 */

void tmecom_boot_sha_test(void);
void tmecom_boot_loopback_test(void);
void tmecom_boot_fuse_read_test(void);
void tmecom_boot_fuse_write_multiple_test(void);
void tmecom_boot_write_config_register_test(void);

#endif /* QTI_TME_BOOT_TEST_H */
