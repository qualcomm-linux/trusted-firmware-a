/*===========================================================================
  Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
  All rights reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
===========================================================================*/

/*
 * QFPROM multi-row fuse write over TMECOM.
 *
 * Ported from the downstream client (ssg/tme/tmeintf/src/TmeFuseWriteMultiple.c).
 *
 * ###########################################################################
 * # QFPROM FUSES ARE ONE-TIME-PROGRAMMABLE.                                 #
 * #                                                                         #
 * # Every set bit in a TMEFuse_t.data[] word permanently blows that fuse bit #
 * # on real silicon.  It cannot be un-blown, the part cannot be recovered,   #
 * # and blowing the wrong row can brick the device or lock it out of secure  #
 * # boot.  An all-zero data[] blows nothing and is the only value that is    #
 * # safe to send speculatively.                                             #
 * #                                                                         #
 * # This function does not and cannot validate the caller's intent - TME FW  #
 * # will happily program whatever it is handed, subject only to its own      #
 * # region permission checks.  The responsibility is entirely at the call    #
 * # site.                                                                   #
 * ###########################################################################
 *
 * Deviations from the downstream reference, both deliberate:
 *
 *   - The request is built in a file-scope static rather than heap-allocated.
 *     The struct is ~772 bytes (see tmeFuseWriteMultipleReq_t), which is too
 *     much to put on a BL31 stack, and TF-A has no heap.  This is safe here:
 *     BL31 is single-threaded and TME processes one request at a time, so at
 *     most one fuse-write request is ever in flight.
 *
 *   - Error codes are returned as positive IxErrno E_* values, matching the
 *     rest of this tree (TmeFuseRead.c, TmeMessage.c).  The downstream client
 *     negates them.
 */

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "IxErrno.h"
#include "TmeInterfaces.h"
#include "TmeInterfacesDefs.h"
#include "TmeMessage.h"
#include "TmeMessagesTags.h"

/*
 * Request scratch.  Static rather than automatic - see the note in the file
 * header about the struct size and the absence of a heap in TF-A.
 */
static tmeFuseWriteMultipleReq_t s_fuseWriteReq;

int TmeFuseWriteMultiple(TMEFuse_t      *fuseArray,
                         size_t          fuseArrayLen,
                         uint32_t *const qfpromApiStatus)
{
  int                       ret         = E_FAILURE;
  tmeFuseWriteMultipleRsp_t rsp         = {0};
  size_t                    responseLen = sizeof(rsp);
  size_t                    i;

  if ((fuseArray == NULL) || (qfpromApiStatus == NULL))
  {
    return E_BAD_ADDRESS;
  }

  if (fuseArrayLen == 0U)
  {
    return E_NO_DATA;
  }

  if (fuseArrayLen > TME_MAX_FUSE_WRITE_REQ)
  {
    return E_DATA_TOO_LARGE;
  }

  *qfpromApiStatus = TME_QFPROM_STATUS_UNSET;

  /*
   * The scratch buffer is reused across calls and the whole fixed-size struct
   * goes on the wire, so clear it first: without this, row addresses left in
   * the unused tail by an earlier request would be re-presented to TME FW.
   * TME FW only honours the first fuseArrayLen entries, but do not rely on
   * that to keep stale addresses harmless.
   */
  memset(&s_fuseWriteReq, 0, sizeof(s_fuseWriteReq));

  s_fuseWriteReq.fuseArrayLen = (uint32_t)fuseArrayLen;

  for (i = 0U; i < fuseArrayLen; i++)
  {
    s_fuseWriteReq.fuseArray[i].addr    = fuseArray[i].addr;
    s_fuseWriteReq.fuseArray[i].data[0] = fuseArray[i].data[0];
    s_fuseWriteReq.fuseArray[i].data[1] = fuseArray[i].data[1];
  }

  ret = TransceiveMessage(TME_MSG_CBOR_TAG_FUSE_WRITE_MULTIPLE,
                          &s_fuseWriteReq,
                          sizeof(s_fuseWriteReq),
                          &rsp,
                          sizeof(rsp),
                          &responseLen);

  if (ret != E_SUCCESS)
  {
    return ret;
  }

  if (responseLen != sizeof(rsp))
  {
    return E_FAILURE;
  }

  /*
   * Mirrors the downstream client: the caller's qfpromApiStatus receives
   * rsp.status (TME's handler status), not rsp.addrErr.  rsp.addrErr carries
   * the qfprom driver's address/error detail and is dropped here, again
   * matching downstream - surface it through a wider signature if it is ever
   * needed for diagnostics.
   */
  *qfpromApiStatus = rsp.status;

  return (rsp.status == TME_QFPROM_NO_ERR) ? E_SUCCESS : E_FAILURE;
}
