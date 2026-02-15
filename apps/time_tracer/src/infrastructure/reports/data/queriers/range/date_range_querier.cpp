#include "infrastructure/reports/data/queriers/range/date_range_querier.hpp"

#include <cctype>
#include <chrono>
#include <format>
#include <limits>
#include <string_view>

#include "infrastructure/schema/day_schema.hpp"

namespace {

auto ParseUnsigned(std::string_view value, int& out_value) -> bool {
  if (value.empty()) {
    return false;
  }
  int parsed = 0;
  for (const char character : value) {
    if (std::isdigit(static_cast<unsigned char>(character)) == 0) {
      return false;
    }
    parsed = parsed * 10 + (character - '0');
  }
  out_value = parsed;
  return true;
}

auto ParseIsoDate(std::string_view value, std::chrono::year_month_day& out_ymd)
    -> bool {
  if (value.size() != 10 || value[4] != '-' || value[7] != '-') {
    return false;
  }

  int year = 0;
  int month = 0;
  int day = 0;
  if (!ParseUnsigned(value.substr(0, 4), year) ||
      !ParseUnsigned(value.substr(5, 2), month) ||
      !ParseUnsigned(value.substr(8, 2), day)) {
    return false;
  }

  const std::chrono::year_month_day ymd(
      std::chrono::year(year), std::chrono::month(static_cast<unsigned>(month)),
      std::chrono::day(static_cast<unsigned>(day)));
  if (!ymd.ok()) {
    return false;
  }
  out_ymd = ymd;
  return true;
}

}  // namespace

DateRangeQuerier::DateRangeQuerier(sqlite3* sqlite_db,
                                   std::string_view start_date,
                                   std::string_view end_date)
    : RangeQuerierBase(
          sqlite_db, DateRangeQueryParam{.start_date = std::string(start_date),
                                         .end_date = std::string(end_date)}) {}

auto DateRangeQuerier::ValidateInput() const -> bool {
  int requested_days = 0;
  return TryBuildRequestedDays(requested_days);
}

void DateRangeQuerier::HandleInvalidInput(PeriodReportData& data) const {
  data.range_label = this->param_.start_date + " to " + this->param_.end_date;
  data.start_date = this->param_.start_date;
  data.end_date = this->param_.end_date;
  data.is_valid = false;
}

void DateRangeQuerier::PrepareData(PeriodReportData& data) const {
  int requested_days = 0;
  if (!TryBuildRequestedDays(requested_days)) {
    data.is_valid = false;
    return;
  }

  data.range_label = this->param_.start_date + " to " + this->param_.end_date;
  data.start_date = this->param_.start_date;
  data.end_date = this->param_.end_date;
  data.requested_days = requested_days;
  data.is_valid = true;
}

auto DateRangeQuerier::GetDateConditionSql() const -> std::string {
  return std::format("{} >= ? AND {} <= ?", schema::day::db::kDate,
                     schema::day::db::kDate);
}

void DateRangeQuerier::BindSqlParameters(sqlite3_stmt* stmt) const {
  sqlite3_bind_text(stmt, 1, this->param_.start_date.c_str(), -1,
                    SQLITE_TRANSIENT);
  sqlite3_bind_text(stmt, 2, this->param_.end_date.c_str(), -1,
                    SQLITE_TRANSIENT);
}

auto DateRangeQuerier::TryBuildRequestedDays(int& requested_days) const
    -> bool {
  std::chrono::year_month_day start_ymd;
  std::chrono::year_month_day end_ymd;
  if (!ParseIsoDate(this->param_.start_date, start_ymd) ||
      !ParseIsoDate(this->param_.end_date, end_ymd)) {
    return false;
  }

  const auto start_days = std::chrono::sys_days(start_ymd);
  const auto end_days = std::chrono::sys_days(end_ymd);
  if (start_days > end_days) {
    return false;
  }

  const auto span_days = (end_days - start_days).count() + 1;
  if (span_days > std::numeric_limits<int>::max()) {
    return false;
  }

  requested_days = static_cast<int>(span_days);
  return true;
}
