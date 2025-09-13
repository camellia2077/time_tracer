// db_inserter/inserter/pipelines/DbConnectionManager.hpp
#ifndef DB_CONNECTION_MANAGER_HPP
#define DB_CONNECTION_MANAGER_HPP

#include <string>
#include <sqlite3.h>

class DbConnectionManager {
public:
    explicit DbConnectionManager(const std::string& db_path);
    ~DbConnectionManager();

    sqlite3* get_db() const;
    bool begin_transaction();
    bool commit_transaction();
    void rollback_transaction();

private:
    sqlite3* db;
};

bool execute_sql(sqlite3* db, const std::string& sql, const std::string& context_msg = "");

#endif // DB_CONNECTION_MANAGER_HPP