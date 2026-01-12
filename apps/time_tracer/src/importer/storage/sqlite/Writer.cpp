// importer/storage/sqlite/Writer.cpp
#include "Writer.hpp"
#include <iostream>

Writer::Writer(sqlite3* db,
               sqlite3_stmt* stmt_day,
               sqlite3_stmt* stmt_record,
               sqlite3_stmt* stmt_select_project,
               sqlite3_stmt* stmt_insert_project)
    : db_(db), 
      stmt_insert_day_(stmt_day),
      stmt_insert_record_(stmt_record),
      stmt_select_project_id_(stmt_select_project),
      stmt_insert_project_(stmt_insert_project) 
{
    // [核心修改] 初始化解析器委托对象
    project_resolver_ = std::make_unique<ProjectResolver>(db_, stmt_insert_project_);
}

Writer::~Writer() = default;

void Writer::insert_days(const std::vector<DayData>& days) {
    // 这部分逻辑未变，保持原样即可
    for (const auto& day_data : days) {
        sqlite3_bind_text(stmt_insert_day_, 1, day_data.date.c_str(), -1, SQLITE_TRANSIENT); 
        sqlite3_bind_int(stmt_insert_day_, 2, day_data.year); 
        sqlite3_bind_int(stmt_insert_day_, 3, day_data.month); 
        sqlite3_bind_int(stmt_insert_day_, 4, day_data.status); 
        sqlite3_bind_int(stmt_insert_day_, 5, day_data.sleep); 
        sqlite3_bind_text(stmt_insert_day_, 6, day_data.remark.c_str(), -1, SQLITE_TRANSIENT); 

        if (day_data.getup_time == "Null" || day_data.getup_time.empty()) {
            sqlite3_bind_null(stmt_insert_day_, 7); 
        } else {
            sqlite3_bind_text(stmt_insert_day_, 7, day_data.getup_time.c_str(), -1, SQLITE_TRANSIENT); 
        }

        sqlite3_bind_int(stmt_insert_day_, 8, day_data.exercise); 
        sqlite3_bind_int(stmt_insert_day_, 9, day_data.stats.total_exercise_time); 
        sqlite3_bind_int(stmt_insert_day_, 10, day_data.stats.cardio_time); 
        sqlite3_bind_int(stmt_insert_day_, 11, day_data.stats.anaerobic_time); 
        sqlite3_bind_int(stmt_insert_day_, 12, day_data.stats.gaming_time); 
        sqlite3_bind_int(stmt_insert_day_, 13, day_data.stats.grooming_time); 
        sqlite3_bind_int(stmt_insert_day_, 14, day_data.stats.toilet_time); 
        sqlite3_bind_int(stmt_insert_day_, 15, day_data.stats.sleep_night_time); 
        sqlite3_bind_int(stmt_insert_day_, 16, day_data.stats.sleep_day_time); 
        sqlite3_bind_int(stmt_insert_day_, 17, day_data.stats.sleep_total_time); 
        sqlite3_bind_int(stmt_insert_day_, 18, day_data.stats.recreation_time); 
        sqlite3_bind_int(stmt_insert_day_, 19, day_data.stats.recreation_zhihu_time); 
        sqlite3_bind_int(stmt_insert_day_, 20, day_data.stats.recreation_bilibili_time); 
        sqlite3_bind_int(stmt_insert_day_, 21, day_data.stats.recreation_douyin_time); 
        sqlite3_bind_int(stmt_insert_day_, 22, day_data.stats.study_time);

        if (sqlite3_step(stmt_insert_day_) != SQLITE_DONE) { 
            std::cerr << "Error inserting day row: " << sqlite3_errmsg(db_) << std::endl; 
        }
        sqlite3_reset(stmt_insert_day_); 
    }
}

void Writer::insert_records(const std::vector<TimeRecordInternal>& records) {
    if (records.empty()) return;

    // 1. 收集需要解析的路径 (Adapter: Struct -> String)
    std::vector<std::string> paths;
    paths.reserve(records.size());
    for (const auto& record : records) {
        paths.push_back(record.project_path);
    }

    // 2. 委托 Resolver 批量预处理 (建立缓存)
    project_resolver_->preload_and_resolve(paths);

    // 3. 极速插入循环
    for (const auto& record_data : records) {
        // 直接从 Resolver 获取 ID
        long long project_id = project_resolver_->get_id(record_data.project_path);

        sqlite3_bind_int64(stmt_insert_record_, 1, record_data.logical_id);
        sqlite3_bind_int64(stmt_insert_record_, 2, record_data.start_timestamp);
        sqlite3_bind_int64(stmt_insert_record_, 3, record_data.end_timestamp);
        sqlite3_bind_text(stmt_insert_record_, 4, record_data.date.c_str(), -1, SQLITE_TRANSIENT);
        
        sqlite3_bind_text(stmt_insert_record_, 5, record_data.start_time_str.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt_insert_record_, 6, record_data.end_time_str.c_str(), -1, SQLITE_TRANSIENT);
        
        sqlite3_bind_int64(stmt_insert_record_, 7, project_id);
        sqlite3_bind_int(stmt_insert_record_, 8, record_data.duration_seconds);

        if (record_data.remark.has_value()) { 
            sqlite3_bind_text(stmt_insert_record_, 9, record_data.remark->c_str(), -1, SQLITE_TRANSIENT); 
        } else {
            sqlite3_bind_null(stmt_insert_record_, 9);
        }

        if (sqlite3_step(stmt_insert_record_) != SQLITE_DONE) {
            std::cerr << "Error inserting record row: " << sqlite3_errmsg(db_) << std::endl;
        }
        sqlite3_reset(stmt_insert_record_);
    }
}