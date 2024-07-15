#include "http_request_handler.hpp"

namespace rfss {
    // ~~~~~~~~~~~~~~~~~~~~~~~~~~ Router ~~~~~~~~~~~~~~~~~~~~~~~~~~
    auto handle_request(HTTPRequest& req, int client_socket) -> void {
        if (req.method == "GET") {
                 if (req.URI == "/")            handle_get_home(req, client_socket);
            else if (req.URI == "/register")    handle_get_register(req, client_socket);
            else if (req.URI == "/login")       handle_get_login(req, client_socket);
            else if (req.URI == "/is_auth")     handle_get_is_auth(req, client_socket);
            else if (req.URI == "/logout")      handle_get_logout(req, client_socket);

        }
        else if (req.method == "POST") {
                 if (req.URI == "/register")    handle_post_register(req, client_socket);
            else if (req.URI == "/login")       handle_post_login(req, client_socket);
            else if (req.URI == "/upload")      handle_file_upload(req, client_socket);
        } else {
            // serve not found static file
        }
    }

}