/*===========================================================================
  Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
  All rights reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
===========================================================================*/

#ifndef _TMECOM_OS_AL_H
#define _TMECOM_OS_AL_H

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
  void *glinkHandle;
  bool  signalledByPoll;
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
int  tmecomOSInitEvent(tmecomOSEvent *event);

/**
  * OS abstraction used by tmecom for event signalling.
  *
  * @param  [in]  event   A pointer to a tmecomOSEvent structure.
  *
  * @return @c 0 on success, failure code otherwise.
  */
int  tmecomOSSignalEvent(tmecomOSEvent *event);

/**
  * OS abstraction used by tmecom for resetting an event signal.
  *
  * @param  [in]  event   A pointer to a tmecomOSEvent structure.
  *
  * @return @c 0 on success, failure code otherwise.
  */
int  tmecomOSResetEvent(tmecomOSEvent *event);

/**
  * OS abstraction used by tmecom to check if an event has been signalled.
  *
  * @param  [in]  event   A pointer to a tmecomOSEvent structure.
  *
  * @return @c true if the event has been signalled, @c false otherwise.
  */
bool tmecomOSIsEventSignalled(tmecomOSEvent *event);

/**
  * OS abstraction used by tmecom for waiting on an event signal.
  *
  * @param  [in]  event   A pointer to a tmecomOSEvent structure.
  *
  * @return @c 1 on success, other value otherwise.
  */
int  tmecomOSWaitForEvent(tmecomOSEvent *event);

/**
  * OS abstraction used by tmecom for waiting on an event signal with a timeout.
  *
  * @param  [in]  event         A pointer to a tmecomOSEvent structure.
  * @param  [in]  timeoutMSec   Timeout specified in milliseconds.
  *
  * @return @c 1 on success, other value otherwise.
  */
int  tmecomOSWaitForEventWithTimeout(tmecomOSEvent *event, uint32_t timeoutMSec);

/**
  * OS abstraction used by tmecom for mutex creation.
  *
  * @param  [in]  mutex   A pointer to a tmecomOSMutex structure.
  *
  * @return @c 0 on success, failure code otherwise.
  */
int  tmecomOSInitMutex(tmecomOSMutex *mutex);

/**
  * OS abstraction used by tmecom for mutex lock.
  *
  * @param  [in]  mutex   A pointer to a tmecomOSMutex structure.
  *
  * @return @c 0 on success, failure code otherwise.
  */
int  tmecomOSLockMutex(tmecomOSMutex *mutex);

/**
  * OS abstraction used by tmecom for mutex unlock.
  *
  * @param  [in]  mutex   A pointer to a tmecomOSMutex structure.
  *
  * @return @c 0 on success, failure code otherwise.
  */
int  tmecomOSUnlockMutex(tmecomOSMutex *mutex);

/**
  * OS abstraction used by tmecom to set the glink event handle.
  *
  * @param  [in]  event        A pointer to a tmecomOSEvent structure.
  * @param  [in]  glinkHandle  The glink handle to associate with the event.
  *
  * @return @c 0 on success, failure code otherwise.
  */
int  tmecomOSSetEventGlinkHandle(tmecomOSEvent *event, void *glinkHandle);

/**
  * OS abstraction used by tmecom for sleep.
  *
  * @param  [in]  msec   Number of milliseconds to sleep for.
  */
void  tmecomSleep(uint32_t msec);

#ifdef __cplusplus
}
#endif

#endif /* _TMECOM_OS_AL_H */
