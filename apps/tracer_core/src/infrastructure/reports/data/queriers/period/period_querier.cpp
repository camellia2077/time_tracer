// infrastructure/reports/data/queriers/period/period_querier.cpp
#include "infrastructure/reports/data/queriers/period/period_querier.hpp"

#include <format>
#include <iomanip>
#include <string>

#include "infrastructure/reports/shared/utils/format/time_format.hpp"
#include "infrastructure/schema/day_schema.hpp"

PeriodQuerier::PeriodQuerier(
    sqlite3* sqlite_db, int days_to_query,
    const tracer_core::application::ports::IPlatformClock& platform_clock)
    : RangeQuerierBase(sqlite_db, days_to_query),
      platform_clock_(platform_clock) {}

auto PeriodQuerier::ValidateInput() const -> bool {
  return param_ > 0;
}

void PeriodQuerier::HandleInvalidInput(PeriodReportData& data) const {
  data.is_valid = false;
}

void PeriodQuerier::PrepareData(PeriodReportData& data) const {
  end_date_ = platform_clock_.TodayLocalDateIso();
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
