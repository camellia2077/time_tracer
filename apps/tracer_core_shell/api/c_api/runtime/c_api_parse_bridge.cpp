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

[[nodiscard]] auto ParseReportDisplayMode(const std::string& value)
    -> tracer_core::core::dto::ReportDisplayMode {
  using tracer_core::core::dto::ReportDisplayMode;
  const std::string normalized = ToLowerAscii(value);
  if (normalized == "day") {
    return ReportDisplayMode::kDay;
  }
  if (normalized == "week") {
    return ReportDisplayMode::kWeek;
  }
  if (normalized == "month") {
    return ReportDisplayMode::kMonth;
  }
  if (normalized == "year") {
    return ReportDisplayMode::kYear;
  }
  if (normalized == "range") {
    return ReportDisplayMode::kRange;
  }
  if (normalized == "recent") {
    return ReportDisplayMode::kRecent;
  }
  throw std::invalid_argument(
      "field `display_mode` must be one of: day|week|month|year|range|recent.");
}

[[nodiscard]] auto ParseReportExportScope(const std::string& value)
    -> tracer_core::core::dto::ReportExportScope {
  using tracer_core::core::dto::ReportExportScope;
  const std::string normalized = ToLowerAscii(value);
  if (normalized == "single") {
    return ReportExportScope::kSingle;
  }
  if (normalized == "all_matching" || normalized == "all-matching") {
    return ReportExportScope::kAllMatching;
  }
  if (normalized == "batch_recent_list" ||
      normalized == "batch-recent-list") {
    return ReportExportScope::kBatchRecentList;
  }
  throw std::invalid_argument(
      "field `export_scope` must be one of: single|all_matching|batch_recent_list.");
}

[[nodiscard]] auto ParseReportOperationKind(const std::string& value)
    -> tracer_core::core::dto::ReportOperationKind {
  using tracer_core::core::dto::ReportOperationKind;
  const std::string normalized = ToLowerAscii(value);
  if (normalized == "query") {
    return ReportOperationKind::kQuery;
  }
  if (normalized == "structured_query" || normalized == "structured-query") {
    return ReportOperationKind::kStructuredQuery;
  }
  if (normalized == "targets") {
    return ReportOperationKind::kTargets;
  }
  if (normalized == "export") {
    return ReportOperationKind::kExport;
  }
  throw std::invalid_argument(
      "field `operation_kind` must be one of: query|structured_query|targets|export.");
}

[[nodiscard]] auto ParseTemporalSelectionKind(const std::string& value)
    -> tracer_core::core::dto::TemporalSelectionKind {
  using tracer_core::core::dto::TemporalSelectionKind;
  const std::string normalized = ToLowerAscii(value);
  if (normalized == "single_day" || normalized == "single-day") {
    return TemporalSelectionKind::kSingleDay;
  }
  if (normalized == "date_range" || normalized == "date-range") {
    return TemporalSelectionKind::kDateRange;
  }
  if (normalized == "recent_days" || normalized == "recent-days") {
    return TemporalSelectionKind::kRecentDays;
  }
  throw std::invalid_argument(
      "field `selection_kind` must be one of: single_day|date_range|recent_days.");
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
