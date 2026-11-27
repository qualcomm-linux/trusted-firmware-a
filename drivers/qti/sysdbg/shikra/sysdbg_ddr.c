/*
 * Copyright (c) 2026, Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * System Debug - BL31 Entry (Shikra Target)
 *
 * Ported from TZ.XF.5.36-00138-SHIKRA-1:
 * trustzone_images/core/systemdebug/sysdbg/monitor/src/target/shikra/sysdbg_mon_shikra.c
 *
 * This file contains the main BL31 entry point for crash reset path.
 * Primary responsibility: Bring DDR out of self-refresh so debug collection
 * can proceed in DDR.
 *
 * Must execute from IMEM as DDR is in self-refresh at entry.
 */

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <cdefs.h>
#include <lib/mmio.h>
#include <sysdbg_hwio.h>
#include <sysdbg_target_config.h>
#include <drivers/qti/sysdbg/sysdbg_crumbs.h>
#include <drivers/qti/sysdbg/sysdbg_log.h>
#include <drivers/qti/sysdbg/sysdbg_main.h>

/*
 * IMEM Function Attribute
 *
 * In TZ: section ".code_imem.SYSDBG_IMEM_FUNC"
 * In TFA: We use linker script with .text_imem section
 */
#define SYSDBG_IMEM_FUNC	__section(".text_imem")

/*
 * IMEM Data Attribute
 *
 * In TZ: section ".uncached_imem.MON_IMEM_UNCACHED_ZI"
 * In TFA: We use .data_imem section
 */
#define SYSDBG_IMEM_DATA	__section(".data_imem")

/* Cookie values from sysdbg_log.h are now used via include */

/*
 * DDR Self-Refresh Function Pointers
 *
 * These are passed from U-Boot SPL via BL31 parameters.
 * U-Boot SPL contains DDR initialization code that knows how to bring
 * DDR out of self-refresh and put it back in.
 *
 * Phase 1: These will be NULL (stubbed) since DDR recovery code is not
 *          yet available in this TFA port.
 * Phase 2: Will be populated from BL31 boot parameters when DDR recovery
 *          code is integrated.
 */
static SYSDBG_IMEM_DATA bool (*ddr_self_refresh_exit)(void);
static SYSDBG_IMEM_DATA bool (*ddr_self_refresh_enter)(void);

/*
 * DDR Initialization Status
 *
 * Tracks whether DDR has been successfully brought out of self-refresh.
 * Prevents multiple cores from attempting DDR exit simultaneously.
 */
static SYSDBG_IMEM_DATA bool ddr_initialized;

/*
 * Shared Data Structure (Placeholder for Phase 1)
 *
 * In TZ: Shared between Monitor and QSEE for DDR self-refresh coordination
 * In TFA: Will be used to coordinate between IMEM and DDR phases
 *
 * For Phase 1: Minimal stub
 * Type definition in sysdbg_main.h
 */
static SYSDBG_IMEM_DATA sysdbg_shared_data shared_data = {0};

/*
 * Coredump Structure (Placeholder for Phase 1)
 *
 * Tracks nested exception state during crash path.
 */
typedef struct {
	bool nested_exp;
} sysdbg_coredump_type;

SYSDBG_IMEM_DATA sysdbg_coredump_type sysdbg_coredump = {0};

/*
 * Forward declarations - IMEM helper functions
 */
SYSDBG_IMEM_FUNC void sysdbg_write_term_cookie(uint32_t cookie);
SYSDBG_IMEM_FUNC void system_debug_bl31_exit(void);
SYSDBG_IMEM_FUNC void sysdbg_bl31_ddr_alt_reset(void);
SYSDBG_IMEM_FUNC void sysdbg_download_mode_set(void);

/*
 * External functions from other modules
 */
extern void mon_save_pimem_reg_dump(void);	/* TODO: Port or stub */
extern void pimem_sdi_init(void);		/* TODO: Port or stub */

/*
 * DDR Function Pointer Getters (Placeholder for Phase 1)
 *
 * In TZ: mon_get_ddr_exit_self_refresh_addr() and mon_get_ddr_enter_self_refresh_addr()
 * In TFA: bl31_get_ddr_exit_self_refresh_addr() and bl31_get_ddr_enter_self_refresh_addr()
 *
 * These return addresses of DDR recovery functions passed from U-Boot SPL.
 * For Phase 1: Return 0 (no DDR recovery available yet)
 */
SYSDBG_IMEM_FUNC uint64_t bl31_get_ddr_exit_self_refresh_addr(void)
{
	/* TODO Phase 2: Return address from BL31 boot parameters */
	return 0;
}

