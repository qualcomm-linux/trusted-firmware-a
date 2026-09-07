/*===========================================================================
  Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
  All rights reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
===========================================================================*/

#ifndef TME_MESSAGES_TAGS_H_INCLUDED
#define TME_MESSAGES_TAGS_H_INCLUDED

/**
 *
 * Please refer go/tmecomapi before updating this file.
 *
 * CBOR Tags that are used with TME FW Messages. The COSE ones are types defined in RFC 8052.
 * The message tags starting from TME_MSG_FW_BASE_TAG are proprietary ones supported in TME FW.
 * These CBOR message tags are assigned as per IANA in the range between 271 and 1000 on a First
 * Come First Served basis.
 *
 * @see TME FW HLD design for CBOR tag assignment.
 */

// clang-format off
typedef enum
{
  TME_MSG_CBOR_TAG_INVALID                       = ~0U,    ///< Invalid CBOR  tag. Same     as  @c          CBOR_TAG_NONE
  TME_MSG_CBOR_TAG_COSE_SIGN                     = 98,     ///< RFC     8152  COSE_Sign
  TME_MSG_CBOR_TAG_COSE_SIGN1                    = 18,     ///< RFC     8152  COSE_Sign1
  TME_MSG_CBOR_TAG_COSE_ENCRYPT                  = 96,     ///< RFC     8152  COSE_Encrypt
  TME_MSG_CBOR_TAG_COSE_ENCRYPT0                 = 16,     ///< RFC     8152  COSE_Encrypt0
  TME_MSG_CBOR_TAG_COSE_MAC                      = 97,     ///< RFC     8152  COSE_Mac
  TME_MSG_CBOR_TAG_COSE_MAC0                     = 17,     ///< RFC     8152  COSE_Mac0
  TME_MSG_CBOR_TAG_FW_BASE_TAG                   = 271,    ///< Base    CBOR  Tag  for      TME proprietary messages
  TME_MSG_CBOR_TAG_REPLAY_PROTECT                = 275,    ///<
  TME_MSG_CBOR_TAG_AES_GCM_ENCRYPT               = 276,    ///<
  TME_MSG_CBOR_TAG_AES_GCM_DECRYPT               = 277,    ///<
  TME_MSG_CBOR_TAG_LOG_SET_CONFIG                = 278,    ///<
  TME_MSG_CBOR_TAG_LOG_GET_CONFIG                = 279,    ///<
  TME_MSG_CBOR_TAG_LOG_GET                       = 280,    ///<
  TME_MSG_CBOR_TAG_ATTESTATION                   = 281,    ///<
  TME_MSG_CBOR_TAG_GET_MODEM_PIL_IMAGE_RANGE     = 282,    ///<
  TME_MSG_CBOR_TAG_BOOTLOG                       = 283,    ///<
  TME_MSG_CBOR_TAG_IMAGE_VERIFY                  = 284,    ///<
  TME_MSG_CBOR_TAG_SUBSYSTEM_RESET               = 285,    ///<
  TME_MSG_CBOR_TAG_IMAGE_START                   = 286,    ///<
  TME_MSG_CBOR_TAG_SHA_DIGEST                    = 287,    ///<
  TME_MSG_CBOR_TAG_HMAC_SHA                      = 288,    ///<
  TME_MSG_CBOR_TAG_DBG_ACCESS_REGISTER_WRITE     = 300,    ///<
  TME_MSG_CBOR_TAG_FUSE_READ                     = 301,    ///<
  TME_MSG_CBOR_TAG_FUSE_WRITE                    = 302,    ///<
  TME_MSG_CBOR_TAG_HWKM                          = 303,    ///<
  TME_MSG_CBOR_TAG_PARALLEL_HASH                 = 304,    ///<
  TME_MSG_CBOR_TAG_UNLOCK_AREA                   = 305,    ///<
  TME_MSG_CBOR_TAG_ECDSA_VERIFY                  = 306,    ///<
  TME_MSG_CBOR_TAG_UPDATE_ROLLBACK_VERSION       = 307,    ///<
  TME_MSG_CBOR_TAG_GET_MISC_DATA                 = 308,    ///<
  TME_MSG_CBOR_TAG_GET_OEM_TEST_ROOT_CERT_HASHES = 309,    ///<
  TME_MSG_CBOR_TAG_GET_TEST_SIGN_IMAGE_HASHES    = 310,    ///<
  TME_MSG_CBOR_TAG_GET_TEST_SIGN_IMAGE_VECTOR    = 311,    ///<
  TME_MSG_CBOR_TAG_ECDSA_SIGN                    = 312,    ///<
  TME_MSG_CBOR_TAG_IMAGE_SIGN_VERIFY             = 313,    ///<
  TME_MSG_CBOR_TAG_SEGMENTS_AUTH                 = 314,    ///<
  TME_MSG_CBOR_TAG_ECDH_SHARED_SECRET            = 315,    ///<
  TME_MSG_CBOR_TAG_FUSE_WRITE_MULTIPLE           = 316,    ///<
  TME_MSG_CBOR_TAG_GET_SUBSYSTEM_DEBUG_OPTION    = 317,    ///<
  TME_MSG_CBOR_TAG_GET_OEM_ENCRYPTION_META_DATA  = 318,    ///<
  TME_MSG_CBOR_TAG_QUERY_LOG                     = 319,    ///< Deprecated - use @c TME_MSG_CBOR_TAG_LOG_GET
  TME_MSG_CBOR_TAG_SET_XPU_DBG_AR                = 320,    ///<
  TME_MSG_CBOR_TAG_INVOKE_AC                     = 321,    ///<
  TME_MSG_CBOR_TAG_ECDSA_GET_PUBLIC_KEY          = 322,    ///< For EDSA sign/verify
  TME_MSG_CBOR_TAG_ECDH_GET_PUBLIC_KEY           = 323,    ///< For ECDH Shared secret
  TME_MSG_CBOR_TAG_UNLOCK_SUBSYSTEM_AREA         = 324,    ///< Unlock PIL Subsystem region
  TME_MSG_CBOR_TAG_CD_TZ_GET_ENC_KEYS            = 325,    ///< Crash Dump: TZ retrieves encryption keys (TME and Modem)
  TME_MSG_CBOR_TAG_CD_TZ_SAVE_WRAPPED_KEYS       = 326,    ///< Crash Dump: TZ sends back wrapped keys (TME and Modem)
  TME_MSG_CBOR_TAG_CD_MODEM_GET_KEYS             = 327,    ///< Crash Dump: *Modem* (not used by TZ) retrieves both Modem keys (plain and wrapped)
  TME_MSG_CBOR_TAG_GET_ARB_VERSION               = 328,    ///< To retrieve ARB versions
  TME_MSG_CBOR_TAG_RSA_MODEXP                    = 329,    ///< To perform RSA mod. exp. in TME SEQ
  TME_MSG_CBOR_TAG_CAV_TEST_CONTROL              = 330,    ///< Start or End CAV test
  TME_MSG_CBOR_TAG_CAV_TEST_PEEK                 = 331,    ///< Read PRNG address space during CAV test
  TME_MSG_CBOR_TAG_CAV_TEST_POKE                 = 332,    ///< Write PRNG address space during CAV test
  TME_MSG_CBOR_TAG_GET_SIGNED_IMAGE_IDS          = 333,    ///< Get Images signed by a signing authority
  TME_MSG_CBOR_TAG_WRITE_CONFIG_REGISTER         = 334,    ///< Write QFPROM configuration register
  TME_MSG_CBOR_TAG_GET_ATTESTATION_CLAIMS        = 335,    ///< Get TME EAT Attestation claims
  TME_MSG_CBOR_TAG_SPSS_BRINGUP                  = 336,    ///< SPSS bringup
  TME_MSG_CBOR_TAG_SPSS_TEARDOWN                 = 337,    ///< SPSS teardown
  TME_MSG_CBOR_TAG_MILESTONE                     = 338,    ///< Generic milestone
  TME_MSG_CBOR_TAG_AES_GCM_ENCRYPT_EXT           = 340,    ///< Deprecated - no longer needed
  TME_MSG_CBOR_TAG_AES_GCM_DECRYPT_EXT           = 341,    ///< Deprecated - no longer needed
  TME_MSG_CBOR_TAG_SHA_DIGEST_EXT                = 342,    ///< Deprecated - no longer needed
  TME_MSG_CBOR_TAG_HMAC_SHA_EXT                  = 343,    ///< Deprecated - no longer needed
  TME_MSG_CBOR_TAG_SIGN_MSG_EXT                  = 344,    ///< Deprecated - no longer needed
  TME_MSG_CBOR_TAG_VERIFY_MSG_EXT                = 345,    ///< Deprecated - no longer needed
  TME_MSG_CBOR_TAG_GENERATE_QBEC_KEYS            = 346,    ///< Generate QBEC wrapped keys
  TME_MSG_CBOR_TMECOM_LOOPBACK                   = 347,    ///< Used for TMECOM loopback testing only
  TME_MSG_CBOR_TAG_EDDSA_GET_PUBLIC_KEY          = 350,    ///< for EdDSA operations
  TME_MSG_CBOR_TAG_EDDSA_SIGN_MSG                = 351,    ///<
  TME_MSG_CBOR_TAG_EDDSA_VERIFY_MSG              = 352,    ///<
  TME_MSG_CBOR_TAG_ADDRESS_WRITE_MULTIPLE        = 353,    ///< Generic whitelisted poke
  TME_MSG_CBOR_TAG_ADDRESS_READ_MULTIPLE         = 354,    ///< Generic whitelisted peek
  TME_MSG_CBOR_TAG_MEASUREMENT_REPORTS           = 355,    ///<
  TME_MSG_CBOR_TAG_UPDATE_GC_EXT_SLP_INCR        = 356,    ///<
  TME_MSG_CBOR_TAG_BCC                           = 357,    ///< Google BCC
  TME_MSG_CBOR_TAG_GET_PIL_REGIONS               = 358,    ///< For retrieving PIL regions for given SWIDs
  TME_MSG_CBOR_TAG_IS_DS_ALLOWED_MSG             = 359,    ///< For handling DS allowed call from TZ
  TME_MSG_CBOR_TAG_WRAP_HDCP_KEYS                = 360,    ///< Encrypts/Wraps the HDCP clear key
  TME_MSG_CBOR_TAG_PROVISION_HDCP_KEYS           = 361,    ///< Decrypts/Unwraps the wrapped key blob and programs the key to the MDSS HDCP registers
  TME_MSG_CBOR_TAG_SHA_DIGEST_MULTIPLE           = 362,    ///< Deprecated - no longer needed
  TME_MSG_CBOR_TAG_SHA_DIGEST_SINGLE             = 363,    ///< Deprecated - no longer needed
  TME_MSG_CBOR_TAG_HMAC_SHA_MULTIPLE             = 364,    ///< Deprecated - no longer needed
  TME_MSG_CBOR_TAG_HMAC_SHA_SINGLE               = 365,    ///< Deprecated - no longer needed
  TME_MSG_CBOR_TAG_MANAGE_SOC_PRNG               = 366,    ///< Manage SoC PRNG
  TME_MSG_CBOR_TAG_SHAKE_DIGEST                  = 367,    ///< SHAKE Digest
  TME_MSG_CBOR_TAG_SHAKE_DIGEST_SQUEEZE          = 368,    ///< SHAKE Digest Squeeze
  TME_MSG_CBOR_TAG_FMF                           = 369,    ///< Feature Management Framework
  TME_MSG_CBOR_TAG_C3D2_GATING                   = 370,    ///< Gate C3D2 entry
  TME_MSG_CBOR_TAG_CFG_DDR_PROTECTION            = 371,    ///< Config DDR protection
  TME_MSG_CBOR_TAG_UNREGISTER_OOBS_WDOG          = 372,    ///< Unregister OOB Secure WDOG
  TME_MSG_CBOR_TAG_BOOT_IMG_ENC_GET_WRAPPING_KEY = 373,    ///< Boot image encryption: Get Wrapping Key
  TME_MSG_CBOR_TAG_BOOT_IMG_ENC_UNWRAP_KEY       = 374,    ///< Boot image encryption: Unwrap Key
  TME_MSG_CBOR_TAG_QWES                          = 375,    ///< QWES
  TME_MSG_CBOR_TAG_ECDH_SHARED_SECRET_RELAXED    = 376,    ///< Calculate an ECDH Shared Secret with Relax Mode enabled
  TME_MSG_CBOR_TAG_QWES_AUTH                     = 377,    ///< QWES Auth.
  TME_MSG_CBOR_TAG_AUTOSST_TMEFW_TEST            = 403,    ///< Auto SST test
  TME_MSG_CBOR_TAG_QBEC_MIPP_TPK                 = 404,    ///< Internally queued by TME FW for TPK broadcast for MIPP/QBEC
  TME_MSG_CBOR_TAG_MLDSA                         = 405,    ///< MLDSA APIs
  TME_MSG_CBOR_TAG_CM_TESTS                      = 0xFFFD, ///< Countermeasure tests.
  TME_MSG_CBOR_TAG_QMP_SIMULATOR_TEST            = 0xFFFE, ///< Simulator-only message, used to run unit-tests
  TME_MSG_CBOR_TAG_MAX                           = 0xFFFF, ///< Max value to restrict the CBOR Tag size to 3 bytes.
  TME_MSG_CBOR_TAG_ERROR                         = TME_MSG_CBOR_TAG_MAX  ///< Also used to transmitt a CBOR @c int error code
} TME_MSG_CBOR_TAG;
// clang-format on

#endif  // TME_MESSAGES_TAGS_H_INCLUDED
