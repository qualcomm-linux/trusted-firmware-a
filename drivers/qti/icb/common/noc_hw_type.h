/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef QTI_NOC_HW_TYPE_H
#define QTI_NOC_HW_TYPE_H

#include <cdefs.h>

#include "noc_error.h"

static struct noc_hw QNOC_4_0 = {
	.swid_low       = 0x0,
	.swid_high      = 0x4,
	.main_ctl_low   = 0x8,
	.err_valid_low  = 0x10,
	.err_clear_low  = 0x18,
	.errlog0_low    = 0x20,
	.errlog0_high   = 0x24,
	.errlog1_low    = 0x28,
	.errlog1_high   = 0x2C,
	.errlog2_low    = 0x30,
	.errlog2_high   = 0x34,
	.errlog3_low    = 0x38,
	.errlog3_high   = 0x3C,
	.errlog2_1_low  = REGISTER_NOT_APPLICABLE,
	.errlog2_1_high = REGISTER_NOT_APPLICABLE,
	.errlog4_3_low  = REGISTER_NOT_APPLICABLE,
	.errlog4_3_high = REGISTER_NOT_APPLICABLE,
	.errlog6_5_low  = REGISTER_NOT_APPLICABLE,
	.errlog6_5_high = REGISTER_NOT_APPLICABLE,
	.errlog8_high   = REGISTER_NOT_APPLICABLE,
};

/* NOC HW Register Offsets Gladiator Version (use ERRORVLD_LOW as base) */
static struct noc_hw GLADIATOR_0_0 __maybe_unused = {
	.swid_low       = REGISTER_NOT_APPLICABLE,
	.swid_high      = REGISTER_NOT_APPLICABLE,
	.main_ctl_low   = 0x0,  /* gladiator errvld_low */
	.err_valid_low  = 0x4,  /* gladiator errvld_high */
	.err_clear_low  = 0x8,  /* gladiator errlog0_low */
	.errlog0_low    = REGISTER_NOT_APPLICABLE,
	.errlog0_high   = 0xC,
	.errlog1_low    = REGISTER_NOT_APPLICABLE,
	.errlog1_high   = REGISTER_NOT_APPLICABLE,
	.errlog2_low    = REGISTER_NOT_APPLICABLE,
	.errlog2_high   = REGISTER_NOT_APPLICABLE,
	.errlog3_low    = REGISTER_NOT_APPLICABLE,
	.errlog3_high   = REGISTER_NOT_APPLICABLE,
	.errlog2_1_low  = 0x10,
	.errlog2_1_high = 0x14,
	.errlog4_3_low  = 0x18,
	.errlog4_3_high = 0x1C,
	.errlog6_5_low  = 0x20,
	.errlog6_5_high = 0x24,
	.errlog8_high   = 0x2C,
};

/* NOC SidebandManager HW Register Offsets */
static struct noc_sideband_hw QNOC_SB_4_0_L = {
	.swid_low             = 0x0,
	.swid_high            = 0x4,
	.faultin_en0_low      = 0x40,
	.faultin_en0_high     = REGISTER_NOT_APPLICABLE,
	.faultin_status0_low  = 0x48,
	.faultin_status0_high = REGISTER_NOT_APPLICABLE,
	.faultin_en1_low      = REGISTER_NOT_APPLICABLE,
	.faultin_en1_high     = REGISTER_NOT_APPLICABLE,
	.faultin_status1_low  = REGISTER_NOT_APPLICABLE,
	.faultin_status1_high = REGISTER_NOT_APPLICABLE,
	.faultin_en2_low      = REGISTER_NOT_APPLICABLE,
	.faultin_en2_high     = REGISTER_NOT_APPLICABLE,
	.faultin_status2_low  = REGISTER_NOT_APPLICABLE,
	.faultin_status2_high = REGISTER_NOT_APPLICABLE,
};

