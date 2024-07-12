#include <iostream>
#include <thread>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string>
#include <cstring>
#include <fcntl.h>
#include <sys/select.h>

#ifndef RFSS_SERVER_HPP
#define RFSS_SERVER_HPP

namespace rfss {
    class HTTP_Server {
    private:
        int server_socket {};
        int port{};
        sockaddr_in server_address {};

    public:
        HTTP_Server(int port);
        ~HTTP_Server();
        void start();
    };
}

#endif