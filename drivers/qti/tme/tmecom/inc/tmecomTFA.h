/*===========================================================================
  Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
  All rights reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
===========================================================================*/

#ifndef _TMECOMTFA_H
#define _TMECOMTFA_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "tmecom.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
  * @addtogroup TMECom
  * @{
  */

/**
  * Abstract tmecom handle.
  *
  * The actual struct definition is internal to the tmecom driver.
  */
#ifndef TMECOMCLIENT_TYPEDEF_DEFINED
#define TMECOMCLIENT_TYPEDEF_DEFINED
typedef struct tmecomClient tmecomClient; /* Forward declaration */
#endif

 /**
   * Client registration info
   *
   * @channelName:    glink logical channel name
   */
typedef struct {
  const char *channelName;
} tmecomClientInfo;

/**
  * Register a client with the tmecom.
  *
  * @param [in]  info       Pointer to a structure containing information
  *                         relating to the client being registered.
  * @param [out] clientPtr  Pointer to a pointer to a unique opaque handle returned
  *                         by the client registration process that must be used
  *                         in later calls to the tmecom interface.
  *
  * @return @c 0 if successfully handled, error code otherwise
  */
int tmecomRegisterClient(const tmecomClientInfo *info, tmecomClient **clientPtr);

/**
  * Unregister a client with the tmecom and release memory allocated by
  * @c tmecomRegisterClient().
  *
  * @param [in]  clientPtr  Pointer to a unique opaque handle identifying the client
  *                         to be unregistered.
  *
  * @return @c 0 if successfully handled, error code otherwise
  */
int tmecomUnregisterClient(tmecomClient *clientPtr);

/**
  * Send a request to another EE (Execution Environment) using the QMP interface.
  *
  * @param [in]     clientPtr    Pointer to tmecom client.
  * @param [in]     reqPtr       Pointer to a buffer to send to another EE.
  * @param [in]     reqSize      Size of the buffer to send to another EE (in bytes).
  * @param [out]    respPtr      Pointer to a buffer to receive a response from another EE.
  * @param [in/out] respSize     Size of the buffer to receive a response from another EE
  *                              (in bytes).
  * @param [in]     timeoutMSec  Timeout in msec for receiving a response.
  *
  * @return @c 0 if successfully handled, error code otherwise
  *
  * @note The function @c tmecomClientSendMessageSync() is a blocking function and
  *       will not return unless there is either a timeout or the other EE returns
  *       with a response.
  */
int tmecomClientSendMessageSync(void     *clientPtr,
                                void     *reqPtr,
                                size_t    reqSize,
                                void     *respPtr,
                                size_t   *respSize,
                                uint32_t  timeoutMSec);

/**
  * Submit a request to the TME SS without waiting for a response.
  *
  * Returns once the request has been handed to the transport layer (this
  * still involves a bounded, internal wait for the local transport's tx
  * acknowledgement) -- it does not wait for the TME SS to finish processing
  * the request. Use @c tmecomClientRecvMessageAsync() to poll for the
  * response.
  *
  * Only one transaction -- sent via this function or
  * @c tmecomClientSendMessageSync() -- may be outstanding on a channel at a
  * time, since the TME SS does not support queuing.
  *
  * @param [in]  clientPtr  Pointer to tmecom client.
  * @param [in]  reqPtr     Pointer to a buffer to send to the TME SS.
  * @param [in]  reqSize    Size of the buffer to send (in bytes).
  * @param [out] txnId      Transaction id identifying this request, to be
  *                          passed to @c tmecomClientRecvMessageAsync().
  *
  * @return @c 0 if the request was submitted.
  *         @c -EAGAIN if a transaction is already outstanding on this
  *         channel; the caller should retry later.
  *         Other error codes on failure to submit the request.
  */
int tmecomClientSendMessageAsync(void     *clientPtr,
                                 void     *reqPtr,
                                 size_t    reqSize,
                                 uint32_t *txnId);

/**
  * Poll, without blocking, for the response to a request previously
  * submitted via @c tmecomClientSendMessageAsync().
  *
  * @param [in]     clientPtr  Pointer to tmecom client.
  * @param [in]     txnId      Transaction id returned by
  *                             @c tmecomClientSendMessageAsync().
  * @param [out]    respPtr    Pointer to a buffer to receive the response.
  * @param [in/out] respSize   Size of the buffer to receive the response (in
  *                             bytes); updated with the actual response size
  *                             on success.
  *
  * @return @c 0 if the response is ready and has been copied out.
  *         @c -EINPROGRESS if the TME SS has not yet responded; the caller
  *         should retry later.
  *         @c -EINVAL if @c txnId does not match the outstanding
  *         transaction on this channel.
  */
int tmecomClientRecvMessageAsync(void     *clientPtr,
                                 uint32_t  txnId,
                                 void     *respPtr,
                                 size_t   *respSize);

/**
  * Check the state of the tmecom link with the TME SS
  *
  * @return @c true if the link is up, @c false otherwise
  */
bool tmecomIsTmeSubsystemLinkUp(void);

/**
  * Check for a connection with the server.
  *
  * @param [in]  clientPtr  Pointer to a unique opaque handle identifying the client.
  *
  * @return @c true if server is connected, false otherwise.
  */
bool tmecomClientIsServerConnected(tmecomClient *clientPtr);

/**
  * Initialize the TMECom interface for test.
  *
  * This is a test function that can be manually compiled into the code base to
  * check the TMECom interface is working as expected.
  *
  * This function is designed to be used during bring-up only.
  *
  * @param  [in]  tmecomInterface   Interface to initialize
  *
  * @return @c 0 if successfully handled, error code otherwise
  */
int tmecomInitTest(eTMEComInterface tmecomInterface);

/** @} */ /* end addtogroup TMECom */

#ifdef __cplusplus
}
#endif

#endif // _TMECOMTFA_H
