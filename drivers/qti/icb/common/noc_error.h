/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef QTI_NOC_ERROR_H
#define QTI_NOC_ERROR_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define CHIPINFO_VERSION(major, minor)  (((major) << 16) | (minor))
#define BIT_FLAG(x) ((uint32_t)(1 << x))
#define REGISTER_NOT_APPLICABLE 0xFFFF
#define NO_INTERRUPT UINTPTR_MAX

/* Error code construction */
#define MODULE_BASE_RSVD                 0
#define DEF_MODULE(ID)                   (MODULE_BASE_RSVD + ID)
#define KRNL                             DEF_MODULE(0x07)

#define EC_MSK_NEGATIVE_BIT_SHFT         (31)
#define EC_MSK_RSVD_BIT_SHFT             (30)
#define EC_MSK_MODULE_BIT_SHFT           (20)
#define EC_MSK_NEGATIVE_MSK              (UINT32_C(0x1) << EC_MSK_NEGATIVE_BIT_SHFT)
#define EC_MSK_RSVD_MSK                  (UINT32_C(0x1) << EC_MSK_RSVD_BIT_SHFT)
#define EC_MSK_MODULE_MSK                (UINT32_C(0x3FF) << EC_MSK_MODULE_BIT_SHFT)
#define EC_MSK_CODE_MSK                  (0xFFFFF)

#define TFA_ERR_CODE(EC_MODULE, CODE) \
	(((EC_MSK_MODULE_MSK & (EC_MODULE << EC_MSK_MODULE_BIT_SHFT)) | \
	  (EC_MSK_CODE_MSK & (CODE))) | \
	 (EC_MSK_RSVD_MSK & (MODULE_BASE_RSVD << EC_MSK_RSVD_BIT_SHFT)))

#define NOC_ERR_FATAL_SYNDROME_REG       TFA_ERR_CODE(KRNL, 572)
#define NOC_FAULT_NAME_SBMS              TFA_ERR_CODE(KRNL, 573)
#define NOC_FAULT_NAME_MSI               TFA_ERR_CODE(KRNL, 574)
#define NOC_POS_NAME_SYNDROME_REG        TFA_ERR_CODE(KRNL, 628)
#define NOC_POC_NAME_SYNDROME_REG        TFA_ERR_CODE(KRNL, 913)
#define NOC_SFTY_NAME_SYNDROME_REG       TFA_ERR_CODE(KRNL, 914)

struct noc_hw {
	uint16_t swid_low;
	uint16_t swid_high;
	uint16_t main_ctl_low;
	uint16_t err_valid_low;
	uint16_t err_clear_low;
	uint16_t errlog0_low;
	uint16_t errlog0_high;
	uint16_t errlog1_low;
	uint16_t errlog1_high;
	uint16_t errlog2_low;
	uint16_t errlog2_high;
	uint16_t errlog3_low;
	uint16_t errlog3_high;
	uint16_t errlog2_1_low;
	uint16_t errlog2_1_high;
	uint16_t errlog4_3_low;
	uint16_t errlog4_3_high;
	uint16_t errlog6_5_low;
	uint16_t errlog6_5_high;
	uint16_t errlog8_high;
};

struct noc_sideband_hw {
	uint16_t swid_low;
	uint16_t swid_high;
	uint16_t faultin_en0_low;
	uint16_t faultin_en0_high;
	uint16_t faultin_status0_low;
	uint16_t faultin_status0_high;
	uint16_t faultin_en1_low;
	uint16_t faultin_en1_high;
	uint16_t faultin_status1_low;
	uint16_t faultin_status1_high;
	uint16_t faultin_en2_low;
	uint16_t faultin_en2_high;
	uint16_t faultin_status2_low;
	uint16_t faultin_status2_high;
};

struct noc_pos_hw {
	uint16_t swid_low;
	uint16_t swid_high;
	uint16_t errlog_low;
	uint16_t errlog_high;
	uint16_t errlogclr_low;
};

struct noc_poc_hw {
	uint16_t swid_low;
	uint16_t swid_high;
	uint16_t errset_low;
	uint16_t errstatus_low;
	uint16_t errack_low;
	uint16_t errlogmain_low;
	uint16_t errlogmain_high;
	uint16_t errlogaddr_low;
	uint16_t errlogaddr_high;
	uint16_t errloguser_low;
	uint16_t errloguser_high;
	uint16_t errlogmisc_low;
	uint16_t errlogmisc_high;
};

