#include <string>
#include <vector>
#include <unordered_map>

#ifndef HTTP_REQUEST_HANDLER_HPP
#define HTTP_REQUEST_HANDLER_HPP

namespace rfss {

    struct HTTPResponse {
        int status_code {};
        std::string status_message {};
        std::string content_type = "text/plain";
        std::string body {};
        std::string location {};
        std::pair<std::string, std::string> cookies {};
        std::string generate_response() const;
        void set_JSON_content(const std::string& json_data);
    };

    struct HTTPRequest {
        std::string method   {};
        std::string URI      {};
        std::string version  {};
        std::vector<std::pair<std::string, std::string>> headers {};
        std::vector<std::pair<std::string, std::string>> cookies {};
        std::string body     {};
    };

    void handle_request(HTTPRequest& req, int client_socket);
    void sendNotFoundResponse(int client_socket);
    void serveStaticFile(const std::string& filePath, int client_socket);
    std::unordered_map<std::string, std::string> parse_parameters(std::string uri);


}

#endif