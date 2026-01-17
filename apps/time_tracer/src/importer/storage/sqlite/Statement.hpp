// importer/storage/sqlite/Statement.hpp
#ifndef IMPORTER_STORAGE_SQLITE_STATEMENT_HPP_
#define IMPORTER_STORAGE_SQLITE_STATEMENT_HPP_

#include <sqlite3.h>

class Statement {
public:
    explicit Statement(sqlite3* db);
    ~Statement();

    sqlite3_stmt* get_insert_day_stmt() const;
    sqlite3_stmt* get_insert_record_stmt() const;

    // --- [FIX] Added missing function declarations and removed the old one ---
    sqlite3_stmt* get_select_project_id_stmt() const;
    sqlite3_stmt* get_insert_project_stmt() const;

private:
    sqlite3* db_; 
    sqlite3_stmt* stmt_insert_day_; 
    sqlite3_stmt* stmt_insert_record_; 

    sqlite3_stmt* stmt_select_project_id_; 
    sqlite3_stmt* stmt_insert_project_; 

    void _prepare_statements();
    void _finalize_statements();
};

#endif // IMPORTER_STORAGE_SQLITE_STATEMENT_HPP_