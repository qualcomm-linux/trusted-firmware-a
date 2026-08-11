/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * QTI ICB (Interconnect Bus) driver - Nord (SA8797P / NordAU) target data.
 */

#include "icbuarbi.h"

/*============================================================================
                          DEFINES
============================================================================*/
/*============================================================================
                   INTERNAL DATA DECLARATIONS
============================================================================*/
/*============================================================================
                        HW NODE DECLARATIONS
============================================================================*/
/* ACV */
static struct icb_hw_request bcm_acv_requests[1];
static struct icb_hw_node bcm_acv =
{
	NULL, /**< node linkage */
	"ACV", /**< name */
	ICB_HW_NODE_KIND_VECTOR, /**< type */
	0, /**< hw_id */
	0, /**< width */
	0, /**< bw_unit */
	0, /**< clk_id */
	false, /**< is_dirty */
	0, /**< vote */
	{0, 0}, /**< state */
	{0, 1, bcm_acv_requests}, /**< request_list */
	0x0 /* output state */
};

/* C0N0 */
static struct icb_hw_request bcm_c0n0_requests[2];
static struct icb_hw_node bcm_c0n0 =
{
	NULL, /**< node linkage */
	"C0N0", /**< name */
	ICB_HW_NODE_KIND_VECTOR, /**< type */
	0, /**< hw_id */
	0, /**< width */
	0, /**< bw_unit */
	0, /**< clk_id */
	false, /**< is_dirty */
	0, /**< vote */
	{0, 0}, /**< state */
	{0, 2, bcm_c0n0_requests}, /**< request_list */
	0x1 /* output state */
};

/* C1N0 */
static struct icb_hw_request bcm_c1n0_requests[2];
static struct icb_hw_node bcm_c1n0 =
{
	NULL, /**< node linkage */
	"C1N0", /**< name */
	ICB_HW_NODE_KIND_VECTOR, /**< type */
	0, /**< hw_id */
	0, /**< width */
	0, /**< bw_unit */
	0, /**< clk_id */
	false, /**< is_dirty */
	0, /**< vote */
	{0, 0}, /**< state */
	{0, 2, bcm_c1n0_requests}, /**< request_list */
	0x1 /* output state */
};

/* C2N0 */
static struct icb_hw_request bcm_c2n0_requests[2];
static struct icb_hw_node bcm_c2n0 =
{
	NULL, /**< node linkage */
	"C2N0", /**< name */
	ICB_HW_NODE_KIND_VECTOR, /**< type */
	0, /**< hw_id */
	0, /**< width */
	0, /**< bw_unit */
	0, /**< clk_id */
	false, /**< is_dirty */
	0, /**< vote */
	{0, 0}, /**< state */
	{0, 2, bcm_c2n0_requests}, /**< request_list */
	0x1 /* output state */
};

/* C3N0 */
static struct icb_hw_request bcm_c3n0_requests[2];
static struct icb_hw_node bcm_c3n0 =
{
	NULL, /**< node linkage */
	"C3N0", /**< name */
	ICB_HW_NODE_KIND_VECTOR, /**< type */
	0, /**< hw_id */
	0, /**< width */
	0, /**< bw_unit */
	0, /**< clk_id */
	false, /**< is_dirty */
	0, /**< vote */
	{0, 0}, /**< state */
	{0, 2, bcm_c3n0_requests}, /**< request_list */
	0x1 /* output state */
};

/* MC0 */
static struct icb_hw_request bcm_mc0_requests[1];
static struct icb_hw_node bcm_mc0 =
{
	NULL, /**< node linkage */
	"MC0", /**< name */
	ICB_HW_NODE_KIND_BANDWIDTH, /**< type */
	0, /**< hw_id */
	0, /**< width */
	0, /**< bw_unit */
	0, /**< clk_id */
	false, /**< is_dirty */
	0, /**< vote */
	{0, 0}, /**< state */
	{0, 1, bcm_mc0_requests}, /**< request_list */
	0 /* vector output state */
};

