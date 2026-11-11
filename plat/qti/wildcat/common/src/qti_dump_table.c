/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * SMC handler for TZ_DUMP_ASSIGN_DUMP_TABLE_REGION (SMC ID 0x02000313).
 *
 * Allows HLOS to register a physically-contiguous, 4K-aligned DDR buffer as
 * the system dump table so that AOP can read it during SDI pass-1.
 *
 * On assignment:
 *   1. Input is validated (4K-aligned, no overflow, NS HLOS range only).
 *   2. AC assignment grants AOP (RPM) + HLOS + BL31 read/write access via
 *      qti_accesscontrol_mem_assign().
 *   3. The buffer is dynamically mapped into the EL3 address space.
 *   4. The address and size are published to Shared IMEM for AOP.
 *
 * De-assignment reverses every step in reverse order.
 */

#include <errno.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <common/debug.h>
#include <drivers/qti/accesscontrol/accesscontrol.h>
#include <lib/mmio.h>
#include <lib/xlat_tables/xlat_tables_defs.h>
#include <lib/xlat_tables/xlat_tables_v2.h>

#include <bl31qtilib_interface.h>
#include <qti_plat.h>

/*
 * Shared IMEM layout (SHARED_IMEM_BASE is defined per-platform in platform_def.h).
 *
 *   Offset 0x010 -- 8-byte dump-table base pointer AOP reads on SDI pass-1.
 *   Offset 0x000 -- 4-byte dump-table size.
 */
#define DUMP_IMEM_ADDR_OFFSET   UL(0x010)
#define DUMP_IMEM_SIZE_OFFSET   UL(0x000)

#define IS_NOT_4K_ALIGNED(x)   ((uintptr_t)(x) & 0xFFFUL)

/* EL3-private state; zero means no region is currently registered. */
static uintptr_t g_dump_table_addr;
static size_t    g_dump_table_size;

static void publish_to_shared_imem(uintptr_t addr, size_t size)
{
	mmio_write_64(SHARED_IMEM_BASE + DUMP_IMEM_ADDR_OFFSET, (uint64_t)addr);
	mmio_write_32(SHARED_IMEM_BASE + DUMP_IMEM_SIZE_OFFSET, (uint32_t)size);
}

/*
 * Wraps qti_accesscontrol_mem_assign for the dump-table use case.
 *
 * Assign  (is_assign=true):
 *   src={HLOS}, dst={RPM:RW, HLOS:RW, TZ:RW}
 *
 * De-assign (is_assign=false):
 *   src={HLOS, RPM, TZ}, dst={HLOS:RW}
 */
static int assign_memory(bool is_assign, uintptr_t addr, size_t size)
{
	qti_accesscontrol_mem_t mem = { .mem_addr = addr, .mem_size = size };

	uint32_t src_assign[]  = { AC_VM_HLOS };
	uint32_t src_deassign[] = { AC_VM_HLOS, AC_VM_RPM, AC_VM_TZ };

	qti_accesscontrol_perm_t dst_assign[] = {
		{ .dst_vm = AC_VM_RPM,  .dst_vm_perm = (AC_PERM_R | AC_PERM_W) },
		{ .dst_vm = AC_VM_HLOS, .dst_vm_perm = (AC_PERM_R | AC_PERM_W) },
		{ .dst_vm = AC_VM_TZ,   .dst_vm_perm = (AC_PERM_R | AC_PERM_W) },
	};
	qti_accesscontrol_perm_t dst_deassign[] = {
		{ .dst_vm = AC_VM_HLOS, .dst_vm_perm = (AC_PERM_R | AC_PERM_W) },
	};

	if (is_assign) {
		return (int)qti_accesscontrol_mem_assign(
			&mem, 1U,
			src_assign, ARRAY_SIZE(src_assign),
			dst_assign, ARRAY_SIZE(dst_assign));
	} else {
		return (int)qti_accesscontrol_mem_assign(
			&mem, 1U,
			src_deassign, ARRAY_SIZE(src_deassign),
			dst_deassign, ARRAY_SIZE(dst_deassign));
	}
}

/*
 * qti_assign_dump_table_region - handler called from qti_syscall.c.
 *
 * @is_assign: non-zero to register the buffer, zero to deregister.
 * @addr:      physical base address (must be 4K-aligned, non-zero).
 * @size:      byte length (must be 4K-aligned, non-zero, <= UINT32_MAX).
 *
 * Returns 0 on success, negative errno otherwise.
 */
