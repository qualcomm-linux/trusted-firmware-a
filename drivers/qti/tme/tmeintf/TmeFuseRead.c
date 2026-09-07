/*===========================================================================
  Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
  All rights reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
===========================================================================*/

/*
 * QFPROM fuse read over TMECOM.
 *
 * Ported from the downstream client (ssg/tme/tmeintf/src/TmeFuseRead.c).  The
 * downstream version also pulls in DALSys/qsee_fuse/qsee_heap; none of those
 * are available in - or needed by - the TF-A build, so only the TMECOM message
 * path is retained here.
 *
 * The write side lives in TmeFuseWriteMultiple.c - read its header before
 * touching it, fuses are one-time-programmable.
 */

#include <stddef.h>
#include <stdint.h>
#include <stringl/stringl.h>

#include "IxErrno.h"
#include "TmeInterfaces.h"
#include "TmeInterfacesDefs.h"
#include "TmeMessage.h"
#include "TmeMessagesTags.h"
#include "tmecom_interfaces.h"

int TmeFuseRead(TmeQfpromAddrSpace_t addrType,
                uint32_t             fuseAddr,
                uint32_t *const      fuseData,
                uint32_t *const      qfpromApiStatus)
{
  int              ret         = E_INVALID_ARG;
  tmeFuseReadReq_t fuseReadReq = {0};
  tmeFuseReadRsp_t fuseReadRsp = {0};
  size_t           responseLen = sizeof(fuseReadRsp);

  CHECK_BAIL((fuseData != NULL) && (qfpromApiStatus != NULL));

  fuseReadReq.addrType = (uint32_t)addrType;
  fuseReadReq.fuseAddr = fuseAddr;

  ret = E_FAILURE;

  CHECK_BAIL(E_SUCCESS == TransceiveMessage(TME_MSG_CBOR_TAG_FUSE_READ,
                                            &fuseReadReq,
                                            sizeof(fuseReadReq),
                                            &fuseReadRsp,
                                            sizeof(fuseReadRsp),
                                            &responseLen));

  CHECK_BAIL(responseLen == sizeof(fuseReadRsp));

  /*
   * memscpy() copies min(dst,src) and returns that count, so compare against
   * the SOURCE size to detect a short copy - the caller is required to supply
   * room for the whole row (TME_QFPROM_FUSE_DATA_WORDS words).
   */
  CHECK_BAIL(sizeof(fuseReadRsp.fuseData) ==
             memscpy(fuseData,
                     sizeof(fuseReadRsp.fuseData),
                     fuseReadRsp.fuseData,
                     sizeof(fuseReadRsp.fuseData)));

  *qfpromApiStatus = fuseReadRsp.qfpromApiStatus;

  ret = E_SUCCESS;

bail:
  return ret;
}
