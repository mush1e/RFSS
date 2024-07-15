#include "controller.hpp"

namespace rfss {

    auto send_bad_request = [](int client_socket) {
        HTTPResponse response;
        std::string http_response;
        response.status_code = 400;
        response.status_message = "Bad Request";
        http_response = response.generate_response();
        send(client_socket, http_response.c_str(), http_response.length(), 0);
    };

    auto send_internal_server_error = [](int client_socket) {
        HTTPResponse response;
        std::string http_response;
        response.status_code = 500;
        response.status_message = "Internal Server Error";
        http_response = response.generate_response();
        send(client_socket, http_response.c_str(), http_response.length(), 0);
    };

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

    // ~~~~~~~~~~~~~~~~~~~~~~~ Helper Function (File extention) ~~~~~~~~~~~~~~~~~~~~~~~
    auto get_file_extention(const std::string& filename) -> std::string {
        size_t dot_pos = filename.find_first_of('.');
        if (dot_pos == std::string::npos)
            return "";
        return filename.substr(dot_pos + 1);
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
    
    auto get_form_field(const std::string& body, const std::string& field_name) -> std::string {
        std::string field_value;
        size_t pos = body.find(field_name + "=");
        if (pos != std::string::npos) {
            pos += field_name.length() + 1;
            size_t end_pos = body.find("&", pos);
            end_pos = (end_pos == std::string::npos) ? body.length() : end_pos;
            field_value = body.substr(pos, end_pos - pos);
        }
        return url_decode(field_value);
    }


    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ controllers ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    auto handle_get_home(HTTPRequest& req, int client_socket) -> void {
        serveStaticFile("./public/index.html", client_socket);
    }

    auto handle_get_register(HTTPRequest& req, int client_socket) -> void {
        serveStaticFile("./public/register.html", client_socket);
    }

    auto handle_post_register(HTTPRequest& req, int client_socket) -> void {
        std::string http_response;
        HTTPResponse response;

        Database& db = Database::get_instance();

        std::string username = get_form_field(req.body, "username");
        std::string password = get_form_field(req.body, "password");
        std::string confirm_password = get_form_field(req.body, "confirm_password");
        std::regex passwordRegex("^(?=.*\\d)(?=.*[a-z])(?=.*[A-Z]).{6,}$");

        if (username.empty() || password.empty() || confirm_password.empty()) {
            send_bad_request(client_socket);
            std::cerr << "Error: Empty Fields!\n";
            return;
        }

        if (!std::regex_match(password, passwordRegex)) {
            send_bad_request(client_socket);
            std::cerr << "Error: Invalid Password!\n";
            return;
        }
 
        if (password != confirm_password) {
            send_bad_request(client_socket);
            std::cerr << "Error: Password mismatch!\n";
            return;
        }

        if (db.username_exists(username)) {
            response.status_code = 409;
            response.status_message = "Conflict";
            http_response = response.generate_response();
            send(client_socket, http_response.c_str(), http_response.length(), 0);
            return;
        }

        if (!db.insert_user(username, password)) {
            std::cerr << "Error: Failed to insert user into database" << std::endl;
            send_internal_server_error(client_socket);
            return;
        }

        response.status_code = 200;
        response.status_message = "OK";
        http_response = response.generate_response();
        send(client_socket, http_response.c_str(), http_response.length(), 0);
    }

    auto handle_get_login(HTTPRequest& req, int client_socket) -> void {
        serveStaticFile("./public/login.html", client_socket);
    }

    auto handle_post_login(HTTPRequest& req, int client_socket) -> void {
        HTTPResponse response {};
        std::string http_response {};

        std::string username = get_form_field(req.body, "username");
        std::string password = get_form_field(req.body, "password");

        Database& db = Database::get_instance();

        if (username.empty() || password.empty()) {
            send_bad_request(client_socket);
            std::cerr << "Error: Empty Fields!\n";
            return;
        }

        if (!db.login(username, password)) {
            // If the credentials don't match, send unauthorized response
            response.status_code = 401;
            response.status_message = "Unauthorized";
            http_response = response.generate_response();
            send(client_socket, http_response.c_str(), http_response.length(), 0);
            return;
        }

        Session_Manager& session_manager = Session_Manager::get_instance();
        std::string session_id = session_manager.create_session(username);

        response.status_code = 200;
        response.status_message = "OK";
        response.cookies = {"session_id", session_id};

        std::string success_response = response.generate_response();
        send(client_socket, success_response.c_str(), success_response.length(), 0);
    }

    auto handle_get_is_auth(HTTPRequest& req, int client_socket) -> void {
        std::string http_response;
        HTTPResponse response;

        Session_Manager& session_manager = Session_Manager::get_instance();

        auto it = std::find_if(req.cookies.begin(), req.cookies.end(), 
                        [](const std::pair<std::string, std::string>& cookie) {
                            return cookie.first == "session_id";
                        });

        if (it != req.cookies.end() && session_manager.is_valid_session(it->second)) {
            response.status_code = 200;
            response.status_message = "OK";
            response.body = R"(
                                <li class="nav-item">
                                    <a class="nav-link" href="/logout">Logout</a>
                                </li>
                              )";
        } else {
            response.status_code = 200;
            response.status_message = "Forbidden";
            response.body = R"(
                                <li class="nav-item">
                                    <a class="nav-link" href="/register">Register</a>
                                </li>
                                <li class="nav-item">
                                    <a class="nav-link" href="/login">Login</a>
                                </li>
                              )";
        }
        http_response = response.generate_response();
        send(client_socket, http_response.c_str(), http_response.length(), 0);
    }

    auto handle_get_logout(HTTPRequest& req, int client_socket) -> void {
        std::string http_response;
        HTTPResponse response; 

        Session_Manager& session_manager = Session_Manager::get_instance();

        auto it = std::find_if(req.cookies.begin(), req.cookies.end(), 
                        [] (const std::pair<std::string, std::string>& cookie) {
                            return cookie.first == "session_id";
                        });
        
        if (it != req.cookies.end()) {
            session_manager.terminate_session(it->second);
        }

        response.status_code = 302;
        response.status_message = "FOUND";
        response.location = "/";
        http_response = response.generate_response();

        send(client_socket, http_response.c_str(), http_response.length(), 0);
    }

    auto handle_file_upload(HTTPRequest& req, int client_socket) -> void {
        std::string http_response;
        HTTPResponse response;
        response.status_code = 200;
        response.status_message = "OK";
        http_response = response.generate_response();

        send(client_socket, http_response.c_str(), http_response.length(), 0);
    }
}