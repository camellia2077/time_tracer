#ifndef DATABASE_IMPORTER_H
#define DATABASE_IMPORTER_H

#include <string>
#include <sqlite3.h>
#include "data_parser.h" // Include parser to get data structures and class definition

class DatabaseImporter {
public:
    DatabaseImporter(const std::string& db_path);
    ~DatabaseImporter();
    bool is_db_open() const;
    void import_data(const DataFileParser& parser);

private:
    sqlite3* db;
    sqlite3_stmt* stmt_insert_day;
    sqlite3_stmt* stmt_insert_record;
    sqlite3_stmt* stmt_insert_parent_child;

    void _initialize_database();
    void _prepare_statements();
    void _finalize_statements();
};

// Non-member helper function
bool execute_sql_importer(sqlite3* db, const std::string& sql, const std::string& context_msg = "");

#endif // DATABASE_IMPORTER_H