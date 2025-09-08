// db_inserter/inserter/DbConnectionManager.cpp
#include "DbConnectionManager.hpp"
#include <iostream>

DbConnectionManager::DbConnectionManager(const std::string& db_path) : db(nullptr) {
    if (sqlite3_open(db_path.c_str(), &db) != SQLITE_OK) {
        std::cerr << "Error: Cannot open database: " << sqlite3_errmsg(db) << std::endl;
        db = nullptr;
    } else {
        // 创建表结构
        const char* create_days_sql = 
                "CREATE TABLE IF NOT EXISTS days ("
                "date TEXT PRIMARY KEY, "
                "year INTEGER, "
                "month INTEGER, "
                "status INTEGER, " // [修改] 类型改为 INTEGER
                "sleep INTEGER, "  // [修改] 类型改为 INTEGER
                "remark TEXT, "
                "getup_time TEXT);";
        execute_sql(db, create_days_sql, "Create days table");

        const char* create_index_sql = 
            "CREATE INDEX IF NOT EXISTS idx_year_month ON days (year, month);";
        execute_sql(db, create_index_sql, "Create index on days(year, month)");

        const char* create_records_sql =
            "CREATE TABLE IF NOT EXISTS time_records (date TEXT, start TEXT, end TEXT, project_path TEXT, duration INTEGER, PRIMARY KEY (date, start), FOREIGN KEY (date) REFERENCES days(date));";
        execute_sql(db, create_records_sql, "Create time_records table");
        
        const char* create_parent_child_sql = 
            "CREATE TABLE IF NOT EXISTS parent_child (child TEXT PRIMARY KEY, parent TEXT);";
        execute_sql(db, create_parent_child_sql, "Create parent_child table");
    }
}

DbConnectionManager::~DbConnectionManager() {
    if (db) {
        sqlite3_close(db);
    }
}

sqlite3* DbConnectionManager::get_db() const {
    return db;
}

bool DbConnectionManager::begin_transaction() {
    return execute_sql(db, "BEGIN TRANSACTION;", "Begin transaction");
}

bool DbConnectionManager::commit_transaction() {
    return execute_sql(db, "COMMIT;", "Commit transaction");
}

void DbConnectionManager::rollback_transaction() {
    execute_sql(db, "ROLLBACK;", "Rollback transaction");
}

bool execute_sql(sqlite3* db, const std::string& sql, const std::string& context_msg) {
    char* err_msg = nullptr;
    if (sqlite3_exec(db, sql.c_str(), 0, 0, &err_msg) != SQLITE_OK) {
        std::cerr << "SQL Error (" << context_msg << "): " << err_msg << std::endl;
        sqlite3_free(err_msg);
        return false;
    }
    return true;
}