#ifndef C_LWS_WEBSOCKET_SERVER_SERVER_H
#define C_LWS_WEBSOCKET_SERVER_SERVER_H


#include "serverp.h"

static int callback_websocket_server(struct lws *a_wsi, enum lws_callback_reasons a_reason,
                                     void *a_user, void *a_in, size_t a_len);

/**
 * Defines the above callback function as a protocol to be used
 * in the creation of the context websocket server
 */
#define LWS_WEBSOCKET_SERVER \
	{ \
		"lws-websocket-server", \
		callback_websocket_server, \
		sizeof(struct per_session_data__websocket_server), \
		1024, \
		0, NULL, 0 \
	}

#endif //C_LWS_WEBSOCKET_SERVER_SERVER_H