struct nocerr_sbm_syndrome {
	uint32_t FAULTINSTATUS0_LOW;
	uint32_t FAULTINSTATUS0_HIGH;
	uint32_t FAULTINSTATUS1_LOW;
	uint32_t FAULTINSTATUS1_HIGH;
	uint32_t FAULTINSTATUS2_LOW;
	uint32_t FAULTINSTATUS2_HIGH;
};

struct nocerr_pos_syndrome {
	uint32_t ERRLOG_LOW;
	uint32_t ERRLOG_HIGH;
};

struct nocerr_poc_syndrome {
	uint32_t ERRLOGSTATUS_LOW;
	uint32_t ERRLOGMAIN_LOW;
	uint32_t ERRLOGMAIN_HIGH;
	uint32_t ERRLOGADDR_LOW;
	uint32_t ERRLOGADDR_HIGH;
	uint32_t ERRLOGUSER_LOW;
	uint32_t ERRLOGUSER_HIGH;
	uint32_t ERRLOGMISC_LOW;
	uint32_t ERRLOGMISC_HIGH;
};

struct nocerr_sfty_ctl_syndrome {
	uint32_t STATUS_LOW;
	uint32_t CFLTA_LOW;
	uint32_t CFLTA_HIGH;
	uint32_t UFLTA_LOW;
	uint32_t UFLTA_HIGH;
	uint32_t CFLTB_LOW;
	uint32_t CFLTB_HIGH;
	uint32_t UFLTB_LOW;
	uint32_t UFLTB_HIGH;
	uint32_t CFLTC_LOW;
	uint32_t CFLTC_HIGH;
	uint32_t UFLTC_LOW;
	uint32_t UFLTC_HIGH;
};

struct nocerr_sfty_agg_syndrome {
	uint32_t STATUS_LOW;
	uint32_t CCNT_LOW;
	uint32_t UCNT_LOW;
};

struct nocerr_msi_syndrome {
	uint32_t MSIENC_ERRLOG0_LOW;
	uint32_t MSIENC_ERRLOG0_HIGH;
	uint32_t MSIENC_ERRLOG1_LOW;
	uint32_t MSIENC_ERRLOG1_HIGH;
	uint32_t MSIENC_ERRLOG2_LOW;
	uint32_t MSIENC_ERRLOG2_HIGH;
};

struct nocerr_syndrome {
	uint32_t ERRLOG0_LOW;
	uint32_t ERRLOG0_HIGH;
	uint32_t ERRLOG1_LOW;
	uint32_t ERRLOG1_HIGH;
	uint32_t ERRLOG2_LOW;
	uint32_t ERRLOG2_HIGH;
	uint32_t ERRLOG3_LOW;
	uint32_t ERRLOG3_HIGH;
	uint32_t ERRLOG2_1_LOW;
	uint32_t ERRLOG2_1_HIGH;
	uint32_t ERRLOG4_3_LOW;
	uint32_t ERRLOG4_3_HIGH;
	uint32_t ERRLOG6_5_LOW;
	uint32_t ERRLOG6_5_HIGH;
	uint32_t ERRLOG8_HIGH;
	struct nocerr_sbm_syndrome *sbms;
	struct nocerr_pos_syndrome *pos;
	struct nocerr_poc_syndrome *poc;
	struct nocerr_sfty_ctl_syndrome *sfty_ctl;
	struct nocerr_sfty_agg_syndrome *sfty_agg;
	struct nocerr_msi_syndrome *msis;
};

struct noc_msi_hw {
	uint16_t msienc_swid_low;
	uint16_t msienc_swid_high;
	uint16_t msienc_errorset_low;
	uint16_t msienc_errorsts_low;
	uint16_t msienc_errorclr_low;
	uint16_t msienc_errlog0_low;
	uint16_t msienc_errlog0_high;
	uint16_t msienc_errlog1_low;
	uint16_t msienc_errlog1_high;
	uint16_t msienc_errlog2_low;
	uint16_t msienc_errlog2_high;
};

struct msi_info {
	uint32_t           num_msis;
	struct noc_msi_hw **msi_hw;
	void             **msi_base_addrs;
};

struct noc_sfty_ctl_hw {
	uint16_t swid_low;
	uint16_t swid_high;
	uint16_t bistctl_low;
	uint16_t status_low;
	uint16_t outen_low;
	uint16_t cflta_low;
	uint16_t cflta_high;
	uint16_t uflta_low;
	uint16_t uflta_high;
	uint16_t cfltb_low;
	uint16_t cfltb_high;
	uint16_t ufltb_low;
	uint16_t ufltb_high;
	uint16_t cfltc_low;
	uint16_t cfltc_high;
	uint16_t ufltc_low;
	uint16_t ufltc_high;
};

