#include "session_manager.hpp"

namespace rfss {
    
    auto Session_Manager::get_instance() -> Session_Manager& {
        static Session_Manager instance;
        return instance;
    }

    auto Session_Manager::generate_session_id() const -> std::string {
        const std::string char_list = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890";
        const size_t session_id_length = 32; 
        std::string session_id;

        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, char_list.length() - 1);

        for (auto i = 0u; i < session_id_length; ++i) 
            session_id += char_list[dis(gen)];

        return session_id;
    }

    auto Session_Manager::create_session(const std::string& user_id) -> std::string {
        std::lock_guard<std::mutex> lock(this->session_mtx);
        std::string session_id = generate_session_id();

        while (this->sessions.find(session_id) != this->sessions.end())
            session_id = generate_session_id();

        Session_Data session_data = {user_id, time(nullptr), time(nullptr) + this->session_duration};   
        this->sessions[session_id] = session_data;
        return session_id;
    }

    auto Session_Manager::is_valid_session(const std::string& session_id) -> bool {
        std::lock_guard<std::mutex> lock(this->session_mtx);
        for(auto session: sessions)
            if (session.first.compare(session_id)) {
                time_t current_time = time(nullptr);
                if (current_time < session.second.expiry_time) 
                    return true; 
                else 
                    sessions.erase(session.first); 
            }
        return false; 
    }

    auto Session_Manager::terminate_session(const std::string& session_id) -> bool {
        std::lock_guard<std::mutex> lock(this->session_mtx);
        for(auto session: sessions)
        if (session.first.compare(session_id)) {
            sessions.erase(session.first); 
            return true; 
        }
        return false;
    }

    auto Session_Manager::get_user_id(const std::string& session_id) const -> std::string {
        for (auto session : sessions) 
            if (session.first.compare(session_id))
                return session.second.user_id;
        return "";
    }
}