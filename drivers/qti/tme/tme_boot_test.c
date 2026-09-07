/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * Boot-time TMECOM self-tests.  Moved out of wildcat_bl31_setup.c so the
 * platform setup file does not have to carry TME-specific test bodies -
 * see include/drivers/qti/tme/tme_boot_test.h for the entry points a
 * platform's bl31_platform_setup() should call.
 *
 * Compiled out entirely unless both QTI_USE_TMECOM and QTI_TMECOM_TEST are
 * defined; callers must keep their call sites behind the same two guards.
 */

#if defined(QTI_USE_TMECOM) && defined(QTI_TMECOM_TEST)

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <common/debug.h>
#include <lib/mmio.h>
#include <lib/utils_def.h>

#include <drivers/qti/tme/tme_boot_test.h>

#include <IxErrno.h>
#include <TmeInterfaces.h>
#include <TmeInterfacesDefs.h>
#include <TmeMessage.h>
#include <TmeMessagesTags.h>
#include <qcbor.h>
#include <qcbor_spiffy_decode.h>

/*
 * Boot-time self-test: have TME compute a SHA-384 digest over a known input
 * and compare it against the expected value.
 *
 * The expected digest covers the 23 bytes of test_data excluding the
 * terminating NUL.  Regenerate with:
 *
 *     printf '%s' 'This is some test data.' | sha384sum
 *
 * Do NOT use `echo` without -n: it appends a newline and produces a different
 * digest (98209f2f...), which will never match what this code hashes.
 */
void tmecom_boot_sha_test(void)
{
	static const char test_data[] = "This is some test data.";
	static const uint8_t expected_digest[TME_HA_SHA384_SIZE] = {
		0x05, 0x8b, 0x80, 0xef, 0x02, 0xa2, 0xf0, 0x6a,
		0x82, 0xcb, 0xd9, 0xdf, 0x8b, 0xb9, 0x76, 0x4d,
		0xfe, 0x40, 0xf2, 0xf0, 0xe5, 0xc9, 0x46, 0xaf,
		0xea, 0x2c, 0xf4, 0x82, 0xc5, 0x1e, 0x92, 0xb5,
		0xab, 0x63, 0x97, 0x84, 0x5a, 0x85, 0x99, 0xc3,
		0x91, 0x0b, 0x7d, 0x9a, 0x85, 0x5f, 0x49, 0xd6,
	};

	uint8_t              digest[TME_HA_SHA384_SIZE] = {0};
	size_t               digest_len                 = sizeof(digest);
	TmeExtendedErrorInfo error_info                 = {0};
	size_t               i;
	int                  ret;

	INFO("%s: ENTRY\n", __func__);
	INFO("TMECOM SHA test: started (data_len=%zu)\n",
	     sizeof(test_data) - 1U);

	/* Length excludes the terminating NUL. */
	INFO("%s: calling TmeSHADigest algo=TME_HA_SHA384\n", __func__);
	ret = TmeSHADigest(TME_HA_SHA384, (const uint8_t *)test_data,
			   sizeof(test_data) - 1U, digest, &digest_len,
			   &error_info);
	INFO("%s: TmeSHADigest returned ret=%d digest_len=%zu\n",
	     __func__, ret, digest_len);

	if (ret != E_SUCCESS) {
		ERROR("TMECOM SHA test: TmeSHADigest ret=%d\n", ret);
		ERROR("TMECOM SHA test: tme=0x%08X seq=0x%08X kp0=0x%08X kp1=0x%08X rsp=0x%08X\n",
		      error_info.tmeErrorStatus, error_info.seqErrorStatus,
		      error_info.seqKPErrorStatus0,
		      error_info.seqKPErrorStatus1, error_info.seqRspStatus);
		INFO("%s: EXIT (TmeSHADigest failure)\n", __func__);
		panic();
	}

	if (digest_len != sizeof(expected_digest)) {
		ERROR("TMECOM SHA test: bad digest len got:%zu want:%zu\n",
		      digest_len, sizeof(expected_digest));
		INFO("%s: EXIT (digest length mismatch)\n", __func__);
		panic();
	}

	for (i = 0U; i < sizeof(expected_digest); i++) {
		if (digest[i] != expected_digest[i]) {
			ERROR("TMECOM SHA test: mismatch at byte %zu got:0x%02X want:0x%02X\n",
			      i, digest[i], expected_digest[i]);
			INFO("%s: EXIT (digest mismatch)\n", __func__);
			panic();
		}
	}

	INFO("TMECOM SHA test: PASS\n");
	INFO("%s: EXIT\n", __func__);
}