int qti_assign_dump_table_region(uintptr_t is_assign, uintptr_t addr, size_t size)
{
	int ret;

	/* ---- Common input validation ---- */
	if (addr == 0UL || size == 0UL) {
		ERROR("DUMP_TABLE: null addr or zero size (addr=0x%lx size=0x%lx)\n",
		      addr, (unsigned long)size);
		return -EINVAL;
	}

	if (size > (size_t)UINT32_MAX) {
		ERROR("DUMP_TABLE: size 0x%lx exceeds 32-bit limit\n",
		      (unsigned long)size);
		return -EINVAL;
	}

	/* addr + size must not wrap. */
	if (size > (UINTPTR_MAX - addr)) {
		ERROR("DUMP_TABLE: addr+size overflow (addr=0x%lx size=0x%lx)\n",
		      addr, (unsigned long)size);
		return -EINVAL;
	}

	if (IS_NOT_4K_ALIGNED(addr) || IS_NOT_4K_ALIGNED(size)) {
		ERROR("DUMP_TABLE: not 4K-aligned (addr=0x%lx size=0x%lx)\n",
		      addr, (unsigned long)size);
		return -EINVAL;
	}

	/* ---- Direction-specific pre-conditions ---- */
	if (is_assign != 0UL) {
		if (g_dump_table_addr != 0UL || g_dump_table_size != 0UL) {
			ERROR("DUMP_TABLE: already registered (cur=0x%lx/0x%lx)\n",
			      g_dump_table_addr, (unsigned long)g_dump_table_size);
			return -EINVAL;
		}

	} else {
		if (addr != g_dump_table_addr || size != g_dump_table_size) {
			ERROR("DUMP_TABLE: deassign mismatch "
			      "(req=0x%lx/0x%lx reg=0x%lx/0x%lx)\n",
			      addr, (unsigned long)size,
			      g_dump_table_addr, (unsigned long)g_dump_table_size);
			return -EINVAL;
		}
	}

	/* ---- Assign ---- */
	if (is_assign != 0UL) {
		/*
		 * Grant AOP (RPM) + HLOS + BL31 read/write access.
		 * Mirrors ACTZAssign(src={HLOS}, dst={RPM:RW, HLOS:RW, TZ:RW}).
		 */
		ret = assign_memory(true, addr, size);
		if (ret != 0) {
			ERROR("DUMP_TABLE: AC assign failed (ret=%d)\n", ret);
			return -EIO;
		}

		g_dump_table_addr = addr;
		g_dump_table_size = size;
		publish_to_shared_imem(addr, size);

		/*
		 * Map the buffer into EL3 as NS cached RW memory so BL31 can
		 * access it directly.
		 */
		ret = qti_mmap_add_dynamic_region(addr, size,
						  MT_MEMORY | MT_RW | MT_NS |
						  MT_EXECUTE_NEVER);
		if (ret != 0) {
			ERROR("DUMP_TABLE: EL3 mmap failed "
			      "(addr=0x%lx size=0x%lx ret=%d)\n",
			      addr, (unsigned long)size, ret);
			/*
			 * Roll back: clear state and IMEM, then revoke AC.
			 * If AC rollback fails the permission state is
			 * unrecoverable -- panic rather than return an
			 * inconsistent state to the caller.
			 */
			publish_to_shared_imem(0UL, 0UL);
			g_dump_table_addr = 0UL;
			g_dump_table_size = 0UL;
			if (assign_memory(false, addr, size) != 0) {
				ERROR("DUMP_TABLE: AC rollback failed\n");
				panic();
			}
			return -EIO;
		}

		INFO("DUMP_TABLE: registered addr=0x%lx size=0x%lx\n",
		     addr, (unsigned long)size);

	/* ---- De-assign ---- */
	} else {
		/* Unmap from EL3 first, then revoke AC, then clear state. */
		ret = qti_mmap_remove_dynamic_region(g_dump_table_addr,
						     g_dump_table_size);
		if (ret != 0) {
			ERROR("DUMP_TABLE: EL3 unmap failed "
			      "(addr=0x%lx size=0x%lx ret=%d)\n",
			      g_dump_table_addr, (unsigned long)g_dump_table_size,
			      ret);
			return -EIO;
		}

		/*
		 * Revoke RPM + TZ access, returning buffer to HLOS-only.
		 * Mirrors ACTZAssign(src={HLOS,RPM,TZ}, dst={HLOS:RW}).
		 *
		 * If this fails after the EL3 unmap, restore the mapping so
		 * the full state (mapping + permissions + IMEM) remains
		 * consistent and the caller can retry. If the re-map also
		 * fails the state is unrecoverable -- panic.
		 */
		ret = assign_memory(false, g_dump_table_addr, g_dump_table_size);
		if (ret != 0) {
			ERROR("DUMP_TABLE: AC deassign failed (ret=%d)\n", ret);
			if (qti_mmap_add_dynamic_region(
					g_dump_table_addr, g_dump_table_size,
					MT_MEMORY | MT_RW | MT_NS |
					MT_EXECUTE_NEVER) != 0) {
				ERROR("DUMP_TABLE: EL3 re-map failed\n");
				panic();
			}
			return -EIO;
		}

		g_dump_table_addr = 0UL;
		g_dump_table_size = 0UL;
		publish_to_shared_imem(0UL, 0UL);

		INFO("DUMP_TABLE: deregistered\n");
	}

	return 0;
}

/*
 * qti_get_dump_table_region - returns the currently registered region.
 *
 * Returns 0 and fills *addr / *size if a region is registered.
 * Returns -ENOENT if nothing is registered.
 */
int qti_get_dump_table_region(uintptr_t *addr, uintptr_t *size)
{
	if (addr == NULL || size == NULL) {
		return -EINVAL;
	}

	if (g_dump_table_addr == 0UL || g_dump_table_size == 0UL) {
		return -ENOENT;
	}

	*addr = g_dump_table_addr;
	*size = (uintptr_t)g_dump_table_size;
	return 0;
}
