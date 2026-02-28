// api/core_c/tracer_core_c_api_reporting.cpp
#include <sodium.h>

#include <array>
#include <exception>
#include <optional>
#include <string>
#include <string_view>

#include "api/core_c/tracer_core_c_api.h"
#include "api/core_c/tracer_core_c_api_internal.hpp"
#include "tracer/transport/envelope.hpp"
#include "tracer/transport/runtime_codec.hpp"

namespace tt_transport = tracer::transport;

using tracer_core::core::c_api::internal::BuildFailureResponse;
using tracer_core::core::c_api::internal::BuildOperationResponse;
using tracer_core::core::c_api::internal::BuildTextResponse;
using tracer_core::core::c_api::internal::BuildTreeResponse;
using tracer_core::core::c_api::internal::ClearLastError;
using tracer_core::core::c_api::internal::ParseDataQueryOutputMode;
using tracer_core::core::c_api::internal::ParseExportType;
using tracer_core::core::c_api::internal::ParseQueryAction;
using tracer_core::core::c_api::internal::ParseReportFormat;
using tracer_core::core::c_api::internal::ParseReportType;
using tracer_core::core::c_api::internal::RequireRuntime;
using tracer_core::core::c_api::internal::ToRequestJsonView;
using tracer_core::core::dto::DataQueryRequest;
using tracer_core::core::dto::PeriodBatchQueryRequest;
using tracer_core::core::dto::ReportExportRequest;
using tracer_core::core::dto::ReportQueryRequest;
using tracer_core::core::dto::TreeQueryRequest;

namespace {

auto ToLowerHex(const unsigned char* bytes, size_t size) -> std::string {
  constexpr char kHex[] = "0123456789abcdef";
  std::string out;
  out.resize(size * 2U);
  for (size_t index = 0; index < size; ++index) {
    const unsigned char byte = bytes[index];
    out[index * 2U] = kHex[(byte >> 4U) & 0x0FU];
    out[index * 2U + 1U] = kHex[byte & 0x0FU];
  }
  return out;
}

auto ComputeSha256Hex(std::string_view text) -> std::optional<std::string> {
  std::array<unsigned char, crypto_hash_sha256_BYTES> digest{};
  const auto* input = reinterpret_cast<const unsigned char*>(text.data());
  if (crypto_hash_sha256(digest.data(), input,
                         static_cast<unsigned long long>(text.size())) != 0) {
    return std::nullopt;
  }
  return ToLowerHex(digest.data(), digest.size());
}

auto BuildReportTextResponse(const tracer_core::core::dto::TextOutput& output)
    -> const char* {
  if (!output.ok) {
    return BuildTextResponse(output);
  }

  auto envelope = tt_transport::BuildResponseEnvelope(
      output.ok, output.error_message, output.content);
  envelope.report_hash_sha256 = ComputeSha256Hex(output.content);
  tracer_core::core::c_api::internal::g_last_response =
      tt_transport::SerializeResponseEnvelope(envelope);
  return tracer_core::core::c_api::internal::g_last_response.c_str();
}

}  // namespace

extern "C" TT_CORE_API auto tracer_core_runtime_query_json(
    TtCoreRuntimeHandle* handle, const char* request_json) -> const char* {
  try {
    ClearLastError();
    ITracerCoreApi& runtime = RequireRuntime(handle);
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

    const auto kResponse = runtime.RunDataQuery(request);
    return BuildTextResponse(kResponse);
  } catch (const std::exception& error) {
    return BuildFailureResponse(error.what());
  } catch (...) {
    return BuildFailureResponse(
        "tracer_core_runtime_query_json failed unexpectedly.");
  }
}

extern "C" TT_CORE_API auto tracer_core_runtime_report_json(
    TtCoreRuntimeHandle* handle, const char* request_json) -> const char* {
  try {
    ClearLastError();
    ITracerCoreApi& runtime = RequireRuntime(handle);
    const auto kPayload =
        tt_transport::DecodeReportRequest(ToRequestJsonView(request_json));

    ReportQueryRequest request{};
    request.type = ParseReportType(kPayload.type);
    request.argument = kPayload.argument;
    if (kPayload.format.has_value()) {
      request.format = ParseReportFormat(*kPayload.format);
    }

    return BuildReportTextResponse(runtime.RunReportQuery(request));
  } catch (const std::exception& error) {
    return BuildFailureResponse(error.what());
  } catch (...) {
    return BuildFailureResponse(
        "tracer_core_runtime_report_json failed unexpectedly.");
  }
}

extern "C" TT_CORE_API auto tracer_core_runtime_report_batch_json(
    TtCoreRuntimeHandle* handle, const char* request_json) -> const char* {
  try {
    ClearLastError();
    ITracerCoreApi& runtime = RequireRuntime(handle);
    const auto kPayload =
        tt_transport::DecodeReportBatchRequest(ToRequestJsonView(request_json));

    PeriodBatchQueryRequest request{};
    request.days_list = kPayload.days_list;
    if (kPayload.format.has_value()) {
      request.format = ParseReportFormat(*kPayload.format);
    }

    return BuildReportTextResponse(runtime.RunPeriodBatchQuery(request));
  } catch (const std::exception& error) {
    return BuildFailureResponse(error.what());
  } catch (...) {
    return BuildFailureResponse(
        "tracer_core_runtime_report_batch_json failed unexpectedly.");
  }
}

extern "C" TT_CORE_API auto tracer_core_runtime_export_json(
    TtCoreRuntimeHandle* handle, const char* request_json) -> const char* {
  try {
    ClearLastError();
    ITracerCoreApi& runtime = RequireRuntime(handle);
    const auto kPayload =
        tt_transport::DecodeExportRequest(ToRequestJsonView(request_json));

    ReportExportRequest request{};
    request.type = ParseExportType(kPayload.type);
    if (kPayload.argument.has_value()) {
      request.argument = *kPayload.argument;
    }
    if (kPayload.format.has_value()) {
      request.format = ParseReportFormat(*kPayload.format);
    }
    if (kPayload.recent_days_list.has_value()) {
      request.recent_days_list = *kPayload.recent_days_list;
    }

    return BuildOperationResponse(runtime.RunReportExport(request));
  } catch (const std::exception& error) {
    return BuildFailureResponse(error.what());
  } catch (...) {
    return BuildFailureResponse(
        "tracer_core_runtime_export_json failed unexpectedly.");
  }
}

extern "C" TT_CORE_API auto tracer_core_runtime_tree_json(
    TtCoreRuntimeHandle* handle, const char* request_json) -> const char* {
  try {
    ClearLastError();
    ITracerCoreApi& runtime = RequireRuntime(handle);
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

    return BuildTreeResponse(runtime.RunTreeQuery(request));
  } catch (const std::exception& error) {
    return BuildFailureResponse(error.what());
  } catch (...) {
    return BuildFailureResponse(
        "tracer_core_runtime_tree_json failed unexpectedly.");
  }
}
