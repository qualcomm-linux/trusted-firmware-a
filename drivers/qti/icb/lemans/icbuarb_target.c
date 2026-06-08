/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Lemans (QCS9075) ICB micro-arbiter target description: BCM nodes,
 * masters, slaves and the routes the NoC error logger needs voted ON
 * before it can program registers.
 */

#include <stddef.h>

#include "icbuarbi.h"

/* BCM hardware nodes */

static struct icb_hw_request bcm_acv_requests[1];
static struct icb_hw_node bcm_acv = {
	NULL, "ACV", ICB_HW_NODE_KIND_VECTOR, 0, 0, 0, 0,
	false, 0, {0, 0}, {0, 1, bcm_acv_requests}
};

static struct icb_hw_request bcm_ce0_requests[2];
static struct icb_hw_node bcm_ce0 = {
	NULL, "CE0", ICB_HW_NODE_KIND_BANDWIDTH, 0, 0, 0, 0,
	false, 0, {0, 0}, {0, 2, bcm_ce0_requests}
};

static struct icb_hw_request bcm_cn0_requests[1];
static struct icb_hw_node bcm_cn0 = {
	NULL, "CN0", ICB_HW_NODE_KIND_BANDWIDTH, 0, 0, 0, 0,
	false, 0, {0, 0}, {0, 1, bcm_cn0_requests}
};

static struct icb_hw_request bcm_cn1_requests[3];
static struct icb_hw_node bcm_cn1 = {
	NULL, "CN1", ICB_HW_NODE_KIND_BANDWIDTH, 0, 0, 0, 0,
	false, 0, {0, 0}, {0, 3, bcm_cn1_requests}
};

static struct icb_hw_request bcm_cn2_requests[4];
static struct icb_hw_node bcm_cn2 = {
	NULL, "CN2", ICB_HW_NODE_KIND_BANDWIDTH, 0, 0, 0, 0,
	false, 0, {0, 0}, {0, 4, bcm_cn2_requests}
};

static struct icb_hw_request bcm_mc0_requests[1];
static struct icb_hw_node bcm_mc0 = {
	NULL, "MC0", ICB_HW_NODE_KIND_BANDWIDTH, 0, 0, 0, 0,
	false, 0, {0, 0}, {0, 1, bcm_mc0_requests}
};

static struct icb_hw_request bcm_mm0_requests[2];
static struct icb_hw_node bcm_mm0 = {
	NULL, "MM0", ICB_HW_NODE_KIND_BANDWIDTH, 0, 0, 0, 0,
	false, 0, {0, 0}, {0, 2, bcm_mm0_requests}
};

static struct icb_hw_request bcm_sh0_requests[1];
static struct icb_hw_node bcm_sh0 = {
	NULL, "SH0", ICB_HW_NODE_KIND_BANDWIDTH, 0, 0, 0, 0,
	false, 0, {0, 0}, {0, 1, bcm_sh0_requests}
};

static struct icb_hw_request bcm_sh2_requests[1];
static struct icb_hw_node bcm_sh2 = {
	NULL, "SH2", ICB_HW_NODE_KIND_BANDWIDTH, 0, 0, 0, 0,
	false, 0, {0, 0}, {0, 1, bcm_sh2_requests}
};

static struct icb_hw_request bcm_sn0_requests[1];
static struct icb_hw_node bcm_sn0 = {
	NULL, "SN0", ICB_HW_NODE_KIND_BANDWIDTH, 0, 0, 0, 0,
	false, 0, {0, 0}, {0, 1, bcm_sn0_requests}
};

static struct icb_hw_request bcm_sn3_requests[2];
static struct icb_hw_node bcm_sn3 = {
	NULL, "SN3", ICB_HW_NODE_KIND_BANDWIDTH, 0, 0, 0, 0,
	false, 0, {0, 0}, {0, 2, bcm_sn3_requests}
};

static struct icb_hw_request bcm_sn4_requests[2];
static struct icb_hw_node bcm_sn4 = {
	NULL, "SN4", ICB_HW_NODE_KIND_BANDWIDTH, 0, 0, 0, 0,
	false, 0, {0, 0}, {0, 2, bcm_sn4_requests}
};

