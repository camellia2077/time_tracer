import tracer.core.application.use_cases.interface;

#include <exception>
#include <string>

#include "api/c_api/capabilities/insights/tracer_core_c_api_insights_internal.hpp"
#include "api/c_api/capabilities/insights/tracer_core_c_api_structured_insights_serializer.hpp"
#include "api/c_api/tracer_core_c_api.h"
#include "api/c_api/runtime/tracer_core_c_api_internal.hpp"
#include "application/dto/insights_requests.hpp"
#include "application/dto/insights_responses.hpp"
#include "application/dto/shared_envelopes.hpp"
#include "nlohmann/json.hpp"
#include "shared/types/insights_errors.hpp"
#include "tracer/transport/runtime_codec_insights.hpp"

namespace tt_transport = tracer::transport;
using tracer::core::application::use_cases::ITracerCoreRuntime;

using tracer_core::core::c_api::internal::BuildFailureResponse;
using tracer_core::core::c_api::internal::BuildOperationResponse;
using tracer_core::core::c_api::internal::BuildInsightsTargetsResponse;
using tracer_core::core::c_api::internal::ClearLastError;
using tracer_core::core::c_api::internal::ParseInsightsDisplayMode;
using tracer_core::core::c_api::internal::ParseInsightsExportScope;
using tracer_core::core::c_api::internal::ParseInsightsFormat;
using tracer_core::core::c_api::internal::ParseInsightsOperationKind;
using tracer_core::core::c_api::internal::ParseTemporalSelectionKind;
using tracer_core::core::c_api::internal::RequireRuntime;
using tracer_core::core::c_api::internal::ToRequestJsonView;
using tracer_core::core::c_api::insights::BuildInsightsTextResponse;
using tracer_core::core::c_api::insights::SerializeTemporalStructuredInsights;
using tracer_core::core::dto::PeriodBatchQueryRequest;
using tracer_core::core::dto::InsightsOperationKind;
using tracer_core::core::dto::TemporalInsightsExportRequest;
using tracer_core::core::dto::TemporalInsightsQueryRequest;
using tracer_core::core::dto::TemporalInsightsTargetsRequest;
using tracer_core::core::dto::TemporalSelectionPayload;
using tracer_core::core::dto::TemporalStructuredInsightsOutput;
using tracer_core::core::dto::TemporalStructuredInsightsQueryRequest;

namespace {

using nlohmann::json;

auto BuildSelectionFromPayload(
    const tt_transport::TemporalInsightsRequestPayload& payload)
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
    const tt_transport::TemporalInsightsRequestPayload& payload)
    -> TemporalInsightsQueryRequest {
  TemporalInsightsQueryRequest request{};
  request.display_mode = ParseInsightsDisplayMode(payload.display_mode);
  request.selection = BuildSelectionFromPayload(payload).value_or(
      TemporalSelectionPayload{});
  if (payload.format.has_value()) {
    request.format = ParseInsightsFormat(*payload.format);
  }
  if (payload.locale.has_value()) {
    request.locale = *payload.locale;
  }
  return request;
}

auto BuildTemporalStructuredQueryRequest(
    const tt_transport::TemporalInsightsRequestPayload& payload)
    -> TemporalStructuredInsightsQueryRequest {
  return {
      .display_mode = ParseInsightsDisplayMode(payload.display_mode),
      .selection = BuildSelectionFromPayload(payload).value_or(
          TemporalSelectionPayload{}),
  };
}

auto BuildTemporalTargetsRequest(
    const tt_transport::TemporalInsightsRequestPayload& payload)
    -> TemporalInsightsTargetsRequest {
  try {
    return {.display_mode = ParseInsightsDisplayMode(payload.display_mode)};
  } catch (const std::invalid_argument&) {
    throw std::invalid_argument(
        "field `type` must be one of: day|week|month|year. Use display_mode "
        "to select the insights target type.");
  }
}

auto BuildTemporalExportRequest(
    const tt_transport::TemporalInsightsRequestPayload& payload,
    const std::filesystem::path& output_root) -> TemporalInsightsExportRequest {
  TemporalInsightsExportRequest request{};
  request.display_mode = ParseInsightsDisplayMode(payload.display_mode);
  request.export_scope =
      ParseInsightsExportScope(payload.export_scope.value_or("single"));
  if (payload.format.has_value()) {
    request.format = ParseInsightsFormat(*payload.format);
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

extern "C" TT_CORE_API auto tracer_core_runtime_temporal_insights_json(
    TtCoreRuntimeHandle* handle, const char* request_json) -> const char* {
  try {
    ClearLastError();
    ITracerCoreRuntime& runtime = RequireRuntime(handle);
    const auto payload =
        tt_transport::DecodeTemporalInsightsRequest(ToRequestJsonView(request_json));

    // The canonical insights ABI now multiplexes query/targets/export through
    // one temporal entrypoint so every host shares the same request contract.
    switch (ParseInsightsOperationKind(payload.operation_kind)) {
      case InsightsOperationKind::kQuery:
        return BuildInsightsTextResponse(
            runtime.insights().RunTemporalInsightsQuery(
                BuildTemporalQueryRequest(payload)));
      case InsightsOperationKind::kStructuredQuery:
        tracer_core::core::c_api::internal::g_last_response =
            SerializeTemporalStructuredInsights(
                runtime.insights().RunTemporalStructuredInsightsQuery(
                    BuildTemporalStructuredQueryRequest(payload)),
                handle->converter_config_toml_path)
                .dump();
        return tracer_core::core::c_api::internal::g_last_response.c_str();
      case InsightsOperationKind::kTargets:
        return BuildInsightsTargetsResponse(runtime.insights().RunTemporalInsightsTargetsQuery(
            BuildTemporalTargetsRequest(payload)));
      case InsightsOperationKind::kExport:
        return BuildOperationResponse(runtime.insights().RunTemporalInsightsExport(
            BuildTemporalExportRequest(payload, handle->output_root)));
    }

    return BuildFailureResponse(
        "Unsupported temporal insights operation kind.",
        "insights.unsupported_operation", "insights",
        {"Use query, structured_query, targets, or export."});
  } catch (const tracer_core::common::InsightsContractError& error) {
    return BuildFailureResponse(error.what(), error.error_code(),
                                error.error_category(), error.hints());
  } catch (const std::exception& error) {
    return BuildFailureResponse(error.what());
  } catch (...) {
    return BuildFailureResponse(
        "tracer_core_runtime_temporal_insights_json failed unexpectedly.");
  }
}

extern "C" TT_CORE_API auto tracer_core_runtime_insights_batch_json(
    TtCoreRuntimeHandle* handle, const char* request_json) -> const char* {
  try {
    ClearLastError();
    ITracerCoreRuntime& runtime = RequireRuntime(handle);
    const auto payload =
        tt_transport::DecodeInsightsBatchRequest(ToRequestJsonView(request_json));

    PeriodBatchQueryRequest request{};
    request.days_list = payload.days_list;
    if (payload.format.has_value()) {
      request.format = ParseInsightsFormat(*payload.format);
    }

    return BuildInsightsTextResponse(runtime.insights().RunPeriodBatchQuery(request));
  } catch (const tracer_core::common::InsightsContractError& error) {
    return BuildFailureResponse(error.what(), error.error_code(),
                                error.error_category(), error.hints());
  } catch (const std::exception& error) {
    return BuildFailureResponse(error.what());
  } catch (...) {
    return BuildFailureResponse(
        "tracer_core_runtime_insights_batch_json failed unexpectedly.");
  }
}
