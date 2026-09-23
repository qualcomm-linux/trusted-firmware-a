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
#else   /*  FEATURE_64_BIT_HSDMA */
typedef uint32_t TmeComAddr_t;
#endif  /*  FEATURE_64_BIT_HSDMA */

/*
 * Extended error information returned by TME for operations that use the
 * sequencer.
 */
typedef struct {
	uint32_t tme_error_status;    /**< TME FW Response status. */
	uint32_t seq_error_status;    /**< Contents of CSR_CMD_ERROR_STATUS */
	uint32_t seq_kp_error_status0; /**< CRYPTO_ENGINE_CRYPTO_KEY_POLICY_ERROR_STATUS0 */
	uint32_t seq_kp_error_status1; /**< CRYPTO_ENGINE_CRYPTO_KEY_POLICY_ERROR_STATUS1 */
	uint32_t seq_rsp_status;      /**< Contents of CSR_CMD_RESPONSE_STATUS */
} TmeExtendedErrorInfo;

/*--------------------------------------------------------------------------*
 *                               QFPROM fuses                                *
 *--------------------------------------------------------------------------*/

/*
 * QFPROM address-space selector.
 *
 * Declared as a plain uint32_t rather than an enum because only the
 * corrected region value is known in this tree; the full enumeration is
 * not vendored here.
 */
typedef uint32_t TmeQfpromAddrSpace_t;

#define TME_QFPROM_ADDR_SPACE_CORR  0x1U /**< Corrected (ECC-applied) region */

/*
 * qfprom_api_status value seen when a row is read without error; 0 is the
 * value TME FW returns on a successful read.
 */
#define TME_QFPROM_NO_ERR           0x0U

/** A QFPROM row is read two 32-bit words at a time. */
#define TME_QFPROM_FUSE_DATA_WORDS  2U

/*
 * Request payload for TME_MSG_CBOR_TAG_FUSE_READ.
 *
 * Field order is confirmed against TME FW: a request of
 * { addr_type = TME_QFPROM_ADDR_SPACE_CORR, fuse_addr = <row> } is accepted and
 * answered with qfprom_api_status == TME_QFPROM_NO_ERR.  Reversing the two would
 * present an invalid address space and be rejected.
 */
typedef PACKED_STRUCT
{
	uint32_t addr_type; /* ! TmeQfpromAddrSpace_t selecting the fuse address space */
	uint32_t fuse_addr; /* ! SoC address of the QFPROM row to read */
} tmeFuseReadReq_t;

/*
 * Response payload for TME_MSG_CBOR_TAG_FUSE_READ.
 *
 * Field order copied verbatim from TME FW's own TmeMessageTypes.h - status
 * FIRST, then the row data, then the qfprom driver status.  TME FW fills
 * .qfprom_api_status from qfprom_read_row() and .status from its handler's
 * return code (tme_handle_fuse_read.cpp).
 *
 * Do not reorder these to "read more naturally".  An earlier version of this
 * struct led with fuse_data and put status last; it has the same 16-byte size,
 * so the exchange still completed and the bring-up row (which reads back all
 * zeroes) still looked like a clean pass - while actually reporting
 * status/fuse_data[0] as the row contents and fuse_data[1] as the driver status.
 */
typedef PACKED_STRUCT
{
	uint32_t status;                               /* ! TME handler status */
	uint32_t fuse_data[TME_QFPROM_FUSE_DATA_WORDS]; /* ! Row contents, low word first */
	uint32_t qfprom_api_status;                      /* ! qfprom driver status */
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
 * Placeholder written to the caller's qfprom_api_status before the exchange, so
 * a caller that ignores the return value never sees a stale or uninitialised
 * "success".  Any nonzero value is equivalent here, since callers only ever
 * test against TME_QFPROM_NO_ERR.
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
	uint32_t addr;                             /* ! SoC address of the row to write */
	uint32_t data[TME_QFPROM_FUSE_DATA_WORDS]; /* ! Value to blow, low word first */
} TMEFuse_t;

/*
 * Request payload for TME_MSG_CBOR_TAG_FUSE_WRITE_MULTIPLE.
 *
 * Layout (array first, count last) copied verbatim from TME FW's
 * TmeMessageTypes.h.  TME FW requires the whole struct: its handler rejects
 * anything shorter than sizeof(tmeFuseWriteMultipleReq_t), so all
 * TME_MAX_FUSE_WRITE_REQ slots go on the wire regardless of fuse_array_len.
 */
typedef PACKED_STRUCT
{
	TMEFuse_t fuse_array[TME_MAX_FUSE_WRITE_REQ]; /* ! Rows to write */
	uint32_t  fuse_array_len;                      /* ! Valid entries in fuse_array */
} tmeFuseWriteMultipleReq_t;

/*
 * Response payload for TME_MSG_CBOR_TAG_FUSE_WRITE_MULTIPLE.
 *
 * TME FW sets .status from its handler's return code and .addr_err from the
 * qfprom driver's per-address error output (tme_handle_fuse_write_multiple.cpp).
 */
