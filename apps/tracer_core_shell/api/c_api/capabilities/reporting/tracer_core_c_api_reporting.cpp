import tracer.core.application.use_cases.interface;

#include <exception>
#include <string>

#include "api/c_api/capabilities/reporting/tracer_core_c_api_reporting_internal.hpp"
#include "api/c_api/tracer_core_c_api.h"
#include "api/c_api/runtime/tracer_core_c_api_internal.hpp"
#include "application/dto/reporting_requests.hpp"
#include "application/dto/reporting_responses.hpp"
#include "application/dto/shared_envelopes.hpp"
#include "nlohmann/json.hpp"
#include "shared/types/reporting_errors.hpp"
#include "tracer/transport/runtime_codec.hpp"

namespace tt_transport = tracer::transport;
using tracer::core::application::use_cases::ITracerCoreRuntime;

using tracer_core::core::c_api::internal::BuildFailureResponse;
using tracer_core::core::c_api::internal::BuildOperationResponse;
using tracer_core::core::c_api::internal::BuildReportTargetsResponse;
using tracer_core::core::c_api::internal::ClearLastError;
using tracer_core::core::c_api::internal::ParseReportDisplayMode;
using tracer_core::core::c_api::internal::ParseReportExportScope;
using tracer_core::core::c_api::internal::ParseReportFormat;
using tracer_core::core::c_api::internal::ParseReportOperationKind;
using tracer_core::core::c_api::internal::ParseTemporalSelectionKind;
using tracer_core::core::c_api::internal::RequireRuntime;
using tracer_core::core::c_api::internal::ToRequestJsonView;
using tracer_core::core::c_api::reporting::BuildReportTextResponse;
using tracer_core::core::dto::PeriodBatchQueryRequest;
using tracer_core::core::dto::ReportOperationKind;
using tracer_core::core::dto::TemporalReportExportRequest;
using tracer_core::core::dto::TemporalReportQueryRequest;
using tracer_core::core::dto::TemporalReportTargetsRequest;
using tracer_core::core::dto::TemporalSelectionPayload;
using tracer_core::core::dto::TemporalStructuredReportOutput;
using tracer_core::core::dto::TemporalStructuredReportQueryRequest;

