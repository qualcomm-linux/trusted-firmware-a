/*===========================================================================
  Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
  All rights reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
===========================================================================*/

#include <stddef.h>
#include <stdint.h>
#include <stringl/stringl.h>

#include "TmeInterfaces.h"
#include "TmeMessage.h"
#include "tmecom_interfaces.h"
#include "tmecom.h"
#include "IxErrno.h"

int TmeSHADigest(TMEHashAlgID_t        inHashAlgorithm,
                 const uint8_t        *inMsg,
                 size_t                inMsgLen,
                 uint8_t              *outDigest,
                 size_t               *outDigestLen,
                 TmeExtendedErrorInfo *errorInfo)
{
  int ret = E_FAILURE;

  do
  {
    if ((outDigest == NULL) || (outDigestLen == NULL) || (*outDigestLen == 0) ||
        (errorInfo == NULL))
    {
      ret = E_INVALID_ARG;
      break;
    }

    tmeShaRsp_t response    = {0};
    size_t      responseLen = sizeof(response);
    tmeShaReq_t request     = {
      .algorithm = (uint32_t)inHashAlgorithm,
      .data      = (TmeComAddr_t)(uintptr_t)inMsg,
      .dataSize  = (uint32_t)inMsgLen,
      .keyID     = (uint32_t)TME_HA_INVALID,
    };

    if (E_SUCCESS != TransceiveMessage(TME_MSG_CBOR_TAG_SHA_DIGEST,
                                       &request,
                                       sizeof(request),
                                       &response,
                                       sizeof(response),
                                       &responseLen))
    {
      break;
    }

    if (sizeof(response) != responseLen)
    {
      break;
    }

    if (E_SUCCESS != UpdatedExtendedErrorInfo(errorInfo, response.info))
    {
      break;
    }

    *outDigestLen = memscpy(outDigest, *outDigestLen,
                            response.output, response.outputLen);

    if (*outDigestLen != response.outputLen)
    {
      break;
    }

    ret = E_SUCCESS;
  } while (0);

  return ret;
}
