/*===========================================================================
  Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
  All rights reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
===========================================================================*/

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <common/debug.h>

#include "tmecom_os_al.h"
#include "tmecomTFA.h"
#include "tmecom_interfaces.h"

static tmecomClient *client = NULL;

int tmecomInterfaceInit(tmecomClient **clientPtr)
{
  int ret = -ENODEV;

  INFO("%s: ENTRY\n", __func__);

  if (NULL == client)
  {
    /*
     * Register client with tmecom.  The name must match a channel registered
     * by the platform mailbox table (plat/qti/.../src/qcom_mbox_plat.c); on
     * upstream wildcat targets that is "tme-qmp".  The downstream name for
     * this channel is "tme_tz".
     */
    tmecomClientInfo clientInfo = {"tme-qmp"};
    CHECK_BAIL(0 == tmecomRegisterClient(&clientInfo, &client));
  }
  else
  {
    *clientPtr = client;
    INFO("%s: EXIT ret=0 (already initialized)\n", __func__);
    return E_SUCCESS;
  }

  /* Register the TME->TZ interrupt used to notify TZ of a fatal error
   * originating in the TME, which will result in an SoC restart. */
  ret = tmecomRegisterErrFatalInterrupt();
  if (ret)
  {
    INFO("%s: EXIT ret=-E_FAILURE (tmecomRegisterErrFatalInterrupt failed)\n",
         __func__);
    return -E_FAILURE;
  }

  /* Check that the client is connected */
  bool     connected          = false;
  uint32_t connectTimeoutMSec = TMECOM_CONNECTION_TIMEOUT_MS;

  while (!connected)
  {
    connected = tmecomClientIsServerConnected(client);
    tmecomSleep(1); /* 1 ms */
    connectTimeoutMSec--;
    CHECK_BAIL(connectTimeoutMSec);
  }

  if (!connected)
  {
    *clientPtr = NULL;
    ret        = -E_TIMER_EXP;
  }
  else
  {
    *clientPtr = client;
    ret        = E_SUCCESS;
  }

bail:
  INFO("%s: EXIT ret=%d\n", __func__, ret);
  return ret;
}

void tmecomInterfaceDeInit(void)
{
  client = NULL;
}
