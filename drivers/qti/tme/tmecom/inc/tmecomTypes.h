/*===========================================================================
  Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
  All rights reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
===========================================================================*/

#ifndef _TMECOMTYPES_H
#define _TMECOMTYPES_H

/**
  * @addtogroup TMECom
  * @{
  */

typedef enum
{
  TME_COM_INTERFACE_NONE      = 0,   /* TMECom interface is not initialized */
  TME_COM_INTERFACE_RMB       = 1,   /* Initialize the TMECom RMB interface */
  TME_COM_INTERFACE_QMP       = 2,   /* Initialize the TMECom QMP interface */
  TME_COM_INTERFACE_MAX,
  TME_COM_INTERFACE_INVALID   = 0x7FFFFFFF

} eTMEComInterface;

/** @} */ /* end addtogroup TMECom */

#endif // _TMECOMTYPES_H
