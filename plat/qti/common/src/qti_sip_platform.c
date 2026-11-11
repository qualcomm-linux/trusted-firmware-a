/*
 * Copyright (c) 2026 Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <errno.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <common/debug.h>
#include <context.h>
#include <drivers/qti/accesscontrol/accesscontrol.h>
#include <lib/coreboot.h>
#include <lib/utils_def.h>
#include <lib/xlat_tables/xlat_tables_v2.h>
#include <smccc_helpers.h>

#include <qti_plat.h>
#include <qti_secure_io_cfg.h>
#include <qti_sip_svc.h>

#define QTI_SIP_SVC_CALL_COUNT			U(0x3)
#define QTI_VM_LAST				U(44)
#define SIZE4K					U(0x1000)
#define QTI_VM_MAX_LIST_SIZE			U(0x20)

struct qti_mmap_params {
	qti_accesscontrol_mem_t mem[QTI_VM_MAX_LIST_SIZE];
	u_register_t mem_cnt;
	qti_accesscontrol_perm_t dst[QTI_VM_LAST];
	u_register_t dst_cnt;
	uint32_t src[QTI_VM_LAST];
	u_register_t src_cnt;
};

static bool qti_is_secure_io_access_allowed(u_register_t addr)
{
	int i = 0;

	for (i = 0; i < ARRAY_SIZE(qti_secure_io_allowed_regs); i++) {
		if ((uintptr_t)addr == qti_secure_io_allowed_regs[i]) {
			return true;
		}
	}

	return false;
}

u_register_t qti_sip_call_count(void)
{
	return QTI_SIP_SVC_CALL_COUNT;
}

bool qti_sip_is_available(u_register_t smc_fid, bool caller_secure)
{
	switch (smc_fid) {
	case QTI_SIP_SVC_CALL_COUNT_ID:
	case QTI_SIP_SVC_UID_ID:
	case QTI_SIP_SVC_VERSION_ID:
	case QTI_SIP_SVC_AVAILABLE_ID:
		return true;
	case QTI_SIP_SVC_SECURE_IO_READ_ID:
	case QTI_SIP_SVC_SECURE_IO_WRITE_ID:
	case QTI_SIP_SVC_MEM_ASSIGN_ID:
		return !caller_secure;
	default:
		return false;
	}
}

int qti_sip_secure_io_read(u_register_t addr, u_register_t *value)
{
	if (!qti_is_secure_io_access_allowed(addr)) {
		return QTI_SIP_INVALID_PARAM;
	}

	*value = *((volatile uint32_t *)addr);
	return QTI_SIP_SUCCESS;
}

int qti_sip_secure_io_write(u_register_t addr, u_register_t value)
{
	if (!qti_is_secure_io_access_allowed(addr)) {
		return QTI_SIP_INVALID_PARAM;
	}

	*((volatile uint32_t *)addr) = value;
	return QTI_SIP_SUCCESS;
}

static bool qti_mem_assign_validate_param(qti_accesscontrol_mem_t *mem_info,
					  u_register_t u_num_mappings,
					  uint32_t *src_vm_list,
					  u_register_t src_vm_list_cnt,
					  qti_accesscontrol_perm_t *dst_vm_list,
					  u_register_t dst_vm_list_cnt)
{
	u_register_t end;
	int i;

	if ((src_vm_list == NULL) || (dst_vm_list == NULL)
	    || (mem_info == NULL) || (src_vm_list_cnt == 0)
	    || (src_vm_list_cnt >= QTI_VM_LAST) || (dst_vm_list_cnt == 0)
	    || (dst_vm_list_cnt >= QTI_VM_LAST) || (u_num_mappings == 0)
	    || u_num_mappings > QTI_VM_MAX_LIST_SIZE) {
		ERROR("vm count is 0 or more then QTI_VM_LAST or empty list\n");
		ERROR("src %p/%u dst %p/%u mem %p/%u\n", src_vm_list,
		     (unsigned int)src_vm_list_cnt, dst_vm_list,
		     (unsigned int)dst_vm_list_cnt, mem_info,
		     (unsigned int)u_num_mappings);
		return false;
	}

	for (i = 0; i < u_num_mappings; i++) {
		if (((mem_info[i].mem_addr & (SIZE4K - 1)) != 0)
		    || (mem_info[i].mem_size == 0)
		    || ((mem_info[i].mem_size & (SIZE4K - 1)) != 0)) {
			ERROR("mem_info passed buffer 0x%x or size 0x%x is not 4k aligned\n",
			     (unsigned int)mem_info[i].mem_addr,
			     (unsigned int)mem_info[i].mem_size);
			return false;
		}

		if (add_overflow(mem_info[i].mem_addr, mem_info[i].mem_size,
				 &end) != 0) {
			ERROR("overflow in mem_addr 0x%x add mem_size 0x%x\n",
			      (unsigned int)mem_info[i].mem_addr,
			      (unsigned int)mem_info[i].mem_size);
			return false;
		}

#if COREBOOT == 1
		coreboot_memory_t mem_type = coreboot_get_memory_type(
						mem_info[i].mem_addr,
						mem_info[i].mem_size);
		if (mem_type != CB_MEM_RAM && mem_type != CB_MEM_RESERVED) {
			ERROR("memory region not RAM/reserved: addr 0x%x size 0x%x\n",
			     (unsigned int)mem_info[i].mem_addr,
			     (unsigned int)mem_info[i].mem_size);
			return false;
		}
#endif
	}
	for (i = 0; i < src_vm_list_cnt; i++) {
		if (src_vm_list[i] >= QTI_VM_LAST) {
			ERROR("src_vm_list[%d] 0x%x is more then QTI_VM_LAST\n",
			      i, (unsigned int)src_vm_list[i]);
			return false;
		}
	}
	for (i = 0; i < dst_vm_list_cnt; i++) {
		if (dst_vm_list[i].dst_vm >= QTI_VM_LAST) {
			ERROR("dst_vm_list[%d] 0x%x is more then QTI_VM_LAST\n",
			      i, (unsigned int)dst_vm_list[i].dst_vm);
			return false;
		}
	}
	return true;
}

static int get_indirect_args(uint32_t smccc, u_register_t *x5,
			     u_register_t *x6, u_register_t *x7)
{
	const uintptr_t addr = (uintptr_t)*x5;
	int ret = -EPERM;
	size_t len = 0;

	if (smccc == SMC_32) {
		len = sizeof(uint32_t) * 4;
	} else {
		len = sizeof(uint64_t) * 4;
	}

	ret = qti_mmap_add_dynamic_region(addr, len, MT_NS | MT_RO_DATA);
	if (ret != 0) {
		ERROR("map failed for params NS Buffer 0x%lx 0x%lx\n",
		      (unsigned long)addr, (unsigned long)len);
		return ret;
	}

	if (smccc == SMC_32) {
		const uint32_t *args = (const uint32_t *)addr;

		*x5 = args[0];
		*x6 = args[1];
		*x7 = args[2];
	} else {
		const uint64_t *args = (const uint64_t *)addr;

		*x5 = args[0];
		*x6 = args[1];
		*x7 = args[2];
	}

	ret = qti_mmap_remove_dynamic_region(addr, len);
	if (ret != 0) {
		ERROR("unmap failed for params NS Buffer 0x%lx 0x%lx\n",
		      (unsigned long)addr, (unsigned long)len);
	}

	return ret;
}

static int get_mem_params(struct qti_mmap_params *params,
			  u_register_t x2, u_register_t x3, u_register_t x4,
			  u_register_t x5, u_register_t x6, u_register_t x7)
{
	u_register_t e2, e4, e6;
	u_register_t start = 0;
	u_register_t end = 0;
	u_register_t len = 0;
	bool rc = false;
	int ret = -EINVAL;
	int ret1 = -EINVAL;

	/* Overflow check:
	 *   args 2,4,6 contain buffer addresses
	 *   args 3,5,7 contain buffer sizes
	 */
	if (x2 == 0 || x4 == 0 || x6 == 0) {
		return -EINVAL;
	}

	if (add_overflow(x2, x3, &e2) != 0 ||
	    add_overflow(x4, x5, &e4) != 0 ||
	    add_overflow(x6, x7, &e6) != 0) {
		ERROR("map failed for params NS Buffer2, invalid params\n");
		return -EINVAL;
	}

	start = MIN(x2, x4);
	start = MIN(start, x6);
	end = MAX(e2, e4);
	end = MAX(end, e6);
	len = end - start;

	ret = qti_mmap_add_dynamic_region((uintptr_t)start, (size_t)len,
					  (MT_NS | MT_RO_DATA));
	if (ret != 0) {
		ERROR("map failed for params NS Buffer2 0x%lx 0x%lx\n",
		      (unsigned long)start, (unsigned long)len);
		return ret;
	}

	/* Parameter validation */
	ret = -EINVAL;

	if ((x3 % sizeof(qti_accesscontrol_mem_t)) != 0U ||
	    (x5 % sizeof(uint32_t)) != 0U ||
	    (x7 % sizeof(qti_accesscontrol_perm_t)) != 0U) {
		ERROR("invalid parameter buffer sizes\n");
		goto error;
	}

	params->mem_cnt = x3 / sizeof(params->mem[0]);
	if (params->mem_cnt > ARRAY_SIZE(params->mem)) {
		ERROR("Param validation failed\n");
		goto error;
	}
	memcpy(params->mem, (void *)(uintptr_t)x2,
	       params->mem_cnt * sizeof(params->mem[0]));

	params->src_cnt = x5 / sizeof(params->src[0]);
	if (params->src_cnt >= ARRAY_SIZE(params->src)) {
		ERROR("Param validation failed\n");
		goto error;
	}
	memcpy(params->src, (void *)(uintptr_t)x4,
	       params->src_cnt * sizeof(params->src[0]));

	params->dst_cnt = x7 / sizeof(params->dst[0]);
	if (params->dst_cnt >= ARRAY_SIZE(params->dst)) {
		ERROR("Param validation failed\n");
		goto error;
	}
	memcpy(params->dst, (void *)(uintptr_t)x6,
	       params->dst_cnt * sizeof(params->dst[0]));

	rc = qti_mem_assign_validate_param(params->mem, params->mem_cnt,
					   params->src, params->src_cnt,
					   params->dst, params->dst_cnt);
	if (rc != true) {
		ERROR("Param validation failed\n");
	}

	ret = rc == true ? 0 : -EINVAL;
