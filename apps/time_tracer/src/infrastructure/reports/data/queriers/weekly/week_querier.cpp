// infrastructure/reports/data/queriers/weekly/week_querier.cpp
#include "infrastructure/reports/data/queriers/weekly/week_querier.hpp"

#include <format>

#include "infrastructure/reports/data/cache/project_name_cache.hpp"
#include "infrastructure/reports/data/utils/project_tree_builder.hpp"
#include "infrastructure/schema/day_schema.hpp"

WeekQuerier::WeekQuerier(sqlite3* sqlite_db, std::string_view iso_week)
    : BaseQuerier(sqlite_db, iso_week) {}

auto WeekQuerier::FetchData() -> WeeklyReportData {
  WeeklyReportData data = BaseQuerier::FetchData();

  FetchActualDays(data);
  auto flag_counts = FetchDayFlagCounts();
  data.status_true_days = flag_counts.status_true_days;
  data.sleep_true_days = flag_counts.sleep_true_days;
  data.exercise_true_days = flag_counts.exercise_true_days;
  data.cardio_true_days = flag_counts.cardio_true_days;
  data.anaerobic_true_days = flag_counts.anaerobic_true_days;

  if (data.total_duration > 0) {
    auto& name_cache = ProjectNameCache::Instance();
    name_cache.EnsureLoaded(db_);

    BuildProjectTreeFromIds(data.project_tree, data.project_stats, name_cache);
  }

  return data;
}

auto WeekQuerier::ValidateInput() const -> bool {
  IsoWeek temp;
  return ParseIsoWeek(param_, temp);
}

void WeekQuerier::HandleInvalidInput(WeeklyReportData& data) const {
  data.is_valid = false;
}

void WeekQuerier::PrepareData(WeeklyReportData& data) const {
  IsoWeek week{};
  if (!ParseIsoWeek(param_, week)) {
    data.is_valid = false;
    return;
  }

  parsed_week_ = week;
  data.range_label = FormatIsoWeek(week);
  constexpr int kDaysInWeek = 7;
  data.requested_days = kDaysInWeek;
  data.start_date = IsoWeekStartDate(week);
  data.end_date = IsoWeekEndDate(week);
  data.is_valid = true;

  start_date_ = data.start_date;
  end_date_ = data.end_date;
}

auto WeekQuerier::GetDateConditionSql() const -> std::string {
  return std::format("{} >= ? AND {} <= ?", schema::day::db::kDate,
                     schema::day::db::kDate);
}

void WeekQuerier::BindSqlParameters(sqlite3_stmt* stmt) const {
  sqlite3_bind_text(stmt, 1, start_date_.c_str(), -1, SQLITE_TRANSIENT);
  sqlite3_bind_text(stmt, 2, end_date_.c_str(), -1, SQLITE_TRANSIENT);
}