SYSDBG_IMEM_FUNC uint64_t bl31_get_ddr_enter_self_refresh_addr(void)
{
	/* TODO Phase 2: Return address from BL31 boot parameters */
	return 0;
}

/*
 * system_debug_bl31_exit - Signal first pass complete and trigger reset
 *
 * Writes to GCC_RESET_FSM_CTRL register to indicate PASS1 is done.
 * Hardware will then trigger reset to enter PASS2 (full debug collection).
 *
 * This function never returns - waits for hardware reset.
 *
 * In TZ: system_debug_mon_exit()
 * In TFA: system_debug_bl31_exit()
 */
SYSDBG_IMEM_FUNC void system_debug_bl31_exit(void)
{
	/*
	 * Mark first pass as complete and trigger reset
	 *
	 * TODO: Verify register address against Shikra IPCAT
	 */
	mmio_write_32(HWIO_GCC_RESET_FSM_CTRL_ADDR,
		      mmio_read_32(HWIO_GCC_RESET_FSM_CTRL_ADDR) |
		      HWIO_GCC_RESET_FSM_CTRL_FIRST_PASS_COMPLETE_BMSK);

	/* Wait for hardware reset */
	while (1)
		;
}

/*
 * sysdbg_bl31_ddr_alt_reset - Perform alternate DDR reset on error
 *
 * If DDR fails to exit self-refresh, assert alternate reset to DDR controller.
 * This is a fallback recovery mechanism.
 *
 * In TZ: sysdbg_mon_ddr_alt_reset()
 * In TFA: sysdbg_bl31_ddr_alt_reset()
 */
SYSDBG_IMEM_FUNC void sysdbg_bl31_ddr_alt_reset(void)
{
	/*
	 * TODO: Shikra-specific alternate reset mechanism
	 * TZ code has this commented out with "TODO_NICOBAR"
	 *
	 * Original TZ code:
	 *   HWIO_OUTF(AOSS_CC_ALT_RESET_ENABLE, ALLOW_BIMC_ALT_ARES, 0x1);
	 *
	 * For Phase 1: No-op
	 */
}

/*
 * sysdbg_download_mode_set - Set download mode as early as possible
 *
 * Configures PMIC/hardware to enter download mode for crash dump collection.
 * This ensures device stays in crash dump mode rather than rebooting normally.
 *
 * TODO: Port from TZ sysdbg_dload.c or stub for Phase 1
 */
SYSDBG_IMEM_FUNC void sysdbg_download_mode_set(void)
{
	/*
	 * TODO Phase 1: Stub - no download mode configuration yet
	 * TODO Phase 2: Port from TZ sysdbg_dload.c
	 *
	 * This function typically writes to PMIC registers via SPMI
	 * to configure download mode behavior.
	 */
}

/*
 * DDR Recovery Stubs (Phase 1)
 *
 * These functions will be properly implemented in Phase 2 when DDR
 * recovery code is integrated from U-Boot SPL.
 */

/*
 * ddr_self_refresh_exit_stub - Stub for DDR exit
 *
 * Phase 1: Returns true (simulate success) to allow crash path to continue.
 * Phase 2: Will call actual function pointer from U-Boot SPL.
 */
SYSDBG_IMEM_FUNC static bool ddr_self_refresh_exit_stub(void)
{
	/*
	 * TODO Phase 2: Call actual DDR recovery function
	 * return ddr_self_refresh_exit();
	 *
	 * For Phase 1: Simulate success to allow flow to continue
	 * WARNING: DDR is NOT actually available! This is for code flow
	 *          testing only. Real hardware testing requires Phase 2.
	 */
	return true;
}

/*
 * ddr_self_refresh_enter_stub - Stub for DDR entry
 *
 * Phase 1: Returns true (simulate success).
 * Phase 2: Will call actual function pointer from U-Boot SPL.
 */
SYSDBG_IMEM_FUNC static bool ddr_self_refresh_enter_stub(void)
{
	/*
	 * TODO Phase 2: Call actual DDR recovery function
	 * return ddr_self_refresh_enter();
	 *
	 * For Phase 1: Simulate success
	 */
	return true;
}

/*
 * mon_save_pimem_reg_dump - Save PIMEM registers for crash dump
 *
 * Stub for Phase 1 - saves PIMEM (Protected Internal Memory) registers.
 */
void mon_save_pimem_reg_dump(void)
{
	/* TODO: Port from TZ or stub for Phase 1 */
}

