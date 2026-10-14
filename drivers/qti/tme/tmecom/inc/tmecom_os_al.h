/*===========================================================================
  Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
  All rights reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
===========================================================================*/

#ifndef TMECOM_OS_AL_H
#define TMECOM_OS_AL_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <lib/libc/errno.h>

typedef struct tmecom_os_mutex {
	int placeholder; /* BL31 is single-threaded; no OS mutex is needed */
} tmecom_os_mutex_t;

typedef struct tmecom_os_event {
	void *glink_handle;
	bool signalled_by_poll;
} tmecom_os_event_t;

/**
  * OS abstraction used by tmecom for sleep.
  *
  * @param  [in]  msec   Number of milliseconds to sleep for.
  */
void tmecom_sleep(uint32_t msec);

#endif /* TMECOM_OS_AL_H */
