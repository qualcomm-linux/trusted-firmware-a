/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <common/debug.h>
#include <lib/spinlock.h>

#include <drivers/qti/cmd_db/cmd_db.h>
#include <drivers/qti/icb/icbuarb.h>
#include <drivers/qti/rpmh/rpmh_client.h>

#include "icbuarbi.h"

/* BCM vote-table command encoding. */
#define BCM_VOTETABLE_COMMIT_BMSK	0x40000000U
#define BCM_VOTETABLE_VOTE_VALID_BMSK	0x20000000U
#define BCM_VOTETABLE_VOTE_X_SHFT	14U
#define BCM_VOTETABLE_VOTE_X_MAX	0x3FFFU
#define BCM_VOTETABLE_VOTE_Y_SHFT	0U
#define BCM_VOTETABLE_VOTE_Y_MAX	0x3FFFU

/* BCM auxiliary data layout in the Command DB. */
struct cmd_db_bcm_aux {
	uint32_t	bw_unit;
	uint16_t	bcm_port;
	uint8_t		clk_id;
};

/* Bound on concurrent clients; platforms can override. */
#ifndef ICBUARB_MAX_CLIENTS
#define ICBUARB_MAX_CLIENTS		8U
#endif

/* Divide and round up to the nearest integer. */
#define CEIL_DIV(dividend, divisor) \
	(((dividend) / (divisor)) + (((dividend) % (divisor)) ? 1U : 0U))
#define BW_SCALE(bw, dividend, divisor) \
	CEIL_DIV((bw) * (dividend), (divisor))

/* Ignore zero message ids when tracking completion. */
#define RPM_MSG_ID_UPDATE(msg_id, new_msg_id) \
	((msg_id) = (new_msg_id) ? (new_msg_id) : (msg_id))

static struct icb_info *info;

/* No heap in BL31: clients are handed out from a fixed pool. */
static struct icb_client client_pool[ICBUARB_MAX_CLIENTS];

static rpmh_client_handle rpmh_handle;
static struct icb_hw_node *commit_queue;
static spinlock_t icb_lock;

static struct icb_client *alloc_client(void)
{
	uint32_t i;

	for (i = 0U; i < ICBUARB_MAX_CLIENTS; i++) {
		if (!client_pool[i].in_use) {
			client_pool[i].in_use = true;
			return &client_pool[i];
		}
	}

	return NULL;
}

static void free_client(struct icb_client *handle)
{
	(void)memset(handle, 0, sizeof(*handle));
}

static bool add_hw_node_request(struct icb_hw_request_list *req_list,
				struct icb_bw_req *req, uint32_t width,
				uint32_t ports)
{
	if (req_list->num_entries < req_list->list_size) {
		req_list->requests[req_list->num_entries].req = req;
		req_list->requests[req_list->num_entries].width = width;
		req_list->requests[req_list->num_entries].ports = ports;
		req_list->num_entries++;
		return true;
	}

	return false;
}

static bool add_sw_node_request(struct icb_bw_request_list *req_list,
				struct icb_bw_req *req)
{
	if (req_list->num_entries >= req_list->list_size) {
		ERROR("icbuarb: SW request list full (size %u)\n",
		      req_list->list_size);
		return false;
	}

	req_list->requests[req_list->num_entries] = req;
	req_list->num_entries++;

	return true;
}

static void remove_sw_node_request(struct icb_bw_request_list *req_list,
				   struct icb_bw_req *req)
{
	uint32_t i;

	for (i = 0U; i < req_list->num_entries; i++) {
		if (req_list->requests[i] == req) {
			break;
		}
	}

	if (i >= req_list->num_entries) {
		return;
	}

	if (i < (req_list->num_entries - 1U)) {
		(void)memmove(&req_list->requests[i], &req_list->requests[i + 1U],
			      (req_list->num_entries - i - 1U) *
			      sizeof(struct icb_bw_req *));
	}

	req_list->num_entries--;
}

static void remove_client_requests(struct icb_client *handle)
{
	struct icb_route *route = handle->route;
	uint32_t i;

	for (i = 0U; i < route->num_hops; i++) {
		struct icb_pair *pair = &route->hops[i];

		if (pair->master != NULL) {
			remove_sw_node_request(&pair->master->request_list,
					       &handle->curr_req);
		}
		if (pair->slave != NULL) {
			remove_sw_node_request(&pair->slave->request_list,
					       &handle->curr_req);
		}
	}
}

static void aggregate_sw_node(struct icb_bw_request_list *req_list,
			      struct icb_bw_req *state)
{
	uint64_t ib = 0U, ab = 0U;
	uint32_t i;

	/* Software nodes: max IB, sum AB. */
	for (i = 0U; i < req_list->num_entries; i++) {
		ib = MAX(ib, req_list->requests[i]->ib);
		ab += req_list->requests[i]->ab;
	}

	state->ib = ib;
	state->ab = ab;
}

