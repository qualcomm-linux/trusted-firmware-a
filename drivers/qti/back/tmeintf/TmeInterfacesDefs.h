/*===========================================================================
  Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
  All rights reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
===========================================================================*/

#ifndef TME_INTERFACES_DEFS_H_INCLUDED
#define TME_INTERFACES_DEFS_H_INCLUDED

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifndef PACKED_STRUCT
  #ifdef _MSC_VER
    #define PACKED_STRUCT __pragma( pack(push, 1) ) struct __pragma( pack(pop) )
  #elif defined(__ARMCC_VERSION)
    #define PACKED_STRUCT struct __attribute__((packed))
  #elif defined(__GNUC__)
    #define PACKED_STRUCT struct __attribute__((packed))
    #define __packed __attribute__((__packed__))
  #else
    #error Unknown compiler
  #endif
#endif

/**
 * Address type to use over TmeCom Interface
 */
#if defined(FEATURE_64_BIT_HSDMA)
typedef uint64_t TmeComAddr_t;
#else   //  FEATURE_64_BIT_HSDMA
typedef uint32_t TmeComAddr_t;
#endif  //  FEATURE_64_BIT_HSDMA

#ifndef TME_BITS_TO_BYTES
  #define TME_BITS_TO_BYTES(bits) (((bits) + 7) >> 3)
#endif

#define TME_HA_SHA256_SIZE  TME_BITS_TO_BYTES(256) /**< Size of SHA-256 digest in bytes */
#define TME_HA_SHA384_SIZE  TME_BITS_TO_BYTES(384) /**< Size of SHA-384 digest in bytes */
#define TME_HA_SHA512_SIZE  TME_BITS_TO_BYTES(512) /**< Size of SHA-512 digest in bytes */


/*--------------------------------------------------------------------------*
 *                                  Key ID                                  *
 *--------------------------------------------------------------------------*/
typedef uint32_t TmeKID;

/**
 * Hash algorithm IDs used for SHA digest and HMAC-SHA operations.
 */
typedef enum
{
  TME_HA_INVALID = 0x00, /**< Hash Algorithm: INVALID */
  TME_HA_SHA256  = 0x02, /**< Hash Algorithm: SHA256  */
  TME_HA_SHA384  = 0x03, /**< Hash Algorithm: SHA384  */
  TME_HA_SHA512  = 0x05  /**< Hash Algorithm: SHA512  */
} TMEHashAlgID_t;

/*
 * Request payload for TME_MSG_CBOR_TAG_SHA_DIGEST.
 */
typedef PACKED_STRUCT
{
  uint32_t     algorithm; //<! SHA algorithm
  TmeComAddr_t data;      //<! Input data address
  uint32_t     dataSize;  //<! Input data size
  uint32_t     keyID;     //<! Key identifier (applicable for HMAC-SHA)
} tmeShaReq_t;

/*
 * Extended error information returned by TME for operations that use the
 * sequencer.
 */
typedef struct {
  uint32_t tmeErrorStatus;    /**< TME FW Response status. */
  uint32_t seqErrorStatus;    /**< Contents of CSR_CMD_ERROR_STATUS */
  uint32_t seqKPErrorStatus0; /**< CRYPTO_ENGINE_CRYPTO_KEY_POLICY_ERROR_STATUS0 */
  uint32_t seqKPErrorStatus1; /**< CRYPTO_ENGINE_CRYPTO_KEY_POLICY_ERROR_STATUS1 */
  uint32_t seqRspStatus;      /**< Contents of CSR_CMD_RESPONSE_STATUS */
} TmeExtendedErrorInfo;

/*
 * Response payload for TME_MSG_CBOR_TAG_SHA_DIGEST.
 */
typedef PACKED_STRUCT
{
  TmeExtendedErrorInfo info;                       //<! Sequencer status information
  uint8_t              output[TME_HA_SHA512_SIZE]; //<! Output digest
  uint32_t             outputLen;                  //<! Output digest length in bytes
} tmeShaRsp_t;

#endif /* TME_INTERFACES_DEFS_H_INCLUDED */