/* Masters */

static struct icb_hw_node *master_chm_apps_hw_nodes[] = { &bcm_sh2 };
static struct icb_master master_chm_apps = {
	ICBID_MASTER_APPSS_PROC, 32, 4, {0, 0}, {0, 0, NULL},
	1, master_chm_apps_hw_nodes
};

static struct icb_master master_llcc_mc = {
	ICBID_MASTER_LLCC, 4, 8, {0, 0}, {0, 0, NULL}, 0, NULL
};

static struct icb_hw_node *master_qnm_aggre1_noc_hw_nodes[] = { &bcm_sn3 };
static struct icb_master master_qnm_aggre1_noc = {
	ICBID_MASTER_A1NOC_SNOC, 32, 1, {0, 0}, {0, 0, NULL},
	1, master_qnm_aggre1_noc_hw_nodes
};

static struct icb_hw_node *master_qnm_aggre2_noc_hw_nodes[] = { &bcm_sn4 };
static struct icb_master master_qnm_aggre2_noc = {
	ICBID_MASTER_A2NOC_SNOC, 16, 1, {0, 0}, {0, 0, NULL},
	1, master_qnm_aggre2_noc_hw_nodes
};

static struct icb_hw_node *master_qnm_gemnoc_cnoc_hw_nodes[] = { &bcm_cn0 };
static struct icb_master master_qnm_gemnoc_cnoc = {
	ICBID_MASTER_GEM_NOC_CNOC, 16, 1, {0, 0}, {0, 0, NULL},
	1, master_qnm_gemnoc_cnoc_hw_nodes
};

static struct icb_master master_qnm_snoc_sf = {
	ICBID_MASTER_SNOC_SF_MEM_NOC, 16, 1, {0, 0}, {0, 0, NULL}, 0, NULL
};

static struct icb_bw_req *master_qnm_mnoc_hf_sw_reqs[4];
static struct icb_master master_qnm_mnoc_hf = {
	ICBID_MASTER_MNOC_HF_MEM_NOC, 32, 2, {0, 0},
	{0, 4, master_qnm_mnoc_hf_sw_reqs}, 0, NULL
};

static struct icb_master master_qnm_pcie = {
	ICBID_MASTER_ANOC_PCIE_GEM_NOC, 32, 1, {0, 0}, {0, 0, NULL}, 0, NULL
};

static struct icb_master master_qnm_gpdsp_sail = {
	ICBID_MASTER_GPDSP_SAIL, 16, 1, {0, 0}, {0, 0, NULL}, 0, NULL
};

static struct icb_hw_node *master_qxm_crypto_0_hw_nodes[] = { &bcm_ce0 };
static struct icb_master master_qxm_crypto_0 = {
	ICBID_MASTER_CRYPTO_CORE0, 8, 1, {0, 0}, {0, 0, NULL},
	1, master_qxm_crypto_0_hw_nodes
};

static struct icb_hw_node *master_qnm_mdp0_0_hw_nodes[] = { &bcm_mm0 };
static struct icb_master master_qnm_mdp0_0 = {
	ICBID_MASTER_MDP0, 32, 1, {0, 0}, {0, 0, NULL},
	1, master_qnm_mdp0_0_hw_nodes
};

/* Slaves */

static struct icb_hw_node *slave_ebi_hw_nodes[] = { &bcm_mc0, &bcm_acv };
static struct icb_slave slave_ebi = {
	ICBID_SLAVE_EBI1, 4, 8, {0, 0}, {0, 0, NULL}, 2, slave_ebi_hw_nodes
};

static struct icb_hw_node *slave_qns_llcc_hw_nodes[] = { &bcm_sh0 };
static struct icb_slave slave_qns_llcc = {
	ICBID_SLAVE_LLCC, 16, 6, {0, 0}, {0, 0, NULL},
	1, slave_qns_llcc_hw_nodes
};

