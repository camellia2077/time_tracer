// reports/period/PeriodQuerier.cpp
#include "PeriodQuerier.hpp"
#include "reports/shared/utils/format/TimeFormat.hpp"      // [修改] 引用了功能更匹配的头文件
#include "reports/shared/utils/report/ReportDataUtils.hpp" 
#include <iomanip>

PeriodQuerier::PeriodQuerier(sqlite3* db, int days_to_query)
    : BaseQuerier(db, days_to_query) {}

PeriodReportData PeriodQuerier::fetch_data() {
    // 调用基类实现获取 records 和 total_duration
    PeriodReportData data = BaseQuerier::fetch_data();

    // 调用此查询器特有的方法
    _fetch_actual_days(data);

    // [核心修正] 在数据获取阶段构建项目树
    // 如果有记录，就从 records 中构建 project_tree
    if (data.total_duration > 0) {
        build_project_tree_from_records(data.project_tree, data.records);
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