static struct noc_sideband_hw QNOC_SB_4_0 __maybe_unused = {
	.swid_low             = 0x0,
	.swid_high            = 0x4,
	.faultin_en0_low      = 0x40,
	.faultin_en0_high     = 0x44,
	.faultin_status0_low  = 0x48,
	.faultin_status0_high = 0x4C,
	.faultin_en1_low      = REGISTER_NOT_APPLICABLE,
	.faultin_en1_high     = REGISTER_NOT_APPLICABLE,
	.faultin_status1_low  = REGISTER_NOT_APPLICABLE,
	.faultin_status1_high = REGISTER_NOT_APPLICABLE,
	.faultin_en2_low      = REGISTER_NOT_APPLICABLE,
	.faultin_en2_high     = REGISTER_NOT_APPLICABLE,
	.faultin_status2_low  = REGISTER_NOT_APPLICABLE,
	.faultin_status2_high = REGISTER_NOT_APPLICABLE,
};

static struct noc_sideband_hw QNOC_SB_4_0_2L = {
	.swid_low             = 0x0,
	.swid_high            = 0x4,
	.faultin_en0_low      = 0x40,
	.faultin_en0_high     = 0x44,
	.faultin_status0_low  = 0x48,
	.faultin_status0_high = 0x4C,
	.faultin_en1_low      = 0x50,
	.faultin_en1_high     = REGISTER_NOT_APPLICABLE,
	.faultin_status1_low  = 0x58,
	.faultin_status1_high = REGISTER_NOT_APPLICABLE,
	.faultin_en2_low      = REGISTER_NOT_APPLICABLE,
	.faultin_en2_high     = REGISTER_NOT_APPLICABLE,
	.faultin_status2_low  = REGISTER_NOT_APPLICABLE,
	.faultin_status2_high = REGISTER_NOT_APPLICABLE,
};

static struct noc_sideband_hw QNOC_SB_4_0_2 __maybe_unused = {
	.swid_low             = 0x0,
	.swid_high            = 0x4,
	.faultin_en0_low      = 0x40,
	.faultin_en0_high     = 0x44,
	.faultin_status0_low  = 0x48,
	.faultin_status0_high = 0x4C,
	.faultin_en1_low      = 0x50,
	.faultin_en1_high     = 0x54,
	.faultin_status1_low  = 0x58,
	.faultin_status1_high = 0x5C,
	.faultin_en2_low      = REGISTER_NOT_APPLICABLE,
	.faultin_en2_high     = REGISTER_NOT_APPLICABLE,
	.faultin_status2_low  = REGISTER_NOT_APPLICABLE,
	.faultin_status2_high = REGISTER_NOT_APPLICABLE,
};

/* FAULTIN2 Sideband Manager - Low only */
static struct noc_sideband_hw QNOC_SB_4_0_3L __maybe_unused = {
	.swid_low             = 0x0,
	.swid_high            = 0x4,
	.faultin_en0_low      = 0x40,
	.faultin_en0_high     = 0x44,
	.faultin_status0_low  = 0x48,
	.faultin_status0_high = 0x4C,
	.faultin_en1_low      = 0x50,
	.faultin_en1_high     = 0x54,
	.faultin_status1_low  = 0x58,
	.faultin_status1_high = 0x5C,
	.faultin_en2_low      = 0x60,
	.faultin_en2_high     = REGISTER_NOT_APPLICABLE,
	.faultin_status2_low  = 0x68,
	.faultin_status2_high = REGISTER_NOT_APPLICABLE,
};

/* FAULTIN2 Sideband Manager - Full */
static struct noc_sideband_hw QNOC_SB_4_0_3 __maybe_unused = {
	.swid_low             = 0x0,
	.swid_high            = 0x4,
	.faultin_en0_low      = 0x40,
	.faultin_en0_high     = 0x44,
	.faultin_status0_low  = 0x48,
	.faultin_status0_high = 0x4C,
	.faultin_en1_low      = 0x50,
	.faultin_en1_high     = 0x54,
	.faultin_status1_low  = 0x58,
	.faultin_status1_high = 0x5C,
	.faultin_en2_low      = 0x60,
	.faultin_en2_high     = 0x64,
	.faultin_status2_low  = 0x68,
	.faultin_status2_high = 0x6C,
};

