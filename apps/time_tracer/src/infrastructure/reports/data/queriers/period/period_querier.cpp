// infrastructure/reports/data/queriers/period/period_querier.cpp
#include "infrastructure/reports/data/queriers/period/period_querier.hpp"

#include <format>
#include <iomanip>
#include <string>

#include "infrastructure/reports/data/cache/project_name_cache.hpp"
#include "infrastructure/reports/data/utils/project_tree_builder.hpp"
#include "infrastructure/reports/shared/utils/format/time_format.hpp"
#include "infrastructure/schema/day_schema.hpp"

PeriodQuerier::PeriodQuerier(sqlite3* sqlite_db, int days_to_query)
    : BaseQuerier(sqlite_db, days_to_query) {}

auto PeriodQuerier::FetchData() -> PeriodReportData {
  PeriodReportData data = BaseQuerier::FetchData();

  FetchActualDays(data);
  auto flag_counts = FetchDayFlagCounts();
  data.status_true_days = flag_counts.status_true_days;
  data.sleep_true_days = flag_counts.sleep_true_days;
  data.exercise_true_days = flag_counts.exercise_true_days;
  data.cardio_true_days = flag_counts.cardio_true_days;
  data.anaerobic_true_days = flag_counts.anaerobic_true_days;

  if (data.total_duration > 0) {
    // [新增] 获取并确保缓存加载
    auto& name_cache = ProjectNameCache::Instance();
    name_cache.EnsureLoaded(db_);

    // [核心修改] 传入 name_cache 替代 db_
    BuildProjectTreeFromIds(data.project_tree, data.project_stats, name_cache);
  }

  return data;
}

auto PeriodQuerier::ValidateInput() const -> bool {
  return param_ > 0;
}

void PeriodQuerier::HandleInvalidInput(PeriodReportData& data) const {
  data.is_valid = false;
}

void PeriodQuerier::PrepareData(PeriodReportData& data) const {
  end_date_ = GetCurrentDateStr();
  start_date_ = AddDaysToDateStr(end_date_, -(this->param_ - 1));

  data.requested_days = this->param_;
  data.end_date = end_date_;
  data.start_date = start_date_;
  data.range_label = std::to_string(this->param_) + " days";
}

auto PeriodQuerier::GetDateConditionSql() const -> std::string {
  return std::format("{} >= ? AND {} <= ?", schema::day::db::kDate,
                     schema::day::db::kDate);
}

void PeriodQuerier::BindSqlParameters(sqlite3_stmt* stmt) const {
  sqlite3_bind_text(stmt, 1, start_date_.c_str(), -1, SQLITE_TRANSIENT);
  sqlite3_bind_text(stmt, 2, end_date_.c_str(), -1, SQLITE_TRANSIENT);
}