/* MM0 */
static struct icb_hw_request bcm_mm0_requests[1];
static struct icb_hw_node bcm_mm0 =
{
	NULL, /**< node linkage */
	"MM0", /**< name */
	ICB_HW_NODE_KIND_BANDWIDTH, /**< type */
	0, /**< hw_id */
	0, /**< width */
	0, /**< bw_unit */
	0, /**< clk_id */
	false, /**< is_dirty */
	0, /**< vote */
	{0, 0}, /**< state */
	{0, 1, bcm_mm0_requests}, /**< request_list */
	0 /* vector output state */
};

/* MM1 */
static struct icb_hw_request bcm_mm1_requests[1];
static struct icb_hw_node bcm_mm1 =
{
	NULL, /**< node linkage */
	"MM1", /**< name */
	ICB_HW_NODE_KIND_VECTOR, /**< type */
	0, /**< hw_id */
	0, /**< width */
	0, /**< bw_unit */
	0, /**< clk_id */
	false, /**< is_dirty */
	0, /**< vote */
	{0, 0}, /**< state */
	{0, 1, bcm_mm1_requests}, /**< request_list */
	0x1 /* output state */
};

/* SH0 */
static struct icb_hw_request bcm_sh0_requests[1];
static struct icb_hw_node bcm_sh0 =
{
	NULL, /**< node linkage */
	"SH0", /**< name */
	ICB_HW_NODE_KIND_BANDWIDTH, /**< type */
	0, /**< hw_id */
	0, /**< width */
	0, /**< bw_unit */
	0, /**< clk_id */
	false, /**< is_dirty */
	0, /**< vote */
	{0, 0}, /**< state */
	{0, 1, bcm_sh0_requests}, /**< request_list */
	0 /* vector output state */
};

/* SH1 */
static struct icb_hw_request bcm_sh1_requests[5];
static struct icb_hw_node bcm_sh1 =
{
	NULL, /**< node linkage */
	"SH1", /**< name */
	ICB_HW_NODE_KIND_VECTOR, /**< type */
	0, /**< hw_id */
	0, /**< width */
	0, /**< bw_unit */
	0, /**< clk_id */
	false, /**< is_dirty */
	0, /**< vote */
	{0, 0}, /**< state */
	{0, 5, bcm_sh1_requests}, /**< request_list */
	0x1 /* output state */
};

/*============================================================================
                        MASTER DECLARATIONS
============================================================================*/
/* LLCC_MC */
static struct icb_master master_llcc_mc =
{
	ICBID_MASTER_LLCC,
	4, /**< width */
	16, /**< num ports */
	{0, 0}, /**< state */
	{0, 0, NULL}, /**< request_list */
	0, /**< number of hw_nodes */
	NULL, /**< hw nodes */
};

/* QNM_MDP0 */
static struct icb_hw_node *master_qnm_mdp0_hw_nodes[] =
{
	&bcm_mm1,
};

static struct icb_master master_qnm_mdp0 =
{
	ICBID_MASTER_MDP0,
	32, /**< width */
	2, /**< num ports */
	{0, 0}, /**< state */
	{0, 0, NULL}, /**< request_list */
	1, /**< number of hw_nodes */
	master_qnm_mdp0_hw_nodes, /**< hw nodes */
};

/* QNM_MNOC_HF */
static struct icb_hw_node *master_qnm_mnoc_hf_hw_nodes[] =
{
	&bcm_sh1,
};

static struct icb_master master_qnm_mnoc_hf =
{
	ICBID_MASTER_MNOC_HF_MEM_NOC,
	32, /**< width */
	2, /**< num ports */
	{0, 0}, /**< state */
	{0, 0, NULL}, /**< request_list */
	1, /**< number of hw_nodes */
	master_qnm_mnoc_hf_hw_nodes, /**< hw nodes */
};

/* QNM_NSP0_HSCNOC */
static struct icb_hw_node *master_qnm_nsp0_hscnoc_hw_nodes[] =
{
	&bcm_sh1,
};

static struct icb_master master_qnm_nsp0_hscnoc =
{
	ICBID_MASTER_NSP0_HSCNOC,
	32, /**< width */
	2, /**< num ports */
	{0, 0}, /**< state */
	{0, 0, NULL}, /**< request_list */
	1, /**< number of hw_nodes */
	master_qnm_nsp0_hscnoc_hw_nodes, /**< hw nodes */
};

/* QNM_NSP1_HSCNOC */
static struct icb_hw_node *master_qnm_nsp1_hscnoc_hw_nodes[] =
{
	&bcm_sh1,
};

