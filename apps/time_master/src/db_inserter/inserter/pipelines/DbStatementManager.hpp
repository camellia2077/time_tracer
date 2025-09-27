// db_inserter/inserter/pipelines/DbStatementManager.hpp
#ifndef DB_STATEMENT_MANAGER_HPP
#define DB_STATEMENT_MANAGER_HPP

#include <sqlite3.h>

class DbStatementManager {
public:
    explicit DbStatementManager(sqlite3* db);
    ~DbStatementManager();

    sqlite3_stmt* get_insert_day_stmt() const;
    sqlite3_stmt* get_insert_record_stmt() const;

    // --- [FIX] Added missing function declarations and removed the old one ---
    sqlite3_stmt* get_select_project_id_stmt() const;
    sqlite3_stmt* get_insert_project_stmt() const;

private:
    sqlite3* db_; // MODIFIED
    sqlite3_stmt* stmt_insert_day_; // MODIFIED
    sqlite3_stmt* stmt_insert_record_; // MODIFIED
    
    // --- [FIX] Added new statement handles and removed the old one ---
    sqlite3_stmt* stmt_select_project_id_; // MODIFIED
    sqlite3_stmt* stmt_insert_project_; // MODIFIED

    void _prepare_statements();
    void _finalize_statements();
};

#endif // DB_STATEMENT_MANAGER_HPP