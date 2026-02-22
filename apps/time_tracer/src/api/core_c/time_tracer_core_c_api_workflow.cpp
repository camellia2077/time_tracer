#include <exception>

#include "api/core_c/time_tracer_core_c_api.h"
#include "api/core_c/time_tracer_core_c_api_internal.hpp"
#include "tracer/transport/runtime_codec.hpp"

namespace tt_transport = tracer::transport;

using time_tracer::core::c_api::internal::BuildFailureResponse;
using time_tracer::core::c_api::internal::BuildOperationResponse;
using time_tracer::core::c_api::internal::ClearLastError;
using time_tracer::core::c_api::internal::ParseDateCheckMode;
using time_tracer::core::c_api::internal::ParseIngestMode;
using time_tracer::core::c_api::internal::RequireRuntime;
using time_tracer::core::c_api::internal::ToRequestJsonView;
using time_tracer::core::dto::ConvertRequest;
using time_tracer::core::dto::ImportRequest;
using time_tracer::core::dto::IngestRequest;
using time_tracer::core::dto::ValidateLogicRequest;
using time_tracer::core::dto::ValidateStructureRequest;

extern "C" TT_CORE_API auto tracer_core_runtime_ingest_json(
    TtCoreRuntimeHandle* handle, const char* request_json) -> const char* {
  try {
    ClearLastError();
    ITimeTracerCoreApi& runtime = RequireRuntime(handle);
    const auto kPayload =
        tt_transport::DecodeIngestRequest(ToRequestJsonView(request_json));

    IngestRequest request{};
    request.input_path = kPayload.input_path;
    if (kPayload.date_check_mode.has_value()) {
      request.date_check_mode = ParseDateCheckMode(*kPayload.date_check_mode);
    }
    if (kPayload.save_processed_output.has_value()) {
      request.save_processed_output = *kPayload.save_processed_output;
    }
    if (kPayload.ingest_mode.has_value()) {
      request.ingest_mode = ParseIngestMode(*kPayload.ingest_mode);
    }

    const auto kResponse = runtime.RunIngest(request);
    return BuildOperationResponse(kResponse);
  } catch (const std::exception& error) {
    return BuildFailureResponse(error.what());
  } catch (...) {
    return BuildFailureResponse(
        "tracer_core_runtime_ingest_json failed unexpectedly.");
  }
}

extern "C" TT_CORE_API auto tracer_core_runtime_convert_json(
    TtCoreRuntimeHandle* handle, const char* request_json) -> const char* {
  try {
    ClearLastError();
    ITimeTracerCoreApi& runtime = RequireRuntime(handle);
    const auto kPayload =
        tt_transport::DecodeConvertRequest(ToRequestJsonView(request_json));

    ConvertRequest request{};
    request.input_path = kPayload.input_path;

    if (kPayload.date_check_mode.has_value()) {
      request.date_check_mode = ParseDateCheckMode(*kPayload.date_check_mode);
    }
    if (kPayload.save_processed_output.has_value()) {
      request.save_processed_output = *kPayload.save_processed_output;
    }
    if (kPayload.validate_logic.has_value()) {
      request.validate_logic = *kPayload.validate_logic;
    }
    if (kPayload.validate_structure.has_value()) {
      request.validate_structure = *kPayload.validate_structure;
    }

    return BuildOperationResponse(runtime.RunConvert(request));
  } catch (const std::exception& error) {
    return BuildFailureResponse(error.what());
  } catch (...) {
    return BuildFailureResponse(
        "tracer_core_runtime_convert_json failed unexpectedly.");
  }
}

extern "C" TT_CORE_API auto tracer_core_runtime_import_json(
    TtCoreRuntimeHandle* handle, const char* request_json) -> const char* {
  try {
    ClearLastError();
    ITimeTracerCoreApi& runtime = RequireRuntime(handle);
    const auto kPayload =
        tt_transport::DecodeImportRequest(ToRequestJsonView(request_json));

    ImportRequest request{};
    request.processed_path = kPayload.processed_path;

    return BuildOperationResponse(runtime.RunImport(request));
  } catch (const std::exception& error) {
    return BuildFailureResponse(error.what());
  } catch (...) {
    return BuildFailureResponse(
        "tracer_core_runtime_import_json failed unexpectedly.");
  }
}

extern "C" TT_CORE_API auto tracer_core_runtime_validate_structure_json(
    TtCoreRuntimeHandle* handle, const char* request_json) -> const char* {
  try {
    ClearLastError();
    ITimeTracerCoreApi& runtime = RequireRuntime(handle);
    const auto kPayload = tt_transport::DecodeValidateStructureRequest(
        ToRequestJsonView(request_json));

    ValidateStructureRequest request{};
    request.input_path = kPayload.input_path;

    return BuildOperationResponse(runtime.RunValidateStructure(request));
  } catch (const std::exception& error) {
    return BuildFailureResponse(error.what());
  } catch (...) {
    return BuildFailureResponse(
        "tracer_core_runtime_validate_structure_json failed unexpectedly.");
  }
}

extern "C" TT_CORE_API auto tracer_core_runtime_validate_logic_json(
    TtCoreRuntimeHandle* handle, const char* request_json) -> const char* {
  try {
    ClearLastError();
    ITimeTracerCoreApi& runtime = RequireRuntime(handle);
    const auto kPayload = tt_transport::DecodeValidateLogicRequest(
        ToRequestJsonView(request_json));

    ValidateLogicRequest request{};
    request.input_path = kPayload.input_path;
    if (kPayload.date_check_mode.has_value()) {
      request.date_check_mode = ParseDateCheckMode(*kPayload.date_check_mode);
    }

    return BuildOperationResponse(runtime.RunValidateLogic(request));
  } catch (const std::exception& error) {
    return BuildFailureResponse(error.what());
  } catch (...) {
    return BuildFailureResponse(
        "tracer_core_runtime_validate_logic_json failed unexpectedly.");
  }
}
