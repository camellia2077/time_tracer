// reports/monthly/queriers/MonthQuerier.cpp
#include "MonthQuerier.hpp"
#include "reports/shared/utils/tree/ProjectTreeBuilder.hpp"
#include "reports/shared/cache/ProjectNameCache.hpp" // [新增]
#include <algorithm>
#include <cctype>

MonthQuerier::MonthQuerier(sqlite3* db, const std::string& year_month)
    : BaseQuerier(db, year_month) {}

MonthlyReportData MonthQuerier::fetch_data() {
    MonthlyReportData data = BaseQuerier::fetch_data();

    _fetch_actual_days(data);

    if (data.total_duration > 0) {
        // [新增] 获取并确保缓存加载
        auto& name_cache = ProjectNameCache::instance();
        name_cache.ensure_loaded(db_);

        // [核心修改] 传入 name_cache 替代 db_
        build_project_tree_from_ids(data.project_tree, data.project_stats, name_cache);
    }
    
    return data;
}

bool MonthQuerier::_validate_input() const {
    if (this->param_.length() != 7) return false;
    if (this->param_[4] != '-') return false;
    
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
    return "date >= ? AND date <= ?";
}

void MonthQuerier::bind_sql_parameters(sqlite3_stmt* stmt) const {
    std::string start_date = param_ + "-01";
    std::string end_date = param_ + "-31";

    sqlite3_bind_text(stmt, 1, start_date.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, end_date.c_str(), -1, SQLITE_TRANSIENT);
}