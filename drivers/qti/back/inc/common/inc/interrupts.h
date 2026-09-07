#ifndef INTERRUPTS_H
#define INTERRUPTS_H

/**
@file interrupts.h
@brief Provides API/services to control interrupts.
*/

/*===========================================================================
   Copyright (c) by Qualcomm Technologies, Inc. and/or its subsidiaries.
   All Rights Reserved.
   Confidential and Proprietary - Qualcomm Technologies, Inc.
===========================================================================*/

#include <stdint.h>
#include <stdbool.h>

/*----------------------------------------------------------------------------
 * Preprocessor Definitions and Constants
 * -------------------------------------------------------------------------*/

#define TZBSP_INT_TARGET_CPU(x) (x)

#define TZBSP_INTF_TRIGGER_LEVEL 0x00000000U
#define TZBSP_INTF_TRIGGER_EDGE 0x00000001U

/* The interrupt is marked registered for the internal OS (QSEE). */
#define TZBSP_INTF_INT_FLAG 0x00000000U
/* If active, interrupt is of non fatal type. Default is fatal type.*/
#define TZBSP_INTF_NON_FATAL_INT 0x00002000U
/* If active, the current interrupt configuration is saved and will be restored
 * when the ISR is uninstalled. */
#define TZBSP_INTF_SAVE_INT 0x00004000U
/* The interrupt is marked registered for the external TEE. */
#define TZBSP_INTF_INT_FLAG_TEE 0x00008000U
/* NOTE: Only valid when generating an SGI. */
#define TZBSP_INT_TARGET_OTHERS 0x01000000U
/* NOTE: Only valid when generating an SGI. */
#define TZBSP_INT_TARGET_SELF 0x02000000U
/* NOTE: Only valid when generating an SGI. */
#define TZBSP_INT_TARGET_CPUNUM 0x04000000U
/* NOTE: Only valid when generating an SGI. */
#define TZBSP_INT_TARGET_ALL (TZBSP_INT_TARGET_OTHERS | TZBSP_INT_TARGET_SELF)
/* Interrupt is configured non-secure. Valid only for unregister. */
#define TZBSP_INTF_INT_FLAG_NON_SECURE 0x10000000U
/* Lower byte indicates the targeted CPUs for a given interrupt */
#define TZBSP_INT_TARGET_CPUID_MASK 0xFFFFFF00U

/* The interrupt is configured for CPU0 only. */
/* The value is no longer used but reserved to be used for other purpose
 * as the same has been provided in documentation to 3rd party OS vendors
 */
#define TZBSP_INTF_CPU0_ONLY 0x20000000U

/* The interrupt is marked registered for all CPUs. This affects only the
 * interrupt registry state, not the interupt controller. */
#define TZBSP_INTF_ALL_CPUS 0x40000000U
/* The ISR is installed, but the interrupt itself won't be configured on the
 * interrupt controller. */
#define TZBSP_INTF_SKIP_INT_CONFIG 0x80000000U

/* Interrupts [0, 15] are Software Generated Interrupts (SGI). */
#define TZBSP_INT_SGI(xx) (xx)

/* Interrupts [16, 31] are Private Peripheral Interrupts (PPI). */
#define TZBSP_INT_PPI(xx) ((xx) + 16)

/* Interrupts starting from 32 are Shared Peripheral Interrupts (SPI). */
#define TZBSP_INT_SPI(xx) ((xx) + 32)

/* Macro for determining whether interrupt is a PPI. */
#define TZBSP_INT_IS_PPI(xx) ((16 <= (xx)) && ((xx) < 32))

/**Defines the first SPI interrupt number */
#define QGIC_SPI_INTERRUPT_BASE_NUM 32U

/* Macro for Invalid Interrupt */
#define TZBSP_INT_INVALID_INT_ID -1

/*
 * Secure SGI Usage Details.
 * SGI[8-15] used by Secure World.
 * SGI[0-7] used by Non-Secure World.
 */


/* SGI 11 used by TFA BL31 EL3 interrupt tests but unused in production QTEE */
#define TZBSP_INT_BL31_TEST_EL3_INTERRUPT       TZBSP_INT_SGI(11)
#define TZBSP_INT_BL31_TEST_EL3_INTERRUPT_DESC  "BL31TestInt"

