// db_inserter/inserter/DataInserter.cpp
#include "DataInserter.hpp"
#include <iostream>

DataInserter::DataInserter(sqlite3* db, sqlite3_stmt* stmt_day, sqlite3_stmt* stmt_record, sqlite3_stmt* stmt_parent_child)
    : db(db), stmt_insert_day(stmt_day), stmt_insert_record(stmt_record), stmt_insert_parent_child(stmt_parent_child) {}

void DataInserter::insert_days(const std::vector<DayData>& days) {
    for (const auto& day_data : days) {
        sqlite3_bind_text(stmt_insert_day, 1, day_data.date.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt_insert_day, 2, day_data.year);
        sqlite3_bind_int(stmt_insert_day, 3, day_data.month);
        sqlite3_bind_int(stmt_insert_day, 4, day_data.status);
        sqlite3_bind_int(stmt_insert_day, 5, day_data.sleep);
        sqlite3_bind_text(stmt_insert_day, 6, day_data.remark.c_str(), -1, SQLITE_TRANSIENT);
        
        if (day_data.getup_time == "Null" || day_data.getup_time.empty()) {
            sqlite3_bind_null(stmt_insert_day, 7);
        } else {
            sqlite3_bind_text(stmt_insert_day, 7, day_data.getup_time.c_str(), -1, SQLITE_TRANSIENT);
        }

        // --- [核心修改] 绑定新的统计字段 ---
        sqlite3_bind_int(stmt_insert_day, 8, day_data.exercise);
        sqlite3_bind_int(stmt_insert_day, 9, day_data.total_exercise_time);
        sqlite3_bind_int(stmt_insert_day, 10, day_data.cardio_time);
        sqlite3_bind_int(stmt_insert_day, 11, day_data.anaerobic_time);
        sqlite3_bind_int(stmt_insert_day, 12, day_data.exercise_both_time);


        if (sqlite3_step(stmt_insert_day) != SQLITE_DONE) {
            std::cerr << "Error inserting day row: " << sqlite3_errmsg(db) << std::endl;
        }
        sqlite3_reset(stmt_insert_day);
    }
}

void DataInserter::insert_records(const std::vector<TimeRecordInternal>& records) {
    for (const auto& record_data : records) {
        // --- [核心修改] 绑定新的字段到预处理语句 ---
        sqlite3_bind_int64(stmt_insert_record, 1, record_data.logical_id);
        sqlite3_bind_int64(stmt_insert_record, 2, record_data.start_timestamp);
        sqlite3_bind_int64(stmt_insert_record, 3, record_data.end_timestamp);
        sqlite3_bind_text(stmt_insert_record, 4, record_data.date.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt_insert_record, 5, record_data.start.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt_insert_record, 6, record_data.end.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt_insert_record, 7, record_data.project_path.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt_insert_record, 8, record_data.duration_seconds);

        // 绑定 activityRemark
        if (record_data.activityRemark.has_value()) {
            sqlite3_bind_text(stmt_insert_record, 9, record_data.activityRemark->c_str(), -1, SQLITE_TRANSIENT);
        } else {
            sqlite3_bind_null(stmt_insert_record, 9);
        }

        if (sqlite3_step(stmt_insert_record) != SQLITE_DONE) {
            std::cerr << "Error inserting record row: " << sqlite3_errmsg(db) << std::endl;
        }
        sqlite3_reset(stmt_insert_record);
    }
}

void DataInserter::insert_parent_child_pairs(const std::unordered_set<std::pair<std::string, std::string>, pair_hash>& parent_child_pairs) {
    for (const auto& pair : parent_child_pairs) {
        sqlite3_bind_text(stmt_insert_parent_child, 1, pair.first.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt_insert_parent_child, 2, pair.second.c_str(), -1, SQLITE_TRANSIENT);

        if (sqlite3_step(stmt_insert_parent_child) != SQLITE_DONE) {
            std::cerr << "Error inserting parent-child row: " << sqlite3_errmsg(db) << std::endl;
        }
        sqlite3_reset(stmt_insert_parent_child);
    }
}