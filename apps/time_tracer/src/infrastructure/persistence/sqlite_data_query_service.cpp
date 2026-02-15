// infrastructure/persistence/sqlite_data_query_service.cpp
#include "infrastructure/persistence/sqlite_data_query_service.hpp"

#include <cctype>
#include <chrono>
#include <iomanip>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

#include "domain/utils/time_utils.hpp"
#include "infrastructure/persistence/sqlite/db_manager.hpp"
#include "infrastructure/query/data/data_query_models.hpp"
#include "infrastructure/query/data/data_query_repository.hpp"
#include "shared/utils/period_utils.hpp"

namespace infra_data_query = time_tracer::infrastructure::query::data;

namespace {

constexpr int kDefaultSuggestLookbackDays = 10;
constexpr int kDefaultSuggestLimit = 5;
constexpr int kDefaultTreeLookbackDays = 7;
constexpr int kMinUnlimitedDepth = -1;

auto TrimCopy(std::string_view value) -> std::string {
  size_t begin = 0;
  size_t end = value.size();
  while (begin < value.size() &&
         std::isspace(static_cast<unsigned char>(value[begin])) != 0) {
    ++begin;
  }
  while (end > begin &&
         std::isspace(static_cast<unsigned char>(value[end - 1])) != 0) {
    --end;
  }
  return std::string(value.substr(begin, end - begin));
}

auto ParseUnsigned(std::string_view value, int& out) -> bool {
  if (value.empty()) {
    return false;
  }
  int parsed = 0;
  for (char character : value) {
    if (std::isdigit(static_cast<unsigned char>(character)) == 0) {
      return false;
    }
    parsed = parsed * 10 + (character - '0');
  }
  out = parsed;
  return true;
}

auto ParseIsoDate(std::string_view value)
    -> std::optional<std::chrono::year_month_day> {
  if (value.size() != 10 || value[4] != '-' || value[7] != '-') {
    return std::nullopt;
  }
  int year = 0;
  int month = 0;
  int day = 0;
  if (!ParseUnsigned(value.substr(0, 4), year) ||
      !ParseUnsigned(value.substr(5, 2), month) ||
      !ParseUnsigned(value.substr(8, 2), day)) {
    return std::nullopt;
  }
  const std::chrono::year_month_day ymd{
      std::chrono::year{year}, std::chrono::month{static_cast<unsigned>(month)},
      std::chrono::day{static_cast<unsigned>(day)}};
  if (!ymd.ok()) {
    return std::nullopt;
  }
  return ymd;
}

auto FormatIsoDate(const std::chrono::year_month_day& ymd) -> std::string {
  std::ostringstream stream;
  stream << std::setw(4) << std::setfill('0') << int(ymd.year()) << "-"
         << std::setw(2) << std::setfill('0') << unsigned(ymd.month()) << "-"
         << std::setw(2) << std::setfill('0') << unsigned(ymd.day());
  return stream.str();
}

auto ParsePositiveInt(std::string_view value, std::string_view field_name)
    -> int {
  const std::string trimmed = TrimCopy(value);
  if (trimmed.empty()) {
    throw std::runtime_error(std::string(field_name) + " must not be empty.");
  }
  int parsed = 0;
  if (!ParseUnsigned(trimmed, parsed) || parsed <= 0) {
    throw std::runtime_error(std::string(field_name) +
                             " must be a positive integer.");
  }
  return parsed;
}

auto NormalizeBoundaryDate(std::string_view input, bool is_end) -> std::string {
  const std::string trimmed = TrimCopy(input);
  std::string digits;
  digits.reserve(trimmed.size());
  for (char character : trimmed) {
    if (std::isdigit(static_cast<unsigned char>(character)) != 0) {
      digits.push_back(character);
    }
  }

  if (digits.size() == 4) {
    int year = 0;
    if (!ParseUnsigned(digits, year)) {
      throw std::runtime_error("Invalid year boundary: " + trimmed);
    }
    if (is_end) {
      return std::to_string(year) + "-12-31";
    }
    return std::to_string(year) + "-01-01";
  }

  if (digits.size() == 6) {
    int year = 0;
    int month = 0;
    if (!ParseUnsigned(std::string_view(digits).substr(0, 4), year) ||
        !ParseUnsigned(std::string_view(digits).substr(4, 2), month) ||
        month < 1 || month > 12) {
      throw std::runtime_error("Invalid month boundary: " + trimmed);
    }
    const auto ym = std::chrono::year{year} /
                    std::chrono::month{static_cast<unsigned>(month)};
    if (is_end) {
      const auto ymd_last = std::chrono::year_month_day_last{
          ym.year(), std::chrono::month_day_last{ym.month()}};
      return FormatIsoDate(
          std::chrono::year_month_day{std::chrono::sys_days{ymd_last}});
    }
    return FormatIsoDate(std::chrono::year_month_day{ym.year(), ym.month(),
                                                     std::chrono::day{1}});
  }

  if (digits.size() == 8) {
    const std::string iso = NormalizeToDateFormat(digits);
    if (!ParseIsoDate(iso).has_value()) {
      throw std::runtime_error("Invalid date boundary: " + trimmed);
    }
    return iso;
  }

  if (trimmed.size() == 10 && trimmed[4] == '-' && trimmed[7] == '-') {
    if (!ParseIsoDate(trimmed).has_value()) {
      throw std::runtime_error("Invalid ISO date boundary: " + trimmed);
    }
    return trimmed;
  }

  throw std::runtime_error(
      "Invalid date boundary. Use YYYY, YYYYMM, YYYYMMDD or YYYY-MM-DD.");
}

void ApplyTreePeriod(time_tracer::core::dto::DataQueryRequest request,
                     sqlite3* db_conn,
                     infra_data_query::QueryFilters& filters) {
  if (!request.tree_period.has_value()) {
    return;
  }

  const std::string period = TrimCopy(*request.tree_period);
  if (period.empty()) {
    throw std::runtime_error("--period must not be empty.");
  }

  filters.kYear.reset();
  filters.kMonth.reset();
  filters.from_date.reset();
  filters.to_date.reset();

  auto require_argument = [&](std::string_view period_name) -> std::string {
    if (!request.tree_period_argument.has_value()) {
      throw std::runtime_error("Missing --period-arg for period '" +
                               std::string(period_name) + "'.");
    }
    const std::string arg = TrimCopy(*request.tree_period_argument);
    if (arg.empty()) {
      throw std::runtime_error("Empty --period-arg for period '" +
                               std::string(period_name) + "'.");
    }
    return arg;
  };

  if (period == "day") {
    const std::string arg = require_argument("day");
    const std::string normalized = NormalizeBoundaryDate(arg, false);
    filters.from_date = normalized;
    filters.to_date = normalized;
    return;
  }

  if (period == "week") {
    const std::string arg = require_argument("week");
    IsoWeek week{};
    if (!ParseIsoWeek(arg, week)) {
      throw std::runtime_error(
          "Invalid week period. Use ISO YYYY-Www (e.g., 2026-W05).");
    }
    filters.from_date = IsoWeekStartDate(week);
    filters.to_date = IsoWeekEndDate(week);
    return;
  }

  if (period == "month") {
    const std::string arg = require_argument("month");
    const std::string normalized = NormalizeToMonthFormat(arg);
    if (normalized.size() != 7 || normalized[4] != '-') {
      throw std::runtime_error(
          "Invalid month period. Use YYYYMM or YYYY-MM (e.g., 202602).");
    }
    filters.from_date = NormalizeBoundaryDate(normalized, false);
    filters.to_date = NormalizeBoundaryDate(normalized, true);
    return;
  }

  if (period == "year") {
    const std::string arg = require_argument("year");
    int year = 0;
    if (!ParseGregorianYear(arg, year)) {
      throw std::runtime_error(
          "Invalid year period. Use Gregorian YYYY (e.g., 2026).");
    }
    const std::string year_text = FormatGregorianYear(year);
    filters.from_date = NormalizeBoundaryDate(year_text, false);
    filters.to_date = NormalizeBoundaryDate(year_text, true);
    return;
  }

  if (period == "recent") {
    int lookback_days = kDefaultTreeLookbackDays;
    if (request.tree_period_argument.has_value() &&
        !TrimCopy(*request.tree_period_argument).empty()) {
      lookback_days =
          ParsePositiveInt(*request.tree_period_argument, "--period-arg");
    } else if (request.lookback_days.has_value()) {
      lookback_days = ParsePositiveInt(std::to_string(*request.lookback_days),
                                       "--lookback-days");
    }

    const auto latest_date = infra_data_query::QueryLatestTrackedDate(db_conn);
    if (!latest_date.has_value()) {
      filters.from_date = "0001-01-01";
      filters.to_date = "0001-01-01";
      return;
    }

    const auto end_ymd = ParseIsoDate(*latest_date);
    if (!end_ymd.has_value()) {
      throw std::runtime_error("Invalid latest tracked date in database: " +
                               *latest_date);
    }
    const auto end_days = std::chrono::sys_days{*end_ymd};
    const auto start_days = end_days - std::chrono::days{lookback_days - 1};
    filters.from_date = FormatIsoDate(std::chrono::year_month_day{start_days});
    filters.to_date = *latest_date;
    return;
  }

  if (period == "range") {
    const std::string arg = require_argument("range");
    size_t separator = arg.find('|');
    if (separator == std::string::npos) {
      separator = arg.find(',');
    }
    if (separator == std::string::npos) {
      throw std::runtime_error(
          "Invalid range period. Use start|end or start,end.");
    }
    const std::string start_raw = TrimCopy(arg.substr(0, separator));
    const std::string end_raw = TrimCopy(arg.substr(separator + 1));
    if (start_raw.empty() || end_raw.empty()) {
      throw std::runtime_error("Range start/end must not be empty.");
    }
    filters.from_date = NormalizeBoundaryDate(start_raw, false);
    filters.to_date = NormalizeBoundaryDate(end_raw, true);
    return;
  }

  throw std::runtime_error(
      "Invalid --period value. Use day/week/month/year/recent/range.");
}

auto EnsureDbConnectionOrThrow(DBManager& db_manager,
                               const std::filesystem::path& db_path)
    -> sqlite3* {
  if (!db_manager.OpenDatabaseIfNeeded()) {
    throw std::runtime_error("Failed to open database at: " + db_path.string());
  }

  sqlite3* db_conn = db_manager.GetDbConnection();
  if (db_conn == nullptr) {
    throw std::runtime_error("Database connection is null.");
  }
  return db_conn;
}

auto ToCliDataQueryAction(time_tracer::core::dto::DataQueryAction action)
    -> infra_data_query::DataQueryAction {
  using CoreAction = time_tracer::core::dto::DataQueryAction;
  switch (action) {
    case CoreAction::kYears:
      return infra_data_query::DataQueryAction::kYears;
    case CoreAction::kMonths:
      return infra_data_query::DataQueryAction::kMonths;
    case CoreAction::kDays:
      return infra_data_query::DataQueryAction::kDays;
    case CoreAction::kDaysDuration:
      return infra_data_query::DataQueryAction::kDaysDuration;
    case CoreAction::kDaysStats:
      return infra_data_query::DataQueryAction::kDaysStats;
    case CoreAction::kSearch:
      return infra_data_query::DataQueryAction::kSearch;
    case CoreAction::kActivitySuggest:
      return infra_data_query::DataQueryAction::kActivitySuggest;
    case CoreAction::kTree:
      return infra_data_query::DataQueryAction::kTree;
  }
  throw std::runtime_error("Unsupported data query action.");
}

auto BuildCliFilters(const time_tracer::core::dto::DataQueryRequest& request)
    -> infra_data_query::QueryFilters {
  infra_data_query::QueryFilters filters;
  filters.kYear = request.year;
  filters.kMonth = request.month;
  filters.from_date = request.from_date;
  filters.to_date = request.to_date;
  filters.remark = request.remark;
  filters.day_remark = request.day_remark;
  filters.project = request.project;
  filters.exercise = request.exercise;
  filters.status = request.status;
  filters.overnight = request.overnight;
  filters.reverse = request.reverse;
  filters.limit = request.limit;
  return filters;
}

}  // namespace

