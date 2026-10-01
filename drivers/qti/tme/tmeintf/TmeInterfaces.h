/*===========================================================================
	Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
	All rights reserved.
	Confidential and Proprietary - Qualcomm Technologies, Inc.
===========================================================================*/

#ifndef TME_INTERFACES_H_INCLUDED
#define TME_INTERFACES_H_INCLUDED

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "IxErrno.h"
#include "TmeInterfacesDefs.h"

/*
 * tme_forward_request() - forward a pre-encoded (CBOR/QBOR) request to
 * TME and return the raw response.
 *
 * The request buffer is forwarded as-is; no serialisation is performed here.
 * A hard-coded communication timeout is applied internally — callers do not
 * supply or influence the timeout value.
 *
 * @param [in]  req_buf       Pointer to the pre-encoded request buffer.
 * @param [in]  req_size      Size of the request buffer in bytes.
 * @param [out] rsp_buf       Pointer to the response buffer.
 * @param [in]  rsp_buf_size   Size of the response buffer in bytes.
 *
 * @return E_SUCCESS on success, error code otherwise.
 */
int tme_forward_request(void *req_buf,
		size_t      req_size,
		void       *rsp_buf,
		size_t     *rsp_buf_size);

/*
 * tme_passthrough_cmd() - asynchronously forward a pre-encoded (CBOR/QBOR)
 * request from to TME.
 *
 * Returns as soon as the request has been handed to the transport; does not
 * wait for TME to finish processing it. The TME CPU processes one request
 * at a time and does not support queuing, so only one request -- submitted
 * via this function or tme_forward_request() -- may be outstanding at a time.
 *
 * Use tme_passthrough_await() with the returned handle to poll for the
 * response.
 *
 * @param [in]  req_buf    Pointer to the pre-encoded request buffer.
 * @param [in]  req_size   Size of the request buffer in bytes.
 * @param [out] handle    Opaque handle identifying this request, to be
 *                        passed to tme_passthrough_await().
 *
 * @return E_SUCCESS and *handle set if the request was submitted.
 *         E_AGAIN if TME is currently processing another request; the
 *         caller should retry later.
 *         Other error code on failure to submit the request.
 */
int tme_passthrough_cmd(void *req_buf, size_t req_size, uint32_t *handle);

/*
 * tme_passthrough_await() - poll, without blocking, for the response to a
 * request previously submitted via tme_passthrough_cmd().
 *
 * @param [in]     handle      Handle returned by tme_passthrough_cmd().
 * @param [out]    rsp_buf      Pointer to the response buffer.
 * @param [in/out] rsp_buf_size  On input: capacity of rsp_buf in bytes.
 *                             On output: actual response size in bytes.
 *
 * @return E_SUCCESS and response copied into rsp_buf if TME has finished
 *         processing the request.
 *         E_IN_PROGRESS if TME is still processing the request; call again
 *         later.
 *         Other error code if handle is invalid/stale.
 */
int tme_passthrough_await(uint32_t handle, void *rsp_buf, size_t *rsp_buf_size);

/*
 * tme_fuse_read() - read one QFPROM row via TME.
 *
 * @param [in]  addr_type         Fuse address space (TME_QFPROM_ADDR_SPACE_*).
 * @param [in]  fuse_addr         SoC address of the QFPROM row to read.
 * @param [out] fuse_data         Receives the row contents, low word first.
 *                               Must point to space for at least
 *                               TME_QFPROM_FUSE_DATA_WORDS uint32_t values -
 *                               a QFPROM row is always read two words at a
 *                               time, regardless of the width of interest.
 * @param [out] qfprom_api_status  Status reported by TME's qfprom driver;
 *                               TME_QFPROM_NO_ERR on a clean read.  Written
 *                               only when the call returns E_SUCCESS.
 *
 * @return E_SUCCESS if the exchange completed and the response was the
 *         expected size, error code otherwise.
 *
 * NOTE: E_SUCCESS only means the request/response exchange itself succeeded.
 * The caller MUST also check @p qfprom_api_status - TME reports a rejected or
 * failed fuse read there, not in the return value.
 */
