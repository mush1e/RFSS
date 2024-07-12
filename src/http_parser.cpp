#include "http_parser.hpp"

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

    // ~~~~~~~~~~~~~~~~~~~~~~~ Parse Request ~~~~~~~~~~~~~~~~~~~~~~~
    auto parse_request(HTTPRequest& req, const std::string& req_str) -> void {
        std::istringstream iss(req_str);
        std::string line;

        std::getline(iss, line);
        std::istringstream line_stream(line);
        line_stream >> req.method >> req.URI >> req.version;

        while (std::getline(iss, line) && line != "\r") {
            size_t pos = line.find(':');
            if (pos != std::string::npos) {
                std::string key = line.substr(0, pos);
                std::string value = line.substr(pos + 1);

                // Trim leading and trailing spaces from key and value
                key.erase(0, key.find_first_not_of(" \t"));
                key.erase(key.find_last_not_of(" \t") + 1);
                value.erase(0, value.find_first_not_of(" \t"));
                value.erase(value.find_last_not_of(" \t") + 1);
                req.headers.emplace_back(key, value);
                if (key == "Cookie") {
                    // Parse cookies
                    std::istringstream cookie_stream(value);
                    std::string cookie_pair;
                    while (std::getline(cookie_stream, cookie_pair, ';')) {
                        size_t eq_pos = cookie_pair.find('=');
                        if (eq_pos != std::string::npos) {
                            std::string cookie_name = cookie_pair.substr(0, eq_pos);
                            std::string cookie_value = cookie_pair.substr(eq_pos + 1);
                            req.cookies.emplace_back(cookie_name, cookie_value);
                        }
                    }
                } else 
                    req.headers.emplace_back(key, value); 
            }
        }
    }


    // ~~~~~~~~~~~~~~~~~~~~~~~ Recieve Request ~~~~~~~~~~~~~~~~~~~~~~~
    auto handle_client(int client_socket) -> void {
        HTTPRequest request {};
        char BUFFER[8192];

        ssize_t bytes_read = recv(client_socket, BUFFER, sizeof(BUFFER), 0);
        
        if (bytes_read <= 0) {
            std::cerr << "Error: Client disconnected from server!\n";
            close(client_socket);
            return;
        }

        std::string http_request_string(BUFFER, bytes_read);
        parse_request(request, http_request_string);
        std::cout << request << std::endl;
    }

}