typedef PACKED_STRUCT
{
	uint32_t status;  /* ! TME handler status; TME_QFPROM_NO_ERR on success */
	uint32_t addr_err; /* ! qfprom driver address/error detail */
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
	uint8_t  id;    /* ! tmeConfigRegisterId_e selecting the register */
	uint32_t value; /* ! Value to write into the register */
} tmeWriteConfigRegisterReq_t;

/*
 * Response payload for TME_MSG_CBOR_TAG_WRITE_CONFIG_REGISTER.
 */
typedef PACKED_STRUCT
{
	uint32_t status; /* ! TME handler status; 0 on success */
} tmeWriteConfigRegisterRsp_t;

/*--------------------------------------------------------------------------*
 *                             XPU DBGAR programming                         *
 *--------------------------------------------------------------------------*/

/*
 * Response payload for TME_MSG_CBOR_TAG_SET_XPU_DBG_AR.
 */
typedef PACKED_STRUCT
{
	uint32_t status; /* ! TME handler status; 0 on success */
} tmeSetXpuDbgarRsp_t;

/*--------------------------------------------------------------------------*
 *                          Signed image ID retrieval                        *
 *--------------------------------------------------------------------------*/

/*
 * Signing authority selector for TME_MSG_CBOR_TAG_GET_SIGNED_IMAGE_IDS.
 */
typedef enum
{
	TMECOM_QTI_CA_ID     = 0x01,
	TMECOM_OEM_CA_ID     = 0x02,
	TMECOM_DELEGATE_K_ID = 0x04
} tmeSoftwareRootCaIds;

/*
 * The wire size here is dictated by the TME firmware binary actually running
 * on target: a boot-time probe against real TME FW on this SoC observed a
 * 108-byte response (4 + 4 + 25*4), i.e. a 25-entry array.  Getting this
 * wrong doesn't corrupt anything - transceive_message()'s length check rejects
 * the mismatched response outright - but every call fails until this matches
 * what TME actually sends.
 */
#define TME_SIGNED_IMAGE_SWIDS_MAX  25U

/*
 * Request payload for TME_MSG_CBOR_TAG_GET_SIGNED_IMAGE_IDS.
 */
typedef PACKED_STRUCT
{
	uint32_t signing_authority; /* ! tmeSoftwareRootCaIds selecting the CA */
} tmeSignedSwIdsReq_t;

/*
 * Response payload for TME_MSG_CBOR_TAG_GET_SIGNED_IMAGE_IDS.
 */
typedef PACKED_STRUCT
{
	uint32_t status;                                /* ! TME handler status; 0 on success */
	uint32_t sw_id_count;                             /* ! Valid entries in sw_ids */
	uint32_t sw_ids[TME_SIGNED_IMAGE_SWIDS_MAX];      /* ! Signed image IDs */
} tmeSignedSwIdsRsp_t;

/*--------------------------------------------------------------------------*
 *                            PIL image region query                         *
 *--------------------------------------------------------------------------*/

#define TMECOM_PIL_IMAGES_MAX_SWIDS    10U
#define TMECOM_PIL_IMAGES_MAX_REGIONS  30U

/*
 * Request payload for TME_MSG_CBOR_TAG_GET_PIL_REGIONS.
 */
typedef PACKED_STRUCT
{
	uint32_t sw_ids[TMECOM_PIL_IMAGES_MAX_SWIDS]; /* ! Software IDs to query */
	uint32_t sw_id_count;                          /* ! Valid entries in sw_ids */
} tmeGetPilImageRegionsReq_t;

/*
 * One PIL region, as reported by TME.
 */
typedef PACKED_STRUCT
{
	uint32_t sw_id;      /* ! Id of the image this region belongs to */
	uint32_t start_addr; /* ! Region start address (SoC view) */
	uint32_t end_addr;   /* ! Region end address (SoC view) */
} tmePilRegion_t;

/*
 * Response payload for TME_MSG_CBOR_TAG_GET_PIL_REGIONS.
 */
typedef PACKED_STRUCT
{
	uint32_t       status;                                    /* ! TME handler status; 0 on success */
	tmePilRegion_t region_list[TMECOM_PIL_IMAGES_MAX_REGIONS];  /* ! Regions found */
	uint32_t       region_list_count;                           /* ! Valid entries in region_list */
} tmeGetPilImageRegionsRsp_t;

/*--------------------------------------------------------------------------*
 *                       Antirollback version update                        *
 *--------------------------------------------------------------------------*/

/*
 * Response payload for TME_MSG_CBOR_TAG_UPDATE_ROLLBACK_VERSION.
 *
 * There is no matching request struct: the message carries an empty payload.
 * TME FW already holds the image versions recorded during authentication and
 * decides for itself which antirollback fuses to blow, so the caller has
 * nothing to send.
 */
typedef PACKED_STRUCT
{
	uint32_t status;  /* ! TME handler status; 0 on success */
	uint32_t err_addr; /* ! Failing fuse address; not meaningful when status is 0 */
} tmeUpdateRollbackVersionRsp_t;

#endif /* TME_INTERFACES_DEFS_H_INCLUDED */