namespace {

using nlohmann::json;

auto BuildSelectionFromPayload(
    const tt_transport::TemporalReportRequestPayload& payload)
    -> std::optional<TemporalSelectionPayload> {
  if (!payload.selection_kind.has_value()) {
    return std::nullopt;
  }

  TemporalSelectionPayload selection{};
  selection.kind = ParseTemporalSelectionKind(*payload.selection_kind);
  if (payload.date.has_value()) {
    selection.date = *payload.date;
  }
  if (payload.start_date.has_value()) {
    selection.start_date = *payload.start_date;
  }
  if (payload.end_date.has_value()) {
    selection.end_date = *payload.end_date;
  }
  if (payload.days.has_value()) {
    selection.days = *payload.days;
  }
  if (payload.anchor_date.has_value()) {
    selection.anchor_date = *payload.anchor_date;
  }
  return selection;
}

auto BuildTemporalQueryRequest(
    const tt_transport::TemporalReportRequestPayload& payload)
    -> TemporalReportQueryRequest {
  TemporalReportQueryRequest request{};
  request.display_mode = ParseReportDisplayMode(payload.display_mode);
  request.selection = BuildSelectionFromPayload(payload).value_or(
      TemporalSelectionPayload{});
  if (payload.format.has_value()) {
    request.format = ParseReportFormat(*payload.format);
  }
  return request;
}

auto BuildTemporalStructuredQueryRequest(
    const tt_transport::TemporalReportRequestPayload& payload)
    -> TemporalStructuredReportQueryRequest {
  return {
      .display_mode = ParseReportDisplayMode(payload.display_mode),
      .selection = BuildSelectionFromPayload(payload).value_or(
          TemporalSelectionPayload{}),
  };
}

auto BuildTemporalTargetsRequest(
    const tt_transport::TemporalReportRequestPayload& payload)
    -> TemporalReportTargetsRequest {
  return {.display_mode = ParseReportDisplayMode(payload.display_mode)};
}

auto BuildTemporalExportRequest(
    const tt_transport::TemporalReportRequestPayload& payload,
    const std::filesystem::path& output_root) -> TemporalReportExportRequest {
  TemporalReportExportRequest request{};
  request.display_mode = ParseReportDisplayMode(payload.display_mode);
  request.export_scope =
      ParseReportExportScope(payload.export_scope.value_or("single"));
  if (payload.format.has_value()) {
    request.format = ParseReportFormat(*payload.format);
  }
  request.selection = BuildSelectionFromPayload(payload);
  request.output_root_path = output_root.string();
  if (payload.recent_days_list.has_value()) {
    request.recent_days_list = *payload.recent_days_list;
  }
  return request;
}

auto ToWireValue(tracer_core::core::dto::ReportDisplayMode display_mode)
    -> std::string {
  using tracer_core::core::dto::ReportDisplayMode;
  switch (display_mode) {
    case ReportDisplayMode::kDay:
      return "day";
    case ReportDisplayMode::kWeek:
      return "week";
    case ReportDisplayMode::kMonth:
      return "month";
    case ReportDisplayMode::kYear:
      return "year";
    case ReportDisplayMode::kRange:
      return "range";
    case ReportDisplayMode::kRecent:
      return "recent";
  }
  return "day";
}

auto ToWireValue(tracer_core::core::dto::TemporalSelectionKind selection_kind)
    -> std::string {
  using tracer_core::core::dto::TemporalSelectionKind;
  switch (selection_kind) {
    case TemporalSelectionKind::kSingleDay:
      return "single_day";
    case TemporalSelectionKind::kDateRange:
      return "date_range";
    case TemporalSelectionKind::kRecentDays:
      return "recent_days";
  }
  return "single_day";
}

auto EncodeProjectNode(const reporting::ProjectNode& node) -> json {
  json children = json::object();
  for (const auto& [name, child] : node.children) {
    children[name] = EncodeProjectNode(child);
  }
  return json{{"duration", node.duration}, {"children", std::move(children)}};
}

auto EncodeProjectTree(const reporting::ProjectTree& tree) -> json {
  json out = json::object();
  for (const auto& [name, node] : tree) {
    out[name] = EncodeProjectNode(node);
  }
  return out;
}

auto EncodeProjectStats(
    const std::vector<std::pair<std::int64_t, std::int64_t>>& stats) -> json {
  json out = json::array();
  for (const auto& [start, duration] : stats) {
    out.push_back(json{{"start", start}, {"duration", duration}});
  }
  return out;
}

auto EncodeDailyReport(const DailyReportData& report) -> json {
  json records = json::array();
  for (const auto& record : report.detailed_records) {
    records.push_back(json{
        {"start_time", record.start_time},
        {"end_time", record.end_time},
        {"project_path", record.project_path},
        {"duration_seconds", record.duration_seconds},
        {"activity_remark", record.activityRemark.value_or("")},
    });
  }

  json stats = json::object();
  for (const auto& [name, duration] : report.stats) {
    stats[name] = duration;
  }

  return json{
      {"date", report.date},
      {"metadata",
       {{"status", report.metadata.status},
        {"wake_anchor", report.metadata.wake_anchor},
        {"remark", report.metadata.remark},
        {"getup_time", report.metadata.getup_time},
        {"exercise", report.metadata.exercise}}},
      {"total_duration", report.total_duration},
      {"project_stats", EncodeProjectStats(report.project_stats)},
      {"detailed_records", std::move(records)},
      {"stats", std::move(stats)},
      {"project_tree", EncodeProjectTree(report.project_tree)},
  };
}

auto EncodePeriodReport(const PeriodReportData& report) -> json {
  return json{
      {"range_label", report.range_label},
      {"start_date", report.start_date},
      {"end_date", report.end_date},
      {"requested_days", report.requested_days},
      {"has_records", report.has_records},
      {"matched_day_count", report.matched_day_count},
      {"matched_record_count", report.matched_record_count},
      {"total_duration", report.total_duration},
      {"actual_days", report.actual_days},
      {"status_true_days", report.status_true_days},
      {"wake_anchor_true_days", report.wake_anchor_true_days},
      {"exercise_true_days", report.exercise_true_days},
      {"cardio_true_days", report.cardio_true_days},
      {"anaerobic_true_days", report.anaerobic_true_days},
      {"is_valid", report.is_valid},
      {"project_stats", EncodeProjectStats(report.project_stats)},
      {"project_tree", EncodeProjectTree(report.project_tree)},
  };
}

auto BuildTemporalStructuredReportResponse(
    const TemporalStructuredReportOutput& output) -> const char* {
  json payload = {
      {"ok", output.ok},
      {"display_mode", ToWireValue(output.display_mode)},
      {"selection_kind", ToWireValue(output.selection_kind)},
      {"error_message", output.error_message},
      {"error_code", output.error_contract.error_code},
      {"error_category", output.error_contract.error_category},
      {"hints", output.error_contract.hints},
  };

  if (output.ok) {
    if (const auto* daily = std::get_if<DailyReportData>(&output.report);
        daily != nullptr) {
      payload["report_kind"] = "day";
      payload["report"] = EncodeDailyReport(*daily);
    } else if (const auto* period =
                   std::get_if<PeriodReportData>(&output.report);
               period != nullptr) {
      payload["report_kind"] = "period";
      payload["report"] = EncodePeriodReport(*period);
    }
  }

  tracer_core::core::c_api::internal::g_last_response = payload.dump();
  return tracer_core::core::c_api::internal::g_last_response.c_str();
}

}  // namespace

