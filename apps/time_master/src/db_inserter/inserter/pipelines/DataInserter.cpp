// db_inserter/inserter/pipelines/DataInserter.cpp
#include "DataInserter.hpp"
#include <iostream>
#include <sstream>
#include <vector>

static std::vector<std::string> split_string(const std::string& s, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(s);
    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

DataInserter::DataInserter(sqlite3* db,
                           sqlite3_stmt* stmt_day,
                           sqlite3_stmt* stmt_record,
                           sqlite3_stmt* stmt_select_project,
                           sqlite3_stmt* stmt_insert_project)
    : db_(db), 
      stmt_insert_day_(stmt_day),
      stmt_insert_record_(stmt_record),
      stmt_select_project_id_(stmt_select_project),
      stmt_insert_project_(stmt_insert_project) {}

void DataInserter::insert_days(const std::vector<DayData>& days) {
    for (const auto& day_data : days) {
        sqlite3_bind_text(stmt_insert_day_, 1, day_data.date.c_str(), -1, SQLITE_TRANSIENT); // MODIFIED
        sqlite3_bind_int(stmt_insert_day_, 2, day_data.year); // MODIFIED
        sqlite3_bind_int(stmt_insert_day_, 3, day_data.month); // MODIFIED
        sqlite3_bind_int(stmt_insert_day_, 4, day_data.status); // MODIFIED
        sqlite3_bind_int(stmt_insert_day_, 5, day_data.sleep); // MODIFIED
        sqlite3_bind_text(stmt_insert_day_, 6, day_data.remark.c_str(), -1, SQLITE_TRANSIENT); // MODIFIED

        if (day_data.getup_time == "Null" || day_data.getup_time.empty()) {
            sqlite3_bind_null(stmt_insert_day_, 7); // MODIFIED
        } else {
            sqlite3_bind_text(stmt_insert_day_, 7, day_data.getup_time.c_str(), -1, SQLITE_TRANSIENT); // MODIFIED
        }

        sqlite3_bind_int(stmt_insert_day_, 8, day_data.exercise); // MODIFIED
        sqlite3_bind_int(stmt_insert_day_, 9, day_data.total_exercise_time); // MODIFIED
        sqlite3_bind_int(stmt_insert_day_, 10, day_data.cardio_time); // MODIFIED
        sqlite3_bind_int(stmt_insert_day_, 11, day_data.anaerobic_time); // MODIFIED
        sqlite3_bind_int(stmt_insert_day_, 12, day_data.gaming_time); // MODIFIED
        sqlite3_bind_int(stmt_insert_day_, 13, day_data.grooming_time); // MODIFIED
        sqlite3_bind_int(stmt_insert_day_, 14, day_data.toilet_time); // MODIFIED

        sqlite3_bind_int(stmt_insert_day_, 15, day_data.sleep_night_time); // MODIFIED
        sqlite3_bind_int(stmt_insert_day_, 16, day_data.sleep_day_time); // MODIFIED
        sqlite3_bind_int(stmt_insert_day_, 17, day_data.sleep_total_time); // MODIFIED

        // --- [新增] 绑定娱乐时间数据 ---
        sqlite3_bind_int(stmt_insert_day_, 18, day_data.recreation_time); // MODIFIED
        sqlite3_bind_int(stmt_insert_day_, 19, day_data.recreation_zhihu_time); // MODIFIED
        sqlite3_bind_int(stmt_insert_day_, 20, day_data.recreation_bilibili_time); // MODIFIED
        sqlite3_bind_int(stmt_insert_day_, 21, day_data.recreation_douyin_time); // MODIFIED

        if (sqlite3_step(stmt_insert_day_) != SQLITE_DONE) { // MODIFIED
            std::cerr << "Error inserting day row: " << sqlite3_errmsg(db_) << std::endl; // MODIFIED
        }
        sqlite3_reset(stmt_insert_day_); // MODIFIED
    }
}

void DataInserter::insert_records(const std::vector<TimeRecordInternal>& records) {
    for (const auto& record_data : records) {
        long long project_id = get_or_create_project_id(record_data.project_path);

        sqlite3_bind_int64(stmt_insert_record_, 1, record_data.logical_id);
        sqlite3_bind_int64(stmt_insert_record_, 2, record_data.start_timestamp);
        sqlite3_bind_int64(stmt_insert_record_, 3, record_data.end_timestamp);
        sqlite3_bind_text(stmt_insert_record_, 4, record_data.date.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt_insert_record_, 5, record_data.start.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt_insert_record_, 6, record_data.end.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int64(stmt_insert_record_, 7, project_id);
        sqlite3_bind_int(stmt_insert_record_, 8, record_data.duration_seconds);

        if (record_data.activity_remark.has_value()) { // MODIFIED
            sqlite3_bind_text(stmt_insert_record_, 9, record_data.activity_remark->c_str(), -1, SQLITE_TRANSIENT); // MODIFIED
        } else {
            sqlite3_bind_null(stmt_insert_record_, 9);
        }

        if (sqlite3_step(stmt_insert_record_) != SQLITE_DONE) {
            std::cerr << "Error inserting record row: " << sqlite3_errmsg(db_) << std::endl;
        }
        sqlite3_reset(stmt_insert_record_);
    }
}

long long DataInserter::get_or_create_project_id(const std::string& project_path) {
    if (project_path_cache_.count(project_path)) {
        return project_path_cache_[project_path];
    }

    std::vector<std::string> parts = split_string(project_path, '_');
    long long current_parent_id = 0; 
    std::string current_path = "";
    for (const auto& part : parts) {
        if (!current_path.empty()) {
            current_path += "_";
        }
        current_path += part;

        if (project_path_cache_.count(current_path)) {
            current_parent_id = project_path_cache_[current_path];
            continue;
        }

        sqlite3_reset(stmt_select_project_id_); // MODIFIED
        sqlite3_bind_text(stmt_select_project_id_, 1, part.c_str(), -1, SQLITE_TRANSIENT); // MODIFIED
        if (current_parent_id == 0) {
            sqlite3_bind_null(stmt_select_project_id_, 2); // MODIFIED
        } else {
            sqlite3_bind_int64(stmt_select_project_id_, 2, current_parent_id); // MODIFIED
        }

        long long found_id = 0;
        if (sqlite3_step(stmt_select_project_id_) == SQLITE_ROW) { // MODIFIED
            found_id = sqlite3_column_int64(stmt_select_project_id_, 0); // MODIFIED
        }

        if (found_id > 0) {
            current_parent_id = found_id;
        } else {
            sqlite3_reset(stmt_insert_project_); // MODIFIED
            sqlite3_bind_text(stmt_insert_project_, 1, part.c_str(), -1, SQLITE_TRANSIENT); // MODIFIED
            if (current_parent_id == 0) {
                sqlite3_bind_null(stmt_insert_project_, 2); // MODIFIED
            } else {
                sqlite3_bind_int64(stmt_insert_project_, 2, current_parent_id); // MODIFIED
            }

            if (sqlite3_step(stmt_insert_project_) != SQLITE_DONE) { // MODIFIED
                std::cerr << "Error inserting project: " << sqlite3_errmsg(db_) << std::endl; // MODIFIED
                return 0; 
            }
            current_parent_id = sqlite3_last_insert_rowid(db_); // MODIFIED
        }
        project_path_cache_[current_path] = current_parent_id;
    }

    return current_parent_id;
}