// db_inserter/inserter/pipelines/DbConnectionManager.cpp
#include "DbConnectionManager.hpp"
#include <iostream>

DbConnectionManager::DbConnectionManager(const std::string& db_path) : db(nullptr) {
    if (sqlite3_open(db_path.c_str(), &db) != SQLITE_OK) {
        std::cerr << "Error: Cannot open database: " << sqlite3_errmsg(db) << std::endl;
        db = nullptr;
    } else {
        // days 表：用于存储每一天的元数据和统计信息。
        // - date: 日期，主键 (TEXT)
        // - year: 年份 (INTEGER)
        // - month: 月份 (INTEGER)
        // - status: 当天状态 (INTEGER)
        // - sleep: 睡眠质量 (INTEGER)
        // - remark: 当天备注 (TEXT)
        // - getup_time: 起床时间 (TEXT)
        // - exercise: 是否锻炼 (INTEGER)
        // - total_exercise_time: 总锻炼时间 (INTEGER)
        // - cardio_time: 有氧运动时间 (INTEGER)
        // - anaerobic_time: 无氧运动时间 (INTEGER)
        // - exercise_both_time: 有氧和无氧结合运动时间 (INTEGER)
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
                "exercise_both_time INTEGER);";
        execute_sql(db, create_days_sql, "Create days table");

        const char* create_index_sql = 
            "CREATE INDEX IF NOT EXISTS idx_year_month ON days (year, month);";
        execute_sql(db, create_index_sql, "Create index on days(year, month)");

        // time_records 表：用于存储每一条具体的活动记录。
        // - logical_id: 逻辑ID，主键 (INTEGER)
        // - start_timestamp: 开始时间戳 (INTEGER)
        // - end_timestamp: 结束时间戳 (INTEGER)
        // - date: 记录关联的日期，外键 (TEXT)
        // - start: 开始时间的文本表示 (TEXT)
        // - end: 结束时间的文本表示 (TEXT)
        // - project_path: 项目路径 (TEXT)
        // - duration: 持续时间（秒）(INTEGER)
        // - activity_remark: 活动备注 (TEXT)
        const char* create_records_sql =
            "CREATE TABLE IF NOT EXISTS time_records ("
            "logical_id INTEGER PRIMARY KEY, "
            "start_timestamp INTEGER, "
            "end_timestamp INTEGER, "
            "date TEXT, "
            "start TEXT, "
            "end TEXT, "
            "project_path TEXT, "
            "duration INTEGER, "
            "activity_remark TEXT, " 
            "FOREIGN KEY (date) REFERENCES days(date));";
        execute_sql(db, create_records_sql, "Create time_records table");
        
        // parent_child 表：用于存储项目层级关系，方便后续进行项目分类查询。
        // - child: 子项目，主键 (TEXT)
        // - parent: 父项目 (TEXT)
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