static void aggregate_hw_node(struct icb_hw_node *hw_node)
{
	uint64_t ib = 0U, ab = 0U;
	uint32_t i;

	/* Hardware nodes: scaled max of IB and AB. */
	for (i = 0U; i < hw_node->request_list.num_entries; i++) {
		struct icb_hw_request *request =
			&hw_node->request_list.requests[i];

		if (hw_node->type == ICB_HW_NODE_KIND_BANDWIDTH) {
			ib = MAX(ib, BW_SCALE(request->req->ib,
					      hw_node->width, request->width));
			ab = MAX(ab, BW_SCALE(request->req->ab, hw_node->width,
					      request->width * request->ports));
		} else {
			ib = MAX(ib, request->req->ib);
			ab = MAX(ab, request->req->ab);
		}
	}

	hw_node->state.ib = ib;
	hw_node->state.ab = ab;

	if (hw_node->type == ICB_HW_NODE_KIND_BANDWIDTH) {
		uint64_t scaled_ab = CEIL_DIV(hw_node->state.ab,
					      hw_node->bw_unit);
		uint64_t scaled_ib = CEIL_DIV(hw_node->state.ib,
					      hw_node->bw_unit);
		uint64_t ceil_ab = (scaled_ab <= BCM_VOTETABLE_VOTE_X_MAX) ?
				   scaled_ab : BCM_VOTETABLE_VOTE_X_MAX;
		uint64_t ceil_ib = (scaled_ib <= BCM_VOTETABLE_VOTE_Y_MAX) ?
				   scaled_ib : BCM_VOTETABLE_VOTE_Y_MAX;

		hw_node->vote =
			(uint32_t)((ceil_ab << BCM_VOTETABLE_VOTE_X_SHFT) |
				   (ceil_ib << BCM_VOTETABLE_VOTE_Y_SHFT));
	} else {
		/* Vector node: vote ON if any non-zero request. */
		if ((hw_node->state.ab > 0U) || (hw_node->state.ib > 0U)) {
			hw_node->vote = hw_node->output;
		} else {
			hw_node->vote = 0U;
		}
	}

	if (hw_node->vote != 0U) {
		hw_node->vote |= BCM_VOTETABLE_VOTE_VALID_BMSK;
	}
}

static void queue_hw_node_request(struct icb_hw_node *hw_node)
{
	struct icb_hw_node *iter, *prev;

	/* Insert into the queue, sorted by clock id. */
	for (iter = commit_queue, prev = NULL;
	     iter != NULL;
	     prev = iter, iter = iter->next) {
		if (hw_node->clk_id < iter->clk_id) {
			break;
		}
	}

	if (iter == commit_queue) {
		hw_node->next = commit_queue;
		commit_queue = hw_node;
	} else {
		prev->next = hw_node;
		hw_node->next = iter;
	}
}

static void commit_hw_requests(void)
{
	rpmh_command_set_t command_set;
	uint32_t num_cmds = 0U, barrier_id = 0U;
	struct icb_hw_node *bcm;

	if (commit_queue == NULL) {
		return;
	}

	(void)memset(&command_set, 0, sizeof(command_set));

	while (commit_queue != NULL) {
		bcm = commit_queue;
		commit_queue = bcm->next;
		bcm->next = NULL;

		num_cmds++;
		command_set.commands[num_cmds - 1U].address = bcm->hw_id;
		command_set.commands[num_cmds - 1U].data = bcm->vote;

		/* Flush at queue end, VCD boundary, or full TCS. */
		if ((commit_queue == NULL) ||
		    (bcm->clk_id != commit_queue->clk_id) ||
		    (num_cmds == IMAGE_TCS_SIZE)) {
			uint32_t msg_id;

			command_set.commands[num_cmds - 1U].data |=
				BCM_VOTETABLE_COMMIT_BMSK;
			command_set.commands[num_cmds - 1U].completion = true;
			command_set.set = RPMH_SET_ACTIVE;
			command_set.num_commands = num_cmds;

			msg_id = rpmh_issue_command_set(rpmh_handle,
							&command_set);
			RPM_MSG_ID_UPDATE(barrier_id, msg_id);
			(void)memset(&command_set, 0, sizeof(command_set));
			num_cmds = 0U;
		}
	}

	rpmh_barrier_all(rpmh_handle, barrier_id);
}

