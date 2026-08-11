#include "api/android_jni/jni_runtime_code_bridge.hpp"

#include <stdexcept>
#include <string>

namespace tracer_core::shell::jni_bridge {

namespace {

constexpr int kDateCheckModeNone = 0;
constexpr int kDateCheckModeContinuity = 1;
constexpr int kDateCheckModeFull = 2;
constexpr int kRecordTimeOrderStrictCalendar = 0;
constexpr int kRecordTimeOrderLogicalDay0600 = 1;

constexpr int kQueryActionYears = 0;
constexpr int kQueryActionMonths = 1;
constexpr int kQueryActionDays = 2;
constexpr int kQueryActionDaysDuration = 3;
constexpr int kQueryActionDaysStats = 4;
constexpr int kQueryActionSearch = 5;
constexpr int kQueryActionActivitySuggest = 6;
constexpr int kQueryActionTree = 7;
constexpr int kQueryActionMappingNames = 8;
constexpr int kQueryActionActivityHierarchyLeafMappings = 9;
constexpr int kQueryActionInsightsChart = 10;
constexpr int kQueryActionMappingAliasKeys = 11;
constexpr int kQueryActionWakeKeywords = 12;
constexpr int kQueryActionAuthorableEventTokens = 13;
constexpr int kQueryActionInsightsComposition = 14;

constexpr int kInsightsTypeDay = 0;
constexpr int kInsightsTypeMonth = 1;
constexpr int kInsightsTypeRecent = 2;
constexpr int kInsightsTypeWeek = 3;
constexpr int kInsightsTypeYear = 4;
constexpr int kInsightsTypeRange = 5;

constexpr int kInsightsFormatMarkdown = 0;
constexpr int kInsightsFormatLatex = 1;
constexpr int kInsightsFormatTypst = 2;

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

[[nodiscard]] auto ParseRecordTimeOrderModeCode(int value) -> std::string {
  if (value == kRecordTimeOrderStrictCalendar) {
    return "strict_calendar";
  }
  if (value == kRecordTimeOrderLogicalDay0600) {
    return "logical_day_0600";
  }
  throw std::invalid_argument(
      "Unsupported time_order_mode code: " + std::to_string(value) +
      ". Allowed: 0(strict_calendar), 1(logical_day_0600).");
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
    return "activity_frequent";
  }
  if (value == kQueryActionTree) {
    return "tree";
  }
  if (value == kQueryActionMappingNames) {
    return "mapping_names";
  }
  if (value == kQueryActionActivityHierarchyLeafMappings) {
    return "activity_alias_mappings";
  }
  if (value == kQueryActionInsightsChart) {
    return "insights_chart";
  }
  if (value == kQueryActionMappingAliasKeys) {
    return "mapping_alias_keys";
  }
  if (value == kQueryActionWakeKeywords) {
    return "wake_keywords";
  }
  if (value == kQueryActionAuthorableEventTokens) {
    return "authorable_event_tokens";
  }
  if (value == kQueryActionInsightsComposition) {
    return "insights_composition";
  }
  throw std::invalid_argument("Unsupported query action code: " +
                              std::to_string(value));
}

[[nodiscard]] auto ParseInsightsTypeCode(int value) -> std::string {
  if (value == kInsightsTypeDay) {
    return "day";
  }
  if (value == kInsightsTypeMonth) {
    return "month";
  }
  if (value == kInsightsTypeRecent) {
    return "recent";
  }
  if (value == kInsightsTypeWeek) {
    return "week";
  }
  if (value == kInsightsTypeYear) {
    return "year";
  }
  if (value == kInsightsTypeRange) {
    return "range";
  }
  throw std::invalid_argument("Unsupported insights type code: " +
                              std::to_string(value));
}

[[nodiscard]] auto ParseInsightsFormatCode(int value) -> std::string {
  if (value == kInsightsFormatMarkdown) {
    return "markdown";
  }
  if (value == kInsightsFormatLatex) {
    return "latex";
  }
  if (value == kInsightsFormatTypst) {
    return "typst";
  }
  throw std::invalid_argument("Unsupported insights format code: " +
                              std::to_string(value));
}

}  // namespace tracer_core::shell::jni_bridge
