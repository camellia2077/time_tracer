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
    : db(db),
      stmt_insert_day(stmt_day),
      stmt_insert_record(stmt_record),
      stmt_select_project_id(stmt_select_project),
      stmt_insert_project(stmt_insert_project) {}

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

        sqlite3_bind_int(stmt_insert_day, 8, day_data.exercise);
        sqlite3_bind_int(stmt_insert_day, 9, day_data.total_exercise_time);
        sqlite3_bind_int(stmt_insert_day, 10, day_data.cardio_time);
        sqlite3_bind_int(stmt_insert_day, 11, day_data.anaerobic_time);

        sqlite3_bind_int(stmt_insert_day, 12, day_data.gaming_time);
        sqlite3_bind_int(stmt_insert_day, 13, day_data.grooming_time);
        sqlite3_bind_int(stmt_insert_day, 14, day_data.toilet_time);

        sqlite3_bind_int(stmt_insert_day, 15, day_data.sleep_night_time);
        sqlite3_bind_int(stmt_insert_day, 16, day_data.sleep_day_time);
        sqlite3_bind_int(stmt_insert_day, 17, day_data.sleep_total_time);

        // --- [新增] 绑定娱乐时间数据 ---
        sqlite3_bind_int(stmt_insert_day, 18, day_data.recreation_time);
        sqlite3_bind_int(stmt_insert_day, 19, day_data.recreation_zhihu_time);
        sqlite3_bind_int(stmt_insert_day, 20, day_data.recreation_bilibili_time);
        sqlite3_bind_int(stmt_insert_day, 21, day_data.recreation_douyin_time);

        if (sqlite3_step(stmt_insert_day) != SQLITE_DONE) {
            std::cerr << "Error inserting day row: " << sqlite3_errmsg(db) << std::endl;
        }
        sqlite3_reset(stmt_insert_day);
    }
}

void DataInserter::insert_records(const std::vector<TimeRecordInternal>& records) {
    for (const auto& record_data : records) {
        long long project_id = get_or_create_project_id(record_data.project_path);

        sqlite3_bind_int64(stmt_insert_record, 1, record_data.logical_id);
        sqlite3_bind_int64(stmt_insert_record, 2, record_data.start_timestamp);
        sqlite3_bind_int64(stmt_insert_record, 3, record_data.end_timestamp);
        sqlite3_bind_text(stmt_insert_record, 4, record_data.date.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt_insert_record, 5, record_data.start.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt_insert_record, 6, record_data.end.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int64(stmt_insert_record, 7, project_id);
        sqlite3_bind_int(stmt_insert_record, 8, record_data.duration_seconds);

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

        sqlite3_reset(stmt_select_project_id);
        sqlite3_bind_text(stmt_select_project_id, 1, part.c_str(), -1, SQLITE_TRANSIENT);
        if (current_parent_id == 0) {
            sqlite3_bind_null(stmt_select_project_id, 2);
        } else {
            sqlite3_bind_int64(stmt_select_project_id, 2, current_parent_id);
        }

        long long found_id = 0;
        if (sqlite3_step(stmt_select_project_id) == SQLITE_ROW) {
            found_id = sqlite3_column_int64(stmt_select_project_id, 0);
        }

        if (found_id > 0) {
            current_parent_id = found_id;
        } else {
            sqlite3_reset(stmt_insert_project);
            sqlite3_bind_text(stmt_insert_project, 1, part.c_str(), -1, SQLITE_TRANSIENT);
            if (current_parent_id == 0) {
                sqlite3_bind_null(stmt_insert_project, 2);
            } else {
                sqlite3_bind_int64(stmt_insert_project, 2, current_parent_id);
            }

            if (sqlite3_step(stmt_insert_project) != SQLITE_DONE) {
                std::cerr << "Error inserting project: " << sqlite3_errmsg(db) << std::endl;
                return 0; 
            }
            current_parent_id = sqlite3_last_insert_rowid(db);
        }
        project_path_cache_[current_path] = current_parent_id;
    }

    return current_parent_id;
}