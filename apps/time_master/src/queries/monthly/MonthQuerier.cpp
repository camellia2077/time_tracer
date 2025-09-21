// queries/monthly/MonthQuerier.cpp
#include "MonthQuerier.hpp"
#include "queries/shared/utils/report/ReportDataUtils.hpp" // [新增] 为构建项目树
#include <algorithm>
#include <cctype>

MonthQuerier::MonthQuerier(sqlite3* db, const std::string& year_month)
    : BaseQuerier(db, year_month) {}

MonthlyReportData MonthQuerier::fetch_data() {
    MonthlyReportData data = BaseQuerier::fetch_data();

    _fetch_actual_days(data);

    if (data.total_duration > 0) {
        // [核心修改] 在数据获取阶段构建项目树
        build_project_tree_from_records(data.project_tree, data.records);
    }
    
    return data;
}

bool MonthQuerier::_validate_input() const {
    return this->param_.length() == 6 && std::all_of(this->param_.begin(), this->param_.end(), ::isdigit);
}

void MonthQuerier::_handle_invalid_input(MonthlyReportData& data) const {
    data.year_month = "INVALID";
}

void MonthQuerier::_prepare_data(MonthlyReportData& data) const {
    data.year_month = param_;
}

std::string MonthQuerier::get_date_condition_sql() const {
    return "SUBSTR(date, 1, 6) = ?";
}

void MonthQuerier::bind_sql_parameters(sqlite3_stmt* stmt) const {
    sqlite3_bind_text(stmt, 1, param_.c_str(), -1, SQLITE_STATIC);
}