/* NOC MSI Encoder HW Register Offsets */
static struct noc_msi_hw QNOC_MSI_1_0 __maybe_unused = {
	.msienc_swid_low     = 0x0,
	.msienc_swid_high    = 0x4,
	.msienc_errorset_low = 0x8,
	.msienc_errorsts_low = 0x10,
	.msienc_errorclr_low = 0x18,
	.msienc_errlog0_low  = 0x20,
	.msienc_errlog0_high = 0x24,
	.msienc_errlog1_low  = 0x28,
	.msienc_errlog1_high = 0x2C,
	.msienc_errlog2_low  = 0x30,
	.msienc_errlog2_high = 0x34,
};

/* NOC Point-Of-Serialization HW Register Offsets */
static struct noc_pos_hw QNOC_POS_4_0 __maybe_unused = {
	.swid_low      = 0x0,
	.swid_high     = 0x4,
	.errlog_low    = 0x8,
	.errlog_high   = 0xC,
	.errlogclr_low = 0x10,
};

/* NOC Point-Of-Coherency HW Register Offsets - no errloguser */
static struct noc_poc_hw QNOC_POC_1_0_L __maybe_unused = {
	.swid_low        = 0x00,
	.swid_high       = 0x04,
	.errset_low      = 0x08,
	.errstatus_low   = 0x10,
	.errack_low      = 0x18,
	.errlogmain_low  = 0x20,
	.errlogmain_high = 0x24,
	.errlogaddr_low  = 0x28,
	.errlogaddr_high = 0x2C,
	.errloguser_low  = REGISTER_NOT_APPLICABLE,
	.errloguser_high = REGISTER_NOT_APPLICABLE,
	.errlogmisc_low  = REGISTER_NOT_APPLICABLE,
	.errlogmisc_high = REGISTER_NOT_APPLICABLE,
};

/* NOC Point-Of-Coherency HW Register Offsets - with errloguser */
static struct noc_poc_hw QNOC_POC_1_0 __maybe_unused = {
	.swid_low        = 0x00,
	.swid_high       = 0x04,
	.errset_low      = 0x08,
	.errstatus_low   = 0x10,
	.errack_low      = 0x18,
	.errlogmain_low  = 0x20,
	.errlogmain_high = 0x24,
	.errlogaddr_low  = 0x28,
	.errlogaddr_high = 0x2C,
	.errloguser_low  = 0x30,
	.errloguser_high = 0x34,
	.errlogmisc_low  = REGISTER_NOT_APPLICABLE,
	.errlogmisc_high = REGISTER_NOT_APPLICABLE,
};

/* NOC Point-Of-Coherency HW Register Offsets - HSCNOC V2 (errlogmisc, no errloguser) */
static struct noc_poc_hw QNOC_POC_1_0_HSCNOC_V2 __maybe_unused = {
	.swid_low        = 0x00,
	.swid_high       = 0x04,
	.errset_low      = 0x08,
	.errstatus_low   = 0x10,
	.errack_low      = 0x18,
	.errlogmain_low  = 0x20,
	.errlogmain_high = 0x24,
	.errlogaddr_low  = 0x28,
	.errlogaddr_high = 0x2C,
	.errloguser_low  = REGISTER_NOT_APPLICABLE,
	.errloguser_high = REGISTER_NOT_APPLICABLE,
	.errlogmisc_low  = 0x30,
	.errlogmisc_high = 0x34,
};

