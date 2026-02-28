// infrastructure/query/data/stats/report_chart_stats_calculator.cpp
#include "infrastructure/query/data/stats/report_chart_stats_calculator.hpp"

#include <chrono>
#include <iomanip>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace tracer_core::infrastructure::query::data::stats {
namespace {

constexpr size_t kIsoDateLength = 10;
constexpr size_t kIsoDateYearSeparatorIndex = 4;
constexpr size_t kIsoDateMonthSeparatorIndex = 7;
constexpr size_t kIsoDateYearLength = 4;
constexpr size_t kIsoDateMonthOffset = 5;
constexpr size_t kIsoDateDayOffset = 8;
constexpr size_t kIsoDateMonthDayLength = 2;
constexpr int kDecimalBase = 10;

auto ParseUnsigned(std::string_view value, int& out) -> bool {
  if (value.empty()) {
    return false;
  }
  int parsed = 0;
  for (const char kCharacter : value) {
    if (kCharacter < '0' || kCharacter > '9') {
      return false;
    }
    parsed = (parsed * kDecimalBase) + (kCharacter - '0');
  }
  out = parsed;
  return true;
}

auto ParseIsoDate(std::string_view value)
    -> std::optional<std::chrono::year_month_day> {
  if (value.size() != kIsoDateLength ||
      value[kIsoDateYearSeparatorIndex] != '-' ||
      value[kIsoDateMonthSeparatorIndex] != '-') {
    return std::nullopt;
  }
  int year = 0;
  int month = 0;
  int day = 0;
  if (!ParseUnsigned(value.substr(0, kIsoDateYearLength), year) ||
      !ParseUnsigned(value.substr(kIsoDateMonthOffset, kIsoDateMonthDayLength),
                     month) ||
      !ParseUnsigned(value.substr(kIsoDateDayOffset, kIsoDateMonthDayLength),
                     day)) {
    return std::nullopt;
  }
  const std::chrono::year_month_day kYmd{
      std::chrono::year{year}, std::chrono::month{static_cast<unsigned>(month)},
      std::chrono::day{static_cast<unsigned>(day)}};
  if (!kYmd.ok()) {
    return std::nullopt;
  }
  return kYmd;
}

auto FormatIsoDate(const std::chrono::year_month_day& ymd) -> std::string {
  std::ostringstream stream;
  stream << std::setw(4) << std::setfill('0') << int(ymd.year()) << "-"
         << std::setw(2) << std::setfill('0') << unsigned(ymd.month()) << "-"
         << std::setw(2) << std::setfill('0') << unsigned(ymd.day());
  return stream.str();
}

auto BuildTotalsByDate(const std::vector<DayDurationRow>& sparse_rows)
    -> std::unordered_map<std::string, long long> {
  std::unordered_map<std::string, long long> totals_by_date;
  totals_by_date.reserve(sparse_rows.size());
  for (const auto& row : sparse_rows) {
    totals_by_date[row.date] = row.total_seconds;
  }
  return totals_by_date;
}

}  // namespace

auto BuildReportChartSeries(std::string_view start_date,
                            std::string_view end_date,
                            const std::vector<DayDurationRow>& sparse_rows)
    -> ReportChartSeriesResult {
  const auto kStartYmd = ParseIsoDate(start_date);
  const auto kEndYmd = ParseIsoDate(end_date);
  if (!kStartYmd.has_value() || !kEndYmd.has_value()) {
    throw std::runtime_error("report-chart resolved invalid date range.");
  }

  const auto kStartDays = std::chrono::sys_days{*kStartYmd};
  const auto kEndDays = std::chrono::sys_days{*kEndYmd};
  if (kStartDays > kEndDays) {
    throw std::runtime_error(
        "report-chart invalid range: from_date must be <= to_date.");
  }

  const auto kTotalsByDate = BuildTotalsByDate(sparse_rows);

  ReportChartSeriesResult result;
  result.series.reserve(
      static_cast<size_t>((kEndDays - kStartDays).count() + 1));
  for (auto cursor = kStartDays; cursor <= kEndDays;
       cursor += std::chrono::days{1}) {
    const std::string kDate =
        FormatIsoDate(std::chrono::year_month_day{cursor});
    const auto kIt = kTotalsByDate.find(kDate);
    const long long kDurationSeconds =
        kIt == kTotalsByDate.end() ? 0LL : kIt->second;
    const long long kEpochDay =
        static_cast<long long>(cursor.time_since_epoch().count());
    result.stats.total_duration_seconds += kDurationSeconds;
    ++result.stats.range_days;
    if (kDurationSeconds > 0) {
      ++result.stats.active_days;
    }
    result.series.push_back(ReportChartSeriesPoint{
        .date = kDate,
        .duration_seconds = kDurationSeconds,
        .epoch_day = kEpochDay,
    });
  }

  if (result.stats.range_days > 0) {
    result.stats.average_duration_seconds =
        result.stats.total_duration_seconds /
        static_cast<long long>(result.stats.range_days);
  }

  return result;
}

}  // namespace tracer_core::infrastructure::query::data::stats
