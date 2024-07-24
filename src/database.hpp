#include <string>
#include <random>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <ctime>
#include "sqlite3.h"

#ifndef RFSS_DATABASE_HPP
#define RFSS_DATABASE_HPP

namespace rfss {

    struct File_Data {
        std::string file_name;
        std::string file_path;
        std::string author;
        size_t file_size;
        time_t creation_time;
    };


    class Database {

    private:
        sqlite3* db;
    
    public:
        Database();
        ~Database();
        static Database& get_instance();
        void create_tables();
        bool execute_query(const char* query);
        void sanitize_input(std::string& input) const;
        sqlite3* get_db_handle() const;

        bool username_exists(const std::string& username);
        bool login(const std::string& username, const std::string& password);
        bool insert_user(std::string& username, std::string& password);
        bool insert_file(File_Data& file);
        std::string get_user(const std::string& username);
    };

    std::string generate_salt();
    std::string hash_password(const std::string& password);

}

#endif