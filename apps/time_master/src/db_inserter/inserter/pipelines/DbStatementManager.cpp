// db_inserter/inserter/pipelines/DbStatementManager.cpp
#include "DbStatementManager.hpp"
#include <stdexcept>

DbStatementManager::DbStatementManager(sqlite3* db)
    : db(db), 
      stmt_insert_day(nullptr), 
      stmt_insert_record(nullptr),
      stmt_select_project_id(nullptr), // Initialize new members
      stmt_insert_project(nullptr)
{
    _prepare_statements();
}

DbStatementManager::~DbStatementManager() {
    _finalize_statements();
}

sqlite3_stmt* DbStatementManager::get_insert_day_stmt() const { return stmt_insert_day; }
sqlite3_stmt* DbStatementManager::get_insert_record_stmt() const { return stmt_insert_record; }

// --- [FIX] Added implementations for the new getter functions ---
sqlite3_stmt* DbStatementManager::get_select_project_id_stmt() const { return stmt_select_project_id; }
sqlite3_stmt* DbStatementManager::get_insert_project_stmt() const { return stmt_insert_project; }

void DbStatementManager::_prepare_statements() {
    // --- Statement for 'days' table (unchanged) ---
    const char* insert_day_sql = 
    "INSERT INTO days (date, year, month, status, sleep, remark, getup_time, "
    "exercise, total_exercise_time, cardio_time, anaerobic_time, exercise_both_time) "
    "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);";
    if (sqlite3_prepare_v2(db, insert_day_sql, -1, &stmt_insert_day, nullptr) != SQLITE_OK) {
        throw std::runtime_error("Failed to prepare day insert statement.");
    }

    // --- [FIX] Updated statement for 'time_records' table to use project_id ---
    const char* insert_record_sql = 
        "INSERT OR REPLACE INTO time_records "
        "(logical_id, start_timestamp, end_timestamp, date, start, end, project_id, duration, activity_remark) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?);";
    if (sqlite3_prepare_v2(db, insert_record_sql, -1, &stmt_insert_record, nullptr) != SQLITE_OK) {
        throw std::runtime_error("Failed to prepare time record insert statement.");
    }

    // --- [FIX] Added statements for the new 'projects' table ---
    const char* select_project_id_sql = 
        "SELECT id FROM projects WHERE name = ? AND parent_id IS ?";
    if (sqlite3_prepare_v2(db, select_project_id_sql, -1, &stmt_select_project_id, nullptr) != SQLITE_OK) {
        throw std::runtime_error("Failed to prepare project select statement.");
    }

    const char* insert_project_sql =
        "INSERT INTO projects (name, parent_id) VALUES (?, ?);";
    if (sqlite3_prepare_v2(db, insert_project_sql, -1, &stmt_insert_project, nullptr) != SQLITE_OK) {
        throw std::runtime_error("Failed to prepare project insert statement.");
    }
}

void DbStatementManager::_finalize_statements() {
    if (stmt_insert_day) sqlite3_finalize(stmt_insert_day);
    if (stmt_insert_record) sqlite3_finalize(stmt_insert_record);
    // --- [FIX] Finalize the new statements ---
    if (stmt_select_project_id) sqlite3_finalize(stmt_select_project_id);
    if (stmt_insert_project) sqlite3_finalize(stmt_insert_project);
}