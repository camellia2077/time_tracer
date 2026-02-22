#include "api/core_c/time_tracer_core_c_api_internal.hpp"

#include <algorithm>
#include <cctype>
#include <ranges>
#include <stdexcept>
#include <utility>

#include "tracer/transport/runtime_codec.hpp"

#ifndef TT_ENABLE_PROCESSED_JSON_IO
#define TT_ENABLE_PROCESSED_JSON_IO 1
#endif

#ifndef TT_REPORT_ENABLE_LATEX
#define TT_REPORT_ENABLE_LATEX 1
#endif

#ifndef TT_REPORT_ENABLE_TYPST
#define TT_REPORT_ENABLE_TYPST 1
#endif

namespace time_tracer::core::c_api::internal {

namespace tt_transport = tracer::transport;

thread_local std::string g_last_error;
thread_local std::string g_last_response;

namespace {

[[nodiscard]] auto BuildAckResponse(bool is_ok,
                                    const std::string& error_message) -> const
    char* {
  g_last_response =
      tt_transport::EncodeIngestResponse(tt_transport::IngestResponsePayload{
          .ok = is_ok,
          .error_message = error_message,
      });
  return g_last_response.c_str();
}

auto ConvertTreeNode(const ProjectTreeNode& node)
    -> tt_transport::ProjectTreeNodePayload {
  tt_transport::ProjectTreeNodePayload out{};
  out.name = node.name;
  out.children.reserve(node.children.size());
  for (const auto& child : node.children) {
    out.children.push_back(ConvertTreeNode(child));
  }
  return out;
}

[[nodiscard]] auto ToLowerAscii(std::string value) -> std::string {
  std::ranges::transform(value, value.begin(),
                         [](unsigned char code_point) -> char {
                           return static_cast<char>(std::tolower(code_point));
                         });
  return value;
}

}  // namespace

void ClearLastError() {
  g_last_error.clear();
}

void SetLastError(const char* message) {
  try {
    g_last_error.assign(message != nullptr ? message : "Unknown error.");
  } catch (...) {
    g_last_error = "Failed to set last error.";
  }
}

[[nodiscard]] auto BuildFailureResponse(std::string message) -> const char* {
  if (message.empty()) {
    message = "Unknown error.";
  }
  SetLastError(message.c_str());
  g_last_response =
      tt_transport::EncodeIngestResponse(tt_transport::IngestResponsePayload{
          .ok = false,
          .error_message = std::move(message),
      });
  return g_last_response.c_str();
}

[[nodiscard]] auto BuildOperationResponse(
    const time_tracer::core::dto::OperationAck& output) -> const char* {
  return BuildAckResponse(output.ok, output.error_message);
}

[[nodiscard]] auto BuildTextResponse(
    const time_tracer::core::dto::TextOutput& output) -> const char* {
  g_last_response =
      tt_transport::EncodeQueryResponse(tt_transport::QueryResponsePayload{
          .ok = output.ok,
          .error_message = output.error_message,
          .content = output.content,
      });
  return g_last_response.c_str();
}

[[nodiscard]] auto BuildTreeResponse(
    const time_tracer::core::dto::TreeQueryResponse& response) -> const char* {
  tt_transport::TreeResponsePayload payload{};
  payload.ok = response.ok;
  payload.found = response.found;
  payload.error_message = response.error_message;
  payload.roots = response.roots;
  payload.nodes.reserve(response.nodes.size());
  for (const auto& node : response.nodes) {
    payload.nodes.push_back(ConvertTreeNode(node));
  }
  g_last_response = tt_transport::EncodeTreeResponse(payload);
  return g_last_response.c_str();
}

[[nodiscard]] auto BuildCapabilitiesResponseJson() -> const char* {
  tt_transport::CapabilitiesResponsePayload payload{};
  payload.abi.name = "tracer_core_c";
  payload.abi.version = 1;
  payload.features.runtime_ingest_json = true;
  payload.features.runtime_convert_json = true;
  payload.features.runtime_import_json = true;
  payload.features.runtime_validate_structure_json = true;
  payload.features.runtime_validate_logic_json = true;
  payload.features.runtime_query_json = true;
  payload.features.runtime_report_json = true;
  payload.features.runtime_report_batch_json = true;
  payload.features.runtime_export_json = true;
  payload.features.runtime_tree_json = true;
  payload.features.processed_json_io = TT_ENABLE_PROCESSED_JSON_IO != 0;
  payload.features.report_markdown = true;
  payload.features.report_latex = TT_REPORT_ENABLE_LATEX != 0;
  payload.features.report_typst = TT_REPORT_ENABLE_TYPST != 0;
  g_last_response = tt_transport::EncodeCapabilitiesResponse(payload);
  return g_last_response.c_str();
}

[[nodiscard]] auto RequireRuntime(TtCoreRuntimeHandle* handle)
    -> ITimeTracerCoreApi& {
  if (handle == nullptr || handle->runtime.core_api == nullptr) {
    throw std::invalid_argument("runtime handle is null.");
  }
  return *handle->runtime.core_api;
}

[[nodiscard]] auto ToRequestJsonView(const char* request_json)
    -> std::string_view {
  return request_json != nullptr ? std::string_view(request_json)
                                 : std::string_view{};
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
    -> time_tracer::core::dto::DataQueryAction {
  using time_tracer::core::dto::DataQueryAction;
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
    -> time_tracer::core::dto::DataQueryOutputMode {
  using time_tracer::core::dto::DataQueryOutputMode;
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
    -> time_tracer::core::dto::ReportExportType {
  using time_tracer::core::dto::ReportExportType;
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
    -> time_tracer::core::dto::ReportQueryType {
  using time_tracer::core::dto::ReportQueryType;
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

}  // namespace time_tracer::core::c_api::internal
