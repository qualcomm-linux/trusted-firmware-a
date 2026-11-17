/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef QTI_PMLIB_H
#define QTI_PMLIB_H

#include <stdint.h>

#define QTIPMLIB_ACD_MAX_VALUES	7U

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

enum qtipmlib_osm_domain_id {
	QTIPMLIB_OSM_L3,
	QTIPMLIB_OSM_SILVER,
	QTIPMLIB_OSM_GOLD,
	QTIPMLIB_OSM_NR_DOMAINS,
};

typedef struct {
	uint8_t vc[QTIPMLIB_ACD_MAX_VALUES];
	uint32_t value[QTIPMLIB_ACD_MAX_VALUES];
} qtipmlib_acd_lut_t;

typedef struct {
	qtipmlib_acd_lut_t dvm;
	qtipmlib_acd_lut_t avg;
	qtipmlib_acd_lut_t acdsscr;
} qtipmlib_acd_config_t;

#ifdef QTI_PMLIB_ENABLED
void qti_qtipmlib_init(void);
int qtipmlib_init(void);
int qtipmlib_config_lpm(enum qtipmlib_lpm_node_id node_id, int mode_id);
int qtipmlib_config_acd(enum qtipmlib_osm_domain_id domain_id,
			 const qtipmlib_acd_config_t *config);
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

static inline int qtipmlib_config_acd(enum qtipmlib_osm_domain_id domain_id,
				      const qtipmlib_acd_config_t *config)
{
	(void)domain_id;
	(void)config;

	return QTIPMLIB_SUCCESS;
}
#endif

#endif /* QTI_PMLIB_H */
