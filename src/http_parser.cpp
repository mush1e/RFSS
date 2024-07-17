#include "http_parser.hpp"

namespace rfss {

    const int BUFFER_SIZE = 4014;

    // ~~~~~~~~~~~~~~~~~~~~~~~~~~ Parse Form Data ~~~~~~~~~~~~~~~~~~~~~~~~~~
    auto parse_form_data(const std::string& form_data, HTTPRequest& req) -> void {
        std::istringstream iss(form_data);
        std::string pair;
        while (std::getline(iss, pair, '&')) {
            size_t pos = pair.find('=');
            if (pos != std::string::npos) {
                std::string key = pair.substr(0, pos);
                std::string value = pair.substr(pos + 1);

                // Decode URL-encoded key and value
                key = url_decode(key);
                value = url_decode(value);
                std::cout << key << ": " << value << std::endl;
                req.body += key + ": " + value + "\n"; // Assuming you want to store key-value pairs in the body
            }
        }
    }

    // ~~~~~~~~~~~~~~~~~~~~~~~~~~ Parse Request ~~~~~~~~~~~~~~~~~~~~~~~~~~
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
                } else {
                    // Add other headers
                    req.headers.emplace_back(key, value);
                }
            }
        }
        for (const auto& header : req.headers) {
            if (header.first == "Content-Length") {
                int content_length = std::stoi(header.second);
                if (content_length > 0) {
                    std::string body_content(content_length, '\0');
                    if (iss.read(&body_content[0], content_length)) {
                        
                        auto content_type_header = std::find_if(req.headers.begin(), req.headers.end(), 
                            [](const std::pair<std::string, std::string>& header) {
                                return header.first == "Content-Type";
                            });

                        if (content_type_header != req.headers.end()) 

                            if (content_type_header->second.find("application/x-www-form-urlencoded") != std::string::npos) 
                                parse_form_data(body_content, req);

                            else if (content_type_header->second.find("multipart/form-data") != std::string::npos) 
                                std::cout << "Encountered form data\n";

                            else 
                                req.body = body_content;
                            
                    } else 
                        std::cerr << "Could not read body content" << std::endl;
                }
                break;
            }
        }
    }

    bool ends_with(const std::string &str, const std::string &suffix) {
        return str.size() >= suffix.size() && str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
    }
    // ~~~~~~~~~~~~~~~~~~~~~~~~~~ Recieve Request ~~~~~~~~~~~~~~~~~~~~~~~~~~
    void handle_client(int client_socket) {
        char buffer[BUFFER_SIZE];
        std::string http_request_string;
        int bytes_read = 0;

        do {
            bytes_read = recv(client_socket, buffer, BUFFER_SIZE, 0);
            if (bytes_read < 0) {
                std::cerr << "Error: Client disconnected from server or no data received!\n";
                close(client_socket);
                return;
            }
            if (bytes_read == 0)
                break;

            http_request_string.append(buffer, bytes_read);

            if (ends_with(http_request_string, "\r\n\r\n"))
                break;

        } while (bytes_read > 0);


        HTTPRequest request;
        parse_request(request, http_request_string);
        handle_request(request, client_socket);
        close(client_socket);
}

}