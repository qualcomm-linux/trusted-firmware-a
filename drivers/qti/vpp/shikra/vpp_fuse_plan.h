/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * CPR10 (Silver), CPR11 (Gold) and CPR12 (L3) eFuse getters, included
 * directly into vpp_voltage_plan.c as fuse_cb_t callbacks referenced by
 * the generated voltage-plan tables.
 */

#ifndef VPP_FUSE_PLAN_H
#define VPP_FUSE_PLAN_H

#include <stdint.h>

#include "vpp_fuse_hwio.h"

/******************************************************************************
 *			RO Select Fuse
 ******************************************************************************/

static int32_t CPR10_LSVS_ROSEL(void) { return 13; }
static int32_t CPR10_SVS_ROSEL(void) { return 13; }
static int32_t CPR10_NOM_ROSEL(void) { return 13; }
static int32_t CPR10_TUR_ROSEL(void) { return 7; }
static int32_t CPR10_BOOST_ROSEL(void) { return 7; }
static int32_t CPR12_LSVS_ROSEL(void) { return 13; }
static int32_t CPR12_SVS_ROSEL(void) { return 13; }
static int32_t CPR12_NOM_ROSEL(void) { return 13; }
static int32_t CPR12_TUR_ROSEL(void) { return 7; }
static int32_t CPR11_LSVS_ROSEL(void) { return 13; }
static int32_t CPR11_SVS_ROSEL(void) { return 13; }
static int32_t CPR11_NOM_ROSEL(void) { return 13; }
static int32_t CPR11_TUR_ROSEL(void) { return 7; }
static int32_t CPR11_BOOST_ROSEL(void) { return 7; }

/******************************************************************************
 *			Target Voltage Fuse
 *
 * Returned value is the offset in mV from the reference voltage.
 ******************************************************************************/

static int32_t CPR10_TARG_VOLT_LSVS(void)
{
	return VPP_OLV(VPP_HWIO_INM(QFPROM_CORR_CALIB_ROW8_LSB, CPR10_TARG_VOLT_LSVS));
}
static int32_t CPR10_TARG_VOLT_SVS(void)
{
	return VPP_OLV(VPP_HWIO_INM(QFPROM_CORR_CALIB_ROW8_MSB, CPR10_TARG_VOLT_SVS));
}
static int32_t CPR10_TARG_VOLT_NOM(void)
{
	return VPP_OLV(VPP_HWIO_INM(QFPROM_CORR_CALIB_ROW9_LSB, CPR10_TARG_VOLT_NOM));
}
static int32_t CPR10_TARG_VOLT_TUR(void)
{
	return VPP_OLV(VPP_HWIO_INM(QFPROM_CORR_CALIB_ROW9_MSB, CPR10_TARG_VOLT_TUR));
}
static int32_t CPR10_TARG_VOLT_BOOST(void)
{
	return VPP_OLV(VPP_HWIO_INM(QFPROM_CORR_CALIB_ROW9_MSB, CPR10_TARG_VOLT_BOOST));
}
static int32_t CPR12_TARG_VOLT_LSVS(void)
{
	return VPP_OLV(VPP_HWIO_INM(QFPROM_CORR_CALIB_ROW4_MSB, CPR12_TARG_VOLT_LSVS));
}
static int32_t CPR12_TARG_VOLT_SVS(void)
{
	return VPP_OLV(VPP_HWIO_INM(QFPROM_CORR_CALIB_ROW5_LSB, CPR12_TARG_VOLT_SVS));
}
static int32_t CPR12_TARG_VOLT_NOM(void)
{
	return VPP_OLV(VPP_HWIO_INM(QFPROM_CORR_CALIB_ROW5_MSB, CPR12_TARG_VOLT_NOM));
}
static int32_t CPR12_TARG_VOLT_TUR(void)
{
	return VPP_OLV(VPP_HWIO_INM(QFPROM_CORR_CALIB_ROW5_MSB, CPR12_TARG_VOLT_TUR));
}
static int32_t CPR11_TARG_VOLT_LSVS(void)
{
	return VPP_OLV(VPP_HWIO_INM(QFPROM_CORR_CALIB_ROW5_MSB, CPR11_TARG_VOLT_LSVS));
}
static int32_t CPR11_TARG_VOLT_SVS(void)
{
	return VPP_OLV(VPP_HWIO_INM(QFPROM_CORR_CALIB_ROW6_LSB, CPR11_TARG_VOLT_SVS));
}
static int32_t CPR11_TARG_VOLT_NOM(void)
{
	return VPP_OLV(VPP_HWIO_INM(QFPROM_CORR_CALIB_ROW6_MSB, CPR11_TARG_VOLT_NOM));
}
static int32_t CPR11_TARG_VOLT_TUR(void)
{
	return VPP_OLV(VPP_HWIO_INM(QFPROM_CORR_CALIB_ROW7_LSB, CPR11_TARG_VOLT_TUR));
}
static int32_t CPR11_TARG_VOLT_BOOST(void)
{
	return VPP_OLV(VPP_HWIO_INM(QFPROM_CORR_CALIB_ROW7_MSB, CPR11_TARG_VOLT_BOOST));
}

