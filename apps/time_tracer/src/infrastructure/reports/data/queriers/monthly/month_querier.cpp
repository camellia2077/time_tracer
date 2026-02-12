// infrastructure/reports/data/queriers/monthly/month_querier.cpp
#include "infrastructure/reports/data/queriers/monthly/month_querier.hpp"

#include <algorithm>
#include <cctype>
#include <format>

#include "infrastructure/schema/day_schema.hpp"

MonthQuerier::MonthQuerier(sqlite3* sqlite_db, std::string_view year_month)
    : RangeQuerierBase(sqlite_db, year_month) {}

auto MonthQuerier::ValidateInput() const -> bool {
  if (this->param_.length() != static_cast<size_t>(kYearMonthLength)) {
    return false;
  }
  if (this->param_[kDashPosition] != '-') {
    return false;
  }

  return (std::isdigit(this->param_[0]) != 0) &&
         (std::isdigit(this->param_[1]) != 0) &&
         (std::isdigit(this->param_[2]) != 0) &&
         (std::isdigit(this->param_[3]) != 0) &&
         (std::isdigit(this->param_[kMonthStartPosition]) != 0) &&
         (std::isdigit(this->param_[kMonthEndPosition]) != 0);
}

void MonthQuerier::HandleInvalidInput(MonthlyReportData& data) const {
  data.is_valid = false;
}

void MonthQuerier::PrepareData(MonthlyReportData& data) const {
  data.range_label = std::string(param_);
  data.start_date = std::string(param_) + "-01";
  data.end_date = std::string(param_) + "-31";
  data.requested_days = 0;
}

auto MonthQuerier::GetDateConditionSql() const -> std::string {
  return std::format("{} >= ? AND {} <= ?", schema::day::db::kDate,
                     schema::day::db::kDate);
}

void MonthQuerier::BindSqlParameters(sqlite3_stmt* stmt) const {
  std::string start_date = std::string(param_) + "-01";
  std::string end_date = std::string(param_) + "-31";

  sqlite3_bind_text(stmt, 1, start_date.c_str(), -1, SQLITE_TRANSIENT);
  sqlite3_bind_text(stmt, 2, end_date.c_str(), -1, SQLITE_TRANSIENT);
}
