/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef AC_CFG_H
#define AC_CFG_H

#include <stdbool.h>
#include <stdint.h>

#include <xpu4.h>

/*
 * Access-control configuration image.
 *
 * XPU4 targets carry no compile-time access-control configuration. A separate
 * image is placed in DDR by an earlier boot stage and BL31 is given only its
 * base address; every table the XPU4 and VMIDMT drivers need is fetched from
 * it at runtime.
 *
 * The image starts with a struct ac_global_data header followed by an array of
 * absolute pointers indexed by enum ac_xpu_target_cfg. Each table is described
 * by two entries: one holding the pointer and one holding the element count.
 *
 * IMPORTANT: every structure below is a wire format shared with the image
 * producer. Field order, field widths and the resulting padding must match the
 * producer exactly. Only the type and field names have been converted to
 * snake_case; nothing has been added, removed, reordered or repacked.
 */

#define AC_TZ_AC_CONFIG_IMAGE_SIZE	0x10000		/* 64 KB */
#define AC_TZ_MAGIC_COOKIE		0x072382d3

#define AC_INTERRUPT_DESC_MAX_LEN	32

/*
 * Index of each table within the config image. The values are an ABI shared
 * with the image producer and must not be renumbered. Entries for features
 * that BL31 does not implement are retained so the surrounding indices keep
 * their meaning.
 */
enum ac_xpu_target_cfg {
	AC_IS_XPU_ENABLED = 0,
	AC_POLICYVERSION_BASE = 1,
	AC_POLICYTIMESTAMP_BASE = 2,
	AC_XPUCFG_ARRAY = 3,
	AC_XPUCFG_ARRAY_SIZE = 4,
	AC_XPUDBGAR_ARRAY = 5,
	AC_XPUDBGAR_ARRAY_SIZE = 6,
	AC_XPUINFOS_ARRAY = 7,
	AC_XPUINFOS_ARRAY_SIZE = 8,
	AC_IS_SILENTLOGGING_ENABLED = 9,
	AC_XPUTCSRREGS_ARRAY = 10,
	AC_XPUTCSRREGS_ARRAY_SIZE = 11,
	AC_XPUTCSRSLE_ADDR = 12,
	AC_XPU_SLE_DISABLE_VALUE = 13,
	AC_XPU_SLE_ENABLE_VALUE = 14,
	AC_VALIDDOMAIN_IDS_ARRAY = 15,
	AC_VALIDDOMAIN_IDS_ARRAY_SIZE = 16,
	AC_XPUHWADDR_MASK_ARRAY = 17,
	AC_XPUHWADDR_MASK_ARRAY_SIZE = 18,
	AC_XPUINTR_POS_XPUMAP_ARRAY = 19,
	AC_XPUINTREN_REG_ARRAY = 20,
	AC_XPUINTRSTATUS_REG_ARRAY = 21,
	AC_XPUINTR_TCSRREG_ARRAY_SIZE = 22,
	AC_XPUPROTECTEDRANGES_ARRAY = 23,
	AC_XPUPROTECTEDRANGES_ARRAY_SIZE = 24,
	AC_OEMSPAREFUSEINFO_ARRAY = 25,
	AC_OEMSPAREFUSEINFO_ARRAY_SIZE = 26,
	AC_OEMSPAREFUSEINFO_COUNT = 27,
	AC_XPUINT_NUMBER = 28,
	AC_RAXPU_INFO_ARRAY = 29,
	AC_RAXPU_INFO_ARRAY_SIZE = 30,
	AC_MS_XPUPROTECTEDRANGES_ARRAY = 31,
	AC_MS_XPUPROTECTEDRANGES_ARRAY_SIZE = 32,
	AC_APURPU_PARTITIONS_ARRAY = 33,
	AC_APURPU_PARTITIONS_ARRAY_SIZE = 34,
	AC_VMIDMT_CFG_ARRAY = 35,
	AC_VMIDMT_CFG_ARRAY_SIZE = 36,
	AC_VMIDMT_MAP_ARRAY = 37,
	AC_VMIDMT_MAP_ARRAY_SIZE = 38,
	AC_VMIDMT_INTR_POS_MAP_ARRAY = 39,
	AC_VMIDMT_INTR_POS_MAP_ARRAY_SIZE = 40,
	AC_VMIDMT_DEVICE_VMIDMT_MAP_ARRAY = 41,
	AC_XPU_VALID_QAD_MASK = 42,
	AC_XPU_DDR_SLICE_XPU_GROUP = 43,
	AC_XPU_DDR_SLICE_XPU_DETAILS = 44,
	AC_XPU_DDR_SLICE_XPU_COUNT = 45,
	AC_XPU_ENUM_MAX_SUPPORTED = 46,
	AC_DDR_SLICE_COUNT = 47,
	AC_TZ_VMID_CFG_ARRAY = 48,
	AC_TZ_VMID_CFG_ARRAY_SIZE = 49,
	AC_TZ_SHADOW_VM_ARRAY = 50,
	AC_TZ_SHADOW_VM_ARRAY_SIZE = 51,
	AC_TZ_DEVICE_MEM_MAP_ARRAY = 52,
	AC_TZ_DEVICE_MEM_MAP_ARRAY_SIZE = 53,
	AC_INTR_DESC_ARRAY = 54,
	AC_INTR_DESC_ARRAY_SIZE = 55,
	ACDB_ALLOC_FROM_DDR_OFFSET = 56,
	AC_XPU_WA_SUPPORTED = 57,
	AC_IMAGE_VERSION_INFO = 58,
	AC_TARGET_PROFILE_MAP_ARRAY = 59,
	AC_TARGET_PROFILE_MAP_COUNT = 60,
	AC_QB_RESTORE_XPU_ARRAY = 61,
	AC_QB_RESTORE_XPU_COUNT = 62,
	ACDB_FIXED_DDR_BASE_ADDR = 63,
	AC_QB_XPU_ARRAY = 64,
	AC_QB_XPU_ARRAY_SIZE = 65,
	AC_QB_VMIDMT_ARRAY = 66,
	AC_QB_VMIDMT_COUNT = 67,
	AC_QB_VMIDMT_ARRAY_ENTRY_COUNT = 68,
	AC_QB_XPU_META_INFO = 69,
	AC_VMIDMT_CFG_ARRAY_V2 = 70,
	AC_VMIDMT_CFG_ARRAY_SIZE_V2 = 71,
	AC_VMIDMT_INFO_ARRAY = 72,
	AC_VMIDMT_INFO_ARRAY_SIZE = 73,
	AC_XPU_INTERRUPTS_ENABLED = 74,
	AC_XPU_PARTITION_ARRAY = 75,
	AC_XPU_PARTITION_ARRAY_SIZE = 76,
	AC_LOCK_RESTRICTED_DOMAINS = 77,
	AC_LOCK_RESTRICTED_DOMAINS_COUNT = 78,
	AC_LOCK_DELEGATE_REDUCED_MASKS = 79,
	AC_LOCK_DELEGATE_REDUCED_MASKS_COUNT = 80,
	AC_MS_XPU_VM = 81,
	AC_MS_XPU_VM_COUNT = 82,
	AC_HIER_DOMAIN_PERM = 83,
	AC_HIER_DOMAIN_PERM_COUNT = 84,
    AC_XBL_SC_EXIT_CFG_ARRAY = 85,
    AC_XBL_SC_EXIT_CFG_ARRAY_SIZE = 86,
	AC_VMIDMT_INTR_REGS_ARRAY = 87,
	AC_VMIDMT_INTR_REGS_ARRAY_SIZE = 88,
	AC_XPU_CFG_MAX
};

