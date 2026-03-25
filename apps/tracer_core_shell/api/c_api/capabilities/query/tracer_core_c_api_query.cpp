import tracer.core.application.use_cases.interface;

#include <exception>

#include "api/c_api/tracer_core_c_api.h"
#include "api/c_api/runtime/tracer_core_c_api_internal.hpp"
#include "application/dto/query_requests.hpp"
#include "application/dto/query_responses.hpp"
#include "tracer/transport/runtime_codec.hpp"

namespace tt_transport = tracer::transport;
using tracer::core::application::use_cases::ITracerCoreRuntime;

using tracer_core::core::c_api::internal::BuildFailureResponse;
using tracer_core::core::c_api::internal::BuildTextResponse;
using tracer_core::core::c_api::internal::BuildTreeResponse;
using tracer_core::core::c_api::internal::ClearLastError;
using tracer_core::core::c_api::internal::ParseDataQueryOutputMode;
using tracer_core::core::c_api::internal::ParseQueryAction;
using tracer_core::core::c_api::internal::RequireRuntime;
using tracer_core::core::c_api::internal::ToRequestJsonView;
using tracer_core::core::dto::DataQueryRequest;
using tracer_core::core::dto::TreeQueryRequest;

extern "C" TT_CORE_API auto tracer_core_runtime_query_json(
    TtCoreRuntimeHandle* handle, const char* request_json) -> const char* {
  try {
    ClearLastError();
    ITracerCoreRuntime& runtime = RequireRuntime(handle);
    const auto kPayload =
        tt_transport::DecodeQueryRequest(ToRequestJsonView(request_json));

    DataQueryRequest request{};
    request.action = ParseQueryAction(kPayload.action);
    if (kPayload.output_mode.has_value()) {
      request.output_mode = ParseDataQueryOutputMode(*kPayload.output_mode);
    }
    request.year = kPayload.year;
    request.month = kPayload.month;
    request.from_date = kPayload.from_date;
    request.to_date = kPayload.to_date;
    request.remark = kPayload.remark;
    request.day_remark = kPayload.day_remark;
    request.project = kPayload.project;
    request.root = kPayload.root;
    request.exercise = kPayload.exercise;
    request.status = kPayload.status;
    request.limit = kPayload.limit;
    request.top_n = kPayload.top_n;
    request.lookback_days = kPayload.lookback_days;
    request.activity_prefix = kPayload.activity_prefix;
    request.tree_period = kPayload.tree_period;
    request.tree_period_argument = kPayload.tree_period_argument;
    request.tree_max_depth = kPayload.tree_max_depth;
    if (kPayload.overnight.has_value()) {
      request.overnight = *kPayload.overnight;
    }
    if (kPayload.reverse.has_value()) {
      request.reverse = *kPayload.reverse;
    }
    if (kPayload.activity_score_by_duration.has_value()) {
      request.activity_score_by_duration = *kPayload.activity_score_by_duration;
    }

    const auto kResponse = runtime.query().RunDataQuery(request);
    return BuildTextResponse(kResponse);
  } catch (const std::exception& error) {
    return BuildFailureResponse(error.what());
  } catch (...) {
    return BuildFailureResponse(
        "tracer_core_runtime_query_json failed unexpectedly.");
  }
}

extern "C" TT_CORE_API auto tracer_core_runtime_tree_json(
    TtCoreRuntimeHandle* handle, const char* request_json) -> const char* {
  try {
    ClearLastError();
    ITracerCoreRuntime& runtime = RequireRuntime(handle);
    const auto kPayload =
        tt_transport::DecodeTreeRequest(ToRequestJsonView(request_json));

    TreeQueryRequest request{};
    if (kPayload.list_roots.has_value()) {
      request.list_roots = *kPayload.list_roots;
    }
    if (kPayload.root_pattern.has_value()) {
      request.root_pattern = *kPayload.root_pattern;
    }
    if (kPayload.max_depth.has_value()) {
      request.max_depth = *kPayload.max_depth;
    }
    if (kPayload.period.has_value()) {
      request.period = *kPayload.period;
    }
    if (kPayload.period_argument.has_value()) {
      request.period_argument = *kPayload.period_argument;
    }
    if (kPayload.root.has_value()) {
      request.root = *kPayload.root;
    }

    return BuildTreeResponse(runtime.query().RunTreeQuery(request));
  } catch (const std::exception& error) {
    return BuildFailureResponse(error.what());
  } catch (...) {
    return BuildFailureResponse(
        "tracer_core_runtime_tree_json failed unexpectedly.");
  }
}
