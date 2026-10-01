/*===========================================================================
	Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
	All rights reserved.
	Confidential and Proprietary - Qualcomm Technologies, Inc.
===========================================================================*/

#include <stdint.h>
#include "IxErrno.h"

#include "tzbsp_err_fatal.h"
#include "bl31qtilib_cb_interface.h"
#include "tzbsp_log.h"
#include "interrupts.h"

#include "TmeInterfaces.h"

#if defined(TMECOM_NOTIFY_ERROR_FATAL)
static void *tmecom_notify_err_fatal_isr(void *ctx)
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

int tmecom_register_err_fatal_interrupt(void)
{
	int ret = -E_FAILURE;

	/* Register the TME notification interrupt */
	ret = int_register_isr(TZBSP_INT_TME_ERR, TZBSP_INT_TME_ERR_DESC,
		 tmecom_notify_err_fatal_isr, NULL,
		 TZBSP_INTF_TRIGGER_EDGE | TZBSP_INTF_ALL_CPUS,
		 TRUE);
	if (ret) {
		TFA_LOG_ERR(TZBSP_TME_INTERRUPT_REGISTRATION_FAIL, ret);
		return TZBSP_TME_INTERRUPT_REGISTRATION_FAIL;
	}

	return E_SUCCESS;
}
#else
int tmecom_register_err_fatal_interrupt(void)
{
	return E_SUCCESS;
}
#endif
