/*===========================================================================
  Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
  All rights reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
===========================================================================*/

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <tmecom_interfaces.h>
#include <tmecom_os_al.h>
#include <tmecom_tfa.h>

static tmecom_client_t *client = NULL;

int tmecom_interface_init(tmecom_client_t **client_ptr)
{
	int ret = -ENODEV;
	bool     connected             = false;
	uint32_t connect_timeout_msec  = TMECOM_CONNECTION_TIMEOUT_MS;

	do {
		if (client_ptr == NULL) {
			ret = -EINVAL;
			break;
		}

		if (client == NULL) {
			tmecom_client_info_t client_info = {"tme-qmp"};
			ret = tmecom_register_client(&client_info, &client);
			if (ret != 0 || client == NULL) {
				*client_ptr = NULL;
				break;
			}
		} else {
			*client_ptr = client;
			ret = 0;
			break;
		}

		/* Check that the client is connected */
		while (!connected && connect_timeout_msec > 0U) {
			connected = tmecom_client_is_server_connected(client);
			if (!connected) {
				tmecom_sleep(1);
				connect_timeout_msec--;
			}
		}

		if (connected == false) {
			tmecom_unregister_client(client);
			client      = NULL;
			*client_ptr = NULL;
			ret         = -ETIMEDOUT;
		} else {
			*client_ptr = client;
			ret         = 0;
		}
	} while (0);
	return ret;
}

void tmecom_interface_deinit(void)
{
	if (client != NULL) {
		tmecom_unregister_client(client);
		client = NULL;
	}
}