/* Header found at the base address of the config image. */
struct ac_global_data {
	uint32_t magic_cookie;
	uint32_t version;
	uint32_t num_entries;
	uintptr_t *globaldata_ptr;
};

/* Shadow virtual machine mapping. */
struct ac_shadow_vm {
	enum ac_virtual_machine_id shadow_vm;
	enum ac_virtual_machine_id mapping_vm;
};

/* One device memory region described by the image. */
struct ac_device_memory_info {
	uint64_t start;
	uint64_t size;
	bool valid;
};

/* Interrupt number and its description string. */
struct ac_interrupt_desc {
	uint32_t intr_num;
	char desc[AC_INTERRUPT_DESC_MAX_LEN];
};

/* SOC to CPU view address translation for one XPU instance. */
struct ac_xpu_hwaddr_mask {
	uint32_t xpu_index;
	uint32_t mask_value;
	uint32_t base_addr;
};

/* Address range statically protected by a named XPU. */
struct ac_xpu_protected_range {
	const uint64_t start;
	const uint64_t end;
	const enum xpu4_id xpu_id;
};

/* Restricted lock-delegate QAD mask for one XPU (XPU 4.3.0 and later). */
struct ac_xpu_id_to_qad_mask {
	const enum xpu4_id xpu_id;
	const uint32_t mask;
};

/* XPU error interrupt status or enable register as described by the image. */
struct ac_xpu_intr_reg {
	uint32_t addr;
	uint32_t mask;
};

/* Register write descriptor used to program XPU static signals. */
struct ac_addr_value_tuple {
	uint32_t addr;
	uint32_t mask;
	uint32_t value;
};