struct noc_sfty_agg_hw {
	uint16_t swid_low;
	uint16_t swid_high;
	uint16_t mainctl_low;
	uint16_t status_low;
	uint16_t ccnt_low;
	uint16_t ucnt_low;
	uint32_t min_version;
	uint32_t max_version;
};

struct noc_sfty_agg {
	void                   *addr;
	struct noc_sfty_agg_hw *hw;
	uint32_t                aggregid;
};

struct noc_qtv {
	uint32_t qultivate_part_type;
	uint32_t idx;
};

struct nocerr_info {
	char                    *name;
	struct noc_hw           *hw;
	void                    *base_addr;
	uintptr_t                intr_vector;
	uint32_t                 num_sbms;
	struct noc_sideband_hw **sb_hw;
	void                   **sb_base_addrs;
	uint32_t                 num_tos;
	void                   **to_addrs;
	struct nocerr_syndrome   syndrome;
	uint32_t                 num_pos;
	struct noc_pos_hw      **pos_hw;
	void                   **pos_base_addrs;
	uint32_t                 num_poc;
	struct noc_poc_hw      **poc_hw;
	void                   **poc_base_addrs;
	uint32_t                 num_sfty_ctl;
	struct noc_sfty_ctl_hw **sfty_ctl_hw;
	void                   **sfty_ctl_addrs;
	uint32_t                *num_sfty_aggs;
	struct noc_sfty_agg    **sfty_aggs;
	uint32_t                 num_qultivate_parts;
	struct noc_qtv          *qultivate_parts;
	bool                     is_part_disabled;
	void                    *summary_intr_enable_addr;
	void                    *summary_intr_status_addr;
	uint32_t                 sfty_source;
	struct msi_info         *msi_info;
};

struct nocerr_sbm_info_oem {
	uint32_t faultin_en0_low;
	uint32_t faultin_en0_high;
	uint32_t faultin_en1_low;
	uint32_t faultin_en1_high;
	uint32_t faultin_en2_low;
	uint32_t faultin_en2_high;
};

struct nocerr_pos_info_oem {
	bool enable;
};

struct nocerr_sfty_ctl_info_oem {
	uint32_t outen_low;
};

struct nocerr_info_oem {
	char                            *name;
	bool                             intr_enable;
	bool                             error_fatal;
	struct nocerr_sbm_info_oem      *sbms;
	struct nocerr_sbm_info_oem      *obs_mask;
	uint32_t                        *to_reg_vals;
	struct nocerr_pos_info_oem      *pos;
	struct nocerr_pos_info_oem      *poc;
	struct nocerr_sfty_ctl_info_oem *sfty_ctl;
	uint32_t                         summary_intr_enable_bit_set;
};

struct nocerr_filter {
	uint32_t  num_extids;
	uint32_t *extids;
	uint32_t  num_errcodes;
	uint32_t *errcodes;
	bool      non_fatal;
	bool      delay_fatal;
	bool      is_smp2p;
};

struct nocerr_filter_oem {
	bool enable;
	bool delay_fatal;
};

struct nocerr_propdata {
	uint32_t             family;
	bool                 match;
	uint32_t             version;
	uint32_t             len;
	struct nocerr_info  *noc_info_list;
	uint32_t             num_clock_regs;
	void               **clock_reg_addrs;
	uint32_t             num_filters;
	struct nocerr_filter *filters;
	uint8_t  *reg_addr;
	uint32_t  reg_mask;
	uint32_t  reg_val;
};

struct nocerr_config_info {
	uint32_t                num_configs;
	struct nocerr_propdata *configs;
};

struct nocerr_propdata_oem {
	uint32_t                  family;
	bool                      match;
	uint32_t                  version;
	uint32_t                  len;
	struct nocerr_info_oem   *noc_info_oem_list;
	uint32_t                 *clock_reg_vals;
	struct nocerr_filter_oem *filters;
	uint8_t  *reg_addr;
	uint32_t  reg_mask;
	uint32_t  reg_val;
};

struct nocerr_config_info_oem {
	uint32_t                    num_configs;
	struct nocerr_propdata_oem *configs;
};

#endif /* QTI_NOC_ERROR_H */
