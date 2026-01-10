// reports/monthly/MonthQuerier.cpp
#include "MonthQuerier.hpp"
#include "reports/shared/utils/report/ReportDataUtils.hpp"
#include <algorithm>
#include <cctype>

MonthQuerier::MonthQuerier(sqlite3* db, const std::string& year_month)
    : BaseQuerier(db, year_month) {}

MonthlyReportData MonthQuerier::fetch_data() {
    MonthlyReportData data = BaseQuerier::fetch_data();

    _fetch_actual_days(data);

    if (data.total_duration > 0) {
        build_project_tree_from_records(data.project_tree, data.records);
    }
    
    return data;
}

bool MonthQuerier::_validate_input() const {
    // [核心修改] 验证格式改为 YYYY-MM (长度7, 第5位是横杠)
    if (this->param_.length() != 7) return false;
    if (this->param_[4] != '-') return false;
    
    // 检查其余位是否为数字
    return std::isdigit(this->param_[0]) && std::isdigit(this->param_[1]) &&
           std::isdigit(this->param_[2]) && std::isdigit(this->param_[3]) &&
           std::isdigit(this->param_[5]) && std::isdigit(this->param_[6]);
}

void MonthQuerier::_handle_invalid_input(MonthlyReportData& data) const {
    data.year_month = "INVALID";
}

void MonthQuerier::_prepare_data(MonthlyReportData& data) const {
    data.year_month = param_;
}

std::string MonthQuerier::get_date_condition_sql() const {
    // [核心修改] 数据库存的是 YYYY-MM-DD
    // 我们需要匹配前7位 (YYYY-MM)
    return "SUBSTR(date, 1, 7) = ?";
}

void MonthQuerier::bind_sql_parameters(sqlite3_stmt* stmt) const {
    sqlite3_bind_text(stmt, 1, param_.c_str(), -1, SQLITE_STATIC);
}