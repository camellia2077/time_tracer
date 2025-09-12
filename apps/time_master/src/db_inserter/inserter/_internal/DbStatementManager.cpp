// db_inserter/inserter/DbStatementManager.cpp
#include "DbStatementManager.hpp"
#include <stdexcept>

DbStatementManager::DbStatementManager(sqlite3* db)
    : db(db), stmt_insert_day(nullptr), stmt_insert_record(nullptr), stmt_insert_parent_child(nullptr) {
    _prepare_statements();
}

DbStatementManager::~DbStatementManager() {
    _finalize_statements();
}

sqlite3_stmt* DbStatementManager::get_insert_day_stmt() const { return stmt_insert_day; }
sqlite3_stmt* DbStatementManager::get_insert_record_stmt() const { return stmt_insert_record; }
sqlite3_stmt* DbStatementManager::get_insert_parent_child_stmt() const { return stmt_insert_parent_child; }

void DbStatementManager::_prepare_statements() {
    // --- [核心修改] 更新 days 表的插入语句 ---
    const char* insert_day_sql = "INSERT OR REPLACE INTO days "
        "(date, year, month, status, sleep, remark, getup_time, exercise, total_exercise_time, cardio_time, anaerobic_time, exercise_both_time) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);";
    if (sqlite3_prepare_v2(db, insert_day_sql, -1, &stmt_insert_day, nullptr) != SQLITE_OK) {
        throw std::runtime_error("Failed to prepare day insert statement.");
    }

    const char* insert_record_sql = "INSERT OR REPLACE INTO time_records "
        "(logical_id, start_timestamp, end_timestamp, date, start, end, project_path, duration, activity_remark) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?);"; // 增加一个占位符
    if (sqlite3_prepare_v2(db, insert_record_sql, -1, &stmt_insert_record, nullptr) != SQLITE_OK) {
        throw std::runtime_error("Failed to prepare time record insert statement.");
    }

    const char* insert_parent_child_sql = "INSERT OR IGNORE INTO parent_child (child, parent) VALUES (?, ?);";
    if (sqlite3_prepare_v2(db, insert_parent_child_sql, -1, &stmt_insert_parent_child, nullptr) != SQLITE_OK) {
        throw std::runtime_error("Failed to prepare parent-child insert statement.");
    }
}

void DbStatementManager::_finalize_statements() {
    if (stmt_insert_day) sqlite3_finalize(stmt_insert_day);
    if (stmt_insert_record) sqlite3_finalize(stmt_insert_record);
    if (stmt_insert_parent_child) sqlite3_finalize(stmt_insert_parent_child);
}