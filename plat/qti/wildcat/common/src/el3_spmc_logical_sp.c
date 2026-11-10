/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * Logical partition (LSP) of the EL3 SPMC.
 *
 * OP-TEE sends FFA_MSG_SEND_DIRECT_REQ2 to this
 * partition to reach services that run in EL3. The partition is generic. It
 * checks who is calling, checks the size of the input buffer (x5), picks the
 * service by the UUID of the request, runs its handler and answers with
 * FFA_MSG_SEND_DIRECT_RESP2: the status of the LSP in x4, the size of the
 * output buffer in x5 and the results of the handler in x6..x17 (see
 * el3_spmc_logical_sp.h for the full ABI).
 *
 * A service is a handler (see lsp_service_handler_t in
 * el3_spmc_logical_sp.h) with a row in lsp_services[]. The services parse
 * the registers x6..x17 of their own commands. They work on the page shared
 * with OP-TEE.
 */

#include <platform_def.h>

#include <errno.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <arch_helpers.h>
#include <common/bl_common.h>
#include <common/debug.h>
#include <common/fdt_wrappers.h>
#include <lib/utils_def.h>
#include <lib/xlat_tables/xlat_tables_v2.h>
#include <libfdt.h>
#include <plat/common/platform.h>
#include <qti_plat.h>
#include <services/el3_spmc_logical_sp.h>
#include <services/ffa_svc.h>
#include <smccc_helpers.h>

#include "el3_spmc_logical_sp.h"
#include "ffa_tme_handler.h"

/* "description" of the memory region of the manifest that is the shared page */
#define LSP_SHMEM_DESC		"qcom,ffa-shmem"

/* The "pages-count" of a memory region of an FF-A manifest is in 4 KiB pages */
#define LSP_SHMEM_PAGE_SIZE	ULL(0x1000)

#define LSP_MANIFEST_MAP_SIZE	(2U * PAGE_SIZE)

/*
 * The FF-A ID of OP-TEE and the shared page, 0 until lsp_init() has read the
 * manifest and mapped the page.
 */
static uint16_t optee_sp_id;
static uintptr_t shmem_base;
static size_t shmem_size;

struct lsp_service {
	uint64_t uuid_x2;
	uint64_t uuid_x3;
	lsp_service_handler_t handler;
};

/*
 * lsp_services array maps the UUID of TF-A services to the service handlers.
 */
static const struct lsp_service lsp_services[] = {
	{ FFA_TME_UUID_X2, FFA_TME_UUID_X3, ffa_tme_handler },
};

/*
 * Validate the request and dispatch it to the service handler.
 * Returns the status of the LSP; the size of the output buffer and
 * the result registers are filled in out_size and rsp_regs.
 */
static uint64_t lsp_dispatch(uint32_t smc_fid, bool secure_origin,
			     uint16_t sender, uint64_t x2, uint64_t x3,
			     void *handle, size_t *out_size, uint64_t *rsp_regs)
{
	/* The size of the input buffer in the shared memory is passed in x5 */
	const uint64_t in_size = SMC_GET_GP(handle, CTX_GPREG_X5);
	uint64_t status;
	size_t i;

	if (smc_fid != FFA_MSG_SEND_DIRECT_REQ2_SMC64) {
		return FFA_LSP_ST_NOT_SUPPORTED;
	}

	if (!secure_origin) {
		VERBOSE("LSP: request from 0x%x denied\n", sender);
		return FFA_LSP_ST_DENIED;
	}

	if ((optee_sp_id == 0U) || (shmem_size == 0U)) {
		return FFA_LSP_ST_FAILURE;
	}

	if (sender != optee_sp_id) {
		VERBOSE("LSP: request from 0x%x denied\n", sender);
		return FFA_LSP_ST_DENIED;
	}

	/* The input buffer has to lie in the shared page. */
	if (in_size > shmem_size) {
		return FFA_LSP_ST_INVALID_PARAM;
	}

	for (i = 0U; i < ARRAY_SIZE(lsp_services); i++) {
		if ((x2 == lsp_services[i].uuid_x2) &&
		    (x3 == lsp_services[i].uuid_x3)) {

			/* Invalidate the cache to read directly from memory */
			inv_dcache_range(shmem_base, shmem_size);

			status = lsp_services[i].handler(handle,
							 (void *)shmem_base,
							 shmem_size,
							 (size_t)in_size,
							 out_size, rsp_regs);

			flush_dcache_range(shmem_base, shmem_size);

			return status;
		}
	}

	return FFA_LSP_ST_NOT_SUPPORTED;
}

