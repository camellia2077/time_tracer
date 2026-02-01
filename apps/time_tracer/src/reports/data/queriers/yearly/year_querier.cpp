// reports/data/queriers/yearly/year_querier.cpp
#include "year_querier.hpp"

#include "reports/data/cache/project_name_cache.hpp"
#include "reports/data/utils/project_tree_builder.hpp"

YearQuerier::YearQuerier(sqlite3* sqlite_db, const std::string& year_str)
    : BaseQuerier(sqlite_db, year_str) {}

auto YearQuerier::fetch_data() -> YearlyReportData {
  YearlyReportData data = BaseQuerier::fetch_data();

  _fetch_actual_days(data);

  if (data.total_duration > 0) {
    auto& name_cache = ProjectNameCache::instance();
    name_cache.ensure_loaded(db_);

    build_project_tree_from_ids(data.project_tree, data.project_stats,
                                name_cache);
  }

  return data;
}

auto YearQuerier::_validate_input() const -> bool {
  int gregorian_year = 0;
  return parse_gregorian_year(param_, gregorian_year);
}

void YearQuerier::_handle_invalid_input(YearlyReportData& data) const {
  data.is_valid = false;
}

void YearQuerier::_prepare_data(YearlyReportData& data) const {
  int gregorian_year = 0;
  if (!parse_gregorian_year(param_, gregorian_year)) {
    data.is_valid = false;
    return;
  }

  gregorian_year_ = gregorian_year;
  std::string year_str = format_gregorian_year(gregorian_year);

  data.range_label = year_str;
  data.requested_days = 0;
  data.start_date = year_str + "-01-01";
  data.end_date = year_str + "-12-31";
  data.is_valid = true;

  start_date_ = data.start_date;
  end_date_ = data.end_date;
}

auto YearQuerier::get_date_condition_sql() const -> std::string {
  return "date >= ? AND date <= ?";
}

void YearQuerier::bind_sql_parameters(sqlite3_stmt* stmt) const {
  sqlite3_bind_text(stmt, 1, start_date_.c_str(), -1, SQLITE_TRANSIENT);
  sqlite3_bind_text(stmt, 2, end_date_.c_str(), -1, SQLITE_TRANSIENT);
}
