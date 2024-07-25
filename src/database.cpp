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

    std::string Database::get_user(const std::string& username) {
        const char* query = "SELECT user_id FROM users WHERE username = ?;";

        sqlite3_stmt* stmt;
        if (sqlite3_prepare_v2(db, query, -1, &stmt, nullptr) != SQLITE_OK) {
            std::cerr << "Error: Failed to create statement - getuser\n";
        }

        sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);
        std::string result;
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            result = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        } else {
            result = "User not found";
        }
        sqlite3_finalize(stmt);
        return result;
    }

    void Database::create_tables() {
        const char* create_users_table_sql = "CREATE TABLE IF NOT EXISTS users ("
                                            "user_id INTEGER PRIMARY KEY,"
                                            "username TEXT UNIQUE,"
                                            "password_hash TEXT,"
                                            "password_salt TEXT"
                                            ");";

        const char* create_files_table_sql = "CREATE TABLE IF NOT EXISTS files ("
                                            "file_id INTEGER,"
                                            "file_name TEXT,"
                                            "file_path TEXT,"
                                            "upload_date DATE,"
                                            "uploader_id INTEGER,"
                                            "PRIMARY KEY (file_name, uploader_id),"
                                            "FOREIGN KEY (uploader_id) REFERENCES users(user_id)"
                                            ");";


        // Execute SQL queries to create tables
        execute_query(create_users_table_sql);
        execute_query(create_files_table_sql);
    }


    std::string time_t_to_sql_date(time_t timestamp) {
        // Convert time_t to tm structure
        struct tm* time_info = std::gmtime(&timestamp);

        // Create a string stream to format the date
        std::ostringstream oss;
        oss << std::put_time(time_info, "%Y-%m-%d");

        // Return the formatted date as a string
        return oss.str();
    }

    auto Database::insert_file(File_Data& file) -> bool {
        const char* insert_file_sql =
            "INSERT INTO files (file_name, file_path, upload_date, uploader_id) "
            "VALUES (?, ?, ?, ?);";

        sqlite3_stmt* stmt;

        int uploader_id = std::stoi(this->get_user(file.author));
        if (sqlite3_prepare_v2(db, insert_file_sql, -1, &stmt, nullptr) != SQLITE_OK) {
            std::cerr << "Error: Could not insert file data into DB" << std::endl;
            return false;
        }

        sqlite3_bind_text(stmt, 1, file.file_name.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, file.file_path.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 3, time_t_to_sql_date(file.creation_time).c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt, 4, uploader_id);


        if (sqlite3_step(stmt) != SQLITE_DONE) {
            std::cerr << "Error: Could not insert file data into DB" << std::endl;
            sqlite3_finalize(stmt);
            return false;
        }

        sqlite3_finalize(stmt);
        return true;
    }

    Database::Database() {
        std::string anon = "Anonymous";
        std::string anon_p = "Anonymous123";

        if(sqlite3_open("./db/model.sql", &db) != SQLITE_OK) {
            std::cerr << "Error: Can't open database: "
                    << sqlite3_errmsg(this->db) << std::endl;
            exit(1);
        }
        this->create_tables();
        this->insert_user(anon, anon_p);
    }

    Database::~Database() {
        sqlite3_close(this->db);
    }


}