/*
 * pimem_sdi_init - Initialize PIMEM for SDI
 *
 * Stub for Phase 1 - turns off freshness and integrity checks on PIMEM windows.
 */
void pimem_sdi_init(void)
{
	/* TODO: Port from TZ or stub for Phase 1 */
}

/*
 * system_debug_bl31_entry - Main crash reset entry point
 *
 * This is the CRITICAL function that brings DDR out of self-refresh.
 * After this function returns, DDR is accessible and debug collection
 * can proceed.
 *
 * Must execute from IMEM as DDR is in self-refresh at entry.
 *
 * Called from: sysdbg_entry_handler() in sysdbg_imem_entry.S
 *
 * Tasks:
 *   (1) Get DDR function pointers from U-Boot SPL
 *   (2) Set download mode
 *   (3) Bring DDR out of self-refresh
 *   (4) Handle DDR init failure if it occurs
 *   (5) Save PIMEM registers
 *   (6) Initialize PIMEM for crash dump
 *
 * In TZ: system_debug_mon_entry()
 * In TFA: system_debug_bl31_entry()
 */
SYSDBG_IMEM_FUNC void system_debug_bl31_entry(void)
{
	/* Set function start marker for execution tracking */
	SYSDBG_SET_DEBUG_MARKER(SYSDBG_MAIN_ENTRY_START_COOKIE);

	/*
	 * Get DDR self-refresh function pointers from U-Boot SPL
	 *
	 * Phase 1: These return 0 (NULL) since DDR recovery not integrated yet
	 * Phase 2: Will return actual function addresses from BL31 boot params
	 */
	ddr_self_refresh_exit = (bool (*)(void))SYSDBG_DDR_SDI_INIT_ADDR;
	ddr_self_refresh_enter = (bool (*)(void))SYSDBG_DDR_SDI_EXIT_ADDR;

	/* Set download mode as early as possible */
	sysdbg_download_mode_set();

	/*
	 * DDR Self-Refresh Exit
	 *
	 * Only primary core (core 0) should attempt DDR init.
	 * Secondary cores skip this using ddr_initialized flag.
	 */
	if (!ddr_initialized) {
		/*
		 * CRITICAL: Bring DDR out of self-refresh
		 *
		 * Phase 1: Using stub that returns true (simulates success)
		 * Phase 2: Will call actual DDR recovery function from U-Boot SPL
		 */
		if (ddr_self_refresh_exit) {
			ddr_initialized = ddr_self_refresh_exit();
		} else {
			/* Phase 1: Use stub since function pointer is NULL */
			ddr_initialized = ddr_self_refresh_exit_stub();
		}

		if (!ddr_initialized) {
			/*
			 * DDR initialization FAILED
			 *
			 * Cannot proceed with crash dump collection since DDR
			 * is not accessible. Write failure cookie and trigger reset.
			 */

			/* Write DDR failure cookie for PASS2 */
			sysdbg_write_term_cookie(SYSDBG_DDRFAIL_COOKIE);

			/* Attempt alternate DDR reset */
			sysdbg_bl31_ddr_alt_reset();

			/*
			 * Save PIMEM registers before exiting
			 *
			 * Note: Done after writing cookie in case PIMEM AXI clock
			 * is off, which would cause SDI to hang on register access.
			 */
			mon_save_pimem_reg_dump();

			/* Exit PASS1 and trigger reset */
			system_debug_bl31_exit();
		} else {
			/*
			 * DDR initialization SUCCEEDED
			 *
			 * DDR is now accessible. Proceed with crash dump setup.
			 */

			/* Save PIMEM registers by default */
			mon_save_pimem_reg_dump();

			/*
			 * Initialize PIMEM for crash dump
			 * Turn off freshness and integrity checks on PIMEM windows
			 */
			pimem_sdi_init();
		}
	}

	/* Set function end marker for execution tracking */
	SYSDBG_SET_DEBUG_MARKER(SYSDBG_MAIN_ENTRY_END_COOKIE);

	/*
	 * RETURN TO CALLER
	 *
	 * At this point, DDR is accessible!
	 * Caller (sysdbg_entry_handler) will continue to system_debug_reset_handler()
	 * which executes from DDR to collect crash dump data.
	 */
}

/*
 * bl31_get_sysdbg_shared_data - Get shared data structure
 *
 * Returns pointer to shared data structure for coordination between
 * IMEM and DDR phases.
 *
 * Parameters:
 *   sysdbg_stack - IMEM stack pointer to save for exception recovery
 *
 * Returns: Pointer to shared data structure
 *
 * In TZ: mon_get_sysdbg_shared_data()
 * In TFA: bl31_get_sysdbg_shared_data()
 */
