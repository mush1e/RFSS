#include <iostream>
#include <string>
#include <ctime>
#include <unordered_map>
#include <random>
#include <mutex>

#ifndef RFSS_SESSION_MANAGER_HPP
#define RFSS_SESSION_MANAGER_HPP

namespace rfss {
    
    struct Session_Data {
        std::string user_id;
        time_t creation_time;
        time_t expiry_time;
    };

    class Session_Manager {
    private:
        std::unordered_map<std::string, Session_Data> sessions;
        const int session_duration = 3600;
        std::mutex session_mtx;
    public:
        Session_Manager() = default;
        static Session_Manager& get_instance();
        bool is_valid_session(const std::string& session_id);
        std::string create_session(const std::string& user_id);
        std::string generate_session_id() const;
        bool terminate_session(const std::string& session_id);
        std::string get_user_id(const std::string& session_id);
    };
}

#endif