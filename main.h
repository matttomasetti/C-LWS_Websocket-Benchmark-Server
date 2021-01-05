#ifndef C_LWS_WEBSOCKET_SERVER_MAIN_H
#define C_LWS_WEBSOCKET_SERVER_MAIN_H

#include <libwebsockets.h>
#include <string.h>
#include <signal.h>
#include "server/server.c"

struct lws_context_creation_info info;
struct lws_context *context;
const char *p;
static int interrupted, port = 8080, options;
int n = 0, logs = LLL_USER | LLL_ERR | LLL_WARN | LLL_NOTICE;
/* for LLL_ verbosity above NOTICE to be built into lws,
 * lws must have been configured and built with
 * -DCMAKE_BUILD_TYPE=DEBUG instead of =RELEASE */
/* | LLL_INFO */ /* | LLL_PARSER */ /* | LLL_HEADER */
/* | LLL_EXT */ /* | LLL_CLIENT */ /* | LLL_LATENCY */
/* | LLL_DEBUG */;


//Array of structures listing supported protocols and a protocol-specific callback for each one.
// The list is ended with an entry that has a NULL callback pointer.
static struct lws_protocols protocols[] = {
        LWS_WEBSOCKET_SERVER,
        { NULL, NULL, 0, 0 } /* terminator */
};

/* pass pointers to shared vars to the protocol */
static const struct lws_protocol_vhost_options pvo_options = {
        NULL,
        NULL,
        "options",		/* pvo name */
        (void *)&options	/* pvo value */
};

static const struct lws_protocol_vhost_options pvo_interrupted = {
        &pvo_options,
        NULL,
        "interrupted",		/* pvo name */
        (void *)&interrupted	/* pvo value */
};

static const struct lws_protocol_vhost_options pvo = {
        NULL,				/* "next" pvo linked-list */
        &pvo_interrupted,		/* "child" pvo linked-list */
        "lws-websocket-server",	/* protocol name we belong to on this vhost */
        ""				/* ignored */
};

static const struct lws_extension extensions[] = {
        {
            "permessage-deflate",
            lws_extension_callback_pm_deflate,
            "permessage-deflate"
            "; client_no_context_takeover"
            "; client_max_window_bits"
        },
        { NULL, NULL, NULL /* terminator */ }
};

void setLogLevel(int argc, const char **argv);
void setServerOptions(int argc, const char **argv);

#endif //C_LWS_WEBSOCKET_SERVER_MAIN_H
