import tracer.core.application.use_cases.interface;

#include <sodium.h>

#include <array>
#include <cstdint>
#include <exception>
#include <optional>
#include <string>
#include <string_view>

#include "api/c_api/tracer_core_c_api.h"
#include "api/c_api/runtime/tracer_core_c_api_internal.hpp"
#include "application/dto/reporting_requests.hpp"
#include "application/dto/shared_envelopes.hpp"
#include "tracer/transport/envelope.hpp"
#include "tracer/transport/runtime_codec.hpp"

namespace tt_transport = tracer::transport;
using tracer::core::application::use_cases::ITracerCoreRuntime;

using tracer_core::core::c_api::internal::BuildFailureResponse;
using tracer_core::core::c_api::internal::BuildOperationResponse;
using tracer_core::core::c_api::internal::BuildTextResponse;
using tracer_core::core::c_api::internal::ClearLastError;
using tracer_core::core::c_api::internal::ParseExportType;
using tracer_core::core::c_api::internal::ParseReportFormat;
using tracer_core::core::c_api::internal::ParseReportType;
using tracer_core::core::c_api::internal::RequireRuntime;
using tracer_core::core::c_api::internal::ToRequestJsonView;
using tracer_core::core::dto::PeriodBatchQueryRequest;
using tracer_core::core::dto::ReportExportRequest;
using tracer_core::core::dto::ReportQueryRequest;

namespace {

auto ToLowerHex(const unsigned char* bytes, size_t size) -> std::string {
  constexpr std::array<char, 16> kHex = {
      '0', '1', '2', '3', '4', '5', '6', '7',
      '8', '9', 'a', 'b', 'c', 'd', 'e', 'f',
  };
  constexpr unsigned kHexShiftBits = 4U;
  constexpr std::uint8_t kHexNibbleMask = 0x0FU;
  std::string out;
  out.resize(size * 2U);
  for (size_t index = 0; index < size; ++index) {
    const unsigned char kByte = bytes[index];
    out[index * 2U] = kHex[(kByte >> kHexShiftBits) & kHexNibbleMask];
    out[(index * 2U) + 1U] = kHex[kByte & kHexNibbleMask];
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

extern "C" TT_CORE_API auto tracer_core_runtime_report_json(
    TtCoreRuntimeHandle* handle, const char* request_json) -> const char* {
  try {
    ClearLastError();
    ITracerCoreRuntime& runtime = RequireRuntime(handle);
    const auto kPayload =
        tt_transport::DecodeReportRequest(ToRequestJsonView(request_json));

    ReportQueryRequest request{};
    request.type = ParseReportType(kPayload.type);
    request.argument = kPayload.argument;
    if (kPayload.format.has_value()) {
      request.format = ParseReportFormat(*kPayload.format);
    }

    return BuildReportTextResponse(runtime.report().RunReportQuery(request));
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
    ITracerCoreRuntime& runtime = RequireRuntime(handle);
    const auto kPayload =
        tt_transport::DecodeReportBatchRequest(ToRequestJsonView(request_json));

    PeriodBatchQueryRequest request{};
    request.days_list = kPayload.days_list;
    if (kPayload.format.has_value()) {
      request.format = ParseReportFormat(*kPayload.format);
    }

    return BuildReportTextResponse(
        runtime.report().RunPeriodBatchQuery(request));
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
    ITracerCoreRuntime& runtime = RequireRuntime(handle);
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

    return BuildOperationResponse(runtime.report().RunReportExport(request));
  } catch (const std::exception& error) {
    return BuildFailureResponse(error.what());
  } catch (...) {
    return BuildFailureResponse(
        "tracer_core_runtime_export_json failed unexpectedly.");
  }
}
