#include "main.h"

#define LWS_PLUGIN_STATIC

/**
 * Signal handler detecting an interrupt signal from inside the server as to whether
 * the server should be interrupted
 */
void sigint_handler()
{
    interrupted = 1;
}

/**
 * Sets up the application environment as well as the websocket server.
 *
 * This is a relatively generic set up for LWS.
 * Custom logic is contained in the "server" files
 *
 * @param argc number of command line arguments
 * @param argv array of command line arguments
 * @return whether the websocket server terminated without error
 */
int main(int argc, const char **argv)
{
    //allows the server to be terminated by using
    //int raise(int sig);
    signal(SIGINT, sigint_handler);

    //set console output level
    setLogLevel(argc, argv);

    lwsl_user("LWS websocket server + permessage-deflate + multifragment bulk message\n");
    lwsl_user("server [-n (no exts)] [-p port] [-o (once)]\n");

    setServerOptions(argc, argv);

    // Create the websocket handler
    // Returns a struct lws_context pointer that represents this server.
    context = lws_create_context(&info);
    if (!context) {
        lwsl_err("lws init failed\n");
        return 1;
    }


    int count = 0;
    while (n >= 0 && !interrupted)
        // Accept new connections to our context's server and
        // call the receive callback for incoming frame data received by
        // server.
        n = lws_service(context, 0);

    // closes any active connections and then frees the context
    lws_context_destroy(context);

    lwsl_user("Completed %s\n", interrupted == 2 ? "OK" : "failed");

    return interrupted != 2;
}


/**
 * Sets the log level of the application using LWS' built in setter
 * @param argc number of command line arguments
 * @param argv array of command line arguments
 */
void setLogLevel(int argc, const char **argv){
    //check command line for if log level should be other than the applications default
    if ((p = lws_cmdline_option(argc, argv, "-d"))) {
        logs = atoi(p);
    }

    //set log level through LWS' setter function
    lws_set_log_level(logs, NULL);
}


/**
 * initializes the lws_context_creation_info struct which specifies needed information to
 * start the websocket server
 * @param argc
 * @param argv
 */
void setServerOptions(int argc, const char **argv) {
    //check command line for if port should be other than the applications default
    if ((p = lws_cmdline_option(argc, argv, "-p")))
        port = atoi(p);

    //check command line if the application should terminate after a single connection
    if (lws_cmdline_option(argc, argv, "-o"))
        options |= 1;

    memset(&info, 0, sizeof info);      //otherwise uninitialized garbage
    info.port = port;                   //port to listen on
    info.protocols = protocols;         //struct containing the defined callback function
    info.pvo = &pvo;                    //linked-list of per-vhost settings
    if (!lws_cmdline_option(argc, argv, "-n"))
        //info.extensions = extensions;   //tells the server to send a Sec-WebSocket-Extensions header with
                                        // per-message deflate
    info.pt_serv_buf_size = 32 * 1024;  //max size of a chunk that can be sent at once
    info.options = LWS_SERVER_OPTION_VALIDATE_UTF8 |
                   LWS_SERVER_OPTION_HTTP_HEADERS_SECURITY_BEST_PRACTICES_ENFORCE;
}