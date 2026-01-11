// importer/storage/sqlite/Connection.hpp
#ifndef CONNECTION_HPP
#define CONNECTION_HPP

#include <string>
#include <sqlite3.h>

class Connection {
public:
    explicit Connection(const std::string& db_path);
    ~Connection();

    sqlite3* get_db() const;
    bool begin_transaction();
    bool commit_transaction();
    void rollback_transaction();

private:
    sqlite3* db_; // MODIFIED
};

bool execute_sql(sqlite3* db, const std::string& sql, const std::string& context_msg = "");

#endif // CONNECTION_HPP