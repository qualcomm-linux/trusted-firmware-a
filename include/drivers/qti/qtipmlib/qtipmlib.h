/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef QTI_PMLIB_H
#define QTI_PMLIB_H

enum qtipmlib_status {
	QTIPMLIB_SUCCESS = 0,
	QTIPMLIB_E_INVAL = -1,
	QTIPMLIB_E_NOMEM = -2,
	QTIPMLIB_E_NOT_FOUND = -3,
};

enum qtipmlib_lpm_node_id {
	QTIPMLIB_LPM_CPU0,
	QTIPMLIB_LPM_CPU1,
	QTIPMLIB_LPM_CPU2,
	QTIPMLIB_LPM_CPU3,
	QTIPMLIB_LPM_L3,
	QTIPMLIB_LPM_NR_NODES,
};

#ifdef QTI_PMLIB_ENABLED
void qti_qtipmlib_init(void);
int qtipmlib_init(void);
int qtipmlib_config_lpm(enum qtipmlib_lpm_node_id node_id, int mode_id);
#else
static inline void qti_qtipmlib_init(void)
{
}

static inline int qtipmlib_init(void)
{
	return QTIPMLIB_SUCCESS;
}

static inline int qtipmlib_config_lpm(enum qtipmlib_lpm_node_id node_id,
				      int mode_id)
{
	(void)node_id;
	(void)mode_id;

	return QTIPMLIB_SUCCESS;
}
#endif

#endif /* QTI_PMLIB_H */
