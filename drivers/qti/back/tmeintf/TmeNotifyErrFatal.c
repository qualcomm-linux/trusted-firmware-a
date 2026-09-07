/*===========================================================================
  Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
  All rights reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
===========================================================================*/

#include <stdint.h>
#include "IxErrno.h"

#include <common/debug.h>

#include "tzbsp_err_fatal.h"
#include "bl31qtilib_cb_interface.h"
#include "tzbsp_log.h"
#include "interrupts.h"

#include "TmeInterfaces.h"

#if defined(TMECOM_NOTIFY_ERROR_FATAL)
static void *tmecomNotifyErrFatalIsr(void *ctx)
{
  /* TME will notify that it has entered its error
   * fatal handler by raising an FIQ (TME interrupt)
   * to the APSS.
   *
   * bl31qtilib_cb_set_error_fatal() is then called to give time to
   * notify other subsystems that the SoC is about to
   * be restarted by TME due to the fatal error */
  TFA_LOG_ERR_FATAL(TZBSP_TME_ERROR_FATAL);
  bl31qtilib_cb_set_error_fatal(TZBSP_ERR_FATAL_TME_ERROR_FATAL);
  return ctx;
}

int tmecomRegisterErrFatalInterrupt(void)
{
  int ret = -E_FAILURE;

  INFO("%s: ENTRY\n", __func__);

  /* Register the TME notification interrupt */
  ret = int_register_isr(TZBSP_INT_TME_ERR, TZBSP_INT_TME_ERR_DESC,
                         tmecomNotifyErrFatalIsr, NULL,
                         TZBSP_INTF_TRIGGER_EDGE | TZBSP_INTF_ALL_CPUS,
                         TRUE);
  if (ret)
  {
    TFA_LOG_ERR(TZBSP_TME_INTERRUPT_REGISTRATION_FAIL, ret);
    INFO("%s: EXIT ret=%d (int_register_isr failed)\n", __func__,
         TZBSP_TME_INTERRUPT_REGISTRATION_FAIL);
    return TZBSP_TME_INTERRUPT_REGISTRATION_FAIL;
  }

  INFO("%s: EXIT ret=E_SUCCESS\n", __func__);
  return E_SUCCESS;
}
#else
int tmecomRegisterErrFatalInterrupt(void)
{
  INFO("%s: ENTRY (TMECOM_NOTIFY_ERROR_FATAL disabled)\n", __func__);
  INFO("%s: EXIT ret=E_SUCCESS\n", __func__);
  return E_SUCCESS;
}
#endif
