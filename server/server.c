#include "server.h"

/**
 * Callback function which gets called for all server events
 * Here we set up just the few events that we need
 *
 * This is the beginning of the custom logic of the application
 *
 * @param wsi Opaque websocket instance pointer
 * @param reason The reason for the call
 * @param user Pointer to per-session user data allocated by library
 * @param in Pointer used for some callback reasons
 * @param len Length set for some callback reasons
 * @return whether the callback finished with an error
 */
static int callback_websocket_server(struct lws *a_wsi, enum lws_callback_reasons a_reason,
                             void *a_user, void *a_in, size_t a_len) {

    int return_value;

    wsi = a_wsi;
    reason = a_reason;
    user = a_user;
    in = a_in;
    len = a_len;

    pss = (struct per_session_data__websocket_server *)user;
    vhd = (struct vhd_websocket_server *) lws_protocol_vh_priv_get(lws_get_vhost(wsi),
                                     lws_get_protocol(wsi));


    switch (reason) {

        //One-time call per protocol
        case LWS_CALLBACK_PROTOCOL_INIT:
            return_value = server_initiate();
            break;

        //after the server completes a handshake with an incoming client
        case LWS_CALLBACK_ESTABLISHED:
            return_value = connection_established();
            break;
        //data has appeared for this server endpoint from a remote client, it can be found at *in and is len bytes long
        case LWS_CALLBACK_RECEIVE:
            return_value = server_receive();
            break;

        //when the websocket session ends
        case LWS_CALLBACK_CLOSED:
            return_value = connection_closed();
            break;

        //return for all other event
        default:
            return_value = 0;
    }

    return return_value;

}