/* NOC Safety Manager HW Register Offsets */
static struct noc_sfty_ctl_hw QNOC_SFTY_CTL_1_0 __maybe_unused = {
	.swid_low    = 0x00,
	.swid_high   = 0x04,
	.bistctl_low = 0x08,
	.status_low  = 0x10,
	.outen_low   = 0x18,
	.cflta_low   = 0x20,
	.cflta_high  = 0x24,
	.uflta_low   = 0x28,
	.uflta_high  = 0x2C,
	.cfltb_low   = REGISTER_NOT_APPLICABLE,
	.cfltb_high  = REGISTER_NOT_APPLICABLE,
	.ufltb_low   = REGISTER_NOT_APPLICABLE,
	.ufltb_high  = REGISTER_NOT_APPLICABLE,
	.cfltc_low   = REGISTER_NOT_APPLICABLE,
	.cfltc_high  = REGISTER_NOT_APPLICABLE,
	.ufltc_low   = REGISTER_NOT_APPLICABLE,
	.ufltc_high  = REGISTER_NOT_APPLICABLE,
};

static struct noc_sfty_ctl_hw QNOC_SFTY_CTL_1_0_L __maybe_unused = {
	.swid_low    = 0x00,
	.swid_high   = 0x04,
	.bistctl_low = 0x08,
	.status_low  = 0x10,
	.outen_low   = 0x18,
	.cflta_low   = 0x20,
	.cflta_high  = REGISTER_NOT_APPLICABLE,
	.uflta_low   = 0x28,
	.uflta_high  = REGISTER_NOT_APPLICABLE,
	.cfltb_low   = REGISTER_NOT_APPLICABLE,
	.cfltb_high  = REGISTER_NOT_APPLICABLE,
	.ufltb_low   = REGISTER_NOT_APPLICABLE,
	.ufltb_high  = REGISTER_NOT_APPLICABLE,
	.cfltc_low   = REGISTER_NOT_APPLICABLE,
	.cfltc_high  = REGISTER_NOT_APPLICABLE,
	.ufltc_low   = REGISTER_NOT_APPLICABLE,
	.ufltc_high  = REGISTER_NOT_APPLICABLE,
};

static struct noc_sfty_ctl_hw QNOC_SFTY_CTL_1_0_B __maybe_unused = {
	.swid_low    = 0x00,
	.swid_high   = 0x04,
	.bistctl_low = 0x08,
	.status_low  = 0x10,
	.outen_low   = 0x18,
	.cflta_low   = 0x20,
	.cflta_high  = 0x24,
	.uflta_low   = 0x28,
	.uflta_high  = 0x2C,
	.cfltb_low   = 0x30,
	.cfltb_high  = REGISTER_NOT_APPLICABLE,
	.ufltb_low   = 0x38,
	.ufltb_high  = REGISTER_NOT_APPLICABLE,
	.cfltc_low   = REGISTER_NOT_APPLICABLE,
	.cfltc_high  = REGISTER_NOT_APPLICABLE,
	.ufltc_low   = REGISTER_NOT_APPLICABLE,
	.ufltc_high  = REGISTER_NOT_APPLICABLE,
};

static struct noc_sfty_ctl_hw QNOC_SFTY_CTL_1_0_BH __maybe_unused = {
	.swid_low    = 0x00,
	.swid_high   = 0x04,
	.bistctl_low = 0x08,
	.status_low  = 0x10,
	.outen_low   = 0x18,
	.cflta_low   = 0x20,
	.cflta_high  = 0x24,
	.uflta_low   = 0x28,
	.uflta_high  = 0x2C,
	.cfltb_low   = 0x30,
	.cfltb_high  = 0x34,
	.ufltb_low   = 0x38,
	.ufltb_high  = 0x3C,
	.cfltc_low   = REGISTER_NOT_APPLICABLE,
	.cfltc_high  = REGISTER_NOT_APPLICABLE,
	.ufltc_low   = REGISTER_NOT_APPLICABLE,
	.ufltc_high  = REGISTER_NOT_APPLICABLE,
};

