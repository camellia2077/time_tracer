// infrastructure/reports/data/queriers/yearly/year_querier.cpp
#include "infrastructure/reports/data/queriers/yearly/year_querier.hpp"

#include <format>

#include "infrastructure/schema/day_schema.hpp"

YearQuerier::YearQuerier(sqlite3* sqlite_db, std::string_view year_str)
    : RangeQuerierBase(sqlite_db, year_str) {}

auto YearQuerier::ValidateInput() const -> bool {
  int gregorian_year = 0;
  return ParseGregorianYear(param_, gregorian_year);
}

void YearQuerier::HandleInvalidInput(YearlyReportData& data) const {
  data.is_valid = false;
}

void YearQuerier::PrepareData(YearlyReportData& data) const {
  int gregorian_year = 0;
  if (!ParseGregorianYear(param_, gregorian_year)) {
    data.is_valid = false;
    return;
  }

  gregorian_year_ = gregorian_year;
  std::string year_str = FormatGregorianYear(gregorian_year);

  data.range_label = year_str;
  data.requested_days = 0;
  data.start_date = year_str + "-01-01";
  data.end_date = year_str + "-12-31";
  data.is_valid = true;

  start_date_ = data.start_date;
  end_date_ = data.end_date;
}

auto YearQuerier::GetDateConditionSql() const -> std::string {
  return std::format("{} >= ? AND {} <= ?", schema::day::db::kDate,
                     schema::day::db::kDate);
}

void YearQuerier::BindSqlParameters(sqlite3_stmt* stmt) const {
  sqlite3_bind_text(stmt, 1, start_date_.c_str(), -1, SQLITE_TRANSIENT);
  sqlite3_bind_text(stmt, 2, end_date_.c_str(), -1, SQLITE_TRANSIENT);
}
