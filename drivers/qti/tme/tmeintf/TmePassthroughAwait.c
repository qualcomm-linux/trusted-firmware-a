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

int TmePassthroughAwait(uint32_t handle, void *rspBuf, size_t *rspBufSize)
{
  tmecomClient *clientPtr = NULL;
  int           ret       = E_INVALID_ARG;

  CHECK_BAIL(rspBuf);
  CHECK_BAIL(rspBufSize);
  CHECK_BAIL(*rspBufSize > 0U && *rspBufSize <= TMECOM_MAX_RESPONSE_SIZE);

  ret = tmecomInterfaceInit(&clientPtr);
  if (ret != E_SUCCESS || clientPtr == NULL)
  {
    return E_FAILURE;
  }

  ret = tmecomClientRecvMessageAsync(clientPtr, handle, rspBuf, rspBufSize);
  if (ret == 0)
  {
    return E_SUCCESS;
  }
  else if (ret == -E_IN_PROGRESS)
  {
    return E_IN_PROGRESS ;
  }

  /* Covers -EINVAL (invalid/stale handle). */
  return E_FAILURE;

bail:
  return ret;
}
