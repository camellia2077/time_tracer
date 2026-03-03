#include "database_importer.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <vector>
#include <filesystem>

namespace fs = std::filesystem;

// --- DatabaseImporter Constructor & Destructor ---

DatabaseImporter::DatabaseImporter(const std::string& db_path) 
    : db(nullptr), stmt_insert_day(nullptr), stmt_insert_record(nullptr), stmt_insert_parent_child(nullptr) {
    if (sqlite3_open(db_path.c_str(), &db)) {
        std::cerr << "Error: Cannot open database: " << sqlite3_errmsg(db) << std::endl;
        db = nullptr;
    } else {
        _initialize_database();
        _prepare_statements();
    }
}

DatabaseImporter::~DatabaseImporter() {
    if (db) {
        _finalize_statements();
        sqlite3_close(db);
    }
}

// --- Public Member Functions ---

bool DatabaseImporter::is_db_open() const {
    return db != nullptr;
}

void DatabaseImporter::import_from_directory(const std::string& dir_path) {
    if (!db) return;

    fs::path dir(dir_path);
    // Corrected lines: added .string() to convert path to string
    _import_csv((dir / "days.csv").string(), stmt_insert_day, 4);
    _import_csv((dir / "records.csv").string(), stmt_insert_record, 5);
    _import_csv((dir / "parent_child.csv").string(), stmt_insert_parent_child, 2);
}

// --- Private Member Functions ---

void DatabaseImporter::_initialize_database() {
    execute_sql_importer(db, "CREATE TABLE IF NOT EXISTS days (date TEXT PRIMARY KEY, status TEXT, remark TEXT, getup_time TEXT);", "Create days table");
    execute_sql_importer(db, "CREATE TABLE IF NOT EXISTS time_records (date TEXT, start TEXT, end TEXT, project_path TEXT, duration INTEGER, PRIMARY KEY (date, start), FOREIGN KEY (date) REFERENCES days(date));", "Create time_records table");
    execute_sql_importer(db, "CREATE TABLE IF NOT EXISTS parent_child (child TEXT PRIMARY KEY, parent TEXT);", "Create parent_child table");
}

void DatabaseImporter::_prepare_statements() {
    // Using INSERT OR REPLACE for days and records, and OR IGNORE for parent_child
    const char* insert_day_sql = "INSERT OR REPLACE INTO days (date, status, remark, getup_time) VALUES (?, ?, ?, ?);";
    if (sqlite3_prepare_v2(db, insert_day_sql, -1, &stmt_insert_day, nullptr) != SQLITE_OK) {
        throw std::runtime_error("Failed to prepare day insert statement.");
    }

    const char* insert_record_sql = "INSERT OR REPLACE INTO time_records (date, start, end, project_path, duration) VALUES (?, ?, ?, ?, ?);";
    if (sqlite3_prepare_v2(db, insert_record_sql, -1, &stmt_insert_record, nullptr) != SQLITE_OK) {
        throw std::runtime_error("Failed to prepare time record insert statement.");
    }

    const char* insert_parent_child_sql = "INSERT OR IGNORE INTO parent_child (child, parent) VALUES (?, ?);";
    if (sqlite3_prepare_v2(db, insert_parent_child_sql, -1, &stmt_insert_parent_child, nullptr) != SQLITE_OK) {
        throw std::runtime_error("Failed to prepare parent-child insert statement.");
    }
}

void DatabaseImporter::_finalize_statements() {
    if (stmt_insert_day) sqlite3_finalize(stmt_insert_day);
    if (stmt_insert_record) sqlite3_finalize(stmt_insert_record);
    if (stmt_insert_parent_child) sqlite3_finalize(stmt_insert_parent_child);
}

bool DatabaseImporter::_import_csv(const std::string& file_path, sqlite3_stmt* stmt, int num_cols) {
    std::ifstream file(file_path);
    if (!file.is_open()) {
        std::cerr << "Warning: Cannot open intermediate file " << file_path << std::endl;
        return false;
    }

    execute_sql_importer(db, "BEGIN TRANSACTION;", "Begin import transaction");

    std::string line;
    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string field;
        std::vector<std::string> fields;

        // Basic CSV parsing (assumes fields are quoted if they contain commas)
        while(std::getline(ss, field, ',')) {
            if (!field.empty() && field.front() == '"') {
                std::string temp = field.substr(1);
                while (ss.good() && (field.empty() || field.back() != '"')) {
                    std::string next_part;
                    std::getline(ss, next_part, ',');
                    temp += "," + next_part;
                }
                if (!temp.empty() && temp.back() == '"') temp.pop_back();
                 fields.push_back(temp);
            } else {
                 fields.push_back(field);
            }
        }
        
        if (fields.size() != num_cols) continue;

        for (int i = 0; i < num_cols; ++i) {
            // The last column for time_records is an integer
            if (stmt == stmt_insert_record && i == 4) {
                 sqlite3_bind_int(stmt, i + 1, std::stoi(fields[i]));
            } else {
                 sqlite3_bind_text(stmt, i + 1, fields[i].c_str(), -1, SQLITE_TRANSIENT);
            }
        }
        
        if (sqlite3_step(stmt) != SQLITE_DONE) {
            std::cerr << "Error inserting row into database: " << sqlite3_errmsg(db) << std::endl;
        }
        sqlite3_reset(stmt);
    }
    
    execute_sql_importer(db, "COMMIT;", "Commit import transaction");
    file.close();
    return true;
}


// Non-member helper function
bool execute_sql_importer(sqlite3* db, const std::string& sql, const std::string& context_msg) {
    char* err_msg = nullptr;
    if (sqlite3_exec(db, sql.c_str(), 0, 0, &err_msg) != SQLITE_OK) {
        std::cerr << "SQL Error (" << context_msg << "): " << err_msg << std::endl;
        sqlite3_free(err_msg);
        return false;
    }
    return true;
}