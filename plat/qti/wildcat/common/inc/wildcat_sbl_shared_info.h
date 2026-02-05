/*
 * Copyright (c) 2026 Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef WILDCAT_SBL_SHARED_INFO_H
#define WILDCAT_SBL_SHARED_INFO_H

#include <stdbool.h>
#include <stdint.h>

#include <platform_def.h>

#define MAX_SOC_VERS 12
#define MAX_SERIAL_NUM 8
#define SECBOOT_IMAGE_HASH_SIZE (48)

struct secboot_pbl_profile_marker {
	uint32_t secboot_pbl_authenticate_init_entry;
	uint32_t secboot_pbl_parse_cert_buffer;
	uint32_t secboot_pbl_hash_certificates;
	uint32_t secboot_pbl_verify_cert_chain;
	uint32_t secboot_pbl_verify_image_signature;
	uint32_t secboot_pbl_authenticate_close;
};

struct secboot_metadata_type {
	uint32_t major_version;
	uint32_t minor_version;
	uint32_t sw_id;
	uint32_t hw_id;
	uint32_t oem_id;
	uint32_t model_id;
	uint32_t secondary_sw_id;
	uint32_t flags;
	uint32_t soc_vers[MAX_SOC_VERS];
	uint32_t serial_num[MAX_SERIAL_NUM];
	uint32_t root_cert_sel;
	uint32_t anti_rollback;
};

struct secboot_code_hash_info_type {
	uintptr_t code_address;
	uint32_t code_length;
	uint32_t image_hash_length;
	uint8_t image_hash[SECBOOT_IMAGE_HASH_SIZE];
};

struct secboot_verified_info_type {
	uint32_t version_id;
	uint64_t sw_id;
	uint32_t enable_debug;
	struct secboot_code_hash_info_type image_hash_info;
	uint32_t enable_root_key_control;
	struct secboot_metadata_type metadata;
	struct secboot_pbl_profile_marker secboot_timestamps;
};

struct boot_images_entry {
	uint32_t image_id;
	uint32_t e_ident;
	uint64_t entry_point;
	struct secboot_verified_info_type image_verified_info;
	uint32_t reserved_1;
	uint32_t reserved_2;
	uint32_t reserved_3;
	uint32_t reserved_4;
};

struct boot_tfabl31_interface {
	uint32_t magic_1;
	uint32_t magic_2;
	uint32_t version;
	uint32_t number_images;
	uint32_t reset_required;
	struct boot_images_entry boot_image_entry[BOOT_IMAGES_NUM_ENTRIES];
	uint64_t ddr_enter_self_refresh;
	uint64_t ddr_exit_self_refresh;
	uint32_t exit_image_index;
	uint8_t exit_image_privilege_level;
	uint8_t reserved_1;
	uint16_t reserved_2;
};

/*
 * Returns a pointer to the boot_tfabl31_interface structure.
 * This structure contains information about images
 * authenticated by boot loader and its entry point
 * The returned pointer should be treated as read-only.
 */
const struct boot_tfabl31_interface *get_sbl_tfabl31_interface(void);

#endif /* WILDCAT_SBL_SHARED_INFO_H */