static struct icb_master master_qnm_nsp1_hscnoc =
{
	ICBID_MASTER_NSP1_HSCNOC,
	32, /**< width */
	2, /**< num ports */
	{0, 0}, /**< state */
	{0, 0, NULL}, /**< request_list */
	1, /**< number of hw_nodes */
	master_qnm_nsp1_hscnoc_hw_nodes, /**< hw nodes */
};

/* QNM_NSP2_HSCNOC */
static struct icb_hw_node *master_qnm_nsp2_hscnoc_hw_nodes[] =
{
	&bcm_sh1,
};

static struct icb_master master_qnm_nsp2_hscnoc =
{
	ICBID_MASTER_NSP2_HSCNOC,
	32, /**< width */
	2, /**< num ports */
	{0, 0}, /**< state */
	{0, 0, NULL}, /**< request_list */
	1, /**< number of hw_nodes */
	master_qnm_nsp2_hscnoc_hw_nodes, /**< hw nodes */
};

/* QNM_NSP3_HSCNOC */
static struct icb_hw_node *master_qnm_nsp3_hscnoc_hw_nodes[] =
{
	&bcm_sh1,
};

static struct icb_master master_qnm_nsp3_hscnoc =
{
	ICBID_MASTER_NSP3_HSCNOC,
	32, /**< width */
	2, /**< num ports */
	{0, 0}, /**< state */
	{0, 0, NULL}, /**< request_list */
	1, /**< number of hw_nodes */
	master_qnm_nsp3_hscnoc_hw_nodes, /**< hw nodes */
};

/* QNM_NSP_DATA00 */
static struct icb_hw_node *master_qnm_nsp_data00_hw_nodes[] =
{
	&bcm_c0n0,
};

static struct icb_master master_qnm_nsp_data00 =
{
	ICBID_MASTER_NSP0_PROC,
	32, /**< width */
	2, /**< num ports */
	{0, 0}, /**< state */
	{0, 0, NULL}, /**< request_list */
	1, /**< number of hw_nodes */
	master_qnm_nsp_data00_hw_nodes, /**< hw nodes */
};

/* QNM_NSP_DATA01 */
static struct icb_hw_node *master_qnm_nsp_data01_hw_nodes[] =
{
	&bcm_c1n0,
};

static struct icb_master master_qnm_nsp_data01 =
{
	ICBID_MASTER_NSP1_PROC,
	32, /**< width */
	2, /**< num ports */
	{0, 0}, /**< state */
	{0, 0, NULL}, /**< request_list */
	1, /**< number of hw_nodes */
	master_qnm_nsp_data01_hw_nodes, /**< hw nodes */
};

/* QNM_NSP_DATA02 */
static struct icb_hw_node *master_qnm_nsp_data02_hw_nodes[] =
{
	&bcm_c2n0,
};

static struct icb_master master_qnm_nsp_data02 =
{
	ICBID_MASTER_NSP2_PROC,
	32, /**< width */
	2, /**< num ports */
	{0, 0}, /**< state */
	{0, 0, NULL}, /**< request_list */
	1, /**< number of hw_nodes */
	master_qnm_nsp_data02_hw_nodes, /**< hw nodes */
};

/* QNM_NSP_DATA03 */
static struct icb_hw_node *master_qnm_nsp_data03_hw_nodes[] =
{
	&bcm_c3n0,
};

static struct icb_master master_qnm_nsp_data03 =
{
	ICBID_MASTER_NSP3_PROC,
	32, /**< width */
	2, /**< num ports */
	{0, 0}, /**< state */
	{0, 0, NULL}, /**< request_list */
	1, /**< number of hw_nodes */
	master_qnm_nsp_data03_hw_nodes, /**< hw nodes */
};

/*============================================================================
                        SLAVE DECLARATIONS
============================================================================*/
/* EBI */
static struct icb_hw_node *slave_ebi_hw_nodes[] =
{
	&bcm_mc0,
	&bcm_acv,
};

static struct icb_slave slave_ebi =
{
	ICBID_SLAVE_EBI1,
	4, /**< width */
	16, /**< num ports */
	{0, 0}, /**< state */
	{0, 0, NULL}, /**< request_list */
	2, /**< number of hw_nodes */
	slave_ebi_hw_nodes, /**< hw nodes */
};

