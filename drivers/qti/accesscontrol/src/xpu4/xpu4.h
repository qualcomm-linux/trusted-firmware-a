/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef XPU4_H
#define XPU4_H

#include <cdefs.h>
#include <stdbool.h>
#include <stdint.h>

#include <lib/cassert.h>
#include <lib/utils_def.h>
#include <xpu_common.h>

/*
 * Width of the virtual-machine bitmap. Selects how many VM ids and permission
 * domains the driver understands.
 */
#define AC_VM_BITMAP_SIZE	128

CASSERT((AC_VM_BITMAP_SIZE % 64) == 0, assert_vm_bitmap_size_multiple_of_64);
CASSERT(AC_VM_BITMAP_SIZE <= 128, assert_vm_bitmap_size_max_128);

/*
 * QAD (qualified access domain) vectors. A permission register holds a bitmap
 * of the QADs allowed to access a resource group. AP secure and AP non-secure
 * share bit 0 and are distinguished by bit 31 and bit 30 respectively.
 */
#define QAD_VECTOR_INVALID	0U
#define QAD_VECTOR_AP_SEC	((1UL << 0) | (1UL << 31))
#define QAD_VECTOR_AP_NS	((1UL << 0) | (1UL << 30))
#define QAD_VECTOR_TME_ROM	(1UL << 1)
#define QAD_VECTOR_TME_FW	(1UL << 2)
#define QAD_VECTOR_DEBUG	(1UL << 3)
#define QAD_VECTOR_AOP		(1UL << 4)
#define QAD_VECTOR_MODEM	(1UL << 5)
#define QAD_VECTOR_SP		(1UL << 6)
#define QAD_VECTOR_OOBNS	(1UL << 9)
#define QAD_VECTOR_OOBS		(1UL << 10)
#define QAD_VECTOR_QECP_DEBUG	(1UL << 11)

/* Bit shared by the AP secure and AP non-secure vectors. */
#define QAD_AP_COMMON_BIT	(1UL << 0)

/*
 * Permission domains, as bit positions. Numbering starts at 31 and descends so
 * it cannot collide with VM ids, which start at 0.
 */
#define TZBSP_DOMAIN_MASK		0xff000000U
#define TZBSP_DOMAIN_MSA		31U
#define TZBSP_DOMAIN_HYP		30U
#define TZBSP_DOMAIN_SP			29U
#define TZBSP_DOMAIN_TZ			28U
#define TZBSP_DOMAIN_AOP		27U
#define TZBSP_DOMAIN_TME_ROM		26U
#define TZBSP_DOMAIN_TME_FW		25U
#define TZBSP_DOMAIN_DEBUG		24U
#define TZBSP_DOMAIN_OOBNS		23U
#define TZBSP_DOMAIN_OOBS		22U
#define TZBSP_DOMAIN_QECP_DEBUG		21U

#define TZBSP_DOMAIN_MSA_BIT		(1U << TZBSP_DOMAIN_MSA)
#define TZBSP_DOMAIN_HYP_BIT		(1U << TZBSP_DOMAIN_HYP)
#define TZBSP_DOMAIN_SP_BIT		(1U << TZBSP_DOMAIN_SP)
#define TZBSP_DOMAIN_TZ_BIT		(1U << TZBSP_DOMAIN_TZ)
#define TZBSP_DOMAIN_AOP_BIT		(1U << TZBSP_DOMAIN_AOP)
#define TZBSP_DOMAIN_TME_ROM_BIT	(1U << TZBSP_DOMAIN_TME_ROM)
#define TZBSP_DOMAIN_TME_FW_BIT		(1U << TZBSP_DOMAIN_TME_FW)
#define TZBSP_DOMAIN_DEBUG_BIT		(1U << TZBSP_DOMAIN_DEBUG)
#define TZBSP_DOMAIN_OOBNS_BIT		(1U << TZBSP_DOMAIN_OOBNS)
#define TZBSP_DOMAIN_OOBS_BIT		(1U << TZBSP_DOMAIN_OOBS)
#define TZBSP_DOMAIN_QECP_DEBUG_BIT	(1U << TZBSP_DOMAIN_QECP_DEBUG)

