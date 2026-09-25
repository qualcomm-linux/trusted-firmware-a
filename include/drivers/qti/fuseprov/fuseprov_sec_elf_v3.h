/*
 * Copyright (c) 2026, Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef FUSEPROV_SEC_ELF_V3_H
#define FUSEPROV_SEC_ELF_V3_H

#include <stdbool.h>
#include <stdint.h>

/* Errors reported while parsing or provisioning SEC.DAT. */
typedef enum {
        FUSEPROV_SUCCESS = 0,
        FUSEPROV_FAILURE = 1,
        FUSEPROV_INVALID_ARG = 2,
        FUSEPROV_NO_MEMORY = 3,
        FUSEPROV_SECDAT_MAGIC_MISMATCH = 4,
        FUSEPROV_SECDAT_REV_NOT_SUPPORTED = 5,
        FUSEPROV_SECDAT_SIZE_LEN_MISMATCH = 6,
        FUSEPROV_QFPROM_READ_ERROR = 7,
        FUSEPROV_QFUSE_REV_NOT_SUPPORTED = 8,
        FUSEPROV_INVALID_HASH = 9,
        FUSEPROV_SECDAT_LOCK_BLOWN = 10,
        FUSEPROV_QFPROM_WRITE_ERROR = 11,
        FUSEPROV_SHK_RD_WR_DISABLE_BLOWN = 12,
        FUSEPROV_SHK_GENERATION_FAILED = 13,
        FUSEPROV_SHK_ALRDY_BLOWN = 14,
        FUSEPROV_SHK_RD_WR_MISMATCH = 15,
        FUSEPROV_SECDAT_DEFAULT_NOFUSES = 16,
        FUSEPROV_SEGMENT_NOT_FOUND = 17,
        FUSEPROV_SECDAT_SEGMENT_NUM_NOT_SUPPORTED = 18,
        FUSEPROV_SECDAT_HASH_SIZE_MISMATCH = 19,
        FUSEPROV_OEM_SPARE_ALRDY_BLOWN = 20,
        FUSEPROV_OEM_SPARE_RAND_GEN_FAILED = 21,
        FUSEPROV_INVALID_REGION_TYPE = 22,
        FUSEPROV_INVALID_OPERATION_TYPE = 23,
} fuseprov_error_etype;

/* SEC.DAT v3 header structure
 *
 * v3 is flat: a fixed header is immediately followed by num_entries
 * fuseprov_qfuse_entry_t records. There are no segments, no footer and no
 * hash -- sec.elf integrity is established upstream by TME/PIL authentication
 * before this buffer is ever parsed.
 */
typedef struct {
        uint32_t magic1;
        uint32_t magic2;
        uint32_t revision;
        uint32_t num_entries;
} fuseprov_secdat_hdr_t;

/* One SEC.DAT fuse entry. */
typedef struct {
        uint32_t region_type;
        uint32_t raw_row_address;
        uint32_t lsb_val;
        uint32_t msb_val;
        uint32_t operation;
} fuseprov_qfuse_entry_t;

/* SEC.DAT region types. */
typedef enum {
        FUSEPROV_REGION_TYPE_OEM_SEC_BOOT = 0x0,
        FUSEPROV_REGION_TYPE_OEM_PK_HASH = 0x1,
        FUSEPROV_REGION_TYPE_SEC_HW_KEY = 0x2,
        FUSEPROV_REGION_TYPE_OEM_CONFIG = 0x3,
        FUSEPROV_REGION_TYPE_READ_PERM = 0x4,
        FUSEPROV_REGION_TYPE_WRITE_PERM = 0x5,
        FUSEPROV_REGION_TYPE_FEC_EN = 0x6,
        FUSEPROV_REGION_TYPE_ANTI_ROLLBACK = 0x7,
        FUSEPROV_REGION_TYPE_IMAGE_ENCR_KEY = 0x8,
        FUSEPROV_REGION_TYPE_MRC_2_0 = 0x9,
        FUSEPROV_REGION_TYPE_OEM_SPARE = 0xA,
        FUSEPROV_REGION_TYPE_OEM_PRODUCT_SEED = 0xB,
} fuseprov_region_type_t;

/* SEC.DAT fuse operations. */
typedef enum {
        FUSEPROV_OPERATION_BLOW = 0x0,
        FUSEPROV_OPERATION_BLOW_RANDOM = 0x1,
} fuseprov_operation_type_t;

/* Ordered fuse-provisioning categories.
 *
 * Categories group one or more region types for ordered provisioning. Any
 * region type without an explicit category falls back to GENERAL, so regions
 * such as
 * OEM_PK_HASH, ANTI_ROLLBACK, IMAGE_ENCR_KEY and MRC_2_0 are blown as part of
 * the GENERAL pass.
 */
typedef enum {
        FUSEPROV_CATEGORY_GENERAL = 0x0,
        FUSEPROV_CATEGORY_SECBOOT = 0x1,
        FUSEPROV_CATEGORY_SHK = 0x2,
        FUSEPROV_CATEGORY_OEM_CONFIG = 0x3,
        FUSEPROV_CATEGORY_READ_PERM = 0x4,
        FUSEPROV_CATEGORY_WRITE_PERM = 0x5,
        FUSEPROV_CATEGORY_FEC_EN = 0x6,
        FUSEPROV_CATEGORY_OEM_SPARE_RAND = 0x7,
        FUSEPROV_CATEGORY_OEM_PRODUCT_SEED = 0x8,
        FUSEPROV_CATEGORY_ANTI_ROLLBACK = 0x9,
} fuseprov_category_t;

/* Map a SEC.DAT region type to its provisioning category.
 * @param region_type SEC.DAT region type
 * @return category, or GENERAL for an unlisted region
 */
fuseprov_category_t fuseprov_get_category_for_region(uint32_t region_type);

/* Check whether a region type belongs to a provisioning category.
 * @param category category to test
 * @param region_type SEC.DAT region type
 * @return true if the region maps to category
 */
bool fuseprov_is_region_in_category(fuseprov_category_t category,
                                    uint32_t region_type);

/* SEC.DAT format and QFPROM correction constants. */
#define FUSEPROV_SECDAT_MAGIC1           0x3B7251CA
#define FUSEPROV_SECDAT_MAGIC2           0x2A126F29
#define FUSEPROV_SECDAT_V3_REV           3
#define FUSEPROV_FEC_ROW_MSB_MASK        0x00FFFFFF
#define FUSEPROV_GEN_ROW_MSB_MASK        0xFFFFFFFF

#endif /* FUSEPROV_SEC_ELF_V3_H */
