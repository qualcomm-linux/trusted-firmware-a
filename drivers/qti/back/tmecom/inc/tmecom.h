/*===========================================================================
  Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
  All rights reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
===========================================================================*/

#ifndef _TMECOM_TME_H
#define _TMECOM_TME_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include "tmecomTypes.h"

#define TMECOM_VERSION(major, minor) (((major) << 8) | (minor))
#define TMECOM_VERSION_NONE          TMECOM_VERSION(0, 0)
#define TMECOM_VERSION_MAJOR         1
#define TMECOM_VERSION_MINOR         0

/**
 * tmecomMsgHdr - Request/Response message header between TFA and TME.
 *
 * This header is proceeding any request specific parameters.
 * The transaction id is used to match request with response.
 */
typedef struct
{
  uint16_t version;       /* TMECom Version */
  uint16_t crc;           /* Message CRC    */
  uint32_t txnId;         /* transaction id */
} __attribute__((__packed__))tmecomMsgHdr;

/**
 * Max allocation for TME COM mailbox
 */
#define TMECOM_MAX_MAILBOX_SIZE (2048)

/**
 * First part of mailbox memory is taken up by QMP descriptor
 *
 * @note  Same as sizeof(xport_qmp_ch_desc_type) but cannot figure out how to
 *        get the header included here
 */
#define TMECOM_LOCAL_MAILBOX_OFFSET  (0xC0)
#define TMECOM_REMOTE_MAILBOX_OFFSET (0x00)

/**
 * Request buffer size for outbound message
 */
/* Round a byte count up to the nearest number of uint32_t words. */
#define TME_BYTES2WORDS(n)  (((n) + sizeof(uint32_t) - 1U) / sizeof(uint32_t))

#define TMECOM_MAX_REQUEST_SIZE      (TMECOM_MAX_MAILBOX_SIZE     - \
                                      TMECOM_LOCAL_MAILBOX_OFFSET - \
                                      sizeof(tmecomMsgHdr))

/**
 * Response buffer size for inbound message
 */
#define TMECOM_MAX_RESPONSE_SIZE     (TMECOM_MAX_MAILBOX_SIZE      - \
                                      TMECOM_REMOTE_MAILBOX_OFFSET - \
                                      sizeof(tmecomMsgHdr))

typedef struct {
  uint8_t  buf[TMECOM_MAX_REQUEST_SIZE];      /* Buffer used to contain the request                 */
  bool     inUse;                             /* Used to indicate if the buffer is currently in use */
} __attribute__((aligned(sizeof(uint32_t))))tmecomReq_t;

typedef struct {
  uint8_t  buf[TMECOM_MAX_RESPONSE_SIZE];     /* Buffer used to contain the response                */
  bool     inUse;                             /* Used to indicate if the buffer is currently in use */
} __attribute__((aligned(sizeof(uint32_t))))tmecomRsp_t;

typedef struct
{
  tmecomMsgHdr  hdr;
  uint8_t       encReqBuf[TMECOM_MAX_REQUEST_SIZE];
} __attribute__((aligned(sizeof(uint32_t))))tmecomMsgReq_t;

typedef struct
{
  tmecomMsgHdr  hdr;
  uint8_t       encRspBuf[TMECOM_MAX_RESPONSE_SIZE];
} __attribute__((aligned(sizeof(uint32_t))))tmecomMsgRsp_t;

#ifdef __cplusplus
extern "C" {
#endif

/**
  * @addtogroup TMECom
  * @{
  */

/**
  * Initialize the TMECom interface.
  *
  * @param  [in]  tmecomInterface   Interface to initialize
  *
  * @return @c 0 if successfully handled, error code otherwise
  */
int tmecomInit(eTMEComInterface tmecomInterface);

/**
  * De-initialize the TMECom interface.
  *
  * @return @c 0 if successfully handled, error code otherwise
  */
int tmecomDeInit(void);

/**
  * Used by tmecom to allocate the request buffer used
  * to communicate with the TME-FW.
  *
  * @param  [in]  size    Size of the request buffer.
  *
  * @return Pointer to the memory if successfully obtained,
  *         @c NULL otherwise
  */
void* tmecomAllocReq(size_t size);

/**
  * Used by tmecom to free the request buffer used
  * to communicate with the TME-FW.
  *
  * @param  [in]  pMem    Pointer to the request buffer to be released
  *
  */
void tmecomFreeReq(void *pMem);

/**
  * Used by tmecom to allocate the response buffer used
  * to communicate with the TME-FW.
  *
  * @param  [in]  size    Size of the response buffer.
  *
  * @return Pointer to the memory if successfully obtained,
  *         @c NULL otherwise
  */
void* tmecomAllocRsp(size_t size);

/**
  * Used by tmecom to release the response buffer used
  * to communicate with the TME-FW.
  *
  * @param  [in]  pMem    Pointer to the response buffer to be released
  *
  */
void tmecomFreeRsp(void *pMem);

/**
  * Temporary API for running TMECom Tests.
  *
  * @return @c 0 if successfully handled, error code otherwise
  */
int tmecomRunTests(void);

/** @} */ /* end addtogroup TMECom */

#ifdef __cplusplus
}
#endif

#endif  // _TMECOM_TME_H
