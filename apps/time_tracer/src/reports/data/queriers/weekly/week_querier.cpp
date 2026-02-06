// reports/data/queriers/weekly/week_querier.cpp
#include "reports/data/queriers/weekly/week_querier.hpp"

#include "reports/data/cache/project_name_cache.hpp"
#include "reports/data/utils/project_tree_builder.hpp"

WeekQuerier::WeekQuerier(sqlite3* sqlite_db, std::string_view iso_week)
    : BaseQuerier(sqlite_db, iso_week) {}

auto WeekQuerier::FetchData() -> WeeklyReportData {
  WeeklyReportData data = BaseQuerier::FetchData();

  FetchActualDays(data);

  if (data.total_duration > 0) {
    auto& name_cache = ProjectNameCache::Instance();
    name_cache.EnsureLoaded(db_);

    BuildProjectTreeFromIds(data.project_tree, data.project_stats,
                                name_cache);
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
  return "date >= ? AND date <= ?";
}

void WeekQuerier::BindSqlParameters(sqlite3_stmt* stmt) const {
  sqlite3_bind_text(stmt, 1, start_date_.c_str(), -1, SQLITE_TRANSIENT);
  sqlite3_bind_text(stmt, 2, end_date_.c_str(), -1, SQLITE_TRANSIENT);
}