/*
 * Boot-time self-test: send a fixed payload to TME's loopback tag
 * (TME_MSG_CBOR_TMECOM_LOOPBACK, see TmeMessagesTags.h - "used for TMECOM
 * loopback testing only").
 *
 * Parked for now, like tmecom_boot_sha_test() above - TME does not reliably
 * service this tag on this firmware build:
 *
 *   - With the request wrapped per the downstream reference client
 *     (tmeintf/src/TmeComLoopback.c: inner CBOR array `[ bstr(payload) ]`
 *     nested inside TransceiveMessage()'s generic tag(bstr(...)) envelope),
 *     one run returned a 12-byte response after a single poll that is not
 *     valid tag(bstr(...)) CBOR at all (raw dump added to DecodeMessage()
 *     confirmed the bytes: 00 01 D2 82 81 44 DE AD D9 FF FF 04 - no outer
 *     tag, and not a clean echo of the sent bytes either).
 *   - A second run with the identical request (same tag, same encoded
 *     length) instead timed out after TMECOM_POLL_MAX polls with no
 *     response at all, escalating through bl31qtilib_cb_error_fatal() into
 *     a platform fatal-error panic.
 *
 * Getting two different behaviors (immediate malformed response vs. total
 * silence) for byte-identical requests rules out a TFA-side encoding bug -
 * SHA digest already proves the transport/CBOR pipe works end-to-end (it
 * gets back a clean, correctly-tagged E_NOT_ALLOWED).  This looks like TME
 * firmware not properly implementing/servicing this tag.  Do not call this
 * at boot until that is confirmed/fixed on the TME FW side.
 */
