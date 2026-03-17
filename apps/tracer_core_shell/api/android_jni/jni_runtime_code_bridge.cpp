#include "api/android_jni/jni_runtime_code_bridge.hpp"

#include <stdexcept>
#include <string>

namespace tracer_core::shell::jni_bridge {

namespace {

constexpr int kDateCheckModeNone = 0;
constexpr int kDateCheckModeContinuity = 1;
constexpr int kDateCheckModeFull = 2;

constexpr int kQueryActionYears = 0;
constexpr int kQueryActionMonths = 1;
constexpr int kQueryActionDays = 2;
constexpr int kQueryActionDaysDuration = 3;
constexpr int kQueryActionDaysStats = 4;
constexpr int kQueryActionSearch = 5;
constexpr int kQueryActionActivitySuggest = 6;
constexpr int kQueryActionTree = 7;
constexpr int kQueryActionMappingNames = 8;
constexpr int kQueryActionReportChart = 9;

constexpr int kReportTypeDay = 0;
constexpr int kReportTypeMonth = 1;
constexpr int kReportTypeRecent = 2;
constexpr int kReportTypeWeek = 3;
constexpr int kReportTypeYear = 4;
constexpr int kReportTypeRange = 5;

constexpr int kReportFormatMarkdown = 0;
constexpr int kReportFormatLatex = 1;
constexpr int kReportFormatTypst = 2;

}  // namespace

[[nodiscard]] auto ParseDateCheckModeCode(int value) -> std::string {
  if (value == kDateCheckModeNone) {
    return "none";
  }
  if (value == kDateCheckModeContinuity) {
    return "continuity";
  }
  if (value == kDateCheckModeFull) {
    return "full";
  }
  throw std::invalid_argument("Unsupported date_check_mode code: " +
                              std::to_string(value));
}

[[nodiscard]] auto ParseDataQueryActionCode(int value) -> std::string {
  if (value == kQueryActionYears) {
    return "years";
  }
  if (value == kQueryActionMonths) {
    return "months";
  }
  if (value == kQueryActionDays) {
    return "days";
  }
  if (value == kQueryActionDaysDuration) {
    return "days_duration";
  }
  if (value == kQueryActionDaysStats) {
    return "days_stats";
  }
  if (value == kQueryActionSearch) {
    return "search";
  }
  if (value == kQueryActionActivitySuggest) {
    return "activity_suggest";
  }
  if (value == kQueryActionTree) {
    return "tree";
  }
  if (value == kQueryActionMappingNames) {
    return "mapping_names";
  }
  if (value == kQueryActionReportChart) {
    return "report_chart";
  }
  throw std::invalid_argument("Unsupported query action code: " +
                              std::to_string(value));
}

[[nodiscard]] auto ParseReportTypeCode(int value) -> std::string {
  if (value == kReportTypeDay) {
    return "day";
  }
  if (value == kReportTypeMonth) {
    return "month";
  }
  if (value == kReportTypeRecent) {
    return "recent";
  }
  if (value == kReportTypeWeek) {
    return "week";
  }
  if (value == kReportTypeYear) {
    return "year";
  }
  if (value == kReportTypeRange) {
    return "range";
  }
  throw std::invalid_argument("Unsupported report type code: " +
                              std::to_string(value));
}

[[nodiscard]] auto ParseReportFormatCode(int value) -> std::string {
  if (value == kReportFormatMarkdown) {
    return "markdown";
  }
  if (value == kReportFormatLatex) {
    return "latex";
  }
  if (value == kReportFormatTypst) {
    return "typst";
  }
  throw std::invalid_argument("Unsupported report format code: " +
                              std::to_string(value));
}

}  // namespace tracer_core::shell::jni_bridge