/* Send FFA_MSG_SEND_DIRECT_RESP2 as SMC return. */
static uint64_t lsp_response(void *handle, uint16_t self, uint16_t peer,
			     uint64_t lsp_ret, size_t out_size,
			     const uint64_t *rsp_regs)
{
	uint64_t w1 = (uint64_t)self << FFA_DIRECT_MSG_SOURCE_SHIFT;

	w1 |= (uint64_t)peer << FFA_DIRECT_MSG_DESTINATION_SHIFT;

	SMC_RET18(handle, FFA_MSG_SEND_DIRECT_RESP2_SMC64, w1, 0ULL, 0ULL,
		  lsp_ret, (uint64_t)out_size,
		  rsp_regs[0], rsp_regs[1], rsp_regs[2], rsp_regs[3],
		  rsp_regs[4], rsp_regs[5], rsp_regs[6], rsp_regs[7],
		  rsp_regs[8], rsp_regs[9], rsp_regs[10], rsp_regs[11]);
}

/* LSP callback to handle FFA_MSG_SEND_DIRECT_REQ2 SMCs */
static uint64_t lsp_direct_req(uint32_t smc_fid, bool secure_origin,
			       uint64_t x1, uint64_t x2, uint64_t x3,
			       uint64_t x4, void *cookie, void *handle,
			       uint64_t flags)
{
	const uint16_t sender = ffa_endpoint_source((unsigned int)x1);
	const uint16_t self = ffa_endpoint_destination((unsigned int)x1);
	uint64_t rsp_regs[LSP_NUM_RSP_REGS] = {0};
	size_t out_size = 0U;
	uint64_t lsp_ret;

	(void)x4;
	(void)cookie;
	(void)flags;

	lsp_ret = lsp_dispatch(smc_fid, secure_origin, sender, x2, x3,
			       handle, &out_size, rsp_regs);

	return lsp_response(handle, self, sender, lsp_ret, out_size, rsp_regs);
}

/*
 * Get the address and the size of the shared page from the memory region of the
 * manifest that describes it:
 *
 * memory-regions {
 *	<any name> {
 *		description = "qcom,ffa-shmem";
 *		pages-count = <number of 4 KiB pages>;
 *		base-address = <upper 32 bits> <lower 32 bits>;
 *	};
 * };
 *
 * Returns 0 and sets *base and *size, -ENOENT if the manifest has no such
 * region, -EINVAL if the region is malformed.
 */
static int lsp_shmem_from_manifest(const void *fdt, uint64_t *base,
				   uint64_t *size)
{
	const char *description;
	uint32_t pages;
	int regions;
	int node;
	int len;

	regions = fdt_subnode_offset(fdt, 0, "memory-regions");
	if (regions < 0) {
		return -ENOENT;
	}

	fdt_for_each_subnode(node, fdt, regions) {
		description = fdt_getprop(fdt, node, "description", &len);
		if ((description == NULL) ||
		    (len != (int)sizeof(LSP_SHMEM_DESC)) ||
		    (memcmp(description, LSP_SHMEM_DESC, (size_t)len) != 0)) {
			continue;
		}

		if ((fdt_read_uint64(fdt, node, "base-address", base) < 0) ||
		    (fdt_read_uint32(fdt, node, "pages-count", &pages) < 0)) {
			return -EINVAL;
		}

		*size = (uint64_t)pages * LSP_SHMEM_PAGE_SIZE;
		return 0;
	}

	return -ENOENT;
}

/*
 * Get the FF-A ID of OP-TEE from the "id" property of the manifest, which is
 * where the SPMC reads it too.
 *
 * Returns 0 and sets *id, -ENOENT if the manifest has no ID, -EINVAL if it is
 * not the ID of an endpoint of the secure world.
 */
static int lsp_optee_id_from_manifest(const void *fdt, uint16_t *id)
{
	uint32_t value;

	if (fdt_read_uint32(fdt, 0, "id", &value) < 0) {
		return -ENOENT;
	}

	if ((value > UINT16_MAX) || !ffa_is_secure_world_id((uint16_t)value)) {
		return -EINVAL;
	}

	*id = (uint16_t)value;
	return 0;
}

/*
 * Read what the LSP needs from the manifest: the ID of OP-TEE and the shared
 * page.
 */
