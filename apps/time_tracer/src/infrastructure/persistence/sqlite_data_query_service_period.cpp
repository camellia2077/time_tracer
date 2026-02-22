#include <cctype>
#include <chrono>
#include <format>
#include <stdexcept>
#include <string>
#include <string_view>

#include "domain/utils/time_utils.hpp"
#include "infrastructure/persistence/sqlite_data_query_service_internal.hpp"
#include "infrastructure/query/data/data_query_repository.hpp"
#include "shared/utils/period_utils.hpp"

namespace infra_data_query = time_tracer::infrastructure::query::data;

namespace infrastructure::persistence::data_query_service_internal {
namespace {

constexpr int kDefaultTreeLookbackDays = 7;
constexpr size_t kYearMonthTextLength = 7;
constexpr size_t kYearMonthSeparatorIndex = 4;
constexpr int kRecentFallbackYear = 1;
constexpr int kRecentFallbackMonth = 1;
constexpr int kRecentFallbackDay = 1;
constexpr int kLookbackInclusiveOffsetDays = 1;
constexpr int kDecimalBase = 10;

auto ParseUnsigned(std::string_view value, int& out) -> bool {
  if (value.empty()) {
    return false;
  }
  int parsed = 0;
  for (const char kCharacter : value) {
    if (std::isdigit(static_cast<unsigned char>(kCharacter)) == 0) {
      return false;
    }
    parsed = (parsed * kDecimalBase) + (kCharacter - '0');
  }
  out = parsed;
  return true;
}

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
auto ParsePositiveInt(std::string_view value, std::string_view field_name)
    -> int {
  const std::string kTrimmed = TrimCopy(value);
  if (kTrimmed.empty()) {
    throw std::runtime_error(std::string(field_name) + " must not be empty.");
  }
  int parsed = 0;
  if (!ParseUnsigned(kTrimmed, parsed) || parsed <= 0) {
    throw std::runtime_error(std::string(field_name) +
                             " must be a positive integer.");
  }
  return parsed;
}

auto RequireTreePeriodArgument(
    const time_tracer::core::dto::DataQueryRequest& request,
    std::string_view period_name) -> std::string {
  if (!request.tree_period_argument.has_value()) {
    throw std::runtime_error("Missing --period-arg for period '" +
                             std::string(period_name) + "'.");
  }
  const std::string kArg = TrimCopy(*request.tree_period_argument);
  if (kArg.empty()) {
    throw std::runtime_error("Empty --period-arg for period '" +
                             std::string(period_name) + "'.");
  }
  return kArg;
}

auto ApplyTreePeriodDay(const time_tracer::core::dto::DataQueryRequest& request,
                        infra_data_query::QueryFilters& filters) -> void {
  const std::string kArg = RequireTreePeriodArgument(request, "day");
  const std::string kNormalized = NormalizeBoundaryDate(kArg, false);
  filters.from_date = kNormalized;
  filters.to_date = kNormalized;
}

auto ApplyTreePeriodWeek(
    const time_tracer::core::dto::DataQueryRequest& request,
    infra_data_query::QueryFilters& filters) -> void {
  const std::string kArg = RequireTreePeriodArgument(request, "week");
  IsoWeek week{};
  if (!ParseIsoWeek(kArg, week)) {
    throw std::runtime_error(
        "Invalid week period. Use ISO YYYY-Www (e.g., 2026-W05).");
  }
  filters.from_date = IsoWeekStartDate(week);
  filters.to_date = IsoWeekEndDate(week);
}

auto ApplyTreePeriodMonth(
    const time_tracer::core::dto::DataQueryRequest& request,
    infra_data_query::QueryFilters& filters) -> void {
  const std::string kArg = RequireTreePeriodArgument(request, "month");
  const std::string kNormalized = NormalizeToMonthFormat(kArg);
  if (kNormalized.size() != kYearMonthTextLength ||
      kNormalized[kYearMonthSeparatorIndex] != '-') {
    throw std::runtime_error(
        "Invalid month period. Use YYYYMM or YYYY-MM (e.g., 202602).");
  }
  filters.from_date = NormalizeBoundaryDate(kNormalized, false);
  filters.to_date = NormalizeBoundaryDate(kNormalized, true);
}

auto ApplyTreePeriodYear(
    const time_tracer::core::dto::DataQueryRequest& request,
    infra_data_query::QueryFilters& filters) -> void {
  const std::string kArg = RequireTreePeriodArgument(request, "year");
  int year = 0;
  if (!ParseGregorianYear(kArg, year)) {
    throw std::runtime_error(
        "Invalid year period. Use Gregorian YYYY (e.g., 2026).");
  }
  const std::string kYearText = FormatGregorianYear(year);
  filters.from_date = NormalizeBoundaryDate(kYearText, false);
  filters.to_date = NormalizeBoundaryDate(kYearText, true);
}

auto ApplyTreePeriodRecent(
    const time_tracer::core::dto::DataQueryRequest& request, sqlite3* db_conn,
    infra_data_query::QueryFilters& filters) -> void {
  int lookback_days = kDefaultTreeLookbackDays;
  if (request.tree_period_argument.has_value() &&
      !TrimCopy(*request.tree_period_argument).empty()) {
    lookback_days =
        ParsePositiveInt(*request.tree_period_argument, "--period-arg");
  } else if (request.lookback_days.has_value()) {
    lookback_days = ParsePositiveInt(std::to_string(*request.lookback_days),
                                     "--lookback-days");
  }

  const auto kLatestDate = infra_data_query::QueryLatestTrackedDate(db_conn);
  if (!kLatestDate.has_value()) {
    filters.from_date = std::format("{:04d}-{:02d}-{:02d}", kRecentFallbackYear,
                                    kRecentFallbackMonth, kRecentFallbackDay);
    filters.to_date = filters.from_date;
    return;
  }

  const auto kEndYmd = ParseIsoDate(*kLatestDate);
  if (!kEndYmd.has_value()) {
    throw std::runtime_error("Invalid latest tracked date in database: " +
                             *kLatestDate);
  }
  const auto kEndDays = std::chrono::sys_days{*kEndYmd};
  const auto kStartDays =
      kEndDays -
      std::chrono::days{lookback_days - kLookbackInclusiveOffsetDays};
  filters.from_date = FormatIsoDate(std::chrono::year_month_day{kStartDays});
  filters.to_date = *kLatestDate;
}

auto ApplyTreePeriodRange(
    const time_tracer::core::dto::DataQueryRequest& request,
    infra_data_query::QueryFilters& filters) -> void {
  const std::string kArg = RequireTreePeriodArgument(request, "range");
  size_t separator = kArg.find('|');
  if (separator == std::string::npos) {
    separator = kArg.find(',');
  }
  if (separator == std::string::npos) {
    throw std::runtime_error(
        "Invalid range period. Use start|end or start,end.");
  }
  const std::string kStartRaw = TrimCopy(kArg.substr(0, separator));
  const std::string kEndRaw = TrimCopy(kArg.substr(separator + 1));
  if (kStartRaw.empty() || kEndRaw.empty()) {
    throw std::runtime_error("Range start/end must not be empty.");
  }
  filters.from_date = NormalizeBoundaryDate(kStartRaw, false);
  filters.to_date = NormalizeBoundaryDate(kEndRaw, true);
}

}  // namespace

auto ApplyTreePeriod(const time_tracer::core::dto::DataQueryRequest& request,
                     sqlite3* db_conn, infra_data_query::QueryFilters& filters)
    -> void {
  if (!request.tree_period.has_value()) {
    return;
  }

  const std::string kPeriod = TrimCopy(*request.tree_period);
  if (kPeriod.empty()) {
    throw std::runtime_error("--period must not be empty.");
  }

  filters.kYear.reset();
  filters.kMonth.reset();
  filters.from_date.reset();
  filters.to_date.reset();

  if (kPeriod == "day") {
    ApplyTreePeriodDay(request, filters);
    return;
  }

  if (kPeriod == "week") {
    ApplyTreePeriodWeek(request, filters);
    return;
  }

  if (kPeriod == "month") {
    ApplyTreePeriodMonth(request, filters);
    return;
  }

  if (kPeriod == "year") {
    ApplyTreePeriodYear(request, filters);
    return;
  }

  if (kPeriod == "recent") {
    ApplyTreePeriodRecent(request, db_conn, filters);
    return;
  }

  if (kPeriod == "range") {
    ApplyTreePeriodRange(request, filters);
    return;
  }

  throw std::runtime_error(
      "Invalid --period value. Use day/week/month/year/recent/range.");
}

}  // namespace infrastructure::persistence::data_query_service_internal
