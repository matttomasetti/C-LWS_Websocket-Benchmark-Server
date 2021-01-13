#include "serverp.h"


/**
 * Allocates a struct for the server when first initialized to be accessed while
 * the websocket server is running
 * @return 0 on success, -1 on error
 */
int server_initiate(){

    //Protocols often find it useful to allocate a per-vhost struct
    struct vhd_websocket_server *vhd = lws_protocol_vh_priv_zalloc(
            lws_get_vhost(wsi),
            lws_get_protocol(wsi),
            sizeof(struct vhd_websocket_server)
        );

    if (!vhd) {
        return -1;
    }


    //lws_protocol_vhost_options - linked list of per-vhost protocol name=value options
    //Returns NULL, or a pointer to the name pvo (per-host option) in the linked-list
    vhd->interrupted = (int *)lws_pvo_search(
            (const struct lws_protocol_vhost_options *)in,
            "interrupted"
        )->value;
    vhd->options = (int *)lws_pvo_search(
            (const struct lws_protocol_vhost_options *)in,
            "options"
        )->value;

    return 0;
}


/**
 * Function to be called when a connection is established to a client.
 * Sends the new client the timestamp of when the server established the
 * connection
 * @return status of the message being sent to the connected client.
 *  0 on success, 1 on error
 */
int connection_established(){

    lwsl_warn("LWS_CALLBACK_ESTABLISHED\n");

    //send client the initial timestamp
    return notify(0);
}


/**
 * Function to be called when the server receives an incoming message
 * from a client.
 * Incoming message should me a JSON string with property "c" representing
 * the current message count.
 * Responds to the client with the timestamp the server received the message
 * @return 0
 */
int server_receive(){
    struct msg amsg;
    int n;

    lwsl_user("LWS_CALLBACK_RECEIVE");

    // get information on current frame of the
    // incoming message
    amsg.first = lws_is_first_fragment(wsi);
    amsg.final = lws_is_final_fragment(wsi);
    amsg.binary = lws_frame_is_binary(wsi);

    // get how many elements can fit in the free space
    n = (int)lws_ring_get_count_free_elements(pss->ring);
    // return if no room is left in the ring buffer
    if (!n) {
        lwsl_user("dropping!\n");
        return 0;
    }

    // determine the length of the incoming message
    if (amsg.final)
        pss->msglen = 0;
    else
        pss->msglen += len;

    // allocate memory for the incoming message
    amsg.payload = malloc(LWS_PRE + len);
    if (!amsg.payload) {
        lwsl_user("OOM: dropping\n");
        return 0;
    }

    // copy the incoming message into the allocated payload buffer
    memcpy((char *)amsg.payload + LWS_PRE, in, len);

    // Attempts to insert as many of the elements at src as possible, up to the maximum max_count.
    // Returns the number of elements actually inserted.
    // Is no elements inserted, free the incoming message and return
    if (!lws_ring_insert(pss->ring, &amsg, 1)) {
        websocket_server_destroy_message(&amsg);
        lwsl_user("dropping!\n");
        return 0;
    }

    struct json_object *parsed_json;
    struct json_object *count;

    //parse json from the incoming message
    parsed_json = json_tokener_parse((amsg.payload) + LWS_PRE);
    //grab the "c" property from the incoming JSON message
    json_object_object_get_ex(parsed_json, "c", &count);

    lws_callback_on_writable(wsi);

    //send client the timestamp
    notify(json_object_get_int(count));

    if (n < 3 && !pss->flow_controlled) {
        pss->flow_controlled = 1;
        lws_rx_flow_control(wsi, 0);
    }

    if ((*vhd->options & 1) && amsg.final)
        pss->completed = 1;

    return 0;
}


/**
 * Function to be called when a client disconnects from the server
 * Simply frees up the ringbuffer, and checks if the server should
 * be terminated
 * @return 0
 */
int connection_closed(){
    lwsl_user("LWS_CALLBACK_CLOSED\n");

    //Destroys the ringbuffer allocation
    lws_ring_destroy(pss->ring);

    // if vhd has option set to true (-o was declared in the command line)
    // terminate the application
    if (*vhd->options & 1) {
        if (!*vhd->interrupted)
            *vhd->interrupted = 1 + pss->completed;
        lws_cancel_service(lws_get_context(wsi));
    }

    return 0;
}


/**
 * Gets the current unix timestamp of the server
 * @return current unix timestamp
 */
int getTimestamp(){

    //get current unix timestamp
    return (int)time(NULL);
}


/**
 * Constructs a JSON string containing the message count and the current timestamp
 * @param c the count of the message
 * @return a JSON string of the message count and the current timestamp
 */
const char *getEvent(int c){

    //create a new json object containing a and the current timestamp
    struct json_object *event;
    event = json_object_new_object();
    json_object_object_add(event, "c", json_object_new_int(c));
    json_object_object_add(event, "ts", json_object_new_int(getTimestamp()));

    //return the json object as a string
    return json_object_to_json_string(event);
}


/**
 * Sends the client a JSON string containing the message count and the current timestamp
 * @param c the count of the message
 * @return 0 on success, 1 on error
 */
int notify(int c){

    if (pss->write_consume_pending) {
        // perform the deferred fifo consume
        lws_ring_consume_single_tail(pss->ring, &pss->tail, 1);
        pss->write_consume_pending = 0;
    }

    snprintf(buffer, sizeof(buffer), getEvent(c));

    // use their stncpy to null-terminate
    // 1 is add to the strlen of the buffer as 1 character always seemed
    // to get cut off without it
    lws_strncpy(&buf[LWS_PRE], buffer, strlen(buffer)+1);

    // write from your buffer after LWS_PRE bytes.
    int m = lws_write(wsi, (unsigned char *) &buf[LWS_PRE], strlen(buffer), LWS_WRITE_TEXT);

    if (m < strlen(buffer)) {
        lwsl_err("ERROR %d writing to ws socket\n", m);
        return -1;
    }

    //creates the ringbuffer and allocates the storage.
    //Returns the new lws_ring *, or NULL if the allocation failed.
    pss->ring = lws_ring_create(sizeof(struct msg), RING_DEPTH, websocket_server_destroy_message);
    if (!pss->ring)
        return 1;
    pss->tail = 0;

    /*
     * Workaround deferred deflate in pmd extension by only
     * consuming the fifo entry when we are certain it has been
     * fully deflated at the next WRITABLE callback.  You only need
     * this if you're using pmd.
     */
    pss->write_consume_pending = 1;
    lws_callback_on_writable(wsi);

    // Enable socket servicing for received packets.
    if (pss->flow_controlled &&
        (int)lws_ring_get_count_free_elements(pss->ring) > RING_DEPTH - 5) {
        lws_rx_flow_control(wsi, 1);
        pss->flow_controlled = 0;
    }

    return 0;
}


/**
 * Frees up memory allocated for the incoming message's payload
 * @param _msg pointer to the incoming message
 */
static void websocket_server_destroy_message(void *_msg) {
    struct msg *msg = _msg;

    //free the memory allocated for an incoming message's payload
    free(msg->payload);
    msg->payload = NULL;
}