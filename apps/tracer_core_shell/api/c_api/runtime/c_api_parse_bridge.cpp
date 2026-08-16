#include "api/c_api/runtime/c_api_parse_bridge.hpp"

#include <algorithm>
#include <cctype>
#include <ranges>
#include <stdexcept>

#include "application/dto/pipeline_requests.hpp"
#include "application/dto/query_requests.hpp"
#include "application/dto/insights_requests.hpp"
#include "domain/insights/types/insights_types.hpp"

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
  if (normalized == "activity_frequent" || normalized == "activity-frequent") {
    return DataQueryAction::kActivityFrequent;
  }
  if (normalized == "mapping_names" || normalized == "mapping-names") {
    return DataQueryAction::kMappingNames;
  }
  if (normalized == "activity_alias_mappings" ||
      normalized == "activity-alias-mappings" ||
      normalized == "alias_mappings" || normalized == "alias-mappings") {
    return DataQueryAction::kActivityHierarchyLeafMappings;
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
  if (normalized == "insights_chart" || normalized == "insights-chart" ||
      normalized == "chart") {
    return DataQueryAction::kInsightsChart;
  }
  if (normalized == "insights_composition" ||
      normalized == "insights-composition" || normalized == "composition") {
    return DataQueryAction::kInsightsComposition;
  }
  if (normalized == "previous_activity_tail" ||
      normalized == "previous-activity-tail") {
    return DataQueryAction::kPreviousActivityTail;
  }
  if (normalized == "latest_activity_record" ||
      normalized == "latest-activity-record") {
    return DataQueryAction::kLatestActivityRecord;
  }
  if (normalized == "tree") {
    return DataQueryAction::kTree;
  }
  throw std::invalid_argument(
      "field `action` must be one of: years|months|days|days_duration|"
      "days_stats|search|activity_frequent|mapping_names|activity_alias_mappings|mapping_alias_keys|"
      "wake_keywords|authorable_event_tokens|insights_chart|"
      "insights_composition|previous_activity_tail|latest_activity_record|tree.");
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

[[nodiscard]] auto ParseInsightsAverageDayBasis(const std::string& value)
    -> tracer_core::core::dto::InsightsAverageDayBasis {
  using tracer_core::core::dto::InsightsAverageDayBasis;
  const std::string normalized = ToLowerAscii(value);
  if (normalized == "active_days" || normalized == "active-days") {
    return InsightsAverageDayBasis::kActiveDays;
  }
  if (normalized == "calendar_days" || normalized == "calendar-days") {
    return InsightsAverageDayBasis::kCalendarDays;
  }
  throw std::invalid_argument(
      "field `average_day_basis` must be one of: active_days|calendar_days.");
}

[[nodiscard]] auto ParseInsightsDisplayMode(const std::string& value)
    -> tracer_core::core::dto::InsightsDisplayMode {
  using tracer_core::core::dto::InsightsDisplayMode;
  const std::string normalized = ToLowerAscii(value);
  if (normalized == "day") {
    return InsightsDisplayMode::kDay;
  }
  if (normalized == "week") {
    return InsightsDisplayMode::kWeek;
  }
  if (normalized == "month") {
    return InsightsDisplayMode::kMonth;
  }
  if (normalized == "year") {
    return InsightsDisplayMode::kYear;
  }
  if (normalized == "range") {
    return InsightsDisplayMode::kRange;
  }
  if (normalized == "recent") {
    return InsightsDisplayMode::kRecent;
  }
  throw std::invalid_argument(
      "field `display_mode` must be one of: day|week|month|year|range|recent.");
}

[[nodiscard]] auto ParseInsightsExportScope(const std::string& value)
    -> tracer_core::core::dto::InsightsExportScope {
  using tracer_core::core::dto::InsightsExportScope;
  const std::string normalized = ToLowerAscii(value);
  if (normalized == "single") {
    return InsightsExportScope::kSingle;
  }
  if (normalized == "all_matching" || normalized == "all-matching") {
    return InsightsExportScope::kAllMatching;
  }
  if (normalized == "batch_recent_list" ||
      normalized == "batch-recent-list") {
    return InsightsExportScope::kBatchRecentList;
  }
  throw std::invalid_argument(
      "field `export_scope` must be one of: single|all_matching|batch_recent_list.");
}

[[nodiscard]] auto ParseInsightsOperationKind(const std::string& value)
    -> tracer_core::core::dto::InsightsOperationKind {
  using tracer_core::core::dto::InsightsOperationKind;
  const std::string normalized = ToLowerAscii(value);
  if (normalized == "query") {
    return InsightsOperationKind::kQuery;
  }
  if (normalized == "structured_query" || normalized == "structured-query") {
    return InsightsOperationKind::kStructuredQuery;
  }
  if (normalized == "targets") {
    return InsightsOperationKind::kTargets;
  }
  if (normalized == "export") {
    return InsightsOperationKind::kExport;
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

[[nodiscard]] auto ParseInsightsFormat(const std::string& value) -> InsightsFormat {
  const std::string normalized = ToLowerAscii(value);
  if (normalized == "markdown" || normalized == "md") {
    return InsightsFormat::kMarkdown;
  }
  if (normalized == "latex" || normalized == "tex") {
    return InsightsFormat::kLaTeX;
  }
  if (normalized == "typst" || normalized == "typ") {
    return InsightsFormat::kTyp;
  }
  throw std::invalid_argument(
      "field `format` must be one of: markdown|latex|typst.");
}

}  // namespace tracer_core::shell::c_api_bridge