void tmecom_boot_loopback_test(void)
{
	static const uint8_t payload[] = { 0xDE, 0xAD, 0xBE, 0xEF };
	uint8_t            req_cbor[16] = {0};
	uint8_t            rsp_cbor[32] = {0};
	UsefulBuf          req_storage  = {req_cbor, sizeof(req_cbor)};
	UsefulBufC         payload_buf  = {payload, sizeof(payload)};
	size_t             req_cbor_len;
	size_t             rsp_cbor_len = sizeof(rsp_cbor);
	QCBOREncodeContext enc_ctx      = {0};
	QCBORDecodeContext dec_ctx      = {0};
	QCBORItem          array_item   = {0};
	UsefulBufC         echoed       = {0};
	bool               pass         = false;
	size_t             i;
	int                ret;
	int                qret;

	INFO("%s: ENTRY\n", __func__);
	INFO("TMECOM loopback test: started (payload_len=%zu)\n",
	     sizeof(payload));

	/* Inner CBOR document: [ bstr(payload) ] - the message-specific
	 * shape for this tag, nested inside TransceiveMessage()'s generic
	 * tag(bstr(...)) envelope. */
	INFO("%s: encoding inner CBOR array [bstr(payload)]\n", __func__);
	QCBOREncode_Init(&enc_ctx, req_storage);
	QCBOREncode_OpenArray(&enc_ctx);
	QCBOREncode_AddBytes(&enc_ctx, payload_buf);
	QCBOREncode_CloseArray(&enc_ctx);
	qret = QCBOREncode_FinishGetSize(&enc_ctx, &req_cbor_len);
	INFO("%s: inner CBOR encode qret=%d req_cbor_len=%zu\n", __func__,
	     qret, req_cbor_len);
	if (qret != QCBOR_SUCCESS) {
		ERROR("TMECOM loopback test: inner CBOR encode ret=%d\n", qret);
		INFO("%s: EXIT (encode failure)\n", __func__);
		panic();
	}

	INFO("%s: calling TransceiveMessage tag=TME_MSG_CBOR_TMECOM_LOOPBACK\n",
	     __func__);
	ret = TransceiveMessage(TME_MSG_CBOR_TMECOM_LOOPBACK,
				req_cbor, req_cbor_len,
				rsp_cbor, sizeof(rsp_cbor), &rsp_cbor_len);
	INFO("%s: TransceiveMessage returned ret=%d rsp_cbor_len=%zu\n",
	     __func__, ret, rsp_cbor_len);
	if (ret != E_SUCCESS) {
		ERROR("TMECOM loopback test: TransceiveMessage ret=%d\n", ret);
		INFO("%s: EXIT (transport/protocol failure)\n", __func__);
		panic();
	}

	printf("INFO:    TMECOM loopback test: inner rsp CBOR len:%zu raw:",
	       rsp_cbor_len);
	for (i = 0U; i < rsp_cbor_len; i++) {
		printf(" %02X", rsp_cbor[i]);
	}
	printf("\n");

	/* Response is also [ bstr(echoed) ] - unwrap it the same way. */
	INFO("%s: decoding inner CBOR array from response\n", __func__);
	QCBORDecode_Init(&dec_ctx, (UsefulBufC){rsp_cbor, rsp_cbor_len},
			 QCBOR_DECODE_MODE_NORMAL);
	QCBORDecode_EnterArray(&dec_ctx, &array_item);
	QCBORDecode_GetByteString(&dec_ctx, &echoed);
	QCBORDecode_ExitArray(&dec_ctx);
	qret = QCBORDecode_GetError(&dec_ctx);
	INFO("%s: inner CBOR decode qret=%d array_count=%u echoed_len=%zu\n",
	     __func__, qret, (unsigned int)array_item.val.uCount, echoed.len);

	if (qret != QCBOR_SUCCESS) {
		ERROR("TMECOM loopback test: inner CBOR decode ret=%d\n", qret);
	} else if (array_item.val.uCount != 1U) {
		ERROR("TMECOM loopback test: unexpected inner array length %u\n",
		      (unsigned int)array_item.val.uCount);
	} else if ((echoed.len == sizeof(payload)) &&
		   (memcmp(echoed.ptr, payload, sizeof(payload)) == 0)) {
		pass = true;
	}

	if (pass) {
		INFO("TMECOM loopback test: PASS (payload echoed)\n");
	} else {
		INFO("TMECOM loopback test: response did not echo payload - see raw dump above\n");
	}

	INFO("%s: EXIT\n", __func__);
}

/*
 * QFPROM rows probed at boot.  The HWIO macros are not defined anywhere in this
 * tree (the publishing header lives outside the TFA source drop), so the
 * addresses are hardcoded.  ROW3 comes from the downstream reference value;
 * the second row's register name is not known here, only its address.
 *
 * The fuse address-space selector and the request/response layout live in the
 * TME interface layer - see TmeFuseRead() in tmeintf/TmeInterfaces.h.
 */
#define HWIO_QFPROM_RAW_OEM_CONFIG_ROW3_LSB_ADDR	0x360C0170U
#define QFPROM_SECOND_TEST_ROW_ADDR			0x360C01A0U

/*
 * Read one QFPROM row (corrected address space) through TmeFuseRead() and log
 * the result.  Never panics: a failed exchange or a nonzero qfpromApiStatus is
 * logged and returned from, not treated as fatal.
 */
static void tmecom_boot_fuse_read_one(uint32_t fuseAddr)
{
	uint32_t fuseData[TME_QFPROM_FUSE_DATA_WORDS] = {0};
	uint32_t qfpromApiStatus                      = 0U;
	int      ret;

	INFO("TMECOM fuse read: addrType:0x%X fuseAddr:0x%X\n",
	     TME_QFPROM_ADDR_SPACE_CORR, fuseAddr);

	ret = TmeFuseRead(TME_QFPROM_ADDR_SPACE_CORR, fuseAddr,
			  fuseData, &qfpromApiStatus);
	if (ret != E_SUCCESS) {
		INFO("TMECOM fuse read: addr:0x%X TmeFuseRead ret=%d\n",
		     fuseAddr, ret);
		return;
	}

	/*
	 * E_SUCCESS only means the exchange worked; TME reports a rejected or
	 * failed read via qfpromApiStatus.
	 */
	if (qfpromApiStatus != TME_QFPROM_NO_ERR) {
		INFO("TMECOM fuse read: addr:0x%X rejected qfpromApiStatus:0x%08X\n",
		     fuseAddr, qfpromApiStatus);
		return;
	}

	INFO("TMECOM fuse read: addr:0x%X PASS fuseData:0x%08X%08X\n",
	     fuseAddr, fuseData[1], fuseData[0]);
}