extern "C" TT_CORE_API auto tracer_core_runtime_temporal_report_json(
    TtCoreRuntimeHandle* handle, const char* request_json) -> const char* {
  try {
    ClearLastError();
    ITracerCoreRuntime& runtime = RequireRuntime(handle);
    const auto payload =
        tt_transport::DecodeTemporalReportRequest(ToRequestJsonView(request_json));

    // The canonical reporting ABI now multiplexes query/targets/export through
    // one temporal entrypoint so every host shares the same request contract.
    switch (ParseReportOperationKind(payload.operation_kind)) {
      case ReportOperationKind::kQuery:
        return BuildReportTextResponse(
            runtime.report().RunTemporalReportQuery(
                BuildTemporalQueryRequest(payload)));
      case ReportOperationKind::kStructuredQuery:
        return BuildTemporalStructuredReportResponse(
            runtime.report().RunTemporalStructuredReportQuery(
                BuildTemporalStructuredQueryRequest(payload)));
      case ReportOperationKind::kTargets:
        return BuildReportTargetsResponse(runtime.report().RunTemporalReportTargetsQuery(
            BuildTemporalTargetsRequest(payload)));
      case ReportOperationKind::kExport:
        return BuildOperationResponse(runtime.report().RunTemporalReportExport(
            BuildTemporalExportRequest(payload, handle->output_root)));
    }

    return BuildFailureResponse(
        "Unsupported temporal report operation kind.",
        "reporting.unsupported_operation", "reporting",
        {"Use query, structured_query, targets, or export."});
  } catch (const tracer_core::common::ReportingContractError& error) {
    return BuildFailureResponse(error.what(), error.error_code(),
                                error.error_category(), error.hints());
  } catch (const std::exception& error) {
    return BuildFailureResponse(error.what());
  } catch (...) {
    return BuildFailureResponse(
        "tracer_core_runtime_temporal_report_json failed unexpectedly.");
  }
}

extern "C" TT_CORE_API auto tracer_core_runtime_report_batch_json(
    TtCoreRuntimeHandle* handle, const char* request_json) -> const char* {
  try {
    ClearLastError();
    ITracerCoreRuntime& runtime = RequireRuntime(handle);
    const auto payload =
        tt_transport::DecodeReportBatchRequest(ToRequestJsonView(request_json));

    PeriodBatchQueryRequest request{};
    request.days_list = payload.days_list;
    if (payload.format.has_value()) {
      request.format = ParseReportFormat(*payload.format);
    }

    return BuildReportTextResponse(runtime.report().RunPeriodBatchQuery(request));
  } catch (const tracer_core::common::ReportingContractError& error) {
    return BuildFailureResponse(error.what(), error.error_code(),
                                error.error_category(), error.hints());
  } catch (const std::exception& error) {
    return BuildFailureResponse(error.what());
  } catch (...) {
    return BuildFailureResponse(
        "tracer_core_runtime_report_batch_json failed unexpectedly.");
  }
}
