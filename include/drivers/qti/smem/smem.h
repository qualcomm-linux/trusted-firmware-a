/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SMEM_H
#define SMEM_H

#include <stdbool.h>
#include <stdint.h>

#include "smem_type.h"

/* SMEM Status return codes */
#define SMEM_STATUS_SUCCESS	0
#define SMEM_STATUS_FAILURE	-1
#define SMEM_STATUS_INVALID_PARAM	-2
#define SMEM_STATUS_OUT_OF_RESOURCES	-3
#define SMEM_STATUS_NOT_FOUND	-4
#define SMEM_STATUS_DUPLICATE	-5
#define SMEM_STATUS_SIZE_MISMATCH	-6

/* Flags for use with smem_alloc_ex() */
#define SMEM_ALLOC_FLAG_NONE	0 /* Default behavior */

/*
 * Parameters structure for smem_alloc_ex and smem_get_addr_ex
 */
struct smem_alloc_params {
	/*
	 * Remote endpoint with which this SMEM item will be shared. Input param.
	 * This field tells the allocation routine which partition to allocate from.
	 * If set to SMEM_INVALID_HOST, then the allocation will be from the
	 * default, unprotected partition.
	 */
	smem_host_type remote_host;
	/*
	 * Identifier for the allocation. Input param.
	 * Allowed types are in the smem_mem_type enum.
	 */
	enum smem_mem_type smem_type;
	/*
	 * Requested size of the allocation. This is an in/out parameter.
	 * Note that allocated size may be larger than requested.
	 * Actual allocated size is updated here upon successful
	 * allocation or address lookup.
	 * Caller should always check the size param after the call to determine
	 * the actual size, which may not match what was requested.
	 */
	uint32_t size;
	/*
	 * Pointer to allocated buffer. This is an in/out parameter.
	 * Allocation routine will set the pointer to the
	 * allocated buffer upon successful allocation or address lookup.
	 */
	void *buffer;
	/*
	 * Flags for the allocation. Input param.
	 * See SMEM_ALLOC_FLAG for options.
	 */
	uint32_t flags;
};

/*
 * smem_init() - Initializes the shared memory allocation structures
 *
 * Shared memory must have been cleared and initialized by the first system
 * bootloader, typically running on the modem processor. Even though calls to
 * this function are gated by the smem_init_finished flag, this function
 * must still be implemented to handle simultaneous calls from multiple
 * threads.
 */
void smem_init(void);

/*
 * smem_alloc() - Requests a pointer to a buffer in shared memory
 */
void *smem_alloc(enum smem_mem_type smem_type, uint32_t buf_size);

/*
 * smem_alloc_ex() - Requests a pointer to a buffer in shared memory.
 *                   Upon success, params.buffer
 *                   points to the allocation in shared memory.
 */
int32_t smem_alloc_ex(struct smem_alloc_params *params);

/*
 * smem_get_static_addr - Requests a pointer to a statically-allocated buffer
 *                        in shared memory
 *
 * This macro calls smem_alloc() directly and is intended as a
 * backward-compatible function for clients that still call the function
 * directly.
 *
 * After all clients have converted back to smem_alloc() for all data items,
 * remove this macro. New code must call smem_alloc() directly, regardless of
 * the allocation class of the shared memory item.
 */
#define smem_get_static_addr	smem_alloc

/*
 * smem_get_addr() - Requests the address and size of an allocated buffer in
 *                   shared memory. Newly allocated shared memory buffers,
 *                   which have never been allocated on any processor,
 *                   are guaranteed to be zero-initialized.
 */
void *smem_get_addr(enum smem_mem_type smem_type, uint32_t *buf_size);

/*
 * smem_get_addr_ex() - Requests the address and size of an allocated buffer
 *                      in shared memory. If found, sets the buffer and size
 *                      fields of the params struct.
 */
int32_t smem_get_addr_ex(struct smem_alloc_params *params);

/*
 * smem_version_set() - Sets the version number for this processor and a given
 *                      object. The version number is compared to all previously
 *                      set version numbers for this object. The last processor
 *                      to register checks against all other processors.
 */
bool smem_version_set(enum smem_mem_type type, uint32_t version, uint32_t mask);

#endif /* SMEM_H */
