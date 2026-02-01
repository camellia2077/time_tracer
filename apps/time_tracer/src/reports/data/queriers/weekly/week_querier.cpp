// reports/data/queriers/weekly/week_querier.cpp
#include "week_querier.hpp"

#include "reports/data/cache/project_name_cache.hpp"
#include "reports/data/utils/project_tree_builder.hpp"

WeekQuerier::WeekQuerier(sqlite3* sqlite_db, const std::string& iso_week)
    : BaseQuerier(sqlite_db, iso_week) {}

auto WeekQuerier::fetch_data() -> WeeklyReportData {
  WeeklyReportData data = BaseQuerier::fetch_data();

  _fetch_actual_days(data);

  if (data.total_duration > 0) {
    auto& name_cache = ProjectNameCache::instance();
    name_cache.ensure_loaded(db_);

    build_project_tree_from_ids(data.project_tree, data.project_stats,
                                name_cache);
  }

  return data;
}

auto WeekQuerier::_validate_input() const -> bool {
  IsoWeek temp;
  return parse_iso_week(param_, temp);
}

void WeekQuerier::_handle_invalid_input(WeeklyReportData& data) const {
  data.is_valid = false;
}

void WeekQuerier::_prepare_data(WeeklyReportData& data) const {
  IsoWeek week{};
  if (!parse_iso_week(param_, week)) {
    data.is_valid = false;
    return;
  }

  parsed_week_ = week;
  data.range_label = format_iso_week(week);
  constexpr int kDaysInWeek = 7;
  data.requested_days = kDaysInWeek;
  data.start_date = iso_week_start_date(week);
  data.end_date = iso_week_end_date(week);
  data.is_valid = true;

  start_date_ = data.start_date;
  end_date_ = data.end_date;
}

auto WeekQuerier::get_date_condition_sql() const -> std::string {
  return "date >= ? AND date <= ?";
}

void WeekQuerier::bind_sql_parameters(sqlite3_stmt* stmt) const {
  sqlite3_bind_text(stmt, 1, start_date_.c_str(), -1, SQLITE_TRANSIENT);
  sqlite3_bind_text(stmt, 2, end_date_.c_str(), -1, SQLITE_TRANSIENT);
}
