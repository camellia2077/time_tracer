#ifndef DATABASE_IMPORTER_H
#define DATABASE_IMPORTER_H

#include <string>
#include <sqlite3.h>

class DatabaseImporter {
public:
    DatabaseImporter(const std::string& db_path);
    ~DatabaseImporter();
    bool is_db_open() const;
    void import_from_directory(const std::string& dir_path);

private:
    sqlite3* db;
    sqlite3_stmt* stmt_insert_day;
    sqlite3_stmt* stmt_insert_record;
    sqlite3_stmt* stmt_insert_parent_child;

    void _initialize_database();
    void _prepare_statements();
    void _finalize_statements();
    
    bool _import_csv(const std::string& file_path, sqlite3_stmt* stmt, int num_cols);
};

// Non-member helper function
bool execute_sql_importer(sqlite3* db, const std::string& sql, const std::string& context_msg = "");

#endif // DATABASE_IMPORTER_H