error:
	ret1 = qti_mmap_remove_dynamic_region((uintptr_t)start, (size_t)len);
	if (ret1 != 0) {
		ERROR("unmap failed for params NS Buffer 0x%lx 0x%lx\n",
		      (unsigned long)start, (unsigned long)len);
	}

	return ret ? ret : ret1;
}

int qti_sip_mem_assign(void *handle, uint32_t smc_cc,
		       u_register_t x1, u_register_t x2,
		       u_register_t x3, u_register_t x4,
		       int *operation_ret)
{
	struct qti_mmap_params params = { 0 };
	int ret = QTI_SIP_NOT_SUPPORTED;
	u_register_t x5, x6, x7;

	if (x1 != QTI_SIP_SVC_MEM_ASSIGN_PARAM_ID) {
		ERROR("invalid mem_assign param id\n");
		goto out;
	}

	x5 = read_ctx_reg(get_gpregs_ctx(handle), CTX_GPREG_X5);
	if (x5 == 0x0) {
		ERROR("no mem_assign mapping info\n");
		goto out;
	}

	if (smc_cc == SMC_32) {
		x5 = (uint32_t)x5;
	}

	ret = get_indirect_args(smc_cc, &x5, &x6, &x7);
	if (ret != 0)
		goto out;

	ret = get_mem_params(&params, x2, x3, x4, x5, x6, x7);
	if (ret != 0)
		goto out;

	ret = qti_accesscontrol_mem_assign(params.mem, params.mem_cnt,
					   params.src, params.src_cnt,
					   params.dst, params.dst_cnt);
out:
	*operation_ret = ret;
	return ret == 0 ? QTI_SIP_SUCCESS : QTI_SIP_INVALID_PARAM;
}

int qti_sip_config_cpu_errata(u_register_t x1)
{
	(void)x1;
	return QTI_SIP_NOT_SUPPORTED;
}

int qti_sip_get_arm_feature_status(u_register_t *status)
{
	(void)status;
	return QTI_SIP_NOT_SUPPORTED;
}

int qti_sip_config_hw_for_offline_ram_dump(u_register_t x2,
					    u_register_t x3)
{
	(void)x2;
	(void)x3;
	return QTI_SIP_NOT_SUPPORTED;
}

int qti_sip_get_subsystem_debug_options(u_register_t x1,
					u_register_t *response0,
					u_register_t *response1,
					u_register_t *response2)
{
	(void)x1;
	(void)response0;
	(void)response1;
	(void)response2;
	return QTI_SIP_NOT_SUPPORTED;
}