/* Per-instance record for an RA (resource-access) XPU. */
struct ac_ra_xpu_info {
	const char *intr_desc;
	struct ac_addr_value_tuple *static_signal_silent_logging;
	struct ac_addr_value_tuple *static_signal_others;
	uint32_t intr_num;
	uint32_t xpu_id;
	uint32_t static_signal_others_count;
};

/*
 * Status and enable register addresses for one VMIDMT error interrupt channel.
 * Four of these are stored in the config image (indexed by vmidmt_intr_id).
 */
struct vmidmt_intr_addr {
	uint32_t status_addr;
	uint32_t status_mask;
	uint32_t enable_addr;
};

/*
 * VMIDMT tables carried by the config image.
 *
 * These are declared generically so ac_cfg does not have to depend on the
 * VMIDMT headers; src/vmidmt/vmidmt_cfg_ac.c applies the concrete types and
 * passes the matching element size so the bounds check stays exact.
 */
enum ac_vmidmt_table {
	AC_VMIDMT_TABLE_CFG,	/* per-instance options */
	AC_VMIDMT_TABLE_INFO,	/* per-instance base addresses */
	AC_VMIDMT_TABLE_MAP,	/* stream-ID to VMID mappings */
	AC_VMIDMT_TABLE_INTR_POS_MAP	/* status bit to instance mapping */
};
/*
 * Parse and validate the config image. Must be called once before any getter
 * below. Returns 0 on success and a negative value if the image is absent or
 * malformed; the caller is expected to treat failure as fatal.
 */
int ac_cfg_init(void);

/* Global XPU enablement and hardware-workaround status. */
int ac_cfg_get_xpu_enabled(bool *enabled);
int ac_cfg_get_xpu_wa_supported(bool *supported);
int ac_cfg_get_silent_logging_enabled(bool *enabled);

/*
 * Silent-logging enable (SLE) control. SLE decides whether an XPU violation is
 * logged and cleared so execution continues, or is fatal. The register address
 * and the two values to write are described by the image.
 */
int ac_cfg_get_sle_addr(uint32_t *addr);
int ac_cfg_get_sle_enable_value(uint32_t *value);
int ac_cfg_get_sle_disable_value(uint32_t *value);

/* Static XPU configuration applied at boot. */
int ac_cfg_get_global_xpu_cfg(const struct ac_xpu_cfg **cfg, uint32_t *count);
int ac_cfg_get_xpu_infos(struct ac_xpu4_priv_info **infos, uint32_t *count);
int ac_cfg_get_addr_offset_mask_info(const struct ac_xpu_hwaddr_mask **mask,
				     uint32_t *count);
int ac_cfg_get_dynamic_protect_ranges(
			const struct ac_xpu_protected_range **range,
			uint32_t *count);
int ac_cfg_get_apu_rpu_partitions_info(uintptr_t **group, uint32_t *count);
int ac_cfg_get_lock_delegate_masks(const struct ac_xpu_id_to_qad_mask **mask,
				   uint32_t *count);

/* Interrupt description and RA XPU error reporting. */
int ac_cfg_get_ra_xpu_info(const struct ac_ra_xpu_info **info, uint32_t *count);
int ac_cfg_get_ra_xpu_interrupt_details(
			const struct ac_xpu_intr_reg **status_reg,
			const struct ac_xpu_intr_reg **enable_reg,
			const enum xpu4_id (**pos_to_xpu_map)[32],
			uint32_t *count);
int ac_cfg_get_xpu_vmidmt_intr_info(struct ac_interrupt_desc **desc,
				    uint32_t *count);

/* Device memory map and shadow VM list. */
int ac_cfg_get_device_memory_map_info(struct ac_device_memory_info **map,
				      uint32_t *count);
int ac_cfg_get_shadow_vm_list_info(struct ac_shadow_vm **list, uint32_t *count);

/* ACDB placement. */
int ac_cfg_get_ddr_allocation_offset(uint64_t *offset);
int ac_cfg_get_fixed_acdb_base_addr(uint64_t *addr);
int ac_cfg_get_xpu_dbgar_info(const uint32_t **dbgars, uint32_t *count);


int ac_cfg_get_vmidmt_intr_regs(const struct vmidmt_intr_addr **regs,
				uint32_t *count);



int ac_cfg_get_vmidmt_table(enum ac_vmidmt_table which, size_t elem_size,
			    const void **data, uint32_t *count);

/*
 * Return the base address and size of the config image. Implemented by the
 * platform. Returns 0 on success, negative on failure.
 */
int plat_qti_ac_get_config_image(uintptr_t *base, size_t *size);

#endif /* AC_CFG_H */
