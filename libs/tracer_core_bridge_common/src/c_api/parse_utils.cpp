#include "c_api/parse_utils.hpp"

#include <algorithm>
#include <cctype>
#include <ranges>
#include <stdexcept>

namespace tracer_core_bridge_common::c_api {

[[nodiscard]] auto ToLowerAscii(std::string value) -> std::string {
  std::ranges::transform(value, value.begin(),
                         [](unsigned char code_point) -> char {
                           return static_cast<char>(std::tolower(code_point));
                         });
  return value;
}

[[nodiscard]] auto ParseDateCheckMode(const std::string& value)
    -> DateCheckMode {
  const std::string kNormalized = ToLowerAscii(value);
  if (kNormalized == "none") {
    return DateCheckMode::kNone;
  }
  if (kNormalized == "continuity") {
    return DateCheckMode::kContinuity;
  }
  if (kNormalized == "full") {
    return DateCheckMode::kFull;
  }
  throw std::invalid_argument(
      "field `date_check_mode` must be one of: none|continuity|full.");
}

[[nodiscard]] auto ParseIngestMode(const std::string& value) -> IngestMode {
  const std::string kNormalized = ToLowerAscii(value);
  if (kNormalized == "standard") {
    return IngestMode::kStandard;
  }
  if (kNormalized == "single_txt_replace_month" ||
      kNormalized == "single-txt-replace-month") {
    return IngestMode::kSingleTxtReplaceMonth;
  }
  throw std::invalid_argument(
      "field `ingest_mode` must be one of: standard|single_txt_replace_month.");
}

[[nodiscard]] auto ParseQueryAction(const std::string& value)
    -> tracer_core::core::dto::DataQueryAction {
  using tracer_core::core::dto::DataQueryAction;
  const std::string kNormalized = ToLowerAscii(value);
  if (kNormalized == "years") {
    return DataQueryAction::kYears;
  }
  if (kNormalized == "months") {
    return DataQueryAction::kMonths;
  }
  if (kNormalized == "days") {
    return DataQueryAction::kDays;
  }
  if (kNormalized == "days_duration" || kNormalized == "days-duration") {
    return DataQueryAction::kDaysDuration;
  }
  if (kNormalized == "days_stats" || kNormalized == "days-stats") {
    return DataQueryAction::kDaysStats;
  }
  if (kNormalized == "search") {
    return DataQueryAction::kSearch;
  }
  if (kNormalized == "activity_suggest" || kNormalized == "activity-suggest") {
    return DataQueryAction::kActivitySuggest;
  }
  if (kNormalized == "mapping_names" || kNormalized == "mapping-names") {
    return DataQueryAction::kMappingNames;
  }
  if (kNormalized == "report_chart" || kNormalized == "report-chart" ||
      kNormalized == "chart") {
    return DataQueryAction::kReportChart;
  }
  if (kNormalized == "tree") {
    return DataQueryAction::kTree;
  }
  throw std::invalid_argument(
      "field `action` must be one of: years|months|days|days_duration|"
      "days_stats|search|activity_suggest|mapping_names|report_chart|tree.");
}

[[nodiscard]] auto ParseDataQueryOutputMode(const std::string& value)
    -> tracer_core::core::dto::DataQueryOutputMode {
  using tracer_core::core::dto::DataQueryOutputMode;
  const std::string kNormalized = ToLowerAscii(value);
  if (kNormalized == "text") {
    return DataQueryOutputMode::kText;
  }
  if (kNormalized == "semantic_json" || kNormalized == "semantic-json" ||
      kNormalized == "json") {
    return DataQueryOutputMode::kSemanticJson;
  }
  throw std::invalid_argument(
      "field `output_mode` must be one of: text|semantic_json.");
}

[[nodiscard]] auto ParseExportType(const std::string& value)
    -> tracer_core::core::dto::ReportExportType {
  using tracer_core::core::dto::ReportExportType;
  const std::string kNormalized = ToLowerAscii(value);
  if (kNormalized == "day") {
    return ReportExportType::kDay;
  }
  if (kNormalized == "month") {
    return ReportExportType::kMonth;
  }
  if (kNormalized == "recent") {
    return ReportExportType::kRecent;
  }
  if (kNormalized == "week") {
    return ReportExportType::kWeek;
  }
  if (kNormalized == "year") {
    return ReportExportType::kYear;
  }
  if (kNormalized == "all_day" || kNormalized == "all-day") {
    return ReportExportType::kAllDay;
  }
  if (kNormalized == "all_month" || kNormalized == "all-month") {
    return ReportExportType::kAllMonth;
  }
  if (kNormalized == "all_recent" || kNormalized == "all-recent") {
    return ReportExportType::kAllRecent;
  }
  if (kNormalized == "all_week" || kNormalized == "all-week") {
    return ReportExportType::kAllWeek;
  }
  if (kNormalized == "all_year" || kNormalized == "all-year") {
    return ReportExportType::kAllYear;
  }
  throw std::invalid_argument(
      "field `type` must be one of: day|month|recent|week|year|all_day|"
      "all_month|all_recent|all_week|all_year.");
}

[[nodiscard]] auto ParseReportType(const std::string& value)
    -> tracer_core::core::dto::ReportQueryType {
  using tracer_core::core::dto::ReportQueryType;
  const std::string kNormalized = ToLowerAscii(value);
  if (kNormalized == "day") {
    return ReportQueryType::kDay;
  }
  if (kNormalized == "month") {
    return ReportQueryType::kMonth;
  }
  if (kNormalized == "recent") {
    return ReportQueryType::kRecent;
  }
  if (kNormalized == "range") {
    return ReportQueryType::kRange;
  }
  if (kNormalized == "week") {
    return ReportQueryType::kWeek;
  }
  if (kNormalized == "year") {
    return ReportQueryType::kYear;
  }
  throw std::invalid_argument(
      "field `type` must be one of: day|month|recent|range|week|year.");
}

[[nodiscard]] auto ParseReportFormat(const std::string& value) -> ReportFormat {
  const std::string kNormalized = ToLowerAscii(value);
  if (kNormalized == "markdown" || kNormalized == "md") {
    return ReportFormat::kMarkdown;
  }
  if (kNormalized == "latex" || kNormalized == "tex") {
    return ReportFormat::kLaTeX;
  }
  if (kNormalized == "typst" || kNormalized == "typ") {
    return ReportFormat::kTyp;
  }
  throw std::invalid_argument(
      "field `format` must be one of: markdown|latex|typst.");
}

}  // namespace tracer_core_bridge_common::c_api