SYSDBG_IMEM_FUNC sysdbg_shared_data *bl31_get_sysdbg_shared_data(uint64_t sysdbg_stack)
{
	shared_data.enter_ddr_self_refresh_addr = SYSDBG_DDR_SDI_EXIT_ADDR;
	shared_data.mon_imem_stack_ptr = sysdbg_stack;

	return &shared_data;
}

/*
 * sysdbg_write_term_cookie - Write termination cookie to SHARED_IMEM
 *
 * **MUST EXECUTE FROM IMEM**
 *
 * Ported from TZ.XF.5.36-00138-SHIKRA-1:
 * trustzone_images/core/systemdebug/sysdbg/monitor/src/target/shikra/sysdbg_mon_shikra.c:51
 *
 * Writes a diagnostic cookie to SHARED_IMEM to indicate the reason for
 * SDI termination (success, exception, DDR failure, etc.).
 *
 * Must be in IMEM because it's called from error paths where DDR may not
 * be available (before DDR init or after DDR back in self-refresh).
 *
 * Parameters:
 *   cookie - Termination reason cookie:
 *            SYSDBG_PASS1_SUCCESS_COOKIE (0xDEADD00D) - Success
 *            SYSDBG_EXCEPTION_COOKIE (0xCAFEBABE) - Exception
 *            SYSDBG_DDRFAIL_COOKIE   (0xCAB0DEAD) - DDR init failed
 *            SYSDBG_BADWAKEUP_COOKIE (0x5D100D0A) - Bad wakeup
 *
 * In TZ: mon_sysdbg_write_term_cookie()
 * In TFA: sysdbg_write_term_cookie()
 */
SYSDBG_IMEM_FUNC void sysdbg_write_term_cookie(uint32_t cookie)
{
	volatile uint32_t *shared_imem_cookie =
		(uint32_t *)(SYSTEM_SHAREDIMEM_BASE + SYSDBG_PASS1COMPL_OFFSET);

	mmio_write_32((uintptr_t)shared_imem_cookie, cookie);
}

/*
 * sysdbg_save_err - Exception handler for crash path
 *
 * **MUST EXECUTE FROM IMEM**
 *
 * Ported from TZ.XF.5.36-00138-SHIKRA-1:
 * trustzone_images/core/systemdebug/sysdbg/monitor/src/target/shikra/sysdbg_mon_shikra.c:153
 *
 * If an exception occurs during crash path execution, this handler
 * attempts to save state and trigger reset.
 *
 * Must be in IMEM as exceptions can occur when DDR is in self-refresh.
 *
 * Tasks:
 *   1. Save PIMEM registers (if not nested exception)
 *   2. Write EXCEPTION cookie to SHARED_IMEM
 *   3. Put DDR back in self-refresh (if initialized)
 *   4. Trigger SoC reset
 *
 * This function never returns - system resets to PASS2.
 *
 * In TZ: mon_sysdbg_save_err()
 * In TFA: sysdbg_save_err()
 */
SYSDBG_IMEM_FUNC void sysdbg_save_err(void)
{
	/* Save PIMEM registers on first exception (not nested) */
	if (!sysdbg_coredump.nested_exp) {
		mon_save_pimem_reg_dump();
	}

	/* Write exception cookie for PASS2 */
	sysdbg_write_term_cookie(SYSDBG_EXCEPTION_COOKIE);

	/*
	 * Put DDR back into self-refresh if it was initialized
	 *
	 * This is IMEM exit path - must switch back to IMEM stack
	 * before calling DDR self-refresh enter function.
	 */
	if (ddr_initialized) {
		/* TODO: Implement __set_stack() for ARMv8 if needed */
		/* Original TZ code: __set_stack((uint64 *)shared_data.mon_imem_stack_ptr); */

		if (ddr_self_refresh_enter) {
			ddr_self_refresh_enter();
		} else {
			/* Phase 1: Use stub */
			ddr_self_refresh_enter_stub();
		}
	}

	/* Mark first pass as complete and trigger reset */
	system_debug_bl31_exit();
}

/*
 * sysdbg_update_wd_disable - Update watchdog disable cookie
 *
 * Sets SDI disable cookie in SHARED_IMEM.
 *
 * In TZ: mon_sysdbg_update_wd_disable()
 * In TFA: sysdbg_update_wd_disable()
 */
void sysdbg_update_wd_disable(void)
{
	/* Set SDI disable cookie */
	SYSDBG_SET_DEBUG_MARKER(SYSDBG_DISABLE_COOKIE);
}
