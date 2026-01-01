// db_inserter/inserter/pipelines/DbStatementManager.cpp
#include "DbStatementManager.hpp"
#include <stdexcept>

DbStatementManager::DbStatementManager(sqlite3* db)
    : db_(db),  
      stmt_insert_day_(nullptr),  
      stmt_insert_record_(nullptr), 
      stmt_select_project_id_(nullptr), 
      stmt_insert_project_(nullptr) 
{
    _prepare_statements();
}

DbStatementManager::~DbStatementManager() {
    _finalize_statements();
}

sqlite3_stmt* DbStatementManager::get_insert_day_stmt() const { return stmt_insert_day_; } 
sqlite3_stmt* DbStatementManager::get_insert_record_stmt() const { return stmt_insert_record_; } 
sqlite3_stmt* DbStatementManager::get_select_project_id_stmt() const { return stmt_select_project_id_; } 
sqlite3_stmt* DbStatementManager::get_insert_project_stmt() const { return stmt_insert_project_; }

void DbStatementManager::_prepare_statements() {
    const char* insert_day_sql = 
    "INSERT INTO days ("
        // [1-7] 基础信息
        "date, year, month, status, sleep, remark, getup_time, "
        // [8-11] 运动相关
        "exercise, total_exercise_time, cardio_time, anaerobic_time, "
        // [12-14] 生活琐事
        "gaming_time, grooming_time, toilet_time, "
        // [15-17] 睡眠统计
        "sleep_night_time, sleep_day_time, sleep_total_time, "
        // [18-21] 娱乐统计
        "recreation_time, recreation_zhihu_time, recreation_bilibili_time, recreation_douyin_time, "
        // [22] 学习统计
        "study_time"
    ") "
    "VALUES ("
        "?, ?, ?, ?, ?, ?, ?, "       // 1-7
        "?, ?, ?, ?, "                // 8-11
        "?, ?, ?, "                   // 12-14
        "?, ?, ?, "                   // 15-17
        "?, ?, ?, ?, "                // 18-21
        "?"                           // 22: study_time
    ");";
    if (sqlite3_prepare_v2(db_, insert_day_sql, -1, &stmt_insert_day_, nullptr) != SQLITE_OK) { 
        throw std::runtime_error("Failed to prepare day insert statement.");
    }

    const char* insert_record_sql = 
        "INSERT OR REPLACE INTO time_records "
        "(logical_id, start_timestamp, end_timestamp, date, start, end, project_id, duration, activity_remark) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?);";
    if (sqlite3_prepare_v2(db_, insert_record_sql, -1, &stmt_insert_record_, nullptr) != SQLITE_OK) { 
        throw std::runtime_error("Failed to prepare time record insert statement.");
    }

    const char* select_project_id_sql = 
        "SELECT id FROM projects WHERE name = ? AND parent_id IS ?";
    if (sqlite3_prepare_v2(db_, select_project_id_sql, -1, &stmt_select_project_id_, nullptr) != SQLITE_OK) { 
        throw std::runtime_error("Failed to prepare project select statement.");
    }

    const char* insert_project_sql =
        "INSERT INTO projects (name, parent_id) VALUES (?, ?);";
    if (sqlite3_prepare_v2(db_, insert_project_sql, -1, &stmt_insert_project_, nullptr) != SQLITE_OK) { 
        throw std::runtime_error("Failed to prepare project insert statement.");
    }
}

void DbStatementManager::_finalize_statements() {
    if (stmt_insert_day_) sqlite3_finalize(stmt_insert_day_); 
    if (stmt_insert_record_) sqlite3_finalize(stmt_insert_record_); 
    if (stmt_select_project_id_) sqlite3_finalize(stmt_select_project_id_); 
    if (stmt_insert_project_) sqlite3_finalize(stmt_insert_project_); 
}