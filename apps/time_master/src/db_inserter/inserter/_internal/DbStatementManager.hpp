// db_inserter/inserter/DbStatementManager.hpp
#ifndef DB_STATEMENT_MANAGER_HPP
#define DB_STATEMENT_MANAGER_HPP

#include <sqlite3.h>

class DbStatementManager {
public:
    explicit DbStatementManager(sqlite3* db);
    ~DbStatementManager();

    sqlite3_stmt* get_insert_day_stmt() const;
    sqlite3_stmt* get_insert_record_stmt() const;
    sqlite3_stmt* get_insert_parent_child_stmt() const;

private:
    sqlite3* db;
    sqlite3_stmt* stmt_insert_day;
    sqlite3_stmt* stmt_insert_record;
    sqlite3_stmt* stmt_insert_parent_child;

    void _prepare_statements();
    void _finalize_statements();
};

#endif // DB_STATEMENT_MANAGER_HPP