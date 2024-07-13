#include <iostream>
#include <iomanip>
#include <unordered_map>

#include "http.hpp"

#ifndef RFSS_CONTROLLER_HPP
#define RFSS_CONTRILLER_HPP

namespace rfss {

    // helpers
    void sendNotFoundResponse(int client_socket);
    void serveStaticFile(const std::string& filePath, int client_socket);
    std::unordered_map<std::string, std::string> parse_parameters(std::string uri);
    std::ostream& operator<<(std::ostream& os, const HTTPRequest& req);


    // controllers
    void handle_get_home(HTTPRequest& req, int client_socket);

}

#endif