static int lsp_read_manifest(const void *fdt, uint16_t *sp_id, uint64_t *base,
			     uint64_t *size)
{
	int ret;

	ret = lsp_optee_id_from_manifest(fdt, sp_id);
	if (ret != 0) {
		ERROR("LSP: no valid ID of OP-TEE in manifest (%d)\n", ret);
		return ret;
	}

	ret = lsp_shmem_from_manifest(fdt, base, size);
	if (ret != 0) {
		ERROR("LSP: no shared page in manifest (%d)\n", ret);
	}

	return ret;
}

/*
 * Get the ID of OP-TEE and the shared page from the manifest, and map the page
 * in BL31.
 */
static int lsp_manifest_setup(void)
{
	const entry_point_info_t *ep_info;
	uintptr_t fdt_addr;
	uintptr_t fdt_page;
	uint64_t base = 0U;
	uint64_t size = 0U;
	uint16_t sp_id = 0U;
	void *fdt;
	int ret;
	int err;

	ep_info = bl31_plat_get_next_image_ep_info(SECURE);
	if ((ep_info == NULL) || (ep_info->args.arg0 == 0U)) {
		WARN("LSP: no manifest of the secure partition\n");
		return -ENOENT;
	}

	fdt_addr = (uintptr_t)ep_info->args.arg0;
	fdt_page = page_align(fdt_addr, DOWN);
	fdt = (void *)fdt_addr;

	ret = mmap_add_dynamic_region(fdt_page, fdt_page, LSP_MANIFEST_MAP_SIZE,
				      MT_RO_DATA);
	if (ret != 0) {
		ERROR("LSP: cannot map the manifest (%d)\n", ret);
		return ret;
	}

	/* The whole manifest has to be in what has just been mapped. */
	if ((fdt_check_header(fdt) != 0) ||
	    (fdt_totalsize(fdt) > (LSP_MANIFEST_MAP_SIZE -
				   (fdt_addr - fdt_page)))) {
		ERROR("LSP: bad manifest at 0x%lx\n", fdt_addr);
		ret = -EINVAL;
	} else {
		ret = lsp_read_manifest(fdt, &sp_id, &base, &size);
	}

	err = mmap_remove_dynamic_region(fdt_page, LSP_MANIFEST_MAP_SIZE);
	if (err != 0) {
		WARN("LSP: cannot unmap the manifest (%d)\n", err);
	}

	if (ret != 0) {
		return ret;
	}

	if ((base == 0U) || (size == 0U) ||
	    ((base % LSP_SHMEM_PAGE_SIZE) != 0U) || ((base + size) < base)) {
		ERROR("LSP: bad shared page 0x%lx, %lu bytes\n", base, size);
		return -EINVAL;
	}

	ret = qti_mmap_add_dynamic_region((uintptr_t)base, (size_t)size,
					  MT_MEMORY | MT_RW | MT_SECURE |
					  MT_EXECUTE_NEVER);
	if (ret != 0) {
		ERROR("LSP: cannot map the shared page 0x%lx (%d)\n", base,
		      ret);
		return ret;
	}

	optee_sp_id = sp_id;
	shmem_base = (uintptr_t)base;
	shmem_size = (size_t)size;
	INFO("QTI LSP: OP-TEE ID 0x%x, shared page at 0x%lx, %zu bytes\n",
	     optee_sp_id, shmem_base, shmem_size);

	return 0;
}

static int32_t lsp_init(void)
{
	INFO("QTI LSP init\n");

	/* A failure here must not fail the init */
	if (lsp_manifest_setup() != 0) {
		ERROR("LSP: no OP-TEE ID or shared page, requests will fail\n");
	}

	return 0;
}

/*
 * UUID of the LSP as four 32-bit words:
 * b771bb32-bb44-4a46-889c-06aa615ab0ef
 */
#define FFA_LSP_UUID	{ 0xb771bb32, 0xbb444a46, 0x889c06aa, 0x615ab0ef }

/*
 * FF-A endpoint of the LSP
 */
#define FFA_LSP_ID			0xC001

/*
 * Logical partition that proxies FF-A direct requests from OP-TEE to the
 * services that run in EL3.
 *
 * Only FFA_MSG_SEND_DIRECT_REQ2 is accepted.
 */
DECLARE_LOGICAL_PARTITION(
	qti_lsp,
	lsp_init,			/* Init function */
	FFA_LSP_ID,			/* FF-A partition ID */
	FFA_LSP_UUID,			/* Partition UUID */
	FFA_PARTITION_DIRECT_REQ2_RECV,	/* Partition properties */
	lsp_direct_req			/* Callback for direct requests */
);
