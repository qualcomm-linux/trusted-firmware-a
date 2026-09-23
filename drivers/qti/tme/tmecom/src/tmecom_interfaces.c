/*===========================================================================
	Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
	All rights reserved.
	Confidential and Proprietary - Qualcomm Technologies, Inc.
===========================================================================*/

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "tmecom_os_al.h"
#include "tmecomTFA.h"
#include "tmecom_interfaces.h"

static tmecomClient *client = NULL;

int tmecom_interface_init(tmecomClient **client_ptr)
{
	int ret = -ENODEV;

	if (client_ptr == NULL) {
		return -EINVAL;
	}

	*client_ptr = NULL;

	if (NULL == client) {
		/*
		 * Register client with tmecom.  The name must match a channel registered
		 * by the platform mailbox table (plat/qti/.../src/qcom_mbox_plat.c) -
		 * "tme-qmp" on this target.
		 */
		tmecomClientInfo client_info = {"tme-qmp"};
		CHECK_BAIL(0 == tmecom_register_client(&client_info, &client));
	}
	else {
		*client_ptr = client;
		return E_SUCCESS;
	}

	/* Register the TME->TZ interrupt used to notify TZ of a fatal error
	 * originating in the TME, which will result in an SoC restart. */
	ret = tmecom_register_err_fatal_interrupt();
	if (ret) {
		return -E_FAILURE;
	}

	/* Check that the client is connected */
	bool     connected          = false;
	uint32_t connect_timeout_msec = TMECOM_CONNECTION_TIMEOUT_MS;

	while ((!connected) && (connect_timeout_msec != 0U)) {
		connected = tmecom_client_is_server_connected(client);
		tmecom_sleep(1); /* 1 ms */
		connect_timeout_msec--;
	}

	if (!connected) {
		ret        = -E_TIMER_EXP;
	}
	else {
		*client_ptr = client;
		ret        = E_SUCCESS;
	}

bail:
	return ret;
}

void tmecom_interface_deinit(void)
{
	client = NULL;
}