int tme_fuse_read(TmeQfpromAddrSpace_t addr_type,
		uint32_t             fuse_addr,
		uint32_t *const      fuse_data,
		uint32_t *const      qfprom_api_status);

/*
 * tme_fuse_write_multiple() - blow up to TME_MAX_FUSE_WRITE_REQ QFPROM rows in a
 * single request via TME.
 *
 * ###########################################################################
 * # DESTRUCTIVE AND IRREVERSIBLE.  QFPROM fuses are one-time-programmable:   #
 * # any bit set in fuse_array[].data[] is blown permanently on real silicon    #
 * # and can never be cleared.  Blowing the wrong row can brick the part or    #
 * # lock it out of secure boot.                                              #
 * #                                                                         #
 * # An all-zero data[] blows nothing, which is what makes it safe to use for  #
 * # exercising this path without altering chip state.                        #
 * ###########################################################################
 *
 * @param [in]  fuse_array        Rows to write.  Not modified.
 * @param [in]  fuse_array_len     Number of entries in fuse_array; must be in
 *                               1..TME_MAX_FUSE_WRITE_REQ.
 * @param [out] qfprom_api_status  Status reported by TME for the write;
 *                               TME_QFPROM_NO_ERR on success.  Always written
 *                               once the arguments validate - set to
 *                               TME_QFPROM_STATUS_UNSET before the exchange.
 *
 * @return E_SUCCESS only if the exchange completed, the response was the
 *         expected size, AND TME reported TME_QFPROM_NO_ERR.
 *         E_BAD_ADDRESS / E_NO_DATA / E_DATA_TOO_LARGE on bad arguments,
 *         other error code on a failed exchange or a rejected write.
 *
 * NOTE: unlike tme_fuse_read(), a nonzero status is folded into the return value
 * here, so E_SUCCESS does mean the write itself was accepted.
 */
int tme_fuse_write_multiple(TMEFuse_t      *fuse_array,
		 size_t          fuse_array_len,
		 uint32_t *const qfprom_api_status);

/*
 * tme_write_config_register() - write a QFPROM configuration register via TME.
 *
 * @param [in]  register_id  Register to write (QFPROM_BIST_CTRL,
 *                           QFPROM_WRITE_DISABLE_STICKY_BIT0/1).
 * @param [in]  value       Value to write into the register.
 *
 * @return E_SUCCESS if the exchange completed, the response was the expected
 *         size, AND TME reported a zero status.  Error code otherwise -
 *         including when TME rejects register_id itself.
 */
int tme_write_config_register(tmeConfigRegisterId_e register_id, uint32_t value);

/*
 * tme_set_xpu_dbgar() - program a set of XPU DBGAR addresses via TME.
 *
 * @param [in] dbgars  Array of XPU DBGAR addresses.  Not modified.
 * @param [in] count   Number of entries in dbgars; must be nonzero.
 *
 * @return E_SUCCESS if the exchange completed, the response was the expected
 *         size, AND TME reported a zero status.  Error code otherwise.
 */
int tme_set_xpu_dbgar(uint32_t *dbgars, size_t count);

/*
 * tme_invoke_ac() - invoke an access-control (AC) module in TME.
 *
 * AC modules write and interpret the content of in_buffer/out_buffer
 * themselves; this call only moves the bytes to and from TME.
 *
 * @param [in]  request_id  Request id identifying the AC module in TME.
 * @param [in]  in_buffer   Data to send to TME.  Not modified.
 * @param [in]  in_size     Size of in_buffer in bytes.
 * @param [out] out_buffer  Receives data from TME.  Must not be NULL.
 * @param [in]  out_size    Capacity of out_buffer in bytes; must be nonzero.
 *
 * @return E_SUCCESS if the exchange completed, TME reported a zero status,
 *         AND the response fit within out_size.  Error code otherwise -
 *         including E_INVALID_ARG for a NULL out_buffer or zero out_size.
 */
