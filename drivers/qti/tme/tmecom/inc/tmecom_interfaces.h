/*===========================================================================
  Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
  All rights reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
===========================================================================*/

#ifndef TMECOM_INTERFACES_H_INCLUDED
#define TMECOM_INTERFACES_H_INCLUDED

#include <stdint.h>

#include "tmecom.h"

/* clang-format off */

#define CHECK_BAIL(is_valid)            \
	do {                                  \
		if (!(is_valid)) {                  \
			 goto bail;                       \
		};                                  \
	} while(0)

#define TMECOM_CONNECTION_TIMEOUT_MS  100

/*
 * Responses from the TME SS can be slow depending on the requested operation.
 * Failure to return within the allotted time period is considered terminal
 * as the TME-FW has become unresponsive.
 */
#define TMECOM_RESPONSE_TIMEOUT_S      10
#define TMECOM_RESPONSE_TIMEOUT_MS    (TMECOM_RESPONSE_TIMEOUT_S * 1000)

/* clang-format on */

/**
  * Initialize tmecom client interface.
  *
  * @param [out] client_ptr  Pointer to a pointer to a unique opaque handle
  *                          returned by the client registration process that
  *                          must be used in later calls to the tmecom
  *                          interface.
  *
  * @return @c 0 if successfully handled, error code otherwise
  */
int tmecom_interface_init(tmecom_client_t **client_ptr);

/**
  * Un-initialize tmecom client interface.
  */
void tmecom_interface_deinit(void);

#endif /* TMECOM_INTERFACES_H_INCLUDED */
