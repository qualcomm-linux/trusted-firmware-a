/*===========================================================================
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * All rights reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 * @file  glink.h
 * @brief GLink public API — reliable, in-order, datagram-based
 *        inter-processor communication over QMP (Qualcomm Messaging
 *        Protocol) transport.
 *
 *===========================================================================*/

#ifndef GLINK_H
#define GLINK_H

#ifdef __cplusplus
extern "C" {
#endif

#include <com_dtypes.h>
#include <stddef.h>

/*===========================================================================
                         MACRO DECLARATIONS
===========================================================================*/

/** Maximum channel name length in bytes (including null terminator) */
#define GLINK_CH_NAME_LEN        32

/** Version number for the glink_link_id_type structure */
#define GLINK_LINK_ID_VER        0x00000001

/** TX option: no special flags */
#define GLINK_TX_NO_OPTIONS      (0)

/** TX option: block and request a remote rx intent if one is not available */
/* Whether to block and request for remote rx intent in
 * case it is not available for this pkt tx */
#define GLINK_TX_REQ_INTENT      0x00000001

/** TX option: single-threaded context — flush data in glink_tx() or fail */
#define GLINK_TX_SINGLE_THREADED 0x00000002

/** Maximum remote subsystem name length in bytes (including null terminator) */
#define GLINK_MAX_HOST_NAME      10

/** Open config option: transport is initial; migration to higher-priority
 *  transports is allowed.  Without this flag, open fails if the transport
 *  does not exist. */
#define GLINK_OPT_INITIAL_XPORT  0x00000001

/** Macro to initialize a glink_link_id_type structure to safe defaults.
 *  Must be called before passing the structure to glink_register_link_state_cb().
 *
 *  @param[in,out] link_id  The glink_link_id_type variable to initialize.
 */
#define GLINK_LINK_ID_STRUCT_INIT(link_id)  \
    (link_id).xport         = 0;            \
    (link_id).remote_ss     = 0;            \
    (link_id).link_notifier = 0;            \
    (link_id).handle        = 0;            \
    (link_id).version       = GLINK_LINK_ID_VER;

/*===========================================================================
                         TYPE DECLARATIONS
===========================================================================*/

/**
 * glink_err_type - GLink status and return codes.
 *
 * Returned by all GLink API functions to indicate success or the
 * specific failure reason.
 */
typedef enum {
    GLINK_STATUS_SUCCESS                = 0,   /**< Operation succeeded */
    GLINK_STATUS_INVALID_PARAM          = -1,  /**< Invalid parameter */
    GLINK_STATUS_NOT_INIT               = -2,  /**< Transport not initialized */
    GLINK_STATUS_OUT_OF_RESOURCES       = -3,  /**< Insufficient resources */
    GLINK_STATUS_NO_TRANSPORT           = -4,  /**< No matching transport found */
    GLINK_STATUS_NO_REMOTE_INTENT_FOUND = -5,  /**< No remote rx intent queued */
    GLINK_STATUS_CH_NOT_FULLY_OPENED    = -6,  /**< Channel not fully open */
    GLINK_STATUS_CH_ALREADY_CLOSED      = -7,  /**< Channel already closed */
    GLINK_STATUS_API_NOT_SUPPORTED      = -8,  /**< API not supported by transport */
    GLINK_STATUS_QOS_FAILURE            = -9,  /**< QoS algorithm failure */
    GLINK_STATUS_TX_CMD_FAILURE         = -10, /**< TX command failure */
    GLINK_STATUS_FAILURE                = -11, /**< Generic failure */
    GLINK_STATUS_TIMEOUT                = -12, /**< Operation timed out */
} glink_err_type;

/**
 * glink_link_state_type - transport-level link state.
 */
typedef enum {
    GLINK_LINK_STATE_UP,   /**< Link is online and ready */
    GLINK_LINK_STATE_DOWN, /**< Link is offline (SSR or not yet up) */
} glink_link_state_type;

/**
 * glink_channel_event_type - logical channel connection state events.
 *
 * Delivered to the notify_state callback registered in glink_open_config_type.
 */
typedef enum {
    GLINK_CONNECTED = 0,       /**< E2E channel fully open; data can flow */
    GLINK_LOCAL_DISCONNECTED,  /**< Local glink_close() sequence completed */
    GLINK_REMOTE_DISCONNECTED, /**< Remote side initiated disconnect */
} glink_channel_event_type;

/**
 * glink_handle_type - opaque handle representing an open GLink channel.
 *
 * Returned by glink_open() and passed to all subsequent channel operations.
 * Must not be modified or interpreted by the caller.
 */
typedef void *glink_handle_type;

/**
 * glink_link_handle_type - opaque handle for a link-state notification
 *                          registration.
 *
 * Returned via the handle field of glink_link_id_type after a successful
 * call to glink_register_link_state_cb().  Used to identify the registration
 * for deregistration.
 */
typedef struct glink_link_notif_data_type *glink_link_handle_type;

/**
 * glink_link_info_type - link-state notification payload.
 *
 * Passed to the link_notifier callback when a transport link changes state.
 *
 * @xport:      Name of the transport (e.g. "qmp"); may be NULL.
 * @remote_ss:  Name of the remote subsystem.
 * @link_state: New link state (GLINK_LINK_STATE_UP or GLINK_LINK_STATE_DOWN).
 */
typedef struct glink_link_info_t {
    const char           *xport;
    const char           *remote_ss;
    glink_link_state_type link_state;
} glink_link_info_type;

/*---------------------------------------------------------------------------
 * Callback type declarations
 *---------------------------------------------------------------------------*/

/**
 * glink_link_state_notif_cb - link-state change notification callback.
 *
 * Invoked when a monitored transport link changes state (UP or DOWN).
 * Registered via glink_register_link_state_cb().
 *
 * @param[in] link_info  Pointer to the link state information structure.
 * @param[in] priv       Caller's private data supplied at registration time.
 */
typedef void (*glink_link_state_notif_cb)(
    glink_link_info_type *link_info,
    void                 *priv);

/**
 * glink_rx_notification_cb - receive data notification callback.
 *
 * Invoked when data arrives on the channel.  The caller MUST call
 * glink_rx_done() after processing @ptr to release the mailbox slot.
 *
 * @param[in] handle      Channel handle.
 * @param[in] priv        Caller's private context from glink_open_config_type.
 * @param[in] pkt_priv    Per-packet private context (always NULL for QMP).
 * @param[in] ptr         Pointer to the received payload in the remote mailbox.
 * @param[in] size        Size of the received payload in bytes.
 * @param[in] intent_used Intent space consumed (equals size for QMP).
 */
typedef void (*glink_rx_notification_cb)(
    glink_handle_type  handle,
    const void        *priv,
    const void        *pkt_priv,
    const void        *ptr,
    size_t             size,
    size_t             intent_used);

/**
 * glink_buffer_provider_fn - buffer provider function for vector TX/RX.
 *
 * Returns a pointer to a contiguous region within a scatter-gather vector.
 *
 * @param[in]  iovec   Pointer to the vector buffer.
 * @param[in]  offset  Byte offset from the start of the vector.
 * @param[out] size    Size of the returned contiguous region in bytes.
 *
 * @return Pointer to the contiguous buffer region, or NULL on error.
 */
typedef void *(*glink_buffer_provider_fn)(
    void   *iovec,
    size_t  offset,
    size_t *size);

/**
 * glink_rxv_notification_cb - vector receive notification callback.
 *
 * Invoked when vector (scatter-gather) data arrives on the channel.
 * Optional; use notify_rx for simple contiguous-buffer receive.
 *
 * @param[in] handle      Channel handle.
 * @param[in] priv        Caller's private context.
 * @param[in] pkt_priv    Per-packet private context.
 * @param[in] iovec       Pointer to the received vector buffer.
 * @param[in] size        Total size of the received payload in bytes.
 * @param[in] intent_used Intent space consumed.
 * @param[in] vprovider   Buffer provider for virtual address space.
 * @param[in] pprovider   Buffer provider for physical address space.
 */
typedef void (*glink_rxv_notification_cb)(
    glink_handle_type        handle,
    const void              *priv,
    const void              *pkt_priv,
    void                    *iovec,
    size_t                   size,
    size_t                   intent_used,
    glink_buffer_provider_fn vprovider,
    glink_buffer_provider_fn pprovider);

/**
 * glink_tx_notification_cb - transmit-done notification callback.
 *
 * Invoked when a previously submitted TX buffer has been consumed by the
 * transport and the remote side has received it.
 *
 * @param[in] handle    Channel handle.
 * @param[in] priv      Caller's private context.
 * @param[in] pkt_priv  Per-packet private context supplied in glink_tx().
 * @param[in] ptr       Pointer to the transmitted data buffer.
 * @param[in] size      Size of the transmitted payload in bytes.
 */
typedef void (*glink_tx_notification_cb)(
    glink_handle_type  handle,
    const void        *priv,
    const void        *pkt_priv,
    const void        *ptr,
    size_t             size);

/**
 * glink_state_notification_cb - channel state change notification callback.
 *
 * Invoked when the channel transitions between GLINK_CONNECTED,
 * GLINK_LOCAL_DISCONNECTED, and GLINK_REMOTE_DISCONNECTED states.
 *
 * @param[in] handle  Channel handle.
 * @param[in] priv    Caller's private context.
 * @param[in] event   New channel state (see glink_channel_event_type).
 */
typedef void (*glink_state_notification_cb)(
    glink_handle_type        handle,
    const void              *priv,
    glink_channel_event_type  event);

/**
 * glink_notify_rx_intent_req_cb - remote rx intent request callback.
 *
 * Invoked when the remote side requests a receive intent of a given size.
 * The callback should return TRUE if the intent will be queued, FALSE to
 * deny the request.
 *
 * @param[in] handle    Channel handle.
 * @param[in] priv      Caller's private context.
 * @param[in] req_size  Requested intent size in bytes.
 *
 * @return TRUE to accept the request; FALSE to deny.
 */
typedef boolean (*glink_notify_rx_intent_req_cb)(
    glink_handle_type  handle,
    const void        *priv,
    size_t             req_size);

/**
 * glink_notify_rx_intent_cb - new rx intent arrival callback.
 *
 * Invoked when the remote side queues a new receive intent.
 *
 * @param[in] handle  Channel handle.
 * @param[in] priv    Caller's private context.
 * @param[in] size    Size of the new intent in bytes.
 */
typedef void (*glink_notify_rx_intent_cb)(
    glink_handle_type  handle,
    const void        *priv,
    size_t             size);

/**
 * glink_notify_rx_sigs_cb - control signal change notification callback.
 *
 * Invoked when the remote side alters its control signals (DTR/CTS/etc.).
 *
 * @param[in] handle  Channel handle.
 * @param[in] priv    Caller's private context.
 * @param[in] prev    Previous remote signal state (32-bit bitmask).
 * @param[in] curr    Current remote signal state (32-bit bitmask).
 */
typedef void (*glink_notify_rx_sigs_cb)(
    glink_handle_type  handle,
    const void        *priv,
    uint32             prev,
    uint32             curr);

/**
 * glink_notify_rx_abort_cb - rx intent abort notification callback.
 *
 * Invoked for every rx intent that was queued when the remote side or local
 * side closes the channel.
 *
 * @param[in] handle    Channel handle.
 * @param[in] priv      Caller's private context.
 * @param[in] pkt_priv  Per-packet private context associated with the intent.
 */
typedef void (*glink_notify_rx_abort_cb)(
    glink_handle_type  handle,
    const void        *priv,
    const void        *pkt_priv);

/**
 * glink_notify_tx_abort_cb - TX abort notification callback.
 *
 * Invoked when an in-progress transmit is aborted because the remote side
 * disconnected before the TX completed.
 *
 * @param[in] handle    Channel handle.
 * @param[in] priv      Caller's private context.
 * @param[in] pkt_priv  Per-packet private context from the aborted glink_tx().
 */
typedef void (*glink_notify_tx_abort_cb)(
    glink_handle_type  handle,
    const void        *priv,
    const void        *pkt_priv);

/*---------------------------------------------------------------------------
 * Aggregate type declarations
 *---------------------------------------------------------------------------*/

/**
 * glink_link_id_type - link notification descriptor.
 *
 * Describes the transport and subsystem to monitor and the callback to
 * invoke on state changes.  Must be initialized with
 * GLINK_LINK_ID_STRUCT_INIT() before use.
 *
 * @version:       Structure version; set by GLINK_LINK_ID_STRUCT_INIT().
 * @xport:         Transport name filter; NULL matches any transport.
 * @remote_ss:     Remote subsystem name filter; NULL matches any subsystem.
 * @link_notifier: Callback invoked on link state changes.
 * @handle:        Set by glink_register_link_state_cb() on success; opaque.
 * @options:       Reserved option flags; set to 0.
 */
typedef struct glink_link_id_t {
    unsigned int              version;
    const char               *xport;
    const char               *remote_ss;
    glink_link_state_notif_cb link_notifier;
    glink_link_handle_type    handle;
    unsigned int              options;
} glink_link_id_type;

/**
 * glink_open_config_type - channel open configuration.
 *
 * Passed to glink_open() to describe the channel to open and register all
 * notification callbacks.  remote_ss, name, and notify_state are mandatory;
 * all other fields are optional and may be set to NULL/0.
 *
 * @transport:           Transport name (e.g. "qmp"); NULL = any transport.
 * @remote_ss:           Remote subsystem name (mandatory).
 * @name:                Channel name; must match the remote side (mandatory).
 * @options:             Open option flags (GLINK_OPT_*).
 * @priv:                Caller's opaque context; passed back in all callbacks.
 * @notify_rx:           Callback for contiguous-buffer receive (optional if
 *                       notify_rxv is provided).
 * @notify_rxv:          Callback for vector receive (optional).
 * @notify_tx_done:      Callback when a TX buffer has been consumed (mandatory
 *                       if glink_tx() will be used).
 * @notify_state:        Callback for channel connect/disconnect events (mandatory).
 * @notify_rx_intent_req: Callback for remote intent requests (optional).
 * @notify_rx_intent:    Callback for new intent arrivals (optional).
 * @notify_rx_sigs:      Callback for control signal changes (optional).
 * @notify_rx_abort:     Callback for rx intent abort on channel close (optional).
 * @notify_tx_abort:     Callback for TX abort on remote disconnect (optional).
 */
typedef struct {
    const char                    *transport;
    const char                    *remote_ss;
    const char                    *name;
    unsigned int                   options;
    const void                    *priv;
    glink_rx_notification_cb       notify_rx;
    glink_rxv_notification_cb      notify_rxv;
    glink_tx_notification_cb       notify_tx_done;
    glink_state_notification_cb    notify_state;
    glink_notify_rx_intent_req_cb  notify_rx_intent_req;
    glink_notify_rx_intent_cb      notify_rx_intent;
    glink_notify_rx_sigs_cb        notify_rx_sigs;
    glink_notify_rx_abort_cb       notify_rx_abort;
    glink_notify_tx_abort_cb       notify_tx_abort;
} glink_open_config_type;

/*===========================================================================
                         PUBLIC API DECLARATIONS
===========================================================================*/

/**
 * glink_open() - Open a logical GLink channel to a remote subsystem.
 *
 * Locates the transport for cfg_ptr->remote_ss, acquires the channel lock,
 * calls the transport's ch_open callback, and registers all notification
 * callbacks in the channel context.
 *
 * @param[in]  cfg_ptr  Channel configuration.  remote_ss, name, and
 *                      notify_state are mandatory.
 * @param[out] handle   Set to the channel handle on success.
 *
 * @return GLINK_STATUS_SUCCESS on success, or a negative glink_err_type code.
 *
 * @sideeffects  Allocates channel resources and signals the remote side.
 */
glink_err_type glink_open(const glink_open_config_type *cfg_ptr,
                          glink_handle_type *handle);

/**
 * glink_close() - Close a previously opened GLink channel.
 *
 * Initiates the channel disconnect sequence.  The caller will receive a
 * GLINK_LOCAL_DISCONNECTED event via notify_state when the close completes.
 *
 * @param[in] handle  Channel handle returned by glink_open().
 *
 * @return GLINK_STATUS_SUCCESS on success, or a negative glink_err_type code.
 *
 * @sideeffects  Signals the remote side to close the channel.
 */
glink_err_type glink_close(glink_handle_type handle);

/**
 * glink_tx() - Transmit a packet over a GLink channel.
 *
 * Copies the payload into the local mailbox and signals the remote side.
 * The notify_tx_done callback is invoked when the remote side has consumed
 * the buffer.
 *
 * @param[in] handle    Channel handle returned by glink_open().
 * @param[in] pkt_priv  Per-packet opaque context; returned in notify_tx_done
 *                      and notify_tx_abort.
 * @param[in] data      Pointer to the payload buffer (must be 4-byte aligned).
 * @param[in] size      Payload size in bytes; must not exceed the local
 *                      mailbox size.
 * @param[in] options   TX option flags (GLINK_TX_NO_OPTIONS or
 *                      GLINK_TX_REQ_INTENT - unsupported).
 *
 * @return GLINK_STATUS_SUCCESS on success, or a negative glink_err_type code.
 *
 * @sideeffects  Causes the remote host to wake up and process the rx packet.
 */
glink_err_type glink_tx(glink_handle_type handle,
                        const void       *pkt_priv,
                        const void       *data,
                        size_t            size,
                        uint32            options);

/**
 * glink_queue_rx_intent() - Queue a receive intent for the channel.
 *
 * Not implemented for QMP transport; always returns
 * GLINK_STATUS_API_NOT_SUPPORTED.
 *
 * @param[in] handle    Channel handle.
 * @param[in] pkt_priv  Per-packet private context.
 * @param[in] size      Intent size in bytes.
 *
 * @return GLINK_STATUS_API_NOT_SUPPORTED.
 */
glink_err_type glink_queue_rx_intent(glink_handle_type handle,
                                     const void       *pkt_priv,
                                     size_t            size);

/**
 * glink_rx_done() - Return a received buffer to the transport layer.
 *
 * Must be called after the caller has finished processing data delivered
 * via the notify_rx callback.  Clears the remote mailbox slot so the
 * remote side can send the next message.
 *
 * @param[in] handle  Channel handle returned by glink_open().
 * @param[in] ptr     Pointer previously delivered in notify_rx.
 * @param[in] reuse   Ignored for QMP transport; pass TRUE or FALSE.
 *
 * @return GLINK_STATUS_SUCCESS on success, or a negative glink_err_type code.
 *
 * @sideeffects  Signals the remote side that the mailbox slot is free.
 */
glink_err_type glink_rx_done(glink_handle_type handle,
                             const void       *ptr,
                             boolean           reuse);

/**
 * glink_notify_clients() - Fire all registered link-state callbacks.
 *
 * Called internally by the transport layer when a link transitions to a new
 * state.  Iterates over all registered callbacks whose remote_ss matches the
 * transport's remote_ss and invokes each one.
 *
 * @param[in] ctx_ptr    Pointer to the transport interface (cast to void *).
 * @param[in] link_state New link state (GLINK_LINK_STATE_UP or
 *                       GLINK_LINK_STATE_DOWN).
 *
 * @sideeffects  Invokes registered link_notifier callbacks.
 */
void glink_notify_clients(void *ctx_ptr, glink_link_state_type link_state);

/**
 * glink_register_link_state_cb() - Register a link state change callback.
 *
 * Registers a callback to be invoked whenever the specified transport link
 * transitions between LINK_UP and LINK_DOWN.  If the link is already UP at
 * registration time, the callback is invoked immediately.
 *
 * The link_id structure must be initialized with GLINK_LINK_ID_STRUCT_INIT()
 * before calling this function.
 *
 * @param[in]  link_id  Pointer to the link notification descriptor.
 *                      link_id->version must equal GLINK_LINK_ID_VER.
 *                      link_id->link_notifier must not be NULL.
 * @param[in]  priv     Caller's opaque context; passed back in the callback.
 *
 * @return GLINK_STATUS_SUCCESS on success, or a negative glink_err_type code.
 *
 * @sideeffects  Adds the callback to the internal notification queue.
 *               May invoke the callback immediately if the link is already UP.
 */
glink_err_type glink_register_link_state_cb(glink_link_id_type *link_id,
                                            void               *priv);

/**
 * glink_link_state_poll() - Poll the current link state.
 *
 * Queries the current link state for the transport associated with the given
 * handle.  If the link is not yet UP and a poll function is registered,
 * drives the transport state machine before returning the state.
 *
 * Typically used when incoming interrupts are disabled.
 *
 * @param[in]  handle  Link handle returned via link_id->handle after a
 *                     successful glink_register_link_state_cb() call.
 * @param[out] state   Set to the current link state on success.
 *
 * @return GLINK_STATUS_SUCCESS on success, or a negative glink_err_type code.
 */
glink_err_type glink_link_state_poll(glink_link_handle_type  handle,
                                     glink_link_state_type  *state);

/**
 * glink_rx_poll() - Poll the transport for any new received data.
 *
 * Drives the transport ISR handler directly to process any pending state
 * machine transitions or received data without waiting for a hardware
 * interrupt.  Typically used when incoming interrupts are disabled.
 *
 * @param[in] handle  Channel handle returned by glink_open().
 *
 * @return GLINK_STATUS_SUCCESS on success, or a negative glink_err_type code.
 */
glink_err_type glink_rx_poll(glink_handle_type handle);

void glink_tfa_init(void);

#ifdef __cplusplus
}
#endif

#endif /* GLINK_H */
