#ifndef C_LWS_WEBSOCKET_SERVER_SERVERP_H
#define C_LWS_WEBSOCKET_SERVER_SERVERP_H

#define RING_DEPTH 4096

#if !defined (LWS_PLUGIN_STATIC)
#include <libwebsockets.h>
#endif

#include <time.h>
#include <string.h>
#include <json-c/json.h>

struct lws *wsi;
enum lws_callback_reasons reason;
void *user;
void *in;
size_t len;
char buf[LWS_PRE + 64];                           // buffer of any size plus the LWS_PRE
char buffer[64];


struct per_session_data__websocket_server *pss;
struct vhd_websocket_server *vhd;

struct msg {
    void *payload; /* is malloc'd */
    char binary;
    char first;
    char final;
};


struct per_session_data__websocket_server {
    struct lws_ring *ring;
    uint32_t msglen;
    uint32_t tail;
    uint8_t completed:1;
    uint8_t flow_controlled:1;
    uint8_t write_consume_pending:1;
};


struct vhd_websocket_server {
    int *interrupted;
    int *options;
};

int server_initiate();
int connection_established();
int server_receive();
int connection_closed();
int getTimestamp();
const char *getEvent(int c);
int notify(int c);
static void websocket_server_destroy_message(void *_msg);

#endif //C_LWS_WEBSOCKET_SERVER_SERVERP_H
