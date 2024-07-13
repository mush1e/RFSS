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

    // ~~~~~~~~~~~~~~~~~~~~~~~ Helper Function (URL Decode) ~~~~~~~~~~~~~~~~~~~~~~~
    std::string url_decode(const std::string& str) {
        int i = 0;
        std::stringstream decoded;

        while (i < str.length()) {
            if (str[i] == '%') {
                if (i + 2 < str.length()) {
                    int hexValue;
                    std::istringstream(str.substr(i + 1, 2)) >> std::hex >> hexValue;
                    decoded << static_cast<char>(hexValue);
                    i += 3;
                } else {
                    // If '%' is at the end of the string, leave it unchanged
                    decoded << '%';
                    i++;
                }
            } else if (str[i] == '+') {
                decoded << ' ';
                i++;
            } else {
                decoded << str[i];
                i++;
            }
        }
        return decoded.str();
    }

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

        for (const auto& header : req.headers) {
            if (header.first == "Content-Length") {
                int content_length = std::stoi(header.second);
                if (content_length > 0) {
                    std::string body_content(content_length, '\0');
                    if (iss.read(&body_content[0], content_length)) {

                        // URL decoding for form data
                        if (req.headers[0].second == "application/x-www-form-urlencoded")
                            parse_form_data(body_content, req);
                        else
                            req.body = body_content;
                    }
                }
                break; 
            }
        }
    }


    // ~~~~~~~~~~~~~~~~~~~~~~~~~~ Recieve Request ~~~~~~~~~~~~~~~~~~~~~~~~~~
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
        handle_request(request, client_socket);
        close(client_socket);
    }

}