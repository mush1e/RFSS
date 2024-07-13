#include <regex>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <sys/socket.h>
#include <unordered_map>

#include "http.hpp"
#include "database.hpp"
#include "session_manager.hpp"

#ifndef RFSS_CONTROLLER_HPP
#define RFSS_CONTRILLER_HPP

namespace rfss {

    // helpers
    void sendNotFoundResponse(int client_socket);
    void serveStaticFile(const std::string& file_path, int client_socket);
    std::unordered_map<std::string, std::string> parse_parameters(std::string uri);
    std::ostream& operator<<(std::ostream& os, const HTTPRequest& req);
    std::string get_form_field(const std::string& body, const std::string& field_name);
    std::string url_decode(const std::string& str);


    // controllers
    void handle_get_home(HTTPRequest& req, int client_socket);
    void handle_get_register(HTTPRequest& req, int client_socket);
    void handle_post_register(HTTPRequest& req, int client_socket);
    void handle_get_login(HTTPRequest& req, int client_socket);
    void handle_post_login(HTTPRequest& req, int client_socket);
    void handle_get_is_auth(HTTPRequest& req, int client_socket);
    void handle_get_logout(HTTPRequest& req, int client_socket);
}

#endif