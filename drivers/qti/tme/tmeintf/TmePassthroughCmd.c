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

int TmePassthroughCmd(void *reqBuf, size_t reqSize, uint32_t *handle)
{
  tmecomClient *clientPtr = NULL;
  int           ret       = E_INVALID_ARG;

  CHECK_BAIL(reqBuf);
  CHECK_BAIL(reqSize > 0U && reqSize <= TMECOM_MAX_REQUEST_SIZE);
  CHECK_BAIL(handle);

  ret = tmecomInterfaceInit(&clientPtr);
  if (ret != E_SUCCESS || clientPtr == NULL)
  {
    return E_FAILURE;
  }

  ret = tmecomClientSendMessageAsync(clientPtr,
                                     (void *)reqBuf,
                                     reqSize,
                                     handle);
  if (ret == 0)
  {
    return E_SUCCESS;
  }
  else if (ret == -E_AGAIN)
  {
    return E_AGAIN;
  }

  return E_FAILURE;

bail:
  return ret;
}
