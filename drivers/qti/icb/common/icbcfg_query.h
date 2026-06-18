/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * ICB configuration query driver interface.
 *
 * Defines the data structures and constants consumed by the platform
 * icbcfg_query_data.c back-end and the common icbcfg_query driver.
 */

#ifndef QTI_ICBCFG_QUERY_H
#define QTI_ICBCFG_QUERY_H

#include <stdbool.h>
#include <stdint.h>

/* Chip version encoding: major.minor packed into a uint32_t */
#ifndef CHIPINFO_VERSION
#define CHIPINFO_VERSION(major, minor)  (((major) << 16) | (minor))
#endif

/*
 * Address-translator hardware variants.
 * ICBCFG_ADDR_TRANS_LLCC_6CH: 6-channel LLCC BEAC address translator
 * (also used for 4-channel and 2-channel SKUs that share the same
 * translator hardware but with fewer active channels).
 */
enum icbcfg_addr_trans_type {
	ICBCFG_ADDR_TRANS_NONE     = 0,
	ICBCFG_ADDR_TRANS_LLCC,
	ICBCFG_ADDR_TRANS_LLCC_6CH,
	ICBCFG_ADDR_TRANS_COUNT,
};

/*
 * struct icbcfg_mem_region - a contiguous physical DDR region that the
 * address translator may map.
 * @start: first byte of the region (inclusive)
 * @end:   last byte of the region (inclusive)
 */
struct icbcfg_mem_region {
	uint64_t start;
	uint64_t end;
};

/*
 * struct icbcfg_seg_offsets - per-segment register offsets within a
 * translation block.
 * @region_cfg2: byte offset of the REGION_CFG2 register
 * @region_cfg3: byte offset of the REGION_CFG3 register
 */
struct icbcfg_seg_offsets {
	uint32_t region_cfg2;
	uint32_t region_cfg3;
};

/*
 * struct icbcfg_data - a single 32-bit register write.
 * @addr: physical address of the register
 * @val:  value to write
 */
struct icbcfg_data {
	uintptr_t addr;
	uint32_t  val;
};

/*
 * struct icbcfg_prop - an ordered list of register writes to apply at
 * initialisation time.
 * @len:  number of entries in @data
 * @data: pointer to the array of register writes
 */
struct icbcfg_prop {
	uint32_t            len;
	struct icbcfg_data *data;
};

/*
 * struct icbcfg_device_config - per-variant (SKU) device configuration.
 *
 * Chip identification fields:
 * @family:  chip family identifier (CHIPINFO_FAMILY_*)
 * @match:   true  => exact version match required
 *           false => match any version >= @version
 * @version: minimum (or exact) chip version, encoded with CHIPINFO_VERSION()
 *
 * Optional SKU fuse/register discriminator (all three must be set together):
 * @reg_addr: physical address of the fuse/status register, or NULL
 * @reg_mask: bitmask to apply before comparing
 * @reg_val:  expected value after masking
 *
 * DDR topology:
 * @map_ddr_region_count: number of entries in @map_ddr_regions
 * @map_ddr_regions:      array of mappable DDR regions
 * @num_segments:         number of address-translation segments
 * @num_channels:         number of active DDR channels
 * @addr_width:           physical address width in bits
 * @trans_type:           address-translator hardware variant
 * @trans_bases:          array of @num_channels translation-block base
 *                        addresses (one per LLCC channel)
 * @seg_offsets:          array of @num_segments segment register offsets
 *
 * Register configuration:
 * @prop_data:       one-time init register writes (applied at boot)
 * @post_prop_data:  post-init register writes (applied after DDR init),
 *                   or NULL if unused
 */
struct icbcfg_device_config {
	/* Chip identification */
	uint32_t  family;
	bool      match;
	uint32_t  version;

	/* Optional SKU discriminator */
	uint8_t  *reg_addr;
	uint32_t  reg_mask;
	uint32_t  reg_val;

	/* DDR topology */
	uint32_t                    map_ddr_region_count;
	struct icbcfg_mem_region   *map_ddr_regions;
	uint32_t                    num_segments;
	uint32_t                    num_channels;
	uint32_t                    addr_width;
	enum icbcfg_addr_trans_type trans_type;
	uint8_t                   **trans_bases;
	struct icbcfg_seg_offsets  *seg_offsets;

	/* Register configuration */
	struct icbcfg_prop *prop_data;
	struct icbcfg_prop *post_prop_data;
};

/*
 * struct icbcfg_info - top-level descriptor exported by each platform's
 * icbcfg_query_data.c.
 * @num_configs: number of entries in @configs
 * @configs:     array of pointers to per-variant configurations, ordered
 *               from most-constrained (fuse-matched) to least-constrained
 *               (fallback)
 */
struct icbcfg_info {
	uint32_t                      num_configs;
	struct icbcfg_device_config **configs;
};

/* Declared in the platform icbcfg_query_data.c */
extern struct icbcfg_info icbcfg_info;

#endif /* QTI_ICBCFG_QUERY_H */
