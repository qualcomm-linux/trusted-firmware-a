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

/*--------------------------------------------------------------------------*
 *                               QFPROM fuses                                *
 *--------------------------------------------------------------------------*/

/*
 * QFPROM address-space selector (TMEQFPROMAddrSpace_t downstream).
 *
 * Declared as a plain uint32_t rather than an enum because only the corrected
 * region value is known in this tree - the full enumeration lives in the
 * downstream qfprom headers, which are not vendored here.
 */
typedef uint32_t TmeQfpromAddrSpace_t;

#define TME_QFPROM_ADDR_SPACE_CORR  0x1U /**< Corrected (ECC-applied) region */

/*
 * qfpromApiStatus value seen when a row is read without error.  The downstream
 * QFPROM_NO_ERR enumerator is not vendored here; 0 is the value TME FW returns
 * on a successful read.
 */
#define TME_QFPROM_NO_ERR           0x0U

/** A QFPROM row is read two 32-bit words at a time. */
#define TME_QFPROM_FUSE_DATA_WORDS  2U

/*
 * Request payload for TME_MSG_CBOR_TAG_FUSE_READ.
 *
 * Field order is confirmed against TME FW: a request of
 * { addrType = TME_QFPROM_ADDR_SPACE_CORR, fuseAddr = <row> } is accepted and
 * answered with qfpromApiStatus == TME_QFPROM_NO_ERR.  Reversing the two would
 * present an invalid address space and be rejected.
 */
typedef PACKED_STRUCT
{
  uint32_t addrType; //<! TmeQfpromAddrSpace_t selecting the fuse address space
  uint32_t fuseAddr; //<! SoC address of the QFPROM row to read
} tmeFuseReadReq_t;

/*
 * Response payload for TME_MSG_CBOR_TAG_FUSE_READ.
 *
 * Field order copied verbatim from TME FW's own TmeMessageTypes.h - status
 * FIRST, then the row data, then the qfprom driver status.  TME FW fills
 * .qfpromApiStatus from qfprom_read_row() and .status from its handler's
 * return code (tme_handle_fuse_read.cpp).
 *
 * Do not reorder these to "read more naturally".  An earlier version of this
 * struct led with fuseData and put status last; it has the same 16-byte size,
 * so the exchange still completed and the bring-up row (which reads back all
 * zeroes) still looked like a clean pass - while actually reporting
 * status/fuseData[0] as the row contents and fuseData[1] as the driver status.
 */
typedef PACKED_STRUCT
{
  uint32_t status;                               //<! TME handler status
  uint32_t fuseData[TME_QFPROM_FUSE_DATA_WORDS]; //<! Row contents, low word first
  uint32_t qfpromApiStatus;                      //<! qfprom driver status
} tmeFuseReadRsp_t;

/*--------------------------------------------------------------------------*
 *                            QFPROM fuse write                              *
 *--------------------------------------------------------------------------*/

/**
 * Maximum number of rows TME FW accepts in one FUSE_WRITE_MULTIPLE request.
 * The request struct is fixed-size and always sent in full, so this also
 * fixes the on-wire request length at sizeof(tmeFuseWriteMultipleReq_t).
 */
#define TME_MAX_FUSE_WRITE_REQ      64U

/*
 * Placeholder written to the caller's qfpromApiStatus before the exchange, so
 * a caller that ignores the return value never sees a stale or uninitialised
 * "success".  The real downstream QFPROM_ERR_UNKNOWN enumerator is not
 * vendored in this tree; any nonzero value is equivalent here, since callers
 * only ever test against TME_QFPROM_NO_ERR.
 */
#define TME_QFPROM_STATUS_UNSET     0xFFFFFFFFU

/*
 * One row of a fuse-write request.
 *
 * WARNING: fuses are one-time-programmable.  A set bit in data[] permanently
 * blows that fuse bit on real silicon; it cannot be cleared afterwards.  An
 * all-zero data[] blows nothing and is the only non-destructive value.
 */
typedef PACKED_STRUCT
{
  uint32_t addr;                             //<! SoC address of the row to write
  uint32_t data[TME_QFPROM_FUSE_DATA_WORDS]; //<! Value to blow, low word first
} TMEFuse_t;

/*
 * Request payload for TME_MSG_CBOR_TAG_FUSE_WRITE_MULTIPLE.
 *
 * Layout (array first, count last) copied verbatim from TME FW's
 * TmeMessageTypes.h.  TME FW requires the whole struct: its handler rejects
 * anything shorter than sizeof(tmeFuseWriteMultipleReq_t), so all
 * TME_MAX_FUSE_WRITE_REQ slots go on the wire regardless of fuseArrayLen.
 */
typedef PACKED_STRUCT
{
  TMEFuse_t fuseArray[TME_MAX_FUSE_WRITE_REQ]; //<! Rows to write
  uint32_t  fuseArrayLen;                      //<! Valid entries in fuseArray
} tmeFuseWriteMultipleReq_t;

/*
 * Response payload for TME_MSG_CBOR_TAG_FUSE_WRITE_MULTIPLE.
 *
 * TME FW sets .status from its handler's return code and .addrErr from the
 * qfprom driver's per-address error output (tme_handle_fuse_write_multiple.cpp).
 */
typedef PACKED_STRUCT
{
  uint32_t status;  //<! TME handler status; TME_QFPROM_NO_ERR on success
  uint32_t addrErr; //<! qfprom driver address/error detail
} tmeFuseWriteMultipleRsp_t;

/*--------------------------------------------------------------------------*
 *                      QFPROM configuration register write                 *
 *--------------------------------------------------------------------------*/

/*
 * Register identifiers accepted by TME_MSG_CBOR_TAG_WRITE_CONFIG_REGISTER.
 * Values copied verbatim from TME FW's TmeMessageTypes.h - do not renumber.
 */
typedef enum
{
  QFPROM_BIST_CTRL = 1,
  QFPROM_WRITE_DISABLE_STICKY_BIT0,
  QFPROM_WRITE_DISABLE_STICKY_BIT1,
  TME_WRITE_CONFIG_REGISTER_MAX = 0xFF
} tmeConfigRegisterId_e;

/*
 * Request payload for TME_MSG_CBOR_TAG_WRITE_CONFIG_REGISTER.
 */
typedef PACKED_STRUCT
{
  uint8_t  id;    //<! tmeConfigRegisterId_e selecting the register
  uint32_t value; //<! Value to write into the register
} tmeWriteConfigRegisterReq_t;

/*
 * Response payload for TME_MSG_CBOR_TAG_WRITE_CONFIG_REGISTER.
 */
typedef PACKED_STRUCT
{
  uint32_t status; //<! TME handler status; 0 on success
} tmeWriteConfigRegisterRsp_t;

#endif /* TME_INTERFACES_DEFS_H_INCLUDED */
