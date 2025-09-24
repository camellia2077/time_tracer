// queries/daily/DayQuerier.cpp
#include "DayQuerier.hpp"
#include "queries/shared/utils/report/ReportDataUtils.hpp" // [新增] 为构建项目树
#include <stdexcept>

DayQuerier::DayQuerier(sqlite3* db, const std::string& date)
    : BaseQuerier(db, date) {}

DailyReportData DayQuerier::fetch_data() {
    // 调用基类实现获取 records 和 total_duration
    DailyReportData data = BaseQuerier::fetch_data();

    // 获取日报特有的数据
    _fetch_metadata(data);
    
    if (data.total_duration > 0) {
        _fetch_detailed_records(data); 
        // --- [核心修改] 调用更新后的函数 ---
        _fetch_generated_stats(data); 
        // [核心修改] 在数据获取阶段构建项目树
        build_project_tree_from_records(data.project_tree, data.records);
    }
    
    return data;
}

std::string DayQuerier::get_date_condition_sql() const {
    return "date = ?";
}

void DayQuerier::bind_sql_parameters(sqlite3_stmt* stmt) const {
    sqlite3_bind_text(stmt, 1, param_.c_str(), -1, SQLITE_STATIC);
}

void DayQuerier::_prepare_data(DailyReportData& data) const {
    data.date = this->param_;
}

void DayQuerier::_fetch_metadata(DailyReportData& data) {
    sqlite3_stmt* stmt;
    std::string sql = "SELECT status, sleep, remark, getup_time, exercise FROM days WHERE date = ?;";
    if (sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, param_.c_str(), -1, SQLITE_STATIC);
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            data.metadata.status = std::to_string(sqlite3_column_int(stmt, 0));
            data.metadata.sleep = std::to_string(sqlite3_column_int(stmt, 1));
            const unsigned char* r = sqlite3_column_text(stmt, 2);
            if (r) data.metadata.remark = reinterpret_cast<const char*>(r);
            const unsigned char* g = sqlite3_column_text(stmt, 3);
            if (g) data.metadata.getup_time = reinterpret_cast<const char*>(g);
            data.metadata.exercise = std::to_string(sqlite3_column_int(stmt, 4));
        }
    }
    sqlite3_finalize(stmt);
}

void DayQuerier::_fetch_detailed_records(DailyReportData& data) {
    sqlite3_stmt* stmt;
    std::string sql = R"(
        WITH RECURSIVE project_paths(id, path) AS (
            SELECT id, name FROM projects WHERE parent_id IS NULL
            UNION ALL
            SELECT p.id, pp.path || '_' || p.name
            FROM projects p
            JOIN project_paths pp ON p.parent_id = pp.id
        )
        SELECT tr.start, tr.end, pp.path, tr.duration, tr.activity_remark 
        FROM time_records tr
        JOIN project_paths pp ON tr.project_id = pp.id
        WHERE tr.date = ? 
        ORDER BY tr.logical_id ASC;
    )";
    if (sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, param_.c_str(), -1, SQLITE_STATIC);
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            TimeRecord record;
            record.start_time = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
            record.end_time = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            record.project_path = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
            record.duration_seconds = sqlite3_column_int64(stmt, 3);
            const unsigned char* remark_text = sqlite3_column_text(stmt, 4);
            if (remark_text) {
                record.activityRemark = reinterpret_cast<const char*>(remark_text);
            }
            data.detailed_records.push_back(record);
        }
    }
    sqlite3_finalize(stmt);
}

// --- [ 核心修改 ] ---
// 扩展SQL查询以包含所有新的娱乐时间统计字段。
void DayQuerier::_fetch_generated_stats(DailyReportData& data) {
    sqlite3_stmt* stmt;
    std::string sql = "SELECT sleepTotalTime, anaerobic_time, cardio_time, grooming_time, "
                      "recreationTime, recreationZhihuTime, recreationBilibiliTime, recreationDouyinTime "
                      "FROM days WHERE date = ?;";

    if (sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, param_.c_str(), -1, SQLITE_STATIC);
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            if (sqlite3_column_type(stmt, 0) != SQLITE_NULL) data.sleep_time = sqlite3_column_int64(stmt, 0);
            if (sqlite3_column_type(stmt, 1) != SQLITE_NULL) data.anaerobic_time = sqlite3_column_int64(stmt, 1);
            if (sqlite3_column_type(stmt, 2) != SQLITE_NULL) data.cardio_time = sqlite3_column_int64(stmt, 2);
            if (sqlite3_column_type(stmt, 3) != SQLITE_NULL) data.grooming_time = sqlite3_column_int64(stmt, 3);
            if (sqlite3_column_type(stmt, 4) != SQLITE_NULL) data.recreation_time = sqlite3_column_int64(stmt, 4);
            if (sqlite3_column_type(stmt, 5) != SQLITE_NULL) data.recreation_zhihu_time = sqlite3_column_int64(stmt, 5);
            if (sqlite3_column_type(stmt, 6) != SQLITE_NULL) data.recreation_bilibili_time = sqlite3_column_int64(stmt, 6);
            if (sqlite3_column_type(stmt, 7) != SQLITE_NULL) data.recreation_douyin_time = sqlite3_column_int64(stmt, 7);
        }
    }
    sqlite3_finalize(stmt);
}