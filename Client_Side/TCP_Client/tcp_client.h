#ifndef MAVLINK_TCP_H
#define MAVLINK_TCP_H

#include <pthread.h> // This uses POSIX Threads
#include <vector>
#include <string>
#include <arpa/inet.h>
#include <sys/time.h>
#include <unistd.h>
#include <sstream>

#include "../Struct/struct.h"
#include "../Translater/translater.h"

// ------------------------------------------------------------------------------
//   Global Buffer
// ------------------------------------------------------------------------------

extern Buffers buffers;

// ------------------------------------------------------------------------------
//   Prototypes
// ------------------------------------------------------------------------------


// helper functions
void* start_tcp_client_send_thread(void *args);
void* start_tcp_client_recv_thread(void *args);

// ----------------------------------------------------------------------------------
//   TCP Client Class
// ----------------------------------------------------------------------------------

class TCP_Client
{
public:
    //Client constructor
    TCP_Client(std::string target_ip, int target_port, Translater *translater_);
    //Server constructor
    TCP_Client(int local_port);
    ~TCP_Client();

    char send_status;
    char recv_status;

    static bool is_valid_ip(const char* ip);

    void start();
    void stop();

    void start_recv_thread();
    void start_send_thread();

    void handle_quit( int sig );
private:

    Translater *translater;

    bool time_to_exit;

    pthread_t send_tid;
    pthread_t recv_tid;

    void send_thread();
    void recv_thread();

    void send_server();
    void recv_server();

    pthread_mutex_t  lock;

    bool connect_client();
    void reconnect();
    bool is_server;
    bool connected;
    std::string _target_ip;
    int _target_port;
    int csock;
    int sock;
    struct sockaddr_in gcAddr;
};

#endif