static struct noc_sfty_ctl_hw QNOC_SFTY_CTL_1_0_2B __maybe_unused = {
	.swid_low    = 0x00,
	.swid_high   = 0x04,
	.bistctl_low = 0x08,
	.status_low  = 0x10,
	.outen_low   = 0x18,
	.cflta_low   = 0x20,
	.cflta_high  = 0x24,
	.uflta_low   = 0x28,
	.uflta_high  = 0x2C,
	.cfltb_low   = 0x30,
	.cfltb_high  = 0x34,
	.ufltb_low   = 0x38,
	.ufltb_high  = 0x3C,
	.cfltc_low   = 0x40,
	.cfltc_high  = 0x44,
	.ufltc_low   = 0x48,
	.ufltc_high  = 0x4C,
};

/* NOC Safety Aggregator HW Register Offsets */
static struct noc_sfty_agg_hw QNOC_SFTY_AGG_1_0 __maybe_unused = {
	.swid_low    = 0x00,
	.swid_high   = 0x04,
	.mainctl_low = 0x08,
	.status_low  = 0x10,
	.ccnt_low    = 0x18,
	.ucnt_low    = 0x20,
	.min_version = CHIPINFO_VERSION(0, 0),
	.max_version = CHIPINFO_VERSION(0, 0),
};

static struct noc_sfty_agg_hw QNOC_SFTY_AGG_V1_1_0 __maybe_unused = {
	.swid_low    = 0x00,
	.swid_high   = 0x04,
	.mainctl_low = 0x08,
	.status_low  = 0x10,
	.ccnt_low    = 0x18,
	.ucnt_low    = 0x20,
	.min_version = CHIPINFO_VERSION(0, 0),
	.max_version = CHIPINFO_VERSION(2, 0),
};

static struct noc_sfty_agg_hw QNOC_SFTY_AGG_V2_1_0 __maybe_unused = {
	.swid_low    = 0x00,
	.swid_high   = 0x04,
	.mainctl_low = 0x08,
	.status_low  = 0x10,
	.ccnt_low    = 0x18,
	.ucnt_low    = 0x20,
	.min_version = CHIPINFO_VERSION(2, 0),
	.max_version = CHIPINFO_VERSION(3, 0),
};

/* Safety Aggregators with no correctable counter (XC variants) */
static struct noc_sfty_agg_hw QNOC_SFTY_AGG_1_0_XC __maybe_unused = {
	.swid_low    = 0x00,
	.swid_high   = 0x04,
	.mainctl_low = 0x08,
	.status_low  = 0x10,
	.ccnt_low    = REGISTER_NOT_APPLICABLE,
	.ucnt_low    = 0x20,
	.min_version = CHIPINFO_VERSION(0, 0),
	.max_version = CHIPINFO_VERSION(0, 0),
};

static struct noc_sfty_agg_hw QNOC_SFTY_AGG_V1_1_0_XC __maybe_unused = {
	.swid_low    = 0x00,
	.swid_high   = 0x04,
	.mainctl_low = 0x08,
	.status_low  = 0x10,
	.ccnt_low    = REGISTER_NOT_APPLICABLE,
	.ucnt_low    = 0x20,
	.min_version = CHIPINFO_VERSION(0, 0),
	.max_version = CHIPINFO_VERSION(2, 0),
};

static struct noc_sfty_agg_hw QNOC_SFTY_AGG_V2_1_0_XC __maybe_unused = {
	.swid_low    = 0x00,
	.swid_high   = 0x04,
	.mainctl_low = 0x08,
	.status_low  = 0x10,
	.ccnt_low    = REGISTER_NOT_APPLICABLE,
	.ucnt_low    = 0x20,
	.min_version = CHIPINFO_VERSION(2, 0),
	.max_version = CHIPINFO_VERSION(3, 0),
};

#endif /* QTI_NOC_HW_TYPE_H */
