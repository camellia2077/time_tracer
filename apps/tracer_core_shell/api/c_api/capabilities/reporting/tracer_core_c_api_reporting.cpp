import tracer.core.application.use_cases.interface;

#include <exception>

#include "api/c_api/capabilities/reporting/tracer_core_c_api_reporting_internal.hpp"
#include "api/c_api/tracer_core_c_api.h"
#include "api/c_api/runtime/tracer_core_c_api_internal.hpp"
#include "application/dto/reporting_requests.hpp"
#include "application/dto/shared_envelopes.hpp"
#include "shared/types/reporting_errors.hpp"
#include "tracer/transport/runtime_codec.hpp"

namespace tt_transport = tracer::transport;
using tracer::core::application::use_cases::ITracerCoreRuntime;

using tracer_core::core::c_api::internal::BuildFailureResponse;
using tracer_core::core::c_api::internal::BuildOperationResponse;
using tracer_core::core::c_api::internal::BuildReportTargetsResponse;
using tracer_core::core::c_api::internal::ClearLastError;
using tracer_core::core::c_api::internal::ParseExportType;
using tracer_core::core::c_api::internal::ParseReportFormat;
using tracer_core::core::c_api::internal::ParseReportType;
using tracer_core::core::c_api::internal::ParseReportTargetType;
using tracer_core::core::c_api::internal::RequireRuntime;
using tracer_core::core::c_api::internal::ToRequestJsonView;
using tracer_core::core::c_api::reporting::BuildReportTextResponse;
using tracer_core::core::c_api::reporting::ExportAllLegacyCompatibility;
using tracer_core::core::c_api::reporting::ExportSpecificLegacyCompatibility;
using tracer_core::core::dto::OperationAck;
using tracer_core::core::dto::PeriodBatchQueryRequest;
using tracer_core::core::dto::ReportExportRequest;
using tracer_core::core::dto::ReportQueryRequest;
using tracer_core::core::dto::ReportTargetsRequest;

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
  } catch (const tracer_core::common::ReportingContractError& error) {
    return BuildFailureResponse(error.what(), error.error_code(),
                                error.error_category(), error.hints());
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

extern "C" TT_CORE_API auto tracer_core_runtime_report_targets_json(
    TtCoreRuntimeHandle* handle, const char* request_json) -> const char* {
  try {
    ClearLastError();
    ITracerCoreRuntime& runtime = RequireRuntime(handle);
    const auto kPayload =
        tt_transport::DecodeReportTargetsRequest(ToRequestJsonView(request_json));

    ReportTargetsRequest request{};
    request.type = ParseReportTargetType(kPayload.type);
    return BuildReportTargetsResponse(runtime.report().RunReportTargetsQuery(request));
  } catch (const tracer_core::common::ReportingContractError& error) {
    return BuildFailureResponse(error.what(), error.error_code(),
                                error.error_category(), error.hints());
  } catch (const std::exception& error) {
    return BuildFailureResponse(error.what());
  } catch (...) {
    return BuildFailureResponse(
        "tracer_core_runtime_report_targets_json failed unexpectedly.");
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

    using tracer_core::core::dto::ReportExportType;
    switch (request.type) {
      case ReportExportType::kDay:
      case ReportExportType::kMonth:
      case ReportExportType::kRecent:
      case ReportExportType::kWeek:
      case ReportExportType::kYear:
        ExportSpecificLegacyCompatibility(runtime, handle->output_root, request);
        break;
      case ReportExportType::kAllDay:
      case ReportExportType::kAllMonth:
      case ReportExportType::kAllRecent:
      case ReportExportType::kAllWeek:
      case ReportExportType::kAllYear:
        ExportAllLegacyCompatibility(runtime, handle->output_root, request);
        break;
    }
    return BuildOperationResponse(OperationAck{.ok = true, .error_message = ""});
  } catch (const tracer_core::common::ReportingContractError& error) {
    return BuildFailureResponse(error.what(), error.error_code(),
                                error.error_category(), error.hints());
  } catch (const std::exception& error) {
    return BuildFailureResponse(error.what());
  } catch (...) {
    return BuildFailureResponse(
        "tracer_core_runtime_export_json failed unexpectedly.");
  }
}
