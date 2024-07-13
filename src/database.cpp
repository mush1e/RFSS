#include "database.hpp"

namespace rfss {

    auto Database::get_instance() -> Database& {
        static Database instance;
        return instance;
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
                                            "password_salt TEXT,"
                                            "is_admin INTEGER"
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