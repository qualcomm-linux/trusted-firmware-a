/**********************************************************************
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause
 *********************************************************************/

#ifndef QTI_SBL_SHARE_INFO_H
#define QTI_SBL_SHARE_INFO_H

#include <stdbool.h>
#include <stdint.h>

#include <platform_def.h>

#define MAX_SOC_VERS 12	/* MAX number for supported soc hardware version */
#define MAX_SERIAL_NUM 8	/* MAX number for supported serial number */
#define SECBOOT_IMAGE_HASH_SIZE (48)

typedef struct _secboot_pbl_profile_marker {
	/*
	 * Start of secboot_authenticate entry.
	 */
	uint32_t secboot_pbl_authenticate_init_entry;
	/*
	 * Start of parse_cert_buffer.
	 */
	uint32_t secboot_pbl_parse_cert_buffer;
	/*
	 * Start of hash certificates.
	 */
	uint32_t secboot_pbl_hash_certificates;
	/*
	 * Start of verify cert chain.
	 */
	uint32_t secboot_pbl_verify_cert_chain;
	/*
	 * Start of verify image signature.
	 */
	uint32_t secboot_pbl_verify_image_signature;
	/*
	 * secboot_authenticate is complete.
	 */
	uint32_t secboot_pbl_authenticate_close;
} secboot_pbl_profile_marker_type;

typedef struct secboot_metadata_type {
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
} secboot_metadata_type;

typedef struct secboot_code_hash_info_type {
	/*
	 * Address (pointer value) of the code that was hashed.
	 */
	unsigned long code_address;
	/* the code length */
	uint32_t code_length;
	/*
	 * hash length - e.g 20 for SHA1, 32 for SHA256.
	 */
	uint32_t image_hash_length;
	/*
	 * hash of HEADER + CODE.
	 */
	uint8_t image_hash[SECBOOT_IMAGE_HASH_SIZE];
} secboot_image_hash_info_type;

typedef struct secboot_verified_info_type {
	/*
	 * The version id (define the secboot lib version).
	 */
	uint32_t version_id;
	/*
	 * The software id (upper 32 bits:version, lower 32 bits:type) the
	 * image was signed with.
	 */
	uint64_t sw_id;
	/*
	 * Value of the debug settings from the attestation cert, i.e.,
	 * SECBOOT_DEBUG_NOP, SECBOOT_DEBUG_DISABLE, SECBOOT_DEBUG_ENABLE.
	 */
	uint32_t enable_debug;
	/*
	 * Hash of the header + code.
	 */
	secboot_image_hash_info_type image_hash_info;
	/*
	 * Value to indicate if the configu fuse region is writeable or not.
	 * i.e SECBOOT_ROOT_KEY_CONTROL_ENABLE,
	 * SECBOOT_ROOT_KEY_CONTROL_DISABLE
	 */
	uint32_t enable_root_key_control;
	/*
	 * Metadata from the image that is verified.
	 */
	secboot_metadata_type metadata;
	/*
	 * Performance timestamps.
	 */
	secboot_pbl_profile_marker_type secboot_timestamps;
} secboot_verified_info_type;

typedef struct boot_images_entry {
	uint32_t image_id;
	uint32_t e_ident;
	uint64_t entry_point;
	secboot_verified_info_type image_verified_info;
	uint32_t reserved_1;
	uint32_t reserved_2;
	uint32_t reserved_3;
	uint32_t reserved_4;
} boot_images_entry;

typedef struct boot_qsee_interface {
	uint32_t magic_1;
	uint32_t magic_2;
	uint32_t version;
	uint32_t number_images;
	uint32_t reset_required;
	boot_images_entry boot_image_entry[BOOT_IMAGES_NUM_ENTRIES];
	uint64_t ddr_enter_self_refresh;
	uint64_t ddr_exit_self_refresh;
	uint32_t exit_image_index;
	uint8_t exit_image_privilege_level;
	uint8_t reserved_1;
	uint16_t reserved_2;
} boot_qsee_interface;

/*
 * QTI platform helper: final XBL-provided MTE decision.
 * Latched during bl31_early_platform_setup() via bl_aux_params handler.
 * Consumed by plat_mte_enabled() to set/clear SCR_EL3.ATA.
 */
bool qti_is_mte_enabled(void);

/*
 * Returns a pointer to the sbl_qsee_interface structure.
 * This structure contains information about images
 * authenticated by boot loader and its entry point
 * The returned pointer should be treated as read-only.
 */
const boot_qsee_interface *get_sbl_qsee_interface(void);

#endif /* QTI_SBL_SHARE_INFO_H */