/* QNS_LLCC */
static struct icb_hw_node *slave_qns_llcc_hw_nodes[] =
{
	&bcm_sh0,
};

static struct icb_slave slave_qns_llcc =
{
	ICBID_SLAVE_LLCC,
	16, /**< width */
	16, /**< num ports */
	{0, 0}, /**< state */
	{0, 0, NULL}, /**< request_list */
	1, /**< number of hw_nodes */
	slave_qns_llcc_hw_nodes, /**< hw nodes */
};

/* QNS_MEM_NOC_HF */
static struct icb_hw_node *slave_qns_mem_noc_hf_hw_nodes[] =
{
	&bcm_mm0,
};

static struct icb_slave slave_qns_mem_noc_hf =
{
	ICBID_SLAVE_MNOC_HF_MEM_NOC,
	32, /**< width */
	2, /**< num ports */
	{0, 0}, /**< state */
	{0, 0, NULL}, /**< request_list */
	1, /**< number of hw_nodes */
	slave_qns_mem_noc_hf_hw_nodes, /**< hw nodes */
};

/* QNS_NSP0_HSC_NOC */
static struct icb_hw_node *slave_qns_nsp0_hsc_noc_hw_nodes[] =
{
	&bcm_c0n0,
};

static struct icb_slave slave_qns_nsp0_hsc_noc =
{
	ICBID_SLAVE_NSP0_HSC_NOC,
	32, /**< width */
	2, /**< num ports */
	{0, 0}, /**< state */
	{0, 0, NULL}, /**< request_list */
	1, /**< number of hw_nodes */
	slave_qns_nsp0_hsc_noc_hw_nodes, /**< hw nodes */
};

/* QNS_NSP1_HSC_NOC */
static struct icb_hw_node *slave_qns_nsp1_hsc_noc_hw_nodes[] =
{
	&bcm_c1n0,
};

static struct icb_slave slave_qns_nsp1_hsc_noc =
{
	ICBID_SLAVE_NSP1_HSC_NOC,
	32, /**< width */
	2, /**< num ports */
	{0, 0}, /**< state */
	{0, 0, NULL}, /**< request_list */
	1, /**< number of hw_nodes */
	slave_qns_nsp1_hsc_noc_hw_nodes, /**< hw nodes */
};

/* QNS_NSP2_HSC_NOC */
static struct icb_hw_node *slave_qns_nsp2_hsc_noc_hw_nodes[] =
{
	&bcm_c2n0,
};

static struct icb_slave slave_qns_nsp2_hsc_noc =
{
	ICBID_SLAVE_NSP2_HSC_NOC,
	32, /**< width */
	2, /**< num ports */
	{0, 0}, /**< state */
	{0, 0, NULL}, /**< request_list */
	1, /**< number of hw_nodes */
	slave_qns_nsp2_hsc_noc_hw_nodes, /**< hw nodes */
};

/* QNS_NSP3_HSC_NOC */
static struct icb_hw_node *slave_qns_nsp3_hsc_noc_hw_nodes[] =
{
	&bcm_c3n0,
};

static struct icb_slave slave_qns_nsp3_hsc_noc =
{
	ICBID_SLAVE_NSP3_HSC_NOC,
	32, /**< width */
	2, /**< num ports */
	{0, 0}, /**< state */
	{0, 0, NULL}, /**< request_list */
	1, /**< number of hw_nodes */
	slave_qns_nsp3_hsc_noc_hw_nodes, /**< hw nodes */
};

/*============================================================================
                        TOPOLOGY ROUTE DECLARATIONS
============================================================================*/
static struct icb_pair route_qnm_mdp0_ebi_hops[3] =
{
	{
		&master_qnm_mdp0,
		&slave_qns_mem_noc_hf
	},
	{
		&master_qnm_mnoc_hf,
		&slave_qns_llcc
	},
	{
		&master_llcc_mc,
		&slave_ebi
	},
};

static struct icb_route route_qnm_mdp0_ebi =
{
	ICBID_MASTER_MDP0,
	ICBID_SLAVE_EBI1,
	3,
	route_qnm_mdp0_ebi_hops
};


