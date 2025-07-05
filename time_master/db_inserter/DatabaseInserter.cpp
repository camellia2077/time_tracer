#include "DatabaseInserter.h"
#include <iostream>
#include <stdexcept>
#include <vector>

// --- DatabaseInserter Constructor & Destructor ---

DatabaseInserter::DatabaseInserter(const std::string& db_path) 
    : db(nullptr), stmt_insert_day(nullptr), stmt_insert_record(nullptr), stmt_insert_parent_child(nullptr) {
    if (sqlite3_open(db_path.c_str(), &db)) {
        std::cerr << "Error: Cannot open database: " << sqlite3_errmsg(db) << std::endl;
        db = nullptr;
    } else {
        _initialize_database();
        _prepare_statements();
    }
}

DatabaseInserter::~DatabaseInserter() {
    if (db) {
        _finalize_statements();
        sqlite3_close(db);
    }
}

// --- Public Member Functions ---

bool DatabaseInserter::is_db_open() const {
    return db != nullptr;
}

void DatabaseInserter::import_data(const DataFileParser& parser) {
    if (!db) return;

    execute_sql_Inserter(db, "BEGIN TRANSACTION;", "Begin import transaction");

    // Import days from the parser's in-memory vector
    for (const auto& day_data : parser.days) {
        sqlite3_bind_text(stmt_insert_day, 1, day_data.date.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt_insert_day, 2, day_data.status.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt_insert_day, 3, day_data.sleep.c_str(), -1, SQLITE_TRANSIENT); // 新增：绑定sleep数据
        sqlite3_bind_text(stmt_insert_day, 4, day_data.remark.c_str(), -1, SQLITE_TRANSIENT); // 修改：索引从3变为4
        
        // --- MODIFICATION START ---
        // 检查 getup_time 是否为 "Null" 或空，如果是，则绑定 SQL NULL
        if (day_data.getup_time == "Null" || day_data.getup_time.empty()) {
            sqlite3_bind_null(stmt_insert_day, 5); // 在第5个参数位置绑定NULL
        } else {
            // 否则，正常绑定文本值
            sqlite3_bind_text(stmt_insert_day, 5, day_data.getup_time.c_str(), -1, SQLITE_TRANSIENT); // 修改：索引从4变为5
        }
        // --- MODIFICATION END ---

        if (sqlite3_step(stmt_insert_day) != SQLITE_DONE) {
            std::cerr << "Error inserting day row: " << sqlite3_errmsg(db) << std::endl;
        }
        sqlite3_reset(stmt_insert_day);
    }

    // Import time records from the parser's in-memory vector
    for (const auto& record_data : parser.records) {
        sqlite3_bind_text(stmt_insert_record, 1, record_data.date.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt_insert_record, 2, record_data.start.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt_insert_record, 3, record_data.end.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt_insert_record, 4, record_data.project_path.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt_insert_record, 5, record_data.duration_seconds);

        if (sqlite3_step(stmt_insert_record) != SQLITE_DONE) {
            std::cerr << "Error inserting record row: " << sqlite3_errmsg(db) << std::endl;
        }
        sqlite3_reset(stmt_insert_record);
    }

    // Import parent-child relationships from the parser's in-memory set
    for (const auto& pair : parser.parent_child_pairs) {
        sqlite3_bind_text(stmt_insert_parent_child, 1, pair.first.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt_insert_parent_child, 2, pair.second.c_str(), -1, SQLITE_TRANSIENT);

        if (sqlite3_step(stmt_insert_parent_child) != SQLITE_DONE) {
            std::cerr << "Error inserting parent-child row: " << sqlite3_errmsg(db) << std::endl;
        }
        sqlite3_reset(stmt_insert_parent_child);
    }

    execute_sql_Inserter(db, "COMMIT;", "Commit import transaction");
}

// --- Private Member Functions ---

void DatabaseInserter::_initialize_database() {
    // 修改：为days表添加sleep列
    execute_sql_Inserter(db, "CREATE TABLE IF NOT EXISTS days (date TEXT PRIMARY KEY, status TEXT, sleep TEXT, remark TEXT, getup_time TEXT);", "Create days table");
    execute_sql_Inserter(db, "CREATE TABLE IF NOT EXISTS time_records (date TEXT, start TEXT, end TEXT, project_path TEXT, duration INTEGER, PRIMARY KEY (date, start), FOREIGN KEY (date) REFERENCES days(date));", "Create time_records table");
    execute_sql_Inserter(db, "CREATE TABLE IF NOT EXISTS parent_child (child TEXT PRIMARY KEY, parent TEXT);", "Create parent_child table");
}

void DatabaseInserter::_prepare_statements() {
    // 修改：INSERT语句以包含sleep列
    const char* insert_day_sql = "INSERT OR REPLACE INTO days (date, status, sleep, remark, getup_time) VALUES (?, ?, ?, ?, ?);";
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

void DatabaseInserter::_finalize_statements() {
    if (stmt_insert_day) sqlite3_finalize(stmt_insert_day);
    if (stmt_insert_record) sqlite3_finalize(stmt_insert_record);
    if (stmt_insert_parent_child) sqlite3_finalize(stmt_insert_parent_child);
}

// Non-member helper function
bool execute_sql_Inserter(sqlite3* db, const std::string& sql, const std::string& context_msg) {
    char* err_msg = nullptr;
    if (sqlite3_exec(db, sql.c_str(), 0, 0, &err_msg) != SQLITE_OK) {
        std::cerr << "SQL Error (" << context_msg << "): " << err_msg << std::endl;
        sqlite3_free(err_msg);
        return false;
    }
    return true;
}