static struct icb_slave slave_qns_gem_noc_cnoc = {
	ICBID_SLAVE_GEM_NOC_CNOC, 16, 1, {0, 0}, {0, 0, NULL}, 0, NULL
};

static struct icb_hw_node *slave_qns_gemnoc_sf_hw_nodes[] = { &bcm_sn0 };
static struct icb_slave slave_qns_gemnoc_sf = {
	ICBID_SLAVE_SNOC_GEM_NOC_SF, 16, 1, {0, 0}, {0, 0, NULL},
	1, slave_qns_gemnoc_sf_hw_nodes
};

static struct icb_hw_node *slave_qns_a1noc_snoc_hw_nodes[] = { &bcm_sn3 };
static struct icb_slave slave_qns_a1noc_snoc = {
	ICBID_SLAVE_A1NOC_SNOC, 32, 1, {0, 0}, {0, 0, NULL},
	1, slave_qns_a1noc_snoc_hw_nodes
};

static struct icb_hw_node *slave_qns_a2noc_snoc_hw_nodes[] = { &bcm_sn4 };
static struct icb_slave slave_qns_a2noc_snoc = {
	ICBID_SLAVE_A2NOC_SNOC, 16, 1, {0, 0}, {0, 0, NULL},
	1, slave_qns_a2noc_snoc_hw_nodes
};

static struct icb_hw_node *slave_qns_mem_noc_hf_hw_nodes[] = { &bcm_mm0 };
static struct icb_bw_req *slave_qns_mem_noc_hf_sw_reqs[4];
static struct icb_slave slave_qns_mem_noc_hf = {
	ICBID_SLAVE_MNOC_HF_MEM_NOC, 32, 2, {0, 0},
	{0, 4, slave_qns_mem_noc_hf_sw_reqs},
	1, slave_qns_mem_noc_hf_hw_nodes
};

static struct icb_hw_node *slave_qhs_crypto0_cfg_hw_nodes[] = { &bcm_cn1 };
static struct icb_slave slave_qhs_crypto0_cfg = {
	ICBID_SLAVE_CRYPTO_0_CFG, 4, 1, {0, 0}, {0, 0, NULL},
	1, slave_qhs_crypto0_cfg_hw_nodes
};

static struct icb_hw_node *slave_qns_pcie_mem_noc_hw_nodes[] = { &bcm_sn3 };
static struct icb_slave slave_qns_pcie_mem_noc = {
	ICBID_SLAVE_ANOC_PCIE_GEM_NOC, 32, 1, {0, 0}, {0, 0, NULL},
	1, slave_qns_pcie_mem_noc_hw_nodes
};

/* Routes kept on for NoC error-logger register access */

/* APPS -> LLCC (GEM_NOC / LLCC rails) */
static struct icb_pair route_chm_apps_qns_llcc_hops[] = {
	{ &master_chm_apps, &slave_qns_llcc },
};
static struct icb_route route_chm_apps_qns_llcc = {
	ICBID_MASTER_APPSS_PROC, ICBID_SLAVE_LLCC,
	1, route_chm_apps_qns_llcc_hops
};

/* APPS -> EBI (full DDR path) */
static struct icb_pair route_chm_apps_ebi_hops[] = {
	{ &master_chm_apps, &slave_qns_llcc },
	{ &master_llcc_mc, &slave_ebi },
};
static struct icb_route route_chm_apps_ebi = {
	ICBID_MASTER_APPSS_PROC, ICBID_SLAVE_EBI1,
	2, route_chm_apps_ebi_hops
};

/* APPS -> CNOC (config NoC for register access) */
static struct icb_pair route_chm_apps_cnoc_hops[] = {
	{ &master_chm_apps, &slave_qns_gem_noc_cnoc },
	{ &master_qnm_gemnoc_cnoc, &slave_qhs_crypto0_cfg },
};
static struct icb_route route_chm_apps_cnoc = {
	ICBID_MASTER_APPSS_PROC, ICBID_SLAVE_CRYPTO_0_CFG,
	2, route_chm_apps_cnoc_hops
};

