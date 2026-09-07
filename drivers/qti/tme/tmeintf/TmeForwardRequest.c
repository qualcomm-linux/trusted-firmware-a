/*===========================================================================
  Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
  All rights reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
===========================================================================*/

#include <stddef.h>
#include <stdint.h>

#include "TmeInterfaces.h"
#include "tmecom_interfaces.h"
#include "tmecom.h"
#include "tmecomTFA.h"
#include "IxErrno.h"

int TmeForwardRequest(void *reqBuf,
                      size_t      reqSize,
                      void       *rspBuf,
                      size_t     *rspBufSize)
{
  tmecomClient *clientPtr = NULL;
  int           ret       = E_INVALID_ARG;

  CHECK_BAIL(reqBuf);
  CHECK_BAIL(reqSize > 0U && reqSize <= TMECOM_MAX_REQUEST_SIZE);
  CHECK_BAIL(rspBuf);
  CHECK_BAIL(*rspBufSize > 0U && *rspBufSize <= TMECOM_MAX_RESPONSE_SIZE);

  ret = tmecomInterfaceInit(&clientPtr);
  if (ret != E_SUCCESS || clientPtr == NULL)
  {
    return E_FAILURE;
  }

  ret = tmecomClientSendMessageSync(clientPtr,
                                    (void *)reqBuf,
                                    reqSize,
                                    rspBuf,
                                    rspBufSize,
                                    TMECOM_RESPONSE_TIMEOUT_MS);
  return (ret == 0) ? E_SUCCESS : E_FAILURE;

bail:
  return ret;
}