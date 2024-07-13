#include "http_request_handler.hpp"

namespace rfss {
    // ~~~~~~~~~~~~~~~~~~~~~~~~~~ Router ~~~~~~~~~~~~~~~~~~~~~~~~~~
    auto handle_request(HTTPRequest& req, int client_socket) -> void {
        if (req.method == "GET") {
            if (req.URI == "/")     handle_get_home(req, client_socket);


        } else {
            // serve not found static file
        }
    }

}