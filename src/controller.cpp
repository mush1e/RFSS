#include "controller.hpp"

namespace rfss {

    // ~~~~~~~~~~~~~~~~~~~~~~~ Helper Function (Print Request) ~~~~~~~~~~~~~~~~~~~~~~~
    std::ostream& operator<<(std::ostream& os, const HTTPRequest& req) {
        os << "Method: " << req.method << "\n";
        os << "URI: " << req.URI << "\n";
        os << "Version: " << req.version << "\n";
        os << "Headers:\n";

        for (const auto& header : req.headers) 
            os << "  " << std::setw(20) << std::left << header.first << ": " << header.second << "\n";

        os << "Cookies:\n";

        for (const auto& cookie : req.cookies) 
            os << "  " << std::setw(20) << std::left << cookie.first << ": " << cookie.second << "\n";
        
        os << "Body: " << req.body << "\n";

        return os;
    }

    auto sendNotFoundResponse(int client_socket) -> void {

        HTTPResponse response {};
        response.status_code = 400;
        response.status_message = "Bad Request";
        std::string http_response = response.generate_response();
        send(client_socket, http_response.c_str(), http_response.length(), 0);

    }

    auto serveStaticFile(const std::string& file_path, int client_socket) -> void {
        std::ifstream file(file_path);

        if (file.good()) {
            std::stringstream buffer;
            buffer << file.rdbuf();
            std::string content = buffer.str();

            std::string response = "HTTP/1.1 200 OK\r\nContent-Length: "
                                    + std::to_string(content.length())
                                    + "\r\n\r\n"
                                    + content;

            send(client_socket, response.c_str(), response.length(), 0);
        }
        else
            sendNotFoundResponse(client_socket);
    }
    

    auto handle_get_home(HTTPRequest& req, int client_socket) -> void {
        serveStaticFile("./public/index.html", client_socket);
    }

    auto handle_get_register(HTTPRequest& req, int client_socket) -> void {
        serveStaticFile("./public/register.html", client_socket);
    }

    auto handle_post_register(HTTPRequest& req, int client_socket) -> void {
        HTTPResponse response;
        std::cout << req << std::endl;
        response.status_code = 200;
        response.status_message = "OK";
        std::string http_response = response.generate_response();
        send(client_socket, http_response.c_str(), http_response.length(), 0);
    }

}