/******************************************************************************
 *			Target Quotient Fuse
 *
 * Returned value is the final target quotient.
 ******************************************************************************/

static int32_t CPR10_QUOT_MIN_LSVS(void)
{
	return VPP_HWIO_INM(QFPROM_CORR_CALIB_ROW7_MSB, CPR10_QUOT_MIN_LSVS);
}
static int32_t CPR10_QUOT_MIN_SVS(void)
{
	return VPP_HWIO_INM(QFPROM_CORR_CALIB_ROW8_LSB, CPR10_QUOT_MIN_SVS);
}
static int32_t CPR10_QUOT_MIN_NOM(void)
{
	return VPP_HWIO_INM(QFPROM_CORR_CALIB_ROW8_MSB, CPR10_QUOT_MIN_NOM);
}
static int32_t CPR10_QUOT_MIN_TUR(void)
{
	return VPP_HWIO_INM(QFPROM_CORR_CALIB_ROW8_MSB, CPR10_QUOT_MIN_TUR);
}
static int32_t CPR10_QUOT_MIN_BOOST(void)
{
	return VPP_HWIO_INM(QFPROM_CORR_CALIB_ROW9_LSB, CPR10_QUOT_MIN_BOOST);
}
static int32_t CPR12_QUOT_MIN_LSVS(void)
{
	return VPP_HWIO_INM(QFPROM_CORR_CALIB_ROW4_LSB, CPR12_QUOT_MIN_LSVS);
}
static int32_t CPR12_QUOT_MIN_SVS(void)
{
	return VPP_HWIO_INM(QFPROM_CORR_CALIB_ROW4_MSB, CPR12_QUOT_MIN_SVS);
}
static int32_t CPR12_QUOT_MIN_NOM(void)
{
	return VPP_HWIO_INM(QFPROM_CORR_CALIB_ROW4_MSB, CPR12_QUOT_MIN_NOM);
}
static int32_t CPR12_QUOT_MIN_TUR(void)
{
	return VPP_HWIO_INM(QFPROM_CORR_CALIB_ROW5_LSB, CPR12_QUOT_MIN_TUR);
}
static int32_t CPR11_QUOT_MIN_LSVS(void)
{
	return VPP_HWIO_INM(QFPROM_CORR_CALIB_ROW6_LSB, CPR11_QUOT_MIN_LSVS);
}
static int32_t CPR11_QUOT_MIN_SVS(void)
{
	return VPP_HWIO_INM(QFPROM_CORR_CALIB_ROW6_LSB, CPR11_QUOT_MIN_SVS);
}
static int32_t CPR11_QUOT_MIN_NOM(void)
{
	return VPP_HWIO_INM(QFPROM_CORR_CALIB_ROW6_MSB, CPR11_QUOT_MIN_NOM);
}
static int32_t CPR11_QUOT_MIN_TUR(void)
{
	return VPP_HWIO_INM(QFPROM_CORR_CALIB_ROW6_MSB, CPR11_QUOT_MIN_TUR);
}
static int32_t CPR11_QUOT_MIN_BOOST(void)
{
	return VPP_HWIO_INM(QFPROM_CORR_CALIB_ROW7_LSB, CPR11_QUOT_MIN_BOOST);
}