/*
 * VMID 0 is the default non-secure VMID. Its bit sits in the same mask as the
 * domain bits above, which is why the two share a numbering space.
 */
#define TZBSP_VMID_VMID_0		0U
#define TZBSP_VMID_VMID_0_BIT		(1U << TZBSP_VMID_VMID_0)

/*
 * Policy profile flags. A configuration entry only applies when its profile
 * flags intersect the profiles enabled for the running device.
 */
#define AC_PROFILE_INVALID			0U
#define AC_PROFILE_DEFAULT			(1U << 0)
#define AC_PROFILE_HW_V1			(1U << 1)
#define AC_PROFILE_HW_V2			(1U << 2)
#define AC_PROFILE_RAMDUMP_ENABLED		(1U << 3)
#define AC_PROFILE_DEBUG_ENABLED		(1U << 4)
#define AC_PROFILE_IGNORE_RG			(1U << 5)
#define AC_PROFILE_VMID_CHECK_DISABLED		(1U << 6)
#define AC_PROFILE_RG_OVERRIDE			(1U << 7)
#define AC_PROFILE_XPRESSCFG			(1U << 8)
#define AC_PROFILE_XBL_SEC			(1U << 9)
#define AC_PROFILE_DYNAMIC_INITIALIZE		(1U << 10)
#define AC_PROFILE_PLATFORM_AUTO_IVI		(1U << 11)
#define AC_PROFILE_PLATFORM_AUTO_ADAS		(1U << 12)
#define AC_PROFILE_PLATFORM_COMPUTE		(1U << 13)
#define AC_PROFILE_WP				(1U << 14)
#define AC_PROFILE_DEBUG_ENABLED_APPS_SECURE	(1U << 15)
#define AC_PROFILE_LCS_INTERNAL			(1U << 16)
#define AC_PROFILE_WA_PRE_ES			(1U << 17)
#define AC_PROFILE_DEBUG_BUSPM_ENABLE		(1U << 18)
#define AC_PROFILE_PLATFORM_LE			(1U << 19)
#define AC_PROFILE_PLATFORM_MN			(1U << 20)
#define AC_PROFILE_DEBUG_SDI_PASS2		(1U << 21)
#define AC_PROFILE_DEBUG_POLICY_READY		(1U << 22)
#define AC_PROFILE_XBL_SC_EXIT			(1U << 23)
#define AC_PROFILE_BIN_COMP_BASE		(1U << 24)
#define AC_PROFILE_BIN_COMP_1			(1U << 25)
#define AC_PROFILE_PLATFORM_RG_MN		(1U << 26)
#define AC_PROFILE_PLATFORM_AUTO_FLEX		(1U << 27)
#define AC_PROFILE_PLATFORM_AUTO		(1U << 28)
#define AC_PROFILE_PLATFORM_CLOUD		(1U << 29)
#define AC_STATIC_POLICY_PROFILES_COUNT		(1U << 30)

/* Bitmap of AC_PROFILE_* values. */
typedef uint32_t ac_xpu_profile;

/* Bitmap of QAD_VECTOR_* values. */
typedef uint32_t ac_xpu4_qad_vector;

/* Per-resource-group XPRESSCFG ignore value. */
typedef uint32_t ac_rg_ignored;

/*
 * XPU instance identifiers.
 *
 * XPU4 obtains its instance list from the config image at runtime, so only the
 * ids the driver compares against are named here. The values are an ABI shared
 * with the image producer; XPU4_ID_COUNT is pinned to the producer's instance
 * count and must not be allowed to drift.
 */
