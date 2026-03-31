#include "api/c_api/runtime/c_api_parse_bridge.hpp"

#include <algorithm>
#include <cctype>
#include <ranges>
#include <stdexcept>

#include "application/dto/pipeline_requests.hpp"
#include "application/dto/query_requests.hpp"
#include "application/dto/reporting_requests.hpp"
#include "domain/reports/types/report_types.hpp"

namespace tracer_core::shell::c_api_bridge {

[[nodiscard]] auto ToLowerAscii(std::string value) -> std::string {
  std::ranges::transform(value, value.begin(),
                         [](unsigned char code_point) -> char {
                           return static_cast<char>(std::tolower(code_point));
                         });
  return value;
}

[[nodiscard]] auto ParseDateCheckMode(const std::string& value)
    -> DateCheckMode {
  const std::string normalized = ToLowerAscii(value);
  if (normalized == "none") {
    return DateCheckMode::kNone;
  }
  if (normalized == "continuity") {
    return DateCheckMode::kContinuity;
  }
  if (normalized == "full") {
    return DateCheckMode::kFull;
  }
  throw std::invalid_argument(
      "field `date_check_mode` must be one of: none|continuity|full.");
}

[[nodiscard]] auto ParseIngestMode(const std::string& value) -> IngestMode {
  const std::string normalized = ToLowerAscii(value);
  if (normalized == "standard") {
    return IngestMode::kStandard;
  }
  if (normalized == "single_txt_replace_month" ||
      normalized == "single-txt-replace-month") {
    return IngestMode::kSingleTxtReplaceMonth;
  }
  throw std::invalid_argument(
      "field `ingest_mode` must be one of: standard|single_txt_replace_month.");
}

[[nodiscard]] auto ParseTimeOrderMode(const std::string& value)
    -> TimeOrderMode {
  const std::string normalized = ToLowerAscii(value);
  if (normalized == "strict_calendar") {
    return TimeOrderMode::kStrictCalendar;
  }
  if (normalized == "logical_day_0600") {
    return TimeOrderMode::kLogicalDay0600;
  }
  throw std::invalid_argument(
      "field `time_order_mode` must be one of: "
      "strict_calendar|logical_day_0600.");
}

[[nodiscard]] auto ParseQueryAction(const std::string& value)
    -> tracer_core::core::dto::DataQueryAction {
  using tracer_core::core::dto::DataQueryAction;
  const std::string normalized = ToLowerAscii(value);
  if (normalized == "years") {
    return DataQueryAction::kYears;
  }
  if (normalized == "months") {
    return DataQueryAction::kMonths;
  }
  if (normalized == "days") {
    return DataQueryAction::kDays;
  }
  if (normalized == "days_duration" || normalized == "days-duration") {
    return DataQueryAction::kDaysDuration;
  }
  if (normalized == "days_stats" || normalized == "days-stats") {
    return DataQueryAction::kDaysStats;
  }
  if (normalized == "search") {
    return DataQueryAction::kSearch;
  }
  if (normalized == "activity_suggest" || normalized == "activity-suggest") {
    return DataQueryAction::kActivitySuggest;
  }
  if (normalized == "mapping_names" || normalized == "mapping-names") {
    return DataQueryAction::kMappingNames;
  }
  if (normalized == "mapping_alias_keys" ||
      normalized == "mapping-alias-keys" ||
      normalized == "alias_keys" || normalized == "alias-keys") {
    return DataQueryAction::kMappingAliasKeys;
  }
  if (normalized == "wake_keywords" || normalized == "wake-keywords") {
    return DataQueryAction::kWakeKeywords;
  }
  if (normalized == "authorable_event_tokens" ||
      normalized == "authorable-event-tokens" ||
      normalized == "authorable_tokens" ||
      normalized == "authorable-tokens") {
    return DataQueryAction::kAuthorableEventTokens;
  }
  if (normalized == "report_chart" || normalized == "report-chart" ||
      normalized == "chart") {
    return DataQueryAction::kReportChart;
  }
  if (normalized == "tree") {
    return DataQueryAction::kTree;
  }
  throw std::invalid_argument(
      "field `action` must be one of: years|months|days|days_duration|"
      "days_stats|search|activity_suggest|mapping_names|mapping_alias_keys|"
      "wake_keywords|authorable_event_tokens|report_chart|tree.");
}

[[nodiscard]] auto ParseDataQueryOutputMode(const std::string& value)
    -> tracer_core::core::dto::DataQueryOutputMode {
  using tracer_core::core::dto::DataQueryOutputMode;
  const std::string normalized = ToLowerAscii(value);
  if (normalized == "text") {
    return DataQueryOutputMode::kText;
  }
  if (normalized == "semantic_json" || normalized == "semantic-json" ||
      normalized == "json") {
    return DataQueryOutputMode::kSemanticJson;
  }
  throw std::invalid_argument(
      "field `output_mode` must be one of: text|semantic_json.");
}

[[nodiscard]] auto ParseExportType(const std::string& value)
    -> tracer_core::core::dto::ReportExportType {
  using tracer_core::core::dto::ReportExportType;
  const std::string normalized = ToLowerAscii(value);
  if (normalized == "day") {
    return ReportExportType::kDay;
  }
  if (normalized == "month") {
    return ReportExportType::kMonth;
  }
  if (normalized == "recent") {
    return ReportExportType::kRecent;
  }
  if (normalized == "week") {
    return ReportExportType::kWeek;
  }
  if (normalized == "year") {
    return ReportExportType::kYear;
  }
  if (normalized == "all_day" || normalized == "all-day") {
    return ReportExportType::kAllDay;
  }
  if (normalized == "all_month" || normalized == "all-month") {
    return ReportExportType::kAllMonth;
  }
  if (normalized == "all_recent" || normalized == "all-recent") {
    return ReportExportType::kAllRecent;
  }
  if (normalized == "all_week" || normalized == "all-week") {
    return ReportExportType::kAllWeek;
  }
  if (normalized == "all_year" || normalized == "all-year") {
    return ReportExportType::kAllYear;
  }
  throw std::invalid_argument(
      "field `type` must be one of: day|month|recent|week|year|all_day|"
      "all_month|all_recent|all_week|all_year.");
}

[[nodiscard]] auto ParseReportType(const std::string& value)
    -> tracer_core::core::dto::ReportQueryType {
  using tracer_core::core::dto::ReportQueryType;
  const std::string normalized = ToLowerAscii(value);
  if (normalized == "day") {
    return ReportQueryType::kDay;
  }
  if (normalized == "month") {
    return ReportQueryType::kMonth;
  }
  if (normalized == "recent") {
    return ReportQueryType::kRecent;
  }
  if (normalized == "range") {
    return ReportQueryType::kRange;
  }
  if (normalized == "week") {
    return ReportQueryType::kWeek;
  }
  if (normalized == "year") {
    return ReportQueryType::kYear;
  }
  throw std::invalid_argument(
      "field `type` must be one of: day|month|recent|range|week|year.");
}

[[nodiscard]] auto ParseReportTargetType(const std::string& value)
    -> tracer_core::core::dto::ReportTargetType {
  using tracer_core::core::dto::ReportTargetType;
  const std::string normalized = ToLowerAscii(value);
  if (normalized == "day") {
    return ReportTargetType::kDay;
  }
  if (normalized == "month") {
    return ReportTargetType::kMonth;
  }
  if (normalized == "week") {
    return ReportTargetType::kWeek;
  }
  if (normalized == "year") {
    return ReportTargetType::kYear;
  }
  throw std::invalid_argument(
      "field `type` must be one of: day|month|week|year.");
}

[[nodiscard]] auto ParseReportFormat(const std::string& value) -> ReportFormat {
  const std::string normalized = ToLowerAscii(value);
  if (normalized == "markdown" || normalized == "md") {
    return ReportFormat::kMarkdown;
  }
  if (normalized == "latex" || normalized == "tex") {
    return ReportFormat::kLaTeX;
  }
  if (normalized == "typst" || normalized == "typ") {
    return ReportFormat::kTyp;
  }
  throw std::invalid_argument(
      "field `format` must be one of: markdown|latex|typst.");
}

}  // namespace tracer_core::shell::c_api_bridge