/******************************************************************************
 *			Quotient Offset Fuse
 ******************************************************************************/

static int32_t CPR10_QUOT_OFFSET_SVS(void)
{
	return VPP_HWIO_INM(QFPROM_CORR_CALIB_ROW7_MSB, CPR10_QUOT_OFFSET) *
	       VPP_CLV_QUOT_OFFSET_PACK_FACTOR;
}
static int32_t CPR10_QUOT_OFFSET_NOM(void)
{
	return VPP_HWIO_INM(QFPROM_CORR_CALIB_ROW7_MSB, CPR10_QUOT_OFFSET) *
	       VPP_CLV_QUOT_OFFSET_PACK_FACTOR;
}
static int32_t CPR10_QUOT_OFFSET_TUR(void)
{
	return VPP_HWIO_INM(QFPROM_CORR_CALIB_ROW7_MSB, CPR10_QUOT_OFFSET) *
	       VPP_CLV_QUOT_OFFSET_PACK_FACTOR;
}
static int32_t CPR10_QUOT_OFFSET_BOOST(void)
{
	return VPP_HWIO_INM(QFPROM_CORR_CALIB_ROW7_MSB, CPR10_QUOT_OFFSET) *
	       VPP_CLV_QUOT_OFFSET_PACK_FACTOR;
}
static int32_t CPR12_QUOT_OFFSET_SVS(void)
{
	return VPP_HWIO_INM(QFPROM_RAW_CALIB_ROW4_LSB, CPR12_QUOT_OFFSET) *
	       VPP_CLV_QUOT_OFFSET_PACK_FACTOR;
}
static int32_t CPR12_QUOT_OFFSET_NOM(void)
{
	return VPP_HWIO_INM(QFPROM_RAW_CALIB_ROW4_LSB, CPR12_QUOT_OFFSET) *
	       VPP_CLV_QUOT_OFFSET_PACK_FACTOR;
}
static int32_t CPR12_QUOT_OFFSET_TUR(void)
{
	return VPP_HWIO_INM(QFPROM_RAW_CALIB_ROW4_LSB, CPR12_QUOT_OFFSET) *
	       VPP_CLV_QUOT_OFFSET_PACK_FACTOR;
}
static int32_t CPR11_QUOT_OFFSET_SVS(void)
{
	return VPP_HWIO_INM(QFPROM_CORR_CALIB_ROW5_MSB, CPR11_QUOT_OFFSET) *
	       VPP_CLV_QUOT_OFFSET_PACK_FACTOR;
}
static int32_t CPR11_QUOT_OFFSET_NOM(void)
{
	return VPP_HWIO_INM(QFPROM_CORR_CALIB_ROW5_MSB, CPR11_QUOT_OFFSET) *
	       VPP_CLV_QUOT_OFFSET_PACK_FACTOR;
}
static int32_t CPR11_QUOT_OFFSET_TUR(void)
{
	return VPP_HWIO_INM(QFPROM_CORR_CALIB_ROW5_MSB, CPR11_QUOT_OFFSET) *
	       VPP_CLV_QUOT_OFFSET_PACK_FACTOR;
}
static int32_t CPR11_QUOT_OFFSET_BOOST(void)
{
	return VPP_HWIO_INM(QFPROM_CORR_CALIB_ROW5_MSB, CPR11_QUOT_OFFSET) *
	       VPP_CLV_QUOT_OFFSET_PACK_FACTOR;
}

#endif /* VPP_FUSE_PLAN_H */
