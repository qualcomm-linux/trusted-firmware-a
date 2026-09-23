/*===========================================================================
	Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
	All rights reserved.
	Confidential and Proprietary - Qualcomm Technologies, Inc.
===========================================================================*/

#ifndef TMECOM_TFA_H
#define TMECOM_TFA_H

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
	 * @channel_name:    glink logical channel name
	 */
typedef struct {
	const char *channel_name;
} tmecomClientInfo;

/**
	* Register a client with the tmecom.
	*
	* @param [in]  info       Pointer to a structure containing information
	*                         relating to the client being registered.
	* @param [out] client_ptr  Pointer to a pointer to a unique opaque handle returned
	*                         by the client registration process that must be used
	*                         in later calls to the tmecom interface.
	*
	* @return @c 0 if successfully handled, error code otherwise
	*/
int tmecom_register_client(const tmecomClientInfo *info, tmecomClient **client_ptr);

/**
	* Unregister a client with the tmecom and release memory allocated by
	* @c tmecom_register_client().
	*
	* @param [in]  client_ptr  Pointer to a unique opaque handle identifying the client
	*                         to be unregistered.
	*
	* @return @c 0 if successfully handled, error code otherwise
	*/
int tmecom_unregister_client(tmecomClient *client_ptr);

/**
	* Send a request to another EE (Execution Environment) using the QMP interface.
	*
	* @param [in]     client_ptr    Pointer to tmecom client.
	* @param [in]     req_ptr       Pointer to a buffer to send to another EE.
	* @param [in]     req_size      Size of the buffer to send to another EE (in bytes).
	* @param [out]    resp_ptr      Pointer to a buffer to receive a response from another EE.
	* @param [in/out] resp_size     Size of the buffer to receive a response from another EE
	*                              (in bytes).
	* @param [in]     timeout_msec  Timeout in msec for receiving a response.
	*
	* @return @c 0 if successfully handled, error code otherwise
	*
	* @note The function @c tmecom_client_send_message_sync() is a blocking function and
	*       will not return unless there is either a timeout or the other EE returns
	*       with a response.
	*/
int tmecom_client_send_message_sync(void     *client_ptr,
		void     *req_ptr,
		size_t    req_size,
		void     *resp_ptr,
		size_t   *resp_size,
		uint32_t  timeout_msec);

/**
	* Submit a request to the TME SS without waiting for a response.
	*
	* Returns once the request has been handed to the transport layer (this
	* still involves a bounded, internal wait for the local transport's tx
	* acknowledgement) -- it does not wait for the TME SS to finish processing
	* the request. Use @c tmecom_client_recv_message_async() to poll for the
	* response.
	*
	* Only one transaction -- sent via this function or
	* @c tmecom_client_send_message_sync() -- may be outstanding on a channel at a
	* time, since the TME SS does not support queuing.
	*
	* @param [in]  client_ptr  Pointer to tmecom client.
	* @param [in]  req_ptr     Pointer to a buffer to send to the TME SS.
	* @param [in]  req_size    Size of the buffer to send (in bytes).
	* @param [out] txn_id      Transaction id identifying this request, to be
	*                          passed to @c tmecom_client_recv_message_async().
	*
	* @return @c 0 if the request was submitted.
	*         @c -EAGAIN if a transaction is already outstanding on this
	*         channel; the caller should retry later.
	*         Other error codes on failure to submit the request.
	*/
int tmecom_client_send_message_async(void     *client_ptr,
		 void     *req_ptr,
		 size_t    req_size,
		 uint32_t *txn_id);

/**
	* Poll, without blocking, for the response to a request previously
	* submitted via @c tmecom_client_send_message_async().
	*
	* @param [in]     client_ptr  Pointer to tmecom client.
	* @param [in]     txn_id      Transaction id returned by
	*                             @c tmecom_client_send_message_async().
	* @param [out]    resp_ptr    Pointer to a buffer to receive the response.
	* @param [in/out] resp_size   Size of the buffer to receive the response (in
	*                             bytes); updated with the actual response size
	*                             on success.
	*
	* @return @c 0 if the response is ready and has been copied out.
	*         @c -EINPROGRESS if the TME SS has not yet responded; the caller
	*         should retry later.
	*         @c -EINVAL if @c txn_id does not match the outstanding
	*         transaction on this channel.
	*/
int tmecom_client_recv_message_async(void     *client_ptr,
		 uint32_t  txn_id,
		 void     *resp_ptr,
		 size_t   *resp_size);

/**
	* Check the state of the tmecom link with the TME SS
	*
	* @return @c true if the link is up, @c false otherwise
	*/
bool tmecom_is_tme_subsystem_link_up(void);

/**
	* Check for a connection with the server.
	*
	* @param [in]  client_ptr  Pointer to a unique opaque handle identifying the client.
	*
	* @return @c true if server is connected, false otherwise.
	*/
bool tmecom_client_is_server_connected(tmecomClient *client_ptr);

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
int tmecom_init_test(eTMEComInterface tmecomInterface);

/** @} */ /* end addtogroup TMECom */

#ifdef __cplusplus
}
#endif

#endif /* TMECOM_TFA_H */
