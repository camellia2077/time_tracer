#include "application/use_cases/report_api_support.hpp"

#include <cctype>
#include <chrono>

#include "application/use_cases/core_api_failure.hpp"

namespace tracer::core::application::use_cases::report_support {

using tracer_core::core::dto::StructuredPeriodBatchOutput;
namespace core_api_failure = tracer::core::application::use_cases::failure;

namespace {

constexpr int kDecimalBase = 10;
constexpr size_t kIsoDateLength = 10U;
constexpr size_t kIsoYearLength = 4U;
constexpr size_t kIsoMonthSeparatorIndex = 7U;
constexpr size_t kIsoMonthStartIndex = 5U;
constexpr size_t kIsoDayStartIndex = 8U;
constexpr size_t kIsoPartLength = 2U;

[[nodiscard]] auto TrimAscii(std::string_view value) -> std::string_view {
  size_t begin = 0;
  while (begin < value.size() &&
         std::isspace(static_cast<unsigned char>(value[begin])) != 0) {
    ++begin;
  }

  size_t end = value.size();
  while (end > begin &&
         std::isspace(static_cast<unsigned char>(value[end - 1])) != 0) {
    --end;
  }

  return value.substr(begin, end - begin);
}

auto ParseUnsigned(std::string_view value, int& out_value) -> bool {
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
  out_value = parsed;
  return true;
}

auto ParseIsoDate(std::string_view value, std::chrono::year_month_day& out_ymd)
    -> bool {
  if (value.size() != kIsoDateLength || value[kIsoYearLength] != '-' ||
      value[kIsoMonthSeparatorIndex] != '-') {
    return false;
  }

  int year = 0;
  int month = 0;
  int day = 0;
  if (!ParseUnsigned(value.substr(0U, kIsoYearLength), year) ||
      !ParseUnsigned(value.substr(kIsoMonthStartIndex, kIsoPartLength),
                     month) ||
      !ParseUnsigned(value.substr(kIsoDayStartIndex, kIsoPartLength), day)) {
    return false;
  }

  const std::chrono::year_month_day kYmd(
      std::chrono::year(year), std::chrono::month(static_cast<unsigned>(month)),
      std::chrono::day(static_cast<unsigned>(day)));
  if (!kYmd.ok()) {
    return false;
  }

  out_ymd = kYmd;
  return true;
}

}  // namespace

auto ParseRecentDaysArgument(std::string_view argument) -> int {
  const std::string_view kTrimmed = TrimAscii(argument);
  int days = 0;
  if (!ParseUnsigned(kTrimmed, days) || days <= 0) {
    throw std::invalid_argument("Recent argument must be a positive integer.");
  }
  return days;
}

auto ParseRangeArgument(std::string_view argument) -> DateRangeArgument {
  const std::string_view kTrimmed = TrimAscii(argument);
  const size_t kSeparatorPos = kTrimmed.find('|');
  const size_t kCommaPos = kTrimmed.find(',');

  size_t split_pos = std::string_view::npos;
  if (kSeparatorPos != std::string_view::npos) {
    split_pos = kSeparatorPos;
  } else if (kCommaPos != std::string_view::npos) {
    split_pos = kCommaPos;
  }

  if (split_pos == std::string_view::npos) {
    throw std::invalid_argument(
        "Range argument must be `start|end` or `start,end` "
        "(ISO YYYY-MM-DD).");
  }

  const std::string_view kStartView = TrimAscii(kTrimmed.substr(0U, split_pos));
  const std::string_view kEndView = TrimAscii(kTrimmed.substr(split_pos + 1U));
  if (kStartView.empty() || kEndView.empty()) {
    throw std::invalid_argument("Range start/end date must not be empty.");
  }

  std::chrono::year_month_day start_ymd;
  std::chrono::year_month_day end_ymd;
  if (!ParseIsoDate(kStartView, start_ymd)) {
    throw std::invalid_argument("Invalid range start date (ISO YYYY-MM-DD): " +
                                std::string(kStartView));
  }
  if (!ParseIsoDate(kEndView, end_ymd)) {
    throw std::invalid_argument("Invalid range end date (ISO YYYY-MM-DD): " +
                                std::string(kEndView));
  }
  if (std::chrono::sys_days(start_ymd) > std::chrono::sys_days(end_ymd)) {
    throw std::invalid_argument("Range start date must be <= end date.");
  }

  return {.start_date = std::string(kStartView),
          .end_date = std::string(kEndView)};
}

auto BuildStructuredPeriodBatchFailure(std::string_view operation,
                                       std::string_view details)
    -> StructuredPeriodBatchOutput {
  return {
      .ok = false,
      .items = {},
      .error_message = core_api_failure::BuildErrorMessage(operation, details)};
}

auto BuildStructuredPeriodBatchFailure(std::string_view operation,
                                       const std::exception& exception)
    -> StructuredPeriodBatchOutput {
  return BuildStructuredPeriodBatchFailure(operation, exception.what());
}

auto BuildStructuredPeriodBatchFailure(std::string_view operation)
    -> StructuredPeriodBatchOutput {
  return BuildStructuredPeriodBatchFailure(operation,
                                           "Unknown non-standard exception.");
}

auto BuildPeriodBatchErrorLine(int days, std::string_view details)
    -> std::string {
  if (details.empty()) {
    return "Error querying period " + std::to_string(days) + " days.";
  }
  return "Error querying period " + std::to_string(days) +
         " days: " + std::string(details);
}

}  // namespace tracer::core::application::use_cases::report_support
