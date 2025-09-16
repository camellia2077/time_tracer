// db_inserter/inserter/pipelines/DbConnectionManager.cpp
#include "DbConnectionManager.hpp"
#include <iostream>

DbConnectionManager::DbConnectionManager(const std::string& db_path) : db(nullptr) {
    if (sqlite3_open(db_path.c_str(), &db) != SQLITE_OK) {
        std::cerr << "Error: Cannot open database: " << sqlite3_errmsg(db) << std::endl;
        db = nullptr;
    } else {
        // days 表：用于存储每一天的元数据和统计信息。
        const char* create_days_sql =
                "CREATE TABLE IF NOT EXISTS days ("
                "date TEXT PRIMARY KEY,   -- 日期，主键 (TEXT)\n"
                "year INTEGER,            -- 年份 (INTEGER)\n"
                "month INTEGER,           -- 月份 (INTEGER)\n"
                "status INTEGER,          -- 当天状态 (INTEGER)\n"
                "sleep INTEGER,           -- 睡眠质量 (INTEGER)\n"
                "remark TEXT,             -- 当天备注 (TEXT)\n"
                "getup_time TEXT,         -- 起床时间 (TEXT)\n"
                "exercise INTEGER,        -- 是否锻炼 (INTEGER)\n"
                "total_exercise_time INTEGER, -- 总锻炼时间 (INTEGER)\n"
                "cardio_time INTEGER,     -- 有氧运动时间 (INTEGER)\n"
                "anaerobic_time INTEGER,  -- 无氧运动时间 (INTEGER)\n"
                "exercise_both_time INTEGER -- 有氧和无氧结合运动时间 (INTEGER)\n"
                ");";
        execute_sql(db, create_days_sql, "Create days table");

        const char* create_index_sql =
            "/* 在 days 表的 year 和 month 列上创建索引以加速查询 */\n"
            "CREATE INDEX IF NOT EXISTS idx_year_month ON days (year, month);";
        execute_sql(db, create_index_sql, "Create index on days(year, month)");

        // time_records 表：用于存储每一条具体的活动记录。
        const char* create_records_sql =
            "CREATE TABLE IF NOT EXISTS time_records ("
            "logical_id INTEGER PRIMARY KEY, -- 逻辑ID，主键 (INTEGER)\n"
            "start_timestamp INTEGER,    -- 开始时间戳 (INTEGER)\n"
            "end_timestamp INTEGER,      -- 结束时间戳 (INTEGER)\n"
            "date TEXT,                  -- 记录关联的日期，外键 (TEXT)\n"
            "start TEXT,                 -- 开始时间的文本表示 (TEXT)\n"
            "end TEXT,                   -- 结束时间的文本表示 (TEXT)\n"
            "project_path TEXT,          -- 项目路径 (TEXT)\n"
            "duration INTEGER,           -- 持续时间（秒）(INTEGER)\n"
            "activity_remark TEXT,       -- 活动备注 (TEXT)\n"
            "FOREIGN KEY (date) REFERENCES days(date)"
            ");";
        execute_sql(db, create_records_sql, "Create time_records table");

        /*
         * parent_child 表：用于存储项目层级关系，
         * 方便后续进行项目分类查询。
         */
        const char* create_parent_child_sql =
            "CREATE TABLE IF NOT EXISTS parent_child ("
            "child TEXT PRIMARY KEY, -- 子项目，主键 (TEXT)\n"
            "parent TEXT             -- 父项目 (TEXT)\n"
            ");";
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