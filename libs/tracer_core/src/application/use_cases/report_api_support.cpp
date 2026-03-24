#include "application/use_cases/report_api_support.hpp"

#include <cctype>
#include <chrono>
#include <optional>

#include "application/ports/reporting/i_report_dto_formatter.hpp"
#include "application/use_cases/core_api_failure.hpp"

namespace tracer::core::application::use_cases::report_support {

using namespace tracer_core::core::dto;
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
  for (const char character : value) {
    if (std::isdigit(static_cast<unsigned char>(character)) == 0) {
      return false;
    }
    parsed = (parsed * kDecimalBase) + (character - '0');
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

auto ParseRangeArgument(std::string_view argument) -> DateRangeArgument {
  const std::string_view trimmed = TrimAscii(argument);
  const size_t separator_pos = trimmed.find('|');
  const size_t comma_pos = trimmed.find(',');

  size_t split_pos = std::string_view::npos;
  if (separator_pos != std::string_view::npos) {
    split_pos = separator_pos;
  } else if (comma_pos != std::string_view::npos) {
    split_pos = comma_pos;
  }

  if (split_pos == std::string_view::npos) {
    throw std::invalid_argument(
        "Range argument must be `start|end` or `start,end` "
        "(ISO YYYY-MM-DD).");
  }

  const std::string_view start_view = TrimAscii(trimmed.substr(0U, split_pos));
  const std::string_view end_view = TrimAscii(trimmed.substr(split_pos + 1U));
  if (start_view.empty() || end_view.empty()) {
    throw std::invalid_argument("Range start/end date must not be empty.");
  }

  std::chrono::year_month_day start_ymd;
  std::chrono::year_month_day end_ymd;
  if (!ParseIsoDate(start_view, start_ymd)) {
    throw std::invalid_argument("Invalid range start date (ISO YYYY-MM-DD): " +
                                std::string(start_view));
  }
  if (!ParseIsoDate(end_view, end_ymd)) {
    throw std::invalid_argument("Invalid range end date (ISO YYYY-MM-DD): " +
                                std::string(end_view));
  }
  if (std::chrono::sys_days(start_ymd) > std::chrono::sys_days(end_ymd)) {
    throw std::invalid_argument("Range start date must be <= end date.");
  }

  return {.start_date = std::string(start_view),
          .end_date = std::string(end_view)};
}

auto BuildStructuredReportFailure(std::string_view operation,
                                  std::string_view details)
    -> StructuredReportOutput {
  return {.ok = false,
          .kind = StructuredReportKind::kDay,
          .report = DailyReportData{},
          .error_message = core_api_failure::BuildErrorMessage(operation, details)};
}

auto BuildStructuredReportFailure(std::string_view operation,
                                  const std::exception& exception)
    -> StructuredReportOutput {
  return BuildStructuredReportFailure(operation, exception.what());
}

auto BuildStructuredReportFailure(std::string_view operation)
    -> StructuredReportOutput {
  return BuildStructuredReportFailure(operation,
                                      "Unknown non-standard exception.");
}

auto BuildStructuredPeriodBatchFailure(std::string_view operation,
                                       std::string_view details)
    -> StructuredPeriodBatchOutput {
  return {.ok = false,
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

auto FormatStructuredReport(
    const StructuredReportOutput& output, ReportFormat format,
    tracer_core::application::ports::IReportDtoFormatter& formatter)
    -> TextOutput {
  switch (output.kind) {
    case StructuredReportKind::kDay: {
      const auto* report = std::get_if<DailyReportData>(&output.report);
      if (report == nullptr) {
        return core_api_failure::BuildTextFailure(
            "RunReportQuery", "Structured report kind/data mismatch: day.");
      }
      return {.ok = true,
              .content = formatter.FormatDaily(*report, format),
              .error_message = ""};
    }
    case StructuredReportKind::kMonth: {
      const auto* report = std::get_if<MonthlyReportData>(&output.report);
      if (report == nullptr) {
        return core_api_failure::BuildTextFailure(
            "RunReportQuery", "Structured report kind/data mismatch: month.");
      }
      return {.ok = true,
              .content = formatter.FormatMonthly(*report, format),
              .error_message = ""};
    }
    case StructuredReportKind::kRecent: {
      const auto* report = std::get_if<PeriodReportData>(&output.report);
      if (report == nullptr) {
        return core_api_failure::BuildTextFailure(
            "RunReportQuery",
            "Structured report kind/data mismatch: recent period.");
      }
      return {.ok = true,
              .content = formatter.FormatPeriod(*report, format),
              .error_message = ""};
    }
    case StructuredReportKind::kRange: {
      const auto* report = std::get_if<PeriodReportData>(&output.report);
      if (report == nullptr) {
        return core_api_failure::BuildTextFailure(
            "RunReportQuery", "Structured report kind/data mismatch: range.");
      }
      return {.ok = true,
              .content = formatter.FormatPeriod(*report, format),
              .error_message = ""};
    }
    case StructuredReportKind::kWeek: {
      const auto* report = std::get_if<WeeklyReportData>(&output.report);
      if (report == nullptr) {
        return core_api_failure::BuildTextFailure(
            "RunReportQuery", "Structured report kind/data mismatch: week.");
      }
      return {.ok = true,
              .content = formatter.FormatWeekly(*report, format),
              .error_message = ""};
    }
    case StructuredReportKind::kYear: {
      const auto* report = std::get_if<YearlyReportData>(&output.report);
      if (report == nullptr) {
        return core_api_failure::BuildTextFailure(
            "RunReportQuery", "Structured report kind/data mismatch: year.");
      }
      return {.ok = true,
              .content = formatter.FormatYearly(*report, format),
              .error_message = ""};
    }
  }

  return core_api_failure::BuildTextFailure(
      "RunReportQuery", "Unhandled structured report query kind.");
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