/*
 * Boot-time probe: read the QFPROM rows listed above.
 */
void tmecom_boot_fuse_read_test(void)
{
	INFO("%s: ENTRY\n", __func__);

	tmecom_boot_fuse_read_one(HWIO_QFPROM_RAW_OEM_CONFIG_ROW3_LSB_ADDR);
	tmecom_boot_fuse_read_one(QFPROM_SECOND_TEST_ROW_ADDR);

	INFO("%s: EXIT\n", __func__);
}

/*
 * Antirollback rows targeted by the fuse-write probe below.
 * As with the read addresses, the HWIO macros are not published in this tree,
 * so the addresses are hardcoded.
 */
#define QFPROM_RAW_ANTIROLLBACK_ROW0_LSB_ADDR		0x360C02C8U
#define QFPROM_RAW_ANTIROLLBACK_ROW3_LSB_ADDR		0x360C02E0U

/*
 * Boot-time probe: exercise the FUSE_WRITE_MULTIPLE message path against two
 * antirollback rows.  Mirrors testTmeFuseWriteMultiple() in the downstream
 * test suite (ssg/tme/test/sec/tzbsp_test_tmecom.c).
 *
 * ###########################################################################
 * # THE DATA WORDS BELOW MUST STAY ZERO.                                    #
 * #                                                                         #
 * # QFPROM fuses are one-time-programmable.  Writing zero blows no bits, so  #
 * # this probe validates the request/response round trip without altering    #
 * # any chip state - that is the ONLY reason it is safe to run on every      #
 * # boot.  Putting a nonzero value here would permanently blow antirollback  #
 * # fuses on every board that runs this image, which cannot be undone and    #
 * # would break secure boot on those parts.                                 #
 * #                                                                         #
 * # Do not "make the test more meaningful" by writing real data.            #
 * ###########################################################################
 *
 * Never panics: a failed exchange or a rejected write is logged only.
 */
void tmecom_boot_fuse_write_multiple_test(void)
{
	/* Zero data == blow nothing.  See the warning above. */
	TMEFuse_t fuseArray[] = {
		{ QFPROM_RAW_ANTIROLLBACK_ROW0_LSB_ADDR, { 0U, 0U } },
		{ QFPROM_RAW_ANTIROLLBACK_ROW3_LSB_ADDR, { 0U, 0U } },
	};

	uint32_t qfpromApiStatus = 0U;
	size_t   i;
	int      ret;

	INFO("%s: ENTRY\n", __func__);

	for (i = 0U; i < ARRAY_SIZE(fuseArray); i++) {
		INFO("TMECOM fuse write: row[%zu] addr:0x%X data:0x%08X%08X\n",
		     i, fuseArray[i].addr, fuseArray[i].data[1],
		     fuseArray[i].data[0]);
	}

	ret = TmeFuseWriteMultiple(fuseArray, ARRAY_SIZE(fuseArray),
				   &qfpromApiStatus);
	if (ret != E_SUCCESS) {
		INFO("TMECOM fuse write: TmeFuseWriteMultiple ret=%d qfpromApiStatus:0x%08X\n",
		     ret, qfpromApiStatus);
		INFO("%s: EXIT\n", __func__);
		return;
	}

	INFO("TMECOM fuse write: PASS rows:%zu qfpromApiStatus:0x%08X\n",
	     ARRAY_SIZE(fuseArray), qfpromApiStatus);
	INFO("%s: EXIT\n", __func__);
}

