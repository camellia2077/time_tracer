// db_inserter/inserter/pipelines/DbConnectionManager.cpp
#include "DbConnectionManager.hpp"
#include <iostream>

DbConnectionManager::DbConnectionManager(const std::string& db_path) : db(nullptr) {
    if (sqlite3_open(db_path.c_str(), &db) != SQLITE_OK) {
        std::cerr << "Error: Cannot open database: " << sqlite3_errmsg(db) << std::endl;
        db = nullptr;
    } else {
        // --- [核心修改] 更新 days 表的定义 ---
        const char* create_days_sql =
                "CREATE TABLE IF NOT EXISTS days ("
                "date TEXT PRIMARY KEY, "
                "year INTEGER, "
                "month INTEGER, "
                "status INTEGER, "
                "sleep INTEGER, "
                "remark TEXT, "
                "getup_time TEXT, "
                "exercise INTEGER, "
                "total_exercise_time INTEGER, "
                "cardio_time INTEGER, "
                "anaerobic_time INTEGER, "
                "gaming_time INTEGER, "
                "grooming_time INTEGER, "
                "toilet_time INTEGER, "
                "sleepNightTime INTEGER, "
                "sleepDayTime INTEGER, "
                "sleepTotalTime INTEGER, "
                "recreationTime INTEGER, " // 新增
                "recreationZhihuTime INTEGER, " // 新增
                "recreationBilibiliTime INTEGER, " // 新增
                "recreationDouyinTime INTEGER);"; // 新增
        execute_sql(db, create_days_sql, "Create days table");

        const char* create_index_sql =
            "CREATE INDEX IF NOT EXISTS idx_year_month ON days (year, month);";
        execute_sql(db, create_index_sql, "Create index on days(year, month)");

        const char* create_projects_sql =
            "CREATE TABLE IF NOT EXISTS projects ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT, "
            "name TEXT NOT NULL, "
            "parent_id INTEGER, "
            "FOREIGN KEY (parent_id) REFERENCES projects(id));";
        execute_sql(db, create_projects_sql, "Create projects table");

        const char* create_records_sql =
            "CREATE TABLE IF NOT EXISTS time_records ("
            "logical_id INTEGER PRIMARY KEY, "
            "start_timestamp INTEGER, "
            "end_timestamp INTEGER, "
            "date TEXT, "
            "start TEXT, "
            "end TEXT, "
            "project_id INTEGER, "
            "duration INTEGER, "
            "activity_remark TEXT, "
            "FOREIGN KEY (date) REFERENCES days(date), "
            "FOREIGN KEY (project_id) REFERENCES projects(id));";
        execute_sql(db, create_records_sql, "Create time_records table");
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