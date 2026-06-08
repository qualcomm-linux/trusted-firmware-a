/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Internal driver context and helpers for the QTI clock driver.
 *
 * The low-level register accessors that used to live behind a separate HAL
 * abstraction are folded directly into the clock driver. Their prototypes are
 * declared here so the chipset back-end and the framework share one view.
 */

#ifndef CLOCK_DRIVER_H
#define CLOCK_DRIVER_H

#include <stdbool.h>
#include <stdint.h>

#include <drivers/qti/clock/clock.h>
#include <drivers/qti/clock/clock_bsp.h>
#include <drivers/qti/rpmh/rpmh_client.h>

/* Main clock driver context. */
struct clock_drv_ctxt {
	struct clock_tz_bsp	*bsp;
	bool			initialized;
	rpmh_client_handle	rpmh;
	int			cx_lvl_idx;
	int			mx_lvl_idx;
	rpmh_command_set_t	cx_cmd_set;
};

/*
 * Image- and CPU-specific init hooks, implemented by the chipset back-end.
 */
int clock_init_image(struct clock_drv_ctxt *drv_ctxt);
int clock_post_init_image(struct clock_drv_ctxt *drv_ctxt);
int clock_init_cpu(struct clock_drv_ctxt *drv_ctxt);

/* Enable a clock source (and its parents). */
int clock_enable_source(struct clock_source *source);

/*
 * Folded register accessors (previously the HAL layer).
 *
 * These operate directly on the descriptor structures defined in
 * clock_types.h. The address of the shared PLL vote register is chipset
 * specific and is provided by the back-end through clock_hal_get_pll_vote_addr().
 */
uintptr_t clock_hal_get_pll_vote_addr(void);

void clock_hal_enable_clock(struct clock_clk_desc *clock);
void clock_hal_disable_clock(struct clock_clk_desc *clock);
int clock_hal_is_clock_on(struct clock_clk_desc *clock);
int clock_hal_wait_for_clock_on(struct clock_clk_desc *clock);

void clock_hal_enable_source(struct clock_source_desc *source);
void clock_hal_disable_source(struct clock_source_desc *source);

void clock_hal_enable_power_domain(struct clock_power_domain_desc *power_domain);
void clock_hal_disable_power_domain(struct clock_power_domain_desc *power_domain);
int clock_hal_is_power_domain_on(struct clock_power_domain_desc *power_domain);
int clock_hal_wait_for_power_domain_on(struct clock_power_domain_desc *power_domain);

#endif /* CLOCK_DRIVER_H */
