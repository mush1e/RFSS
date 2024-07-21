#include "http_parser.hpp"

namespace rfss {
    const size_t BUFFER_SIZE = 90000;
     

    // ~~~~~~~~~~~~~~~~~~~~~~~~~~ Parse Form Data ~~~~~~~~~~~~~~~~~~~~~~~~~~
    auto parse_form_data(const std::string& form_data, HTTPRequest& req) -> void {
        std::cout << form_data << std::endl;
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
                req.body.append(key + ": " + value + "\n");
            }
        }
    }

    std::string trim(const std::string& str) {
        // Find the first non-whitespace character
        auto start = str.find_first_not_of(" \t\n\r\f\v");
        if (start == std::string::npos) {
            return ""; // All whitespace
        }

        // Find the last non-whitespace character
        auto end = str.find_last_not_of(" \t\n\r\f\v");

        // Return the substring from start to end
        return str.substr(start, end - start + 1);
    }

    // ~~~~~~~~~~~~~~~~~~~~~~~~~~ Parse Request ~~~~~~~~~~~~~~~~~~~~~~~~~~
    auto parse_request(HTTPRequest& req, const std::string& req_str) -> void {
        std::istringstream iss(req_str);
        std::string line;

        std::getline(iss, line);
        std::istringstream line_stream(line);
        line_stream >> req.method >> req.URI >> req.version;
        req.method = trim(req.method);
        req.URI = trim(req.URI);
        req.version = trim(req.version);

        while (std::getline(iss, line) && line != "\r") {
            if (line.back() == '\r') {
                line.pop_back(); 
            }
            size_t pos = line.find(':');
            if (pos != std::string::npos) {
                std::string key = line.substr(0, pos);
                std::string value = line.substr(pos + 1);

                key.erase(0, key.find_first_not_of(" \t"));
                key.erase(key.find_last_not_of(" \t") + 1);
                value.erase(0, value.find_first_not_of(" \t"));
                value.erase(value.find_last_not_of(" \t") + 1);

                if (key == "Cookie") {
                    std::istringstream cookie_stream(value);
                    std::string cookie_pair;
                    while (std::getline(cookie_stream, cookie_pair, ';')) {
                        size_t eq_pos = cookie_pair.find('=');
                        if (eq_pos != std::string::npos) {
                            std::string cookie_name = trim(cookie_pair.substr(0, eq_pos));
                            std::string cookie_value = trim(cookie_pair.substr(eq_pos + 1));
                            req.cookies.emplace_back(cookie_name, cookie_value);
                        }
                    }
                } else {
                    req.headers.emplace_back(key, value);
                }
            }
        }

        for (const auto& header : req.headers) {
            if (header.first == "Content-Length") {
                int content_length = std::stoi(header.second);
                if (content_length > 0) {
                    std::string body_content(content_length, '\0');
                    iss.read(&body_content[0], content_length);

                    bool is_form_data = false;
                    bool is_multipart_data = false;
                    for (const auto& header : req.headers) {
                        if (header.first == "Content-Type") {
                            if (header.second.find("application/x-www-form-urlencoded") != std::string::npos) {
                                is_form_data = true;
                            } else if (header.second.find("multipart/form-data") != std::string::npos) {
                                is_multipart_data = true;
                            }
                        }
                    }

                    if (is_form_data) {
                        parse_form_data(body_content, req);
                    } else {
                        req.body = body_content;
                    }
                }
                break; 
            }
        }
    }



    // ~~~~~~~~~~~~~~~~~~~~~~~~~~ Recieve Request ~~~~~~~~~~~~~~~~~~~~~~~~~~
    void handle_client(int client_socket) {
        char buffer[BUFFER_SIZE];
        size_t body_size = 0;
        HTTPRequest request;
        std::string http_request_string;


        size_t bytes_read = recv(client_socket, buffer, BUFFER_SIZE, 0);
        if (bytes_read < 0) {
            std::cerr << "Error: Client disconnected from server or no data received!\n";
            close(client_socket);
            return;
        }
        http_request_string.append(buffer,bytes_read);
        parse_request(request, http_request_string);
        handle_request(request, client_socket);
        close(client_socket);
    }

}