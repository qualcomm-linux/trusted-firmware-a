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

#include "IxErrno.h"

#ifndef EFAULT
	#define EFAULT      E_FAILURE
	#define EINVAL      E_INVALID_ARG
	#define EAGAIN      E_AGAIN
	#define EINPROGRESS E_IN_PROGRESS
	#define ETIMEDOUT   E_TIMER_EXP
	#define ENOTCONN    E_NO_ENTRY
	#define ENOMEM      E_NO_MEMORY
	#define ENODEV      E_NO_DEV
#endif

typedef struct {
	int placeholder; /* BL31 is single-threaded; no OS mutex is needed */
} tmecomOSMutex;

typedef struct {
	void *glink_handle;
	bool  signalled_by_poll;
} tmecomOSEvent;

#ifdef __cplusplus
extern "C" {
#endif

/**
	* OS abstraction used by tmecom for event creation.
	*
	* @param  [out] event   A pointer to a tmecomOSEvent structure.
	*
	* @return @c 0 on success, failure code otherwise.
	*/
int  tmecom_os_init_event(tmecomOSEvent *event);

/**
	* OS abstraction used by tmecom for event signalling.
	*
	* @param  [in]  event   A pointer to a tmecomOSEvent structure.
	*
	* @return @c 0 on success, failure code otherwise.
	*/
int  tmecom_os_signal_event(tmecomOSEvent *event);

/**
	* OS abstraction used by tmecom for resetting an event signal.
	*
	* @param  [in]  event   A pointer to a tmecomOSEvent structure.
	*
	* @return @c 0 on success, failure code otherwise.
	*/
int  tmecom_os_reset_event(tmecomOSEvent *event);

/**
	* OS abstraction used by tmecom to check if an event has been signalled.
	*
	* @param  [in]  event   A pointer to a tmecomOSEvent structure.
	*
	* @return @c true if the event has been signalled, @c false otherwise.
	*/
bool tmecom_os_is_event_signalled(tmecomOSEvent *event);

/**
	* OS abstraction used by tmecom for waiting on an event signal.
	*
	* @param  [in]  event   A pointer to a tmecomOSEvent structure.
	*
	* @return @c 1 on success, other value otherwise.
	*/
int  tmecom_os_wait_for_event(tmecomOSEvent *event);

/**
	* OS abstraction used by tmecom for waiting on an event signal with a timeout.
	*
	* @param  [in]  event         A pointer to a tmecomOSEvent structure.
	* @param  [in]  timeout_msec   Timeout specified in milliseconds.
	*
	* @return @c 1 on success, other value otherwise.
	*/
int  tmecom_os_wait_for_event_with_timeout(tmecomOSEvent *event, uint32_t timeout_msec);

/**
	* OS abstraction used by tmecom for mutex creation.
	*
	* @param  [in]  mutex   A pointer to a tmecomOSMutex structure.
	*
	* @return @c 0 on success, failure code otherwise.
	*/
int  tmecom_os_init_mutex(tmecomOSMutex *mutex);

/**
	* OS abstraction used by tmecom for mutex lock.
	*
	* @param  [in]  mutex   A pointer to a tmecomOSMutex structure.
	*
	* @return @c 0 on success, failure code otherwise.
	*/
int  tmecom_os_lock_mutex(tmecomOSMutex *mutex);

/**
	* OS abstraction used by tmecom for mutex unlock.
	*
	* @param  [in]  mutex   A pointer to a tmecomOSMutex structure.
	*
	* @return @c 0 on success, failure code otherwise.
	*/
int  tmecom_os_unlock_mutex(tmecomOSMutex *mutex);

/**
	* OS abstraction used by tmecom to set the glink event handle.
	*
	* @param  [in]  event        A pointer to a tmecomOSEvent structure.
	* @param  [in]  glink_handle  The glink handle to associate with the event.
	*
	* @return @c 0 on success, failure code otherwise.
	*/
int  tmecom_os_set_event_glink_handle(tmecomOSEvent *event, void *glink_handle);

/**
	* OS abstraction used by tmecom for sleep.
	*
	* @param  [in]  msec   Number of milliseconds to sleep for.
	*/
void  tmecom_sleep(uint32_t msec);

#ifdef __cplusplus
}
#endif

#endif /* TMECOM_OS_AL_H */
