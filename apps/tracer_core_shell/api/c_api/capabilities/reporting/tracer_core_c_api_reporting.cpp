import tracer.core.application.use_cases.interface;

#include <exception>
#include <string>

#include "api/c_api/capabilities/reporting/tracer_core_c_api_reporting_internal.hpp"
#include "api/c_api/capabilities/reporting/tracer_core_c_api_structured_report_serializer.hpp"
#include "api/c_api/tracer_core_c_api.h"
#include "api/c_api/runtime/tracer_core_c_api_internal.hpp"
#include "application/dto/reporting_requests.hpp"
#include "application/dto/reporting_responses.hpp"
#include "application/dto/shared_envelopes.hpp"
#include "nlohmann/json.hpp"
#include "shared/types/reporting_errors.hpp"
#include "tracer/transport/runtime_codec_report.hpp"

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
using tracer_core::core::c_api::reporting::SerializeTemporalStructuredReport;
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
  if (payload.locale.has_value()) {
    request.locale = *payload.locale;
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
  try {
    return {.display_mode = ParseReportDisplayMode(payload.display_mode)};
  } catch (const std::invalid_argument&) {
    throw std::invalid_argument(
        "field `type` must be one of: day|week|month|year. Use display_mode "
        "to select the report target type.");
  }
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
  if (payload.locale.has_value()) {
    request.locale = *payload.locale;
  }
  request.selection = BuildSelectionFromPayload(payload);
  request.output_root_path = output_root.string();
  if (payload.recent_days_list.has_value()) {
    request.recent_days_list = *payload.recent_days_list;
  }
  return request;
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
        tracer_core::core::c_api::internal::g_last_response =
            SerializeTemporalStructuredReport(
                runtime.report().RunTemporalStructuredReportQuery(
                    BuildTemporalStructuredQueryRequest(payload)))
                .dump();
        return tracer_core::core::c_api::internal::g_last_response.c_str();
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