int tme_invoke_ac(uint32_t request_id,
		 uint8_t *in_buffer,
		 size_t   in_size,
		 uint8_t *out_buffer,
		 size_t   out_size);

/*
 * tme_get_signed_image_ids() - retrieve the software image IDs signed by a given
 * signing authority.
 *
 * @param [in]  signing_authority  CA whose signed image IDs to retrieve.
 * @param [out] output_sw_ids       Receives the image IDs.
 * @param [in]  output_sw_id_max     Capacity of output_sw_ids, in entries; must
 *                                be nonzero.
 * @param [out] output_sw_id_count   Receives the number of entries written to
 *                                output_sw_ids.
 *
 * @return E_SUCCESS if the exchange completed, the response was the expected
 *         size, AND TME reported a zero status.  E_DATA_TOO_LARGE if TME's
 *         list does not fit in output_sw_id_max.  E_INVALID_ARG for a NULL
 *         output_sw_ids/output_sw_id_count or a zero output_sw_id_max.
 */
int tme_get_signed_image_ids(tmeSoftwareRootCaIds signing_authority,
		uint32_t            *output_sw_ids,
		size_t               output_sw_id_max,
		size_t              *output_sw_id_count);

/*
 * tme_get_pil_image_regions() - retrieve the PIL (Peripheral Image Loader)
 * memory regions TME has recorded for a set of software IDs.
 *
 * @param [in]     sw_id_count        Number of entries in sw_ids; must be
 *                                  nonzero and at most TMECOM_PIL_IMAGES_MAX_SWIDS.
 * @param [in]     sw_ids            Software IDs to query.
 * @param [in,out] region_list_count  On input: capacity of region_list, in
 *                                  entries; must be nonzero and at most
 *                                  TMECOM_PIL_IMAGES_MAX_REGIONS.  On output:
 *                                  number of entries TME reported.
 * @param [out]    region_list       Receives the regions TME reported.
 *
 * @return E_SUCCESS if the exchange completed, the response was the expected
 *         size, AND TME reported a zero status.  E_OUT_OF_RANGE if sw_id_count
 *         or the input region_list_count exceeds its maximum.  E_INVALID_ARG
 *         for a NULL pointer, a zero sw_id_count/region_list_count, or an input
 *         region_list_count too small for what TME reported.
 */
int tme_get_pil_image_regions(uint32_t       *const sw_id_count,
		 uint32_t       *const sw_ids,
		 uint32_t       *const region_list_count,
		 tmePilRegion_t *const region_list);

/*
 * tme_update_rollback_version() - tell TME to commit the recorded image versions
 * into the antirollback (ARB) fuses.
 *
 * ###########################################################################
 * # DESTRUCTIVE AND IRREVERSIBLE.  This blows antirollback fuses, which are #
 * # one-time-programmable: once TME has raised the stored ARB version for   #
 * # an image, that part will permanently refuse to boot any older-versioned #
 * # build of it.  There is no "undo" and no non-destructive dry run -       #
 * # unlike tme_fuse_write_multiple(), there is no payload to zero out,         #
 * # because TME chooses the fuses itself from versions it recorded during   #
 * # authentication.                                                         #
 * #                                                                         #
 * # Do NOT call this from a boot-time probe or self-test.                   #
 * ###########################################################################
 *
 * Until this is called, TME caches version information instead of committing
 * it.  Afterwards it also updates the rollback version as part of signature
 * verification for images authenticated later (e.g. modem).  TME FW tracks
 * that this call was made, so it is a one-time transition per boot.
 *
 * The intended single call site is the point at which the current boot is
 * declared good: at TZ/BL31 cold boot when A/B OTA is disabled, or, when A/B
 * OTA is enabled, only after HLOS has signalled a successful boot.  Calling it
 * before that defeats the purpose of A/B rollback.
 *
 * @return E_SUCCESS if the exchange completed, the response was the expected
 *         size, AND TME reported a zero status.  Error code otherwise.
 */
int tme_update_rollback_version(void);

#endif /* TME_INTERFACES_H_INCLUDED */