namespace infrastructure::persistence {

SqliteDataQueryService::SqliteDataQueryService(std::filesystem::path db_path)
    : db_path_(std::move(db_path)) {}

auto SqliteDataQueryService::RunDataQuery(
    const time_tracer::core::dto::DataQueryRequest& request)
    -> time_tracer::core::dto::TextOutput {
  DBManager db_manager(db_path_.string());
  sqlite3* db_conn = EnsureDbConnectionOrThrow(db_manager, db_path_);

  const auto kAction = ToCliDataQueryAction(request.action);
  const auto kBaseFilters = BuildCliFilters(request);

  switch (kAction) {
    case infra_data_query::DataQueryAction::kYears:
      return {.ok = true,
              .content = infra_data_query::RenderList(
                  infra_data_query::QueryYears(db_conn)),
              .error_message = ""};
    case infra_data_query::DataQueryAction::kMonths:
      return {.ok = true,
              .content = infra_data_query::RenderList(
                  infra_data_query::QueryMonths(db_conn, kBaseFilters.kYear)),
              .error_message = ""};
    case infra_data_query::DataQueryAction::kDays:
      return {
          .ok = true,
          .content = infra_data_query::RenderList(infra_data_query::QueryDays(
              db_conn, kBaseFilters.kYear, kBaseFilters.kMonth,
              kBaseFilters.from_date, kBaseFilters.to_date,
              kBaseFilters.reverse, kBaseFilters.limit)),
          .error_message = ""};
    case infra_data_query::DataQueryAction::kDaysDuration:
      return {.ok = true,
              .content = infra_data_query::RenderDayDurations(
                  infra_data_query::QueryDayDurations(db_conn, kBaseFilters)),
              .error_message = ""};
    case infra_data_query::DataQueryAction::kDaysStats: {
      auto stats_filters = kBaseFilters;
      if (request.tree_period.has_value()) {
        ApplyTreePeriod(request, db_conn, stats_filters);
      }
      stats_filters.limit.reset();
      stats_filters.reverse = false;
      const auto kRows =
          infra_data_query::QueryDayDurations(db_conn, stats_filters);
      std::string content = infra_data_query::RenderDayDurationStats(
          infra_data_query::ComputeDayDurationStats(kRows));
      if (request.top_n.has_value()) {
        content +=
            infra_data_query::RenderTopDayDurations(kRows, *request.top_n);
      }
      return {.ok = true, .content = std::move(content), .error_message = ""};
    }
    case infra_data_query::DataQueryAction::kSearch:
      return {.ok = true,
              .content = infra_data_query::RenderList(
                  infra_data_query::QueryDatesByFilters(db_conn, kBaseFilters)),
              .error_message = ""};
    case infra_data_query::DataQueryAction::kActivitySuggest: {
      infra_data_query::ActivitySuggestionQueryOptions options;
      options.lookback_days =
          request.lookback_days.value_or(kDefaultSuggestLookbackDays);
      options.limit =
          request.top_n.value_or(request.limit.value_or(kDefaultSuggestLimit));
      options.prefix = request.activity_prefix;
      options.score_by_duration = request.activity_score_by_duration;
      return {.ok = true,
              .content = infra_data_query::RenderActivitySuggestions(
                  infra_data_query::QueryActivitySuggestions(db_conn, options)),
              .error_message = ""};
    }
    case infra_data_query::DataQueryAction::kTree: {
      auto tree_filters = kBaseFilters;
      ApplyTreePeriod(request, db_conn, tree_filters);
      const int max_depth = request.tree_max_depth.value_or(kMinUnlimitedDepth);
      if (max_depth < kMinUnlimitedDepth) {
        throw std::runtime_error("--level must be >= -1.");
      }
      return {.ok = true,
              .content = infra_data_query::RenderProjectTree(
                  infra_data_query::QueryProjectTree(db_conn, tree_filters),
                  max_depth),
              .error_message = ""};
    }
  }

  throw std::runtime_error("Unhandled data query action.");
}

}  // namespace infrastructure::persistence