/* A1NOC -> SNOC -> GEM_NOC_SF (AGGRE1 / SNOC rails) */
static struct icb_pair route_a1noc_gemnoc_sf_hops[] = {
	{ &master_qnm_aggre1_noc, &slave_qns_gemnoc_sf },
	{ &master_qnm_snoc_sf, &slave_qns_llcc },
};
static struct icb_route route_a1noc_gemnoc_sf = {
	ICBID_MASTER_A1NOC_SNOC, ICBID_SLAVE_LLCC,
	2, route_a1noc_gemnoc_sf_hops
};

/* A2NOC -> SNOC -> GEM_NOC_SF (AGGRE2 / SNOC rails) */
static struct icb_pair route_a2noc_gemnoc_sf_hops[] = {
	{ &master_qnm_aggre2_noc, &slave_qns_gemnoc_sf },
	{ &master_qnm_snoc_sf, &slave_qns_llcc },
};
static struct icb_route route_a2noc_gemnoc_sf = {
	ICBID_MASTER_A2NOC_SNOC, ICBID_SLAVE_LLCC,
	2, route_a2noc_gemnoc_sf_hops
};

/* MNOC HF -> mem-NoC HF (votes bcm_mm0 ON for MMSS NoC access) */
static struct icb_pair route_mnoc_hf_hops[] = {
	{ &master_qnm_mnoc_hf, &slave_qns_mem_noc_hf },
};
static struct icb_route route_mnoc_hf = {
	ICBID_MASTER_MNOC_HF_MEM_NOC, ICBID_SLAVE_MNOC_HF_MEM_NOC,
	1, route_mnoc_hf_hops
};

/* Aggregated topology */

static struct icb_route *icb_route_list[] = {
	&route_chm_apps_qns_llcc,
	&route_chm_apps_ebi,
	&route_chm_apps_cnoc,
	&route_a1noc_gemnoc_sf,
	&route_a2noc_gemnoc_sf,
	&route_mnoc_hf,
};

static struct icb_master *icb_master_list[] = {
	&master_chm_apps,
	&master_llcc_mc,
	&master_qnm_aggre1_noc,
	&master_qnm_aggre2_noc,
	&master_qnm_gemnoc_cnoc,
	&master_qnm_snoc_sf,
	&master_qnm_mnoc_hf,
	&master_qnm_pcie,
	&master_qnm_gpdsp_sail,
	&master_qxm_crypto_0,
	&master_qnm_mdp0_0,
};

static struct icb_slave *icb_slave_list[] = {
	&slave_ebi,
	&slave_qns_llcc,
	&slave_qns_gem_noc_cnoc,
	&slave_qns_gemnoc_sf,
	&slave_qns_a1noc_snoc,
	&slave_qns_a2noc_snoc,
	&slave_qns_mem_noc_hf,
	&slave_qhs_crypto0_cfg,
	&slave_qns_pcie_mem_noc,
};

static struct icb_hw_node *icb_hw_node_list[] = {
	&bcm_acv,
	&bcm_ce0,
	&bcm_cn0,
	&bcm_cn1,
	&bcm_cn2,
	&bcm_mc0,
	&bcm_mm0,
	&bcm_sh0,
	&bcm_sh2,
	&bcm_sn0,
	&bcm_sn3,
	&bcm_sn4,
};

static struct icb_info lemans_icb_info = {
	.num_routes   = ARRAY_SIZE(icb_route_list),
	.routes       = icb_route_list,
	.num_masters  = ARRAY_SIZE(icb_master_list),
	.masters      = icb_master_list,
	.num_slaves   = ARRAY_SIZE(icb_slave_list),
	.slaves       = icb_slave_list,
	.num_hw_nodes = ARRAY_SIZE(icb_hw_node_list),
	.hw_nodes     = icb_hw_node_list,
};

struct icb_info *icbuarb_target_get_info(void)
{
	return &lemans_icb_info;
}

bool icbuarb_target_init(struct icb_info *info)
{
	(void)info;
	return true;
}
