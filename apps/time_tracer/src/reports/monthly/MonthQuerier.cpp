// reports/monthly/MonthQuerier.cpp
#include "MonthQuerier.hpp"
#include "reports/shared/utils/tree/ProjectTreeBuilder.hpp"
#include <algorithm>
#include <cctype>

MonthQuerier::MonthQuerier(sqlite3* db, const std::string& year_month)
    : BaseQuerier(db, year_month) {}

MonthlyReportData MonthQuerier::fetch_data() {
    MonthlyReportData data = BaseQuerier::fetch_data();

    _fetch_actual_days(data);

    if (data.total_duration > 0) {
        build_project_tree_from_ids(data.project_tree, data.project_stats, db_);
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
    // [优化] 使用范围查询以利用数据库索引 (Index Range Scan)
    // 替代原来的 SUBSTR(date, 1, 7) = ? (Full Table Scan)
    return "date >= ? AND date <= ?";
}

void MonthQuerier::bind_sql_parameters(sqlite3_stmt* stmt) const {
    // 构造日期范围: "YYYY-MM-01" 到 "YYYY-MM-31"
    // 注意：使用 "-31" 进行字符串比较对于任何月份都是安全的上界，
    // 因为数据库中的有效日期(如 2025-02-28)在字典序上永远小于 2025-02-31。
    std::string start_date = param_ + "-01";
    std::string end_date = param_ + "-31";

    // 使用 SQLITE_TRANSIENT 告知 SQLite 立即拷贝字符串，因为 start_date/end_date 是局部变量
    sqlite3_bind_text(stmt, 1, start_date.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, end_date.c_str(), -1, SQLITE_TRANSIENT);
}