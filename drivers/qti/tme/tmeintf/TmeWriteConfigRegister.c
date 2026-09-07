/*===========================================================================
  Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
  All rights reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
===========================================================================*/

/*
 * QFPROM configuration register write over TMECOM.
 *
 * Ported from the downstream client (ssg/tme/tmeintf/src/TmeWriteConfigRegister.c).
 * The downstream version also logs the failure via TZBSP_LOG_ERR; that macro
 * is not available in the TF-A build, so only the TMECOM message path is
 * retained here.
 */

#include <stddef.h>
#include <stdint.h>

#include "IxErrno.h"
#include "TmeInterfaces.h"
#include "TmeInterfacesDefs.h"
#include "TmeMessage.h"
#include "TmeMessagesTags.h"
#include "tmecom_interfaces.h"

int TmeWriteConfigRegister(tmeConfigRegisterId_e registerId, uint32_t value)
{
  int                         ret = E_FAILURE;
  tmeWriteConfigRegisterReq_t req = {
    .id    = (uint8_t)registerId,
    .value = value,
  };
  tmeWriteConfigRegisterRsp_t rsp         = {0};
  size_t                      responseLen = sizeof(rsp);

  CHECK_BAIL(E_SUCCESS == TransceiveMessage(TME_MSG_CBOR_TAG_WRITE_CONFIG_REGISTER,
                                            &req,
                                            sizeof(req),
                                            &rsp,
                                            sizeof(rsp),
                                            &responseLen));

  CHECK_BAIL(responseLen == sizeof(rsp));

  ret = (0 == rsp.status) ? E_SUCCESS : E_FAILURE;

bail:
  return ret;
}