#define QTI_TEST_EL3_INTR_ID              TZBSP_INT_BL31_TEST_EL3_INTERRUPT
#define QTI_TEST_EL3_INTR_ID_DESC         TZBSP_INT_BL31_TEST_EL3_INTERRUPT_DESC

/*----------------------------------------------------------------------------
 * Types of interrupts/exceptions. 1 Bit for each type.
 * typedef enum int_type_bmsk_t
 -------------------------------------------------------------------------*/
#define INT_SEC_IRQ 0x1U
#define INT_MON_IRQ 0x2U
#define INT_NSEC_IRQ 0x4U
#define INT_EXT_ABORT_EXP 0x8U
#define INT_TYPE_MAX UINT32_MAX

/*----------------------------------------------------------------------------
 * Function Declarations and Documentation
 * -------------------------------------------------------------------------*/

 // NEEDSWORK wrapper for fatal error, to be cleaned up with when libaccesscontrol_static.a is updated/removed
int int_register_isr(uint32_t intnum, const char *int_desc, void *(*fn)(void *),
		     void *ctx, uint32_t flags, bool enable);

/**
 * Installs an ISR function in TFA. The corresponding interrupt is registered
 * for the calling CPU (interrupt target is self).
 *
 * @param [in] intnum   Interrupt number the ISR is registered for.
 * @param [in] intdesc  Character string description of the interrupt.
 * @param [in] fn       The ISR function itself.
 * @param [in] ctx      The context passed to ISR.
 * @param [in] flags    Combination of \c BL31QTILIB_INTR_* flags.
 * @param [in] enable   Initial interrupt enable/disable status on the
 *                      interrupt controller.
 *
 * @return Zero on success, error code otherwise.
 */
int int_register_non_fatal_isr(uint32_t intnum, const char *intdesc, void *(*fn)(void *),
		     void *ctx, uint32_t flags, bool enable);

/**
 * Register a fatal error ISR for a given interrupt number. The corresponding
 * interrupt is registered for the calling CPU (interrupt target is self). The
 * interrupt will be registered in TFA or QTEE depending on whether qteed is
 * enabled
 *
 * @param [in] intnum   Interrupt number the ISR is registered for.
 * @param [in] intdesc  Character string description of the interrupt.
 * @param [in] fn       The ISR function itself.
 * @param [in] ctx      The context passed to ISR.
 * @param [in] flags    Combination of \c BL31QTILIB_INTR_* flags.
 * @param [in] enable   Initial interrupt enable/disable status on the
 *                      interrupt controller.
 *
 * @return Zero on success, error code otherwise.
 */
int int_register_fatal_isr(uint32_t intnum, const char *intdesc,
        void *(*fn)(void *), void *ctx, uint32_t flags, bool enable);

/**
 * Uninstalls an ISR function. If no installed ISR is found for the interrupt
 * number, then it is not an error and is ignored silently.
 *
 * @param [in] intnum   Interrupt number the ISR is registered for.
 * @param [in] flags    (Unused) Combination of \c BL31QTILIB_INTR_* flags.
 *
 * @return Zero on success, error code otherwise.
 */
int int_unregister_non_fatal_isr(uint32_t intnum, uint32_t flags);

/* Unregistering fatal IRS is not supported currently
int int_unregister_fatal_isr(uint32_t intnum, uint32_t flags);
*/

/**
 * Disables the given interrupt on the interrupt controller.
 *
 * @param [in] intnum   The interrupt to be disabled.
 *
 * @return E_SUCCESS if successful, Error code otherwise.
 */
int int_disable_non_fatal(uint32_t intnum);

/**
 * Disables the given interrupt on the interrupt controller for a fatal error.
 * Depending on whether qteed is enabled, this will either disable the int in
 * TFA or in QTEE.
 *
 * @param [in] intnum   The interrupt to be disabled.
 *
 * @return E_SUCCESS if successful, Error code otherwise.
 */
int int_disable_fatal(uint32_t intnum);

/**
 * Enables the given interrupt on the interrupt controller.
 *
 * @param [in] intnum   The interrupt to be enabled.
 *
 * @return E_SUCCESS if successful, Error code otherwise.
 */
int int_enable_non_fatal(uint32_t intnum);

/* The enabling of fatal interrupts is not supported
int enable_fatal_int(uint32_t intnum);
*/
#endif /* INTERRUPTS_H */