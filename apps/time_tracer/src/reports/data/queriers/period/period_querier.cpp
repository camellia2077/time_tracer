// reports/data/queriers/period/period_querier.cpp
#include "period_querier.hpp"
#include "reports/shared/utils/format/time_format.hpp"
#include "reports/data/utils/project_tree_builder.hpp" 
#include "reports/data/cache/project_name_cache.hpp"
#include <iomanip>

PeriodQuerier::PeriodQuerier(sqlite3* db, int days_to_query)
    : BaseQuerier(db, days_to_query) {}

PeriodReportData PeriodQuerier::fetch_data() {
    PeriodReportData data = BaseQuerier::fetch_data();

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

bool PeriodQuerier::_validate_input() const {
    return param_ > 0;
}

void PeriodQuerier::_handle_invalid_input(PeriodReportData& data) const {
    data.days_to_query = -1;
}

void PeriodQuerier::_prepare_data(PeriodReportData& data) const {
    end_date_ = get_current_date_str();
    start_date_ = add_days_to_date_str(end_date_, -(this->param_ - 1));

    data.days_to_query = this->param_;
    data.end_date = end_date_;
    data.start_date = start_date_;
}

std::string PeriodQuerier::get_date_condition_sql() const {
    return "date >= ? AND date <= ?";
}

void PeriodQuerier::bind_sql_parameters(sqlite3_stmt* stmt) const {
    sqlite3_bind_text(stmt, 1, start_date_.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, end_date_.c_str(), -1, SQLITE_TRANSIENT);
}