static bool icbuarb_hw_init(void)
{
	uint32_t i, node;

	for (i = 0U; i < info->num_hw_nodes; i++) {
		struct icb_hw_node *hw_node = info->hw_nodes[i];
		struct cmd_db_bcm_aux bcm_aux;
		uint8_t len = sizeof(bcm_aux);

		if (cmd_db_query_aux_data(hw_node->name, &len,
					  (uint8_t *)&bcm_aux) != 0) {
			return false;
		}

		hw_node->hw_id = cmd_db_query_addr(hw_node->name);
		hw_node->width = bcm_aux.bcm_port;
		hw_node->bw_unit = bcm_aux.bw_unit;
		hw_node->clk_id = bcm_aux.clk_id;
		hw_node->is_dirty = false;
	}

	/* RPMh has no common init hook; first consumer initialises. */
	rpmh_client_init();
	rpmh_handle = rpmh_create_handle(RSC_DRV_TZ, "ICB");
	if (rpmh_handle == NULL) {
		return false;
	}

	/* Attach master/slave state to their BCM nodes. */
	for (i = 0U; i < info->num_masters; i++) {
		struct icb_master *master = info->masters[i];

		for (node = 0U; node < master->num_hw_nodes; node++) {
			struct icb_hw_node *bcm = master->hw_nodes[node];

			(void)add_hw_node_request(&bcm->request_list,
						  &master->state,
						  master->width, master->ports);
		}
	}
	for (i = 0U; i < info->num_slaves; i++) {
		struct icb_slave *slave = info->slaves[i];

		for (node = 0U; node < slave->num_hw_nodes; node++) {
			struct icb_hw_node *bcm = slave->hw_nodes[node];

			(void)add_hw_node_request(&bcm->request_list,
						  &slave->state,
						  slave->width, slave->ports);
		}
	}

	return true;
}

bool icbuarb_init(void)
{
	bool ret = false;

	(void)memset(client_pool, 0, sizeof(client_pool));

	info = icbuarb_target_get_info();
	if (info != NULL) {
		ret = icbuarb_hw_init() && icbuarb_target_init(info);
	}

	return ret;
}

icb_client_handle icbuarb_create_client(enum icbid_master master,
					enum icbid_slave slave)
{
	struct icb_client *handle;
	uint32_t i;

	if (info == NULL) {
		return NULL;
	}

	/* Find a matching route. */
	for (i = 0U; i < info->num_routes; i++) {
		if ((info->routes[i] != NULL) &&
		    (info->routes[i]->master == master) &&
		    (info->routes[i]->slave == slave)) {
			break;
		}
	}

	if (i >= info->num_routes) {
		return NULL;
	}

	spin_lock(&icb_lock);

	handle = alloc_client();
	if (handle == NULL) {
		spin_unlock(&icb_lock);
		ERROR("icbuarb: client pool exhausted\n");
		return NULL;
	}

	handle->master = master;
	handle->slave = slave;
	handle->route = info->routes[i];
	(void)memset(&handle->curr_req, 0, sizeof(struct icb_bw_req));

	/* Add this client's request to every node along the route. */
	for (i = 0U; i < handle->route->num_hops; i++) {
		if (handle->route->hops[i].master != NULL) {
			if (!add_sw_node_request(
				    &handle->route->hops[i].master->request_list,
				    &handle->curr_req)) {
				break;
			}
		}
		if (handle->route->hops[i].slave != NULL) {
			if (!add_sw_node_request(
				    &handle->route->hops[i].slave->request_list,
				    &handle->curr_req)) {
				break;
			}
		}
	}

	if (i < handle->route->num_hops) {
		/* Roll back partial insertion and release the handle. */
		remove_client_requests(handle);
		free_client(handle);
		handle = NULL;
	}

	spin_unlock(&icb_lock);

	return handle;
}

bool icbuarb_issue_request(icb_client_handle handle, struct icb_bw_req *req)
{
	struct icb_route *route;
	uint32_t i, node;

	if (req == NULL || info == NULL || handle == NULL) {
		return false;
	}

	spin_lock(&icb_lock);

	route = handle->route;
	if (route == NULL) {
		spin_unlock(&icb_lock);
		return false;
	}

	handle->curr_req = *req;

	for (i = 0U; i < route->num_hops; i++) {
		if (route->hops[i].master != NULL) {
			struct icb_master *m = route->hops[i].master;

			aggregate_sw_node(&m->request_list, &m->state);
			for (node = 0U; node < m->num_hw_nodes; node++) {
				m->hw_nodes[node]->is_dirty = true;
			}
		}
		if (route->hops[i].slave != NULL) {
			struct icb_slave *s = route->hops[i].slave;

			aggregate_sw_node(&s->request_list, &s->state);
			for (node = 0U; node < s->num_hw_nodes; node++) {
				s->hw_nodes[node]->is_dirty = true;
			}
		}
	}

	/* Recompute and queue votes for every node touched above. */
	for (node = 0U; node < info->num_hw_nodes; node++) {
		struct icb_hw_node *hw_node = info->hw_nodes[node];

		if (hw_node->is_dirty) {
			aggregate_hw_node(hw_node);
			queue_hw_node_request(hw_node);
			hw_node->is_dirty = false;
		}
	}

	commit_hw_requests();

	spin_unlock(&icb_lock);

	return true;
}

bool icbuarb_destroy_client(icb_client_handle handle)
{
	struct icb_bw_req req;
	bool ret = false;

	if (handle == NULL) {
		return false;
	}

	/* Cancel the existing vote before releasing the client. */
	(void)memset(&req, 0, sizeof(req));
	if (icbuarb_issue_request(handle, &req)) {
		spin_lock(&icb_lock);
		remove_client_requests(handle);
		free_client(handle);
		ret = true;
		spin_unlock(&icb_lock);
	}

	return ret;
}
