/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SMEM_TARGET_H
#define SMEM_TARGET_H

#include <stdint.h>

/*
 * Identifier for the SMEM target info struct.
 * "SIII" in little-endian.
 */
#define SMEM_TARG_INFO_IDENTIFIER	0x49494953

/* SMEM hosts supported identifier field */
#define SMEM_TARG_INFO_HOSTS_IDENTIFIER	((uint16_t)0x4853) /* SH */

/*
 * The structure that is filled in by smem_boot_init() to
 * maintain a single source that determines target information like the SMEM
 * base physical address and size. All images then use these values to set the
 * base virtual address pointer at the beginning of smem_init().
 *
 * Note: We cannot move these fields, and this struct must grow towards greater
 * addresses in order to support interprocessor versioning. Badger only has a
 * set 16-Bytes of reserved space, so any fields added to this struct cannot
 * be used on Badger targets.
 */
struct __packed smem_targ_info {
	uint32_t identifier;
	uint32_t smem_size;
	uint64_t smem_base_phys_addr;
	uint16_t smem_max_items;
	uint16_t smem_num_tlvs;
};

struct __packed smem_targ_info_tlv {
	uint16_t tlv_type;
	uint16_t tlv_length;
	uint8_t tlv_val[];
};

/* SMEM Target Info TLV: SMEM Hosts */
struct __packed smem_targ_info_tlv_hosts {
	/* SMEM_TARG_INFO_HOSTS_IDENTIFIER */
	uint16_t tlv_type;
	/* Number of SMEM hosts * sizeof(uint16_t) */
	uint16_t tlv_length;
	/* SMEM Hosts */
	uint16_t hosts[];
};

/* SMEM This host target information */
struct smem_targ_host_info {
	uint16_t this_host;
	uint64_t phys_addr;
	uint32_t size;
	uint64_t targ_info_phy_addr;
	uint16_t max_items;
	uint8_t version_offset;
	struct smem_targ_info *targ_info;
};

/*
 * Target specific initialization and returns SMEM information
 */
struct smem_targ_host_info *smem_targ_init(void);

#endif /* SMEM_TARGET_H */