enum xpu4_id {
	XPU4_ID_TCSR_REGS = 20,
	XPU4_ID_BIMC_MPU0 = 46,
	XPU4_ID_LLCC_BROADCAST_MPU = 58,
	XPU4_ID_LLCC_AND_BROADCAST_MPU = 130,
	XPU4_ID_LLCC_OR_BROADCAST_MPU = 131,
	XPU4_ID_BIMC_AND_GLB_MPU = 194,
	XPU4_ID_MACHX_AND_BCAST_LLCC_MPU = 380,
	XPU4_ID_COUNT = 498,
	XPU4_ID_UNSUPPORTED = XPU4_ID_COUNT,
	XPU4_ID_SIZE = 0x7FFFFFFF	/* force a 32-bit enum */
};

/*
 * Unit type as described by the config image.
 *
 * NOTE: this is NOT the same encoding as enum xpu4_variant below, which comes
 * from the IDR0.XPU_TYPE hardware field. The two are reversed (here MPU is 2,
 * there MPU is 0) and must never be used interchangeably.
 */
enum xpu4_device_type {
	XPU4_DEVICE_RPU = 0,
	XPU4_DEVICE_APU = 1,
	XPU4_DEVICE_MPU = 2,
	XPU4_DEVICE_COUNT,
	XPU4_DEVICE_DNE = 0x7FFFFFFE,
	XPU4_DEVICE_SIZE = 0x7FFFFFFF
};

/*
 * Unit type as reported by the IDR0.XPU_TYPE hardware field. See the warning
 * on enum xpu4_device_type: the encodings are reversed.
 */
enum xpu4_variant {
	AC_MPU = 0,
	AC_APU = 1,
	AC_RPU = 2
};

/*
 * Guard the reversal documented above: xpu4_device_type has MPU=2 and RPU=0,
 * xpu4_variant has the opposite. If either enum is ever renumbered the mix-up
 * becomes a silent bug, so fail at compile time instead.
 */
CASSERT(XPU4_DEVICE_MPU != AC_MPU, assert_device_type_variant_mpu_reversed);
CASSERT(XPU4_DEVICE_RPU != AC_RPU, assert_device_type_variant_rpu_reversed);

enum xpu4_flag {
	AC_XPU_FLAG_RA_XPU = 1U << 0,
	AC_XPU_FLAG_STATIC = 1U << 1
};

/* How much of the XPU configuration is applied. */
enum xpu_level {
	XPU_DISABLE_NONE,
	XPU_DISABLE_NON_MSS,
	XPU_DISABLE_ALL,
	XPU_ENABLE_SLE
};

/*
 * Virtual machine identifiers. Only the ids the driver names are listed; the
 * values are an ABI shared with the image producer.
 */
enum ac_virtual_machine_id {
	AC_VM_NONE = 0,
	AC_VM_TZ = 1,
	AC_VM_HLOS = 3,
	AC_VM_SSC_Q6_ELF = 5,
	AC_VM_ADSP_Q6_ELF = 6,
	AC_VM_MSS_MSA = 15,
	AC_VM_SPSS_SP = 26,
	AC_VM_HLOS_GSI = 32,
	AC_VM_TRUSTED_UI = 45,
	AC_VM_OEM = 49,
	AC_VM_HOST = 51,
	AC_VM_GVM1 = 52,
	AC_VM_GVM2 = 53,
	AC_VM_GVM3 = 54,
	AC_VM_GVM4 = 55,
	AC_VM_LAST = 127,
	AC_VM_MAX = 0x7FFFFFFF
};

/*
 * Resource group configuration for MPUs, as stored in the config image.
 *
 * The structures below through struct ac_xpu_info are wire formats shared with
 * the image producer. Field order, widths and the resulting padding must match
 * it exactly.
 */
struct ac_mpu_rg {
	uint64_t start;		/* Start of the partition */
	uint64_t end;		/* End (exclusive) of the partition */
	ac_xpu_profile profile_flags;
	uint32_t read_qads;	/* Bitmap of QADs able to read this partition */
	uint32_t write_qads;	/* Bitmap of QADs able to write this partition */
	uint16_t rg_num;
};