static struct icb_pair route_qnm_nsp_data00_ebi_hops[3] =
{
	{
		&master_qnm_nsp_data00,
		&slave_qns_nsp0_hsc_noc
	},
	{
		&master_qnm_nsp0_hscnoc,
		&slave_qns_llcc
	},
	{
		&master_llcc_mc,
		&slave_ebi
	},
};

static struct icb_route route_qnm_nsp_data00_ebi =
{
	ICBID_MASTER_NSP0_PROC,
	ICBID_SLAVE_EBI1,
	3,
	route_qnm_nsp_data00_ebi_hops
};


static struct icb_pair route_qnm_nsp_data01_ebi_hops[3] =
{
	{
		&master_qnm_nsp_data01,
		&slave_qns_nsp1_hsc_noc
	},
	{
		&master_qnm_nsp1_hscnoc,
		&slave_qns_llcc
	},
	{
		&master_llcc_mc,
		&slave_ebi
	},
};

static struct icb_route route_qnm_nsp_data01_ebi =
{
	ICBID_MASTER_NSP1_PROC,
	ICBID_SLAVE_EBI1,
	3,
	route_qnm_nsp_data01_ebi_hops
};


static struct icb_pair route_qnm_nsp_data02_ebi_hops[3] =
{
	{
		&master_qnm_nsp_data02,
		&slave_qns_nsp2_hsc_noc
	},
	{
		&master_qnm_nsp2_hscnoc,
		&slave_qns_llcc
	},
	{
		&master_llcc_mc,
		&slave_ebi
	},
};

static struct icb_route route_qnm_nsp_data02_ebi =
{
	ICBID_MASTER_NSP2_PROC,
	ICBID_SLAVE_EBI1,
	3,
	route_qnm_nsp_data02_ebi_hops
};


static struct icb_pair route_qnm_nsp_data03_ebi_hops[3] =
{
	{
		&master_qnm_nsp_data03,
		&slave_qns_nsp3_hsc_noc
	},
	{
		&master_qnm_nsp3_hscnoc,
		&slave_qns_llcc
	},
	{
		&master_llcc_mc,
		&slave_ebi
	},
};

static struct icb_route route_qnm_nsp_data03_ebi =
{
	ICBID_MASTER_NSP3_PROC,
	ICBID_SLAVE_EBI1,
	3,
	route_qnm_nsp_data03_ebi_hops
};



/*============================================================================
                   NODE AND ROUTE LISTS
============================================================================*/
static struct icb_route *icb_route_list[5] =
{
	&route_qnm_mdp0_ebi,
	&route_qnm_nsp_data00_ebi,
	&route_qnm_nsp_data01_ebi,
	&route_qnm_nsp_data02_ebi,
	&route_qnm_nsp_data03_ebi,
};

static struct icb_master *icb_master_list[11] =
{
	&master_llcc_mc,
	&master_qnm_mdp0,
	&master_qnm_mnoc_hf,
	&master_qnm_nsp0_hscnoc,
	&master_qnm_nsp1_hscnoc,
	&master_qnm_nsp2_hscnoc,
	&master_qnm_nsp3_hscnoc,
	&master_qnm_nsp_data00,
	&master_qnm_nsp_data01,
	&master_qnm_nsp_data02,
	&master_qnm_nsp_data03,
};

static struct icb_slave *icb_slave_list[7] =
{
	&slave_ebi,
	&slave_qns_llcc,
	&slave_qns_mem_noc_hf,
	&slave_qns_nsp0_hsc_noc,
	&slave_qns_nsp1_hsc_noc,
	&slave_qns_nsp2_hsc_noc,
	&slave_qns_nsp3_hsc_noc,
};

static struct icb_hw_node *icb_hw_node_list[10] =
{
	&bcm_acv,
	&bcm_c0n0,
	&bcm_c1n0,
	&bcm_c2n0,
	&bcm_c3n0,
	&bcm_mc0,
	&bcm_mm0,
	&bcm_mm1,
	&bcm_sh0,
	&bcm_sh1,
};

/*============================================================================
                   EXTERNAL DATA DECLARATIONS
============================================================================*/
const struct icb_info info =
{
	5,
	icb_route_list,
	11,
	icb_master_list,
	7,
	icb_slave_list,
	10,
	icb_hw_node_list
};
