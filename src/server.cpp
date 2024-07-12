#include "server.hpp"
#include "http_parser.hpp"

namespace rfss {
    HTTP_Server::HTTP_Server(int port) : port(port) {

        if((this->server_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            std::cerr << "Error: Failed to create socket!\n";
            exit(1);
        }

        this->server_address.sin_addr.s_addr = INADDR_ANY;
        this->server_address.sin_family = AF_INET;
        this->server_address.sin_port = htons(this->port);

        if (bind(server_socket, (sockaddr*)&server_address, sizeof(server_address)) == -1) {
            std::cerr << "Error: Failed to bind socket to port!\n";
            exit(1);
        }

    }

    HTTP_Server::~HTTP_Server() {
        close(this->server_socket);
    }

    auto HTTP_Server::start() -> void {
        
        if(listen(server_socket, 10) < 0) {
            std::cerr << "Error: Failed to listen for connections!\n";
            exit(1);
        }

        std::cout << "Server Started! Listening on port: " << this->port << std::endl;
        
        for(;;) {
            int client_socket = accept(this->server_socket, nullptr, nullptr);

            if(client_socket < 0) {
                std::cerr << "Error: Failed to accept connection!\n";
                continue;
            }

            this->thread_pool.enqueue([client_socket] {
                handle_client(client_socket);
            });
        }
    }
}