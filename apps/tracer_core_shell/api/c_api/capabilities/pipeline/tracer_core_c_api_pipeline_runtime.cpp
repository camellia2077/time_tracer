import tracer.core.application.use_cases.interface;

#include <exception>

#include "api/c_api/tracer_core_c_api.h"
#include "api/c_api/runtime/tracer_core_c_api_internal.hpp"
#include "application/dto/pipeline_requests.hpp"
#include "tracer/transport/runtime_codec.hpp"

namespace tt_transport = tracer::transport;
using tracer::core::application::use_cases::IPipelineApi;

using tracer_core::core::c_api::internal::BuildFailureResponse;
using tracer_core::core::c_api::internal::BuildOperationResponse;
using tracer_core::core::c_api::internal::ClearLastError;
using tracer_core::core::c_api::internal::ParseDateCheckMode;
using tracer_core::core::c_api::internal::ParseIngestMode;
using tracer_core::core::c_api::internal::RequirePipelineRuntime;
using tracer_core::core::c_api::internal::ToRequestJsonView;
using tracer_core::core::dto::IngestRequest;
using tracer_core::core::dto::ValidateLogicRequest;
using tracer_core::core::dto::ValidateStructureRequest;

extern "C" TT_CORE_API auto tracer_core_pipeline_runtime_ingest_json(
    TtCoreRuntimeHandle* handle, const char* request_json) -> const char* {
  try {
    ClearLastError();
    IPipelineApi& pipeline = RequirePipelineRuntime(handle);
    const auto payload =
        tt_transport::DecodeIngestRequest(ToRequestJsonView(request_json));

    IngestRequest request{};
    request.input_path = payload.input_path;
    if (payload.date_check_mode.has_value()) {
      request.date_check_mode = ParseDateCheckMode(*payload.date_check_mode);
    }
    if (payload.save_processed_output.has_value()) {
      request.save_processed_output = *payload.save_processed_output;
    }
    if (payload.ingest_mode.has_value()) {
      request.ingest_mode = ParseIngestMode(*payload.ingest_mode);
    }
    return BuildOperationResponse(pipeline.RunIngest(request));
  } catch (const std::exception& error) {
    return BuildFailureResponse(error.what());
  } catch (...) {
    return BuildFailureResponse(
        "tracer_core_pipeline_runtime_ingest_json failed unexpectedly.");
  }
}

extern "C" TT_CORE_API auto
tracer_core_pipeline_runtime_validate_structure_json(
    TtCoreRuntimeHandle* handle, const char* request_json) -> const char* {
  try {
    ClearLastError();
    IPipelineApi& pipeline = RequirePipelineRuntime(handle);
    const auto payload = tt_transport::DecodeValidateStructureRequest(
        ToRequestJsonView(request_json));
    return BuildOperationResponse(pipeline.RunValidateStructure(
        ValidateStructureRequest{.input_path = payload.input_path}));
  } catch (const std::exception& error) {
    return BuildFailureResponse(error.what());
  } catch (...) {
    return BuildFailureResponse(
        "tracer_core_pipeline_runtime_validate_structure_json failed "
        "unexpectedly.");
  }
}

extern "C" TT_CORE_API auto tracer_core_pipeline_runtime_validate_logic_json(
    TtCoreRuntimeHandle* handle, const char* request_json) -> const char* {
  try {
    ClearLastError();
    IPipelineApi& pipeline = RequirePipelineRuntime(handle);
    const auto payload = tt_transport::DecodeValidateLogicRequest(
        ToRequestJsonView(request_json));
    ValidateLogicRequest request{};
    request.input_path = payload.input_path;
    if (payload.date_check_mode.has_value()) {
      request.date_check_mode = ParseDateCheckMode(*payload.date_check_mode);
    }
    return BuildOperationResponse(pipeline.RunValidateLogic(request));
  } catch (const std::exception& error) {
    return BuildFailureResponse(error.what());
  } catch (...) {
    return BuildFailureResponse(
        "tracer_core_pipeline_runtime_validate_logic_json failed "
        "unexpectedly.");
  }
}