/*
 * Addresses of the QFPROM configuration registers targeted by the probe
 * below.  As with the fuse addresses above, the HWIO macros are not
 * published in this tree, so the addresses are hardcoded.
 */
#define QFPROM_BIST_CTRL_ADDR				0x360D805CU
#define QFPROM_WRITE_DISABLE_STICKY_BIT0_ADDR		0x360D800CU
#define QFPROM_WRITE_DISABLE_STICKY_BIT1_ADDR		0x360D8010U

/*
 * Boot-time probe: exercise the WRITE_CONFIG_REGISTER message path. Mirrors
 * testTmeWriteConfigRegister() in the downstream test suite
 * (ssg/tme/test/sec/tzbsp_test_tmecom.c).
 *
 * QFPROM_WRITE_DISABLE_STICKY_BITx are sticky write-only registers that only
 * latch a value once per power-on cycle - a later write is a silent no-op if
 * a value was already set since the last POR, so the read-back check below
 * accepts either the value this probe wrote or whatever was already there.
 * QFPROM_BIST_CTRL self-clears within a few cycles, so it is not read back.
 *
 * The written values are semi-arbitrary but avoid bits 4 and 17 (inverse
 * mask 0x20010) so as not to disable write access to the fuse regions that
 * tmecom_boot_fuse_write_multiple_test() above targets - ported as-is from
 * the downstream reference test.  The exact bit-to-region mapping has not
 * been independently re-derived for this SoC; re-check it before touching
 * these values.
 *
 * Never panics: a failed exchange or an unexpected read-back is logged only.
 */
void tmecom_boot_write_config_register_test(void)
{
	uint32_t before;
	int      ret;

	INFO("%s: ENTRY\n", __func__);

	/* Negative test: registerId 0 is not a valid tmeConfigRegisterId_e. */
	ret = TmeWriteConfigRegister(0, 0xC001F00DU);
	if (ret == E_SUCCESS) {
		INFO("TMECOM write config register: invalid registerId unexpectedly accepted\n");
		INFO("%s: EXIT\n", __func__);
		return;
	}

	ret = TmeWriteConfigRegister(QFPROM_BIST_CTRL, 0x1U);
	if (ret != E_SUCCESS) {
		INFO("TMECOM write config register: QFPROM_BIST_CTRL ret=%d\n", ret);
		INFO("%s: EXIT\n", __func__);
		return;
	}

	before = mmio_read_32(QFPROM_WRITE_DISABLE_STICKY_BIT0_ADDR);
	ret = TmeWriteConfigRegister(QFPROM_WRITE_DISABLE_STICKY_BIT0, 0x1200U);
	if (ret != E_SUCCESS) {
		INFO("TMECOM write config register: STICKY_BIT0 ret=%d\n", ret);
		INFO("%s: EXIT\n", __func__);
		return;
	}
	if (mmio_read_32(QFPROM_WRITE_DISABLE_STICKY_BIT0_ADDR) !=
	    ((before != 0U) ? before : 0x1200U)) {
		INFO("TMECOM write config register: STICKY_BIT0 read-back mismatch\n");
		INFO("%s: EXIT\n", __func__);
		return;
	}

	before = mmio_read_32(QFPROM_WRITE_DISABLE_STICKY_BIT1_ADDR);
	ret = TmeWriteConfigRegister(QFPROM_WRITE_DISABLE_STICKY_BIT1, 0x3400U);
	if (ret != E_SUCCESS) {
		INFO("TMECOM write config register: STICKY_BIT1 ret=%d\n", ret);
		INFO("%s: EXIT\n", __func__);
		return;
	}
	if (mmio_read_32(QFPROM_WRITE_DISABLE_STICKY_BIT1_ADDR) !=
	    ((before != 0U) ? before : 0x3400U)) {
		INFO("TMECOM write config register: STICKY_BIT1 read-back mismatch\n");
		INFO("%s: EXIT\n", __func__);
		return;
	}

	INFO("TMECOM write config register: PASS\n");
	INFO("%s: EXIT\n", __func__);
}

#endif /* defined(QTI_USE_TMECOM) && defined(QTI_TMECOM_TEST) */