/* Resource group configuration for APUs and RPUs. */
struct ac_rpu_rg {
	ac_xpu_profile profile_flags;
	uint32_t read_qads;
	uint32_t write_qads;
	uint16_t rg_num;
};

/* Configuration for one XPU instance. */
struct ac_xpu_cfg {
	uint32_t base_addr;
	ac_xpu_profile profile_flags;
	uint32_t xpu_id;
	uint16_t status;
	uint16_t nrg;		/* Number of configured resource groups */
	union {
		const void *raw;
		const struct ac_rpu_rg *rpu;
		const struct ac_mpu_rg *mpu;
	} rg;
	/*
	 * QAD holding permission to the XPU's own config registers. Zero leaves
	 * APNSPE in control; otherwise the named QAD gets exclusive access.
	 */
	uint32_t cfg_owner;
	uint32_t umr_perm;	/* Unmapped region permissions, MPU only */
	const ac_rg_ignored *rg_ignored;
};

/* Range of resource groups reserved for runtime assignment. */
struct ac_xpu_dynamic_rgs {
	uint32_t rg_start;
	uint32_t rg_count;
	uint32_t *shadow_read_perm;
	uint32_t *shadow_write_perm;
};

/* Cached hardware identity for one XPU instance. */
struct ac_xpu4_priv_info {
	const struct ac_xpu_dynamic_rgs *dyn_rgs;
	uint32_t soc_addr;	/* Absolute SOC address */
	uint32_t addr;		/* Address from the CPU view */
	uint32_t xpu_id;
	uint32_t idr[3];
	uint32_t rev;
};

/* One partition entry. Packed in the image; do not add padding. */
struct ac_partition {
	uint32_t start;
	uint32_t end;
	uint16_t rg_num;
} __packed;

/* Address range and partition list for one APU/RPU instance. */
struct ac_xpu_info {
	uint64_t range_start;
	uint64_t range_end;
	enum xpu4_id xpu_id;
	enum xpu4_device_type xpu_id_type;
	struct ac_partition *partition_info;
	uint32_t num_partitions;
};

/*
 * Runtime-only structures below. These are not part of the image and may be
 * changed freely.
 */

/* Decoded state of one resource group. */
struct xpu4_rg_info {
	uint64_t start_addr;
	uint64_t end_addr;
	ac_xpu4_qad_vector read_perm_vector;
	ac_xpu4_qad_vector write_perm_vector;
	ac_xpu4_qad_vector lock_vector;
	ac_xpu4_qad_vector shadow_read_perm_vector;
	ac_xpu4_qad_vector shadow_write_perm_vector;
	bool rge;
	bool wowp;
};

/* Result of searching the resource groups for an address range. */
struct xpu4_partition_overlap {
	bool has_exact_match;	/* At least one RG matches the range exactly */
	uint32_t num_overlaps;	/* Number of RGs overlapping the range */
	uint32_t rg_num;	/* First RG matching the range exactly */
};

/* Error syndrome captured from a violating XPU. */
struct xpu4_error_syndrome {
	uint32_t esr;
	uint32_t synar0;
	uint32_t synar1;
	uint32_t synr0;
	uint32_t synr1;
	uint32_t synr2;
};

/*
 * Translation between the permission-domain masks the generic layer speaks and
 * the QAD vectors the XPU4 hardware uses. Implemented in xpu4.c.
 */
ac_xpu4_qad_vector xpu4_domain_mask_to_qad_vector(uint32_t domain_mask);
uint32_t xpu4_qad_vector_to_domain_mask(ac_xpu4_qad_vector qad_vector);

/*
 * Latch the silent-logging state and program the write-once static-signal
 * registers of every RA XPU. Returns 0 on success. Implemented in xpu4_isr.c.
 */
int xpu4_set_ra_static_signals(bool enable_silent_logging);

#endif /* XPU4_H */
