// infrastructure/persistence/sqlite_data_query_service_request.cpp
#include <cctype>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>

#include "domain/utils/time_utils.hpp"
#include "infrastructure/persistence/sqlite/db_manager.hpp"
#include "infrastructure/persistence/sqlite_data_query_service_internal.hpp"

namespace infra_data_query = tracer_core::infrastructure::query::data;

namespace infrastructure::persistence::data_query_service_internal {
namespace {

constexpr int kDecimalBase = 10;
constexpr size_t kIsoDateLength = 10;
constexpr size_t kIsoDateYearSeparatorIndex = 4;
constexpr size_t kIsoDateMonthSeparatorIndex = 7;
constexpr size_t kIsoDateYearLength = 4;
constexpr size_t kIsoDateMonthOffset = 5;
constexpr size_t kIsoDateDayOffset = 8;
constexpr size_t kIsoDateMonthDayLength = 2;
constexpr size_t kBoundaryYearLength = 4;
constexpr size_t kBoundaryYearMonthLength = 6;
constexpr size_t kBoundaryDateLength = 8;
constexpr int kFirstMonth = 1;
constexpr int kLastMonth = 12;
constexpr int kFirstDayInMonth = 1;
constexpr int kTmYearBase = 1900;

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

}  // namespace

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

auto NormalizeProjectRootFilter(const std::optional<std::string>& project)
    -> std::optional<std::string> {
  if (!project.has_value()) {
    return std::nullopt;
  }
  std::string normalized = TrimCopy(*project);
  if (normalized.empty()) {
    return std::nullopt;
  }
  const size_t kSeparator = normalized.find('_');
  if (kSeparator != std::string::npos) {
    normalized = normalized.substr(0, kSeparator);
  }
  return normalized;
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

[[nodiscard]] auto ResolveCurrentSystemLocalDate()
    -> std::chrono::year_month_day {
  const std::time_t kNow = std::time(nullptr);
  std::tm local_time{};
#ifdef _WIN32
  localtime_s(&local_time, &kNow);
#else
  localtime_r(&kNow, &local_time);
#endif

  return std::chrono::year_month_day{
      std::chrono::year{local_time.tm_year + kTmYearBase},
      std::chrono::month{static_cast<unsigned>(local_time.tm_mon + 1)},
      std::chrono::day{static_cast<unsigned>(local_time.tm_mday)},
  };
}

auto ResolvePositiveLookbackDays(const std::optional<int>& candidate,
                                 int fallback, std::string_view field_name)
    -> int {
  const int kParsed = candidate.value_or(fallback);
  if (kParsed <= 0) {
    throw std::runtime_error(std::string(field_name) +
                             " must be greater than 0.");
  }
  return kParsed;
}

auto NormalizeBoundaryDate(std::string_view input, bool is_end) -> std::string {
  const std::string kTrimmed = TrimCopy(input);
  std::string digits;
  digits.reserve(kTrimmed.size());
  for (const char kCharacter : kTrimmed) {
    if (std::isdigit(static_cast<unsigned char>(kCharacter)) != 0) {
      digits.push_back(kCharacter);
    }
  }

  if (digits.size() == kBoundaryYearLength) {
    int year = 0;
    if (!ParseUnsigned(digits, year)) {
      throw std::runtime_error("Invalid year boundary: " + kTrimmed);
    }
    if (is_end) {
      return std::to_string(year) + "-12-31";
    }
    return std::to_string(year) + "-01-01";
  }

  if (digits.size() == kBoundaryYearMonthLength) {
    int year = 0;
    int month = 0;
    if (!ParseUnsigned(std::string_view(digits).substr(0, kIsoDateYearLength),
                       year) ||
        !ParseUnsigned(std::string_view(digits).substr(kIsoDateYearLength,
                                                       kIsoDateMonthDayLength),
                       month) ||
        month < kFirstMonth || month > kLastMonth) {
      throw std::runtime_error("Invalid month boundary: " + kTrimmed);
    }
    const auto kYm = std::chrono::year{year} /
                     std::chrono::month{static_cast<unsigned>(month)};
    if (is_end) {
      const auto kYmdLast = std::chrono::year_month_day_last{
          kYm.year(), std::chrono::month_day_last{kYm.month()}};
      return FormatIsoDate(
          std::chrono::year_month_day{std::chrono::sys_days{kYmdLast}});
    }
    return FormatIsoDate(std::chrono::year_month_day{
        kYm.year(), kYm.month(), std::chrono::day{kFirstDayInMonth}});
  }

  if (digits.size() == kBoundaryDateLength) {
    const std::string kIso = NormalizeToDateFormat(digits);
    if (!ParseIsoDate(kIso).has_value()) {
      throw std::runtime_error("Invalid date boundary: " + kTrimmed);
    }
    return kIso;
  }

  if (kTrimmed.size() == kIsoDateLength &&
      kTrimmed[kIsoDateYearSeparatorIndex] == '-' &&
      kTrimmed[kIsoDateMonthSeparatorIndex] == '-') {
    if (!ParseIsoDate(kTrimmed).has_value()) {
      throw std::runtime_error("Invalid ISO date boundary: " + kTrimmed);
    }
    return kTrimmed;
  }

  throw std::runtime_error(
      "Invalid date boundary. Use YYYY, YYYYMM, YYYYMMDD or YYYY-MM-DD.");
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

auto ToCliDataQueryAction(tracer_core::core::dto::DataQueryAction action)
    -> infra_data_query::DataQueryAction {
  using CoreAction = tracer_core::core::dto::DataQueryAction;
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
    case CoreAction::kMappingNames:
      throw std::runtime_error(
          "Mapping names action must be handled before SQL query conversion.");
    case CoreAction::kReportChart:
      return infra_data_query::DataQueryAction::kReportChart;
    case CoreAction::kTree:
      return infra_data_query::DataQueryAction::kTree;
  }
  throw std::runtime_error("Unsupported data query action.");
}

auto BuildCliFilters(const tracer_core::core::dto::DataQueryRequest& request)
    -> infra_data_query::QueryFilters {
  infra_data_query::QueryFilters filters;
  filters.kYear = request.year;
  filters.kMonth = request.month;
  filters.from_date = request.from_date;
  filters.to_date = request.to_date;
  filters.remark = request.remark;
  filters.day_remark = request.day_remark;
  filters.root = NormalizeProjectRootFilter(request.root);
  if (!filters.root.has_value()) {
    filters.project = request.project;
  }
  filters.exercise = request.exercise;
  filters.status = request.status;
  filters.overnight = request.overnight;
  filters.reverse = request.reverse;
  filters.limit = request.limit;
  return filters;
}

}  // namespace infrastructure::persistence::data_query_service_internal
