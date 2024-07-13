#include "database.hpp"

namespace rfss {

    auto Database::get_instance() -> Database& {
        static Database instance;
        return instance;
    }

    auto generate_salt() -> std::string {
        const std::string char_list = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890";
        const size_t salt_length = 16; 
        std::string salt;
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, char_list.length() - 1);

        for (auto i = 0u; i < salt_length; ++i) 
            salt += char_list[dis(gen)];

        return salt;
    }

    auto hash_password(const std::string& password) -> std::string {
        std::hash<std::string> hash_fn;
        return std::to_string(hash_fn(password));
    }

    auto Database::login(const std::string& username_, const std::string& password_) -> bool {
        std::string username = username_;
        std::string password = password_;

        sanitize_input(username);
        sanitize_input(password);

        std::string query = "SELECT password_salt, password_hash FROM users WHERE username = '" + username + "'";
        sqlite3_stmt* stmt;
        
        if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, NULL) != SQLITE_OK) {
            std::cerr << "Error preparing SQL statement" << std::endl;
            return false;
        }

        std::string stored_salt;
        std::string stored_hash;
        
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            stored_salt = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
            stored_hash = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        }

        sqlite3_finalize(stmt);

        // Hash the provided password with the retrieved salt and compare it with the stored hash
        std::string salted_password = stored_salt + password;
        std::string hashed_password = hash_password(salted_password);

        return hashed_password == stored_hash;
    } 

    auto Database::username_exists(const std::string& username) -> bool {
        std::string sanitized_username = username;
        sanitize_input(sanitized_username);
        std::string query = "SELECT COUNT(*) FROM users WHERE username = '" + sanitized_username + "'";
        sqlite3_stmt* stmt;
        if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, NULL) != SQLITE_OK) {
            std::cerr << "Error preparing SQL statement" << std::endl;
            return false;
        }
        int count = 0;
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            count = sqlite3_column_int(stmt, 0);
        }
        sqlite3_finalize(stmt);
        return count > 0;
    }

    auto Database::insert_user(std::string& username_, std::string& password_) -> bool {

        std::string username = username_;
        std::string password = password_;
        sanitize_input(username);
        sanitize_input(password);
        
        std::string salt = generate_salt();
        std::string salted_password = salt + password;
        std::string hashed_password = hash_password(salted_password);


        std::string query = "INSERT INTO users (username, password_hash, password_salt) VALUES ('" 
                                + username + "', '" 
                                + hashed_password + "', '" 
                                + salt + "')";

        return execute_query(query.c_str());
    }

    auto Database::sanitize_input(std::string& input) const -> void {
        std::ostringstream sanitized;
        for (char c : input) {
            if (c == '\'') {
                sanitized << "''"; 
            } else {
                sanitized << c;
            }
        }
        input = sanitized.str();
    }

    auto Database::execute_query(const char* sql_query) -> bool {
        int result = sqlite3_exec(db, sql_query, nullptr, nullptr, nullptr);

        if (result != SQLITE_OK) {
            std::cerr << "Error: SQL error: " << sqlite3_errmsg(db) << std::endl;
            return false;
        } 
        else 
            return true;
    }

    void Database::create_tables() {
        const char* create_users_table_sql = "CREATE TABLE IF NOT EXISTS users ("
                                            "user_id INTEGER PRIMARY KEY,"
                                            "username TEXT UNIQUE,"
                                            "password_hash TEXT,"
                                            "password_salt TEXT"
                                            ");";

        const char* create_files_table_sql = "CREATE TABLE IF NOT EXISTS files ("
                                            "file_id INTEGER PRIMARY KEY,"
                                            "file_name TEXT,"
                                            "file_type TEXT,"
                                            "file_path TEXT,"
                                            "upload_date DATE,"
                                            "uploader_id INTEGER,"
                                            "FOREIGN KEY (uploader_id) REFERENCES users(user_id)"
                                            ");";

        const char* create_permissions_table_sql = "CREATE TABLE IF NOT EXISTS permissions ("
                                                "permission_id INTEGER PRIMARY KEY,"
                                                "file_id INTEGER,"
                                                "user_id INTEGER,"
                                                "is_public INTEGER,"
                                                "FOREIGN KEY (file_id) REFERENCES files(file_id),"
                                                "FOREIGN KEY (user_id) REFERENCES users(user_id)"
                                                ");";

        // Execute SQL queries to create tables
        execute_query(create_users_table_sql);
        execute_query(create_files_table_sql);
        execute_query(create_permissions_table_sql);
    }

    Database::Database() {
        if(sqlite3_open("./db/model.sql", &db) != SQLITE_OK) {
            std::cerr << "Error: Can't open database: " 
                    << sqlite3_errmsg(this->db) << std::endl;
            exit(1);
        }
        this->create_tables();
    }

    Database::~Database() {
        sqlite3_close(this->db);
    }


}