/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#ifndef QTI_MBOX_TYPES_H
#define QTI_MBOX_TYPES_H

/*
 * Shared type definitions for the Qualcomm mailbox framework.
 *
 * This header is included by both the framework core (drivers/qti/mbox/)
 * and platform integration code (plat/qti/...).  It contains only the
 * structural types required for static channel and slot allocation.
 *
 * The full definition of struct qti_mbox_ops (the transport vtable) is
 * kept in qti_mbox_private.h (driver-internal).  Platform code uses only
 * a pointer to struct qti_mbox_ops and does not need the full definition.
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/*
 * Forward declaration of the transport operations table.
 *
 * The full definition is in qti_mbox_private.h (driver-internal).
 * Platform code holds only a const pointer to this type via
 * qti_mbox_chan_config.ops and does not need the full definition.
 */
struct qti_mbox_ops;

/*
 * Runtime channel state.  One instance per slot; zeroed on release.
 *
 * cfg:             back-pointer to the immutable channel configuration.
 * ready:           true after a successful init(); cleared on release.
 * mtu:             effective TX capacity in bytes; 0 until transport
 *                  reports it.
 * pending_events:  accumulated edge/level events not yet returned to caller.
 * sticky_events:   latched error events; cleared only on release.
 *
 * Note: these fields are managed exclusively by the framework core
 * (qti_mbox.c) and the transport backend.  Platform code must not
 * access them directly.
 */
struct qti_mbox_chan {
	const struct qti_mbox_chan_config *cfg;
	bool ready;
	size_t mtu;
	uint32_t pending_events;
	uint32_t sticky_events;
};

/*
 * Immutable per-channel configuration supplied by platform code.
 *
 * name:            unique channel identifier string; must be non-NULL and
 *                  non-empty.
 * ops:             pointer to the transport operations table; all function
 *                  pointers must be non-NULL.
 * transport_cfg:   pointer to immutable transport-specific configuration;
 *                  interpretation is transport-defined.
 * transport_priv:  pointer to mutable transport-specific runtime state;
 *                  interpretation is transport-defined.
 */
struct qti_mbox_chan_config {
	const char *name;
	const struct qti_mbox_ops *ops;
	const void *transport_cfg;
	void *transport_priv;
};

/*
 * Runtime slot.  Configuration entry N maps permanently to slot N.
 *
 * chan:    embedded channel state; managed by the framework core.
 * in_use:  true while the slot is claimed by a caller; protected by the
 *          framework's internal spinlock.
 */
struct qti_mbox_chan_slot {
	struct qti_mbox_chan chan;
	bool in_use;
};

#endif /* QTI_MBOX_TYPES_H */
