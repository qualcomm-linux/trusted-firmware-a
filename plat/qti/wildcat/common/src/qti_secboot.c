/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <TmeInterfaces.h>
#include <common/debug.h>

#include <qti_secboot.h>

int qti_secboot_update_rollback_fuse_version(void)
{
#if defined(QTI_ARB_TEST)
        const fuseprov_transport_t *transport = fuseprov_port_tme_get();
        qti_arb_read_test(transport);
#endif /* QTI_ARB_TEST */
        int status = tme_update_rollback_version();
#if defined(QTI_ARB_TEST)
        qti_arb_read_test(transport);
#endif /* QTI_ARB_TEST */
        return status;
}

void qti_secboot_post_milestone_setup(void)
{
        int ret;

        NOTICE("AntiRollBack: starting ARB fuse version update\n");
        ret = qti_secboot_update_rollback_fuse_version();
        if (ret != E_SUCCESS) {
                ERROR("AntiRollBack: rollback fuse version update failed "
                      "(%d)\n", ret);
                return;
        }
        NOTICE("AntiRollBack: ARB fuse version update successful\n");
        NOTICE("AntiRollBack: ARB update complete\n");

        (void)qti_fuseprov_init();
}
