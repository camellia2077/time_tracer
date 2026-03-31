#include "application/use_cases/pipeline_api.hpp"

#include "application/use_cases/core_api_failure.hpp"

import tracer.core.domain.types.app_options;

namespace tracer::core::application::use_cases {

using tracer_core::core::dto::ConvertRequest;
using tracer_core::core::dto::IngestSyncStatusOutput;
using tracer_core::core::dto::IngestSyncStatusRequest;
using tracer_core::core::dto::OperationAck;
using tracer_core::core::dto::IngestRequest;
using tracer_core::core::dto::ImportRequest;
using tracer_core::core::dto::ValidateStructureRequest;
using tracer_core::core::dto::ValidateLogicRequest;
using tracer_core::core::dto::RecordActivityAtomicallyRequest;
using tracer_core::core::dto::RecordActivityAtomicallyResponse;
using tracer::core::domain::types::AppOptions;
namespace core_api_failure = tracer::core::application::use_cases::failure;

PipelineApi::PipelineApi(pipeline::IPipelineWorkflow& pipeline_workflow)
    : pipeline_workflow_(pipeline_workflow) {}

auto PipelineApi::RunConvert(const ConvertRequest& request) -> OperationAck {
  try {
    AppOptions options;
    options.input_path = request.input_path;
    options.convert = true;
    options.validate_structure = request.validate_structure;
    options.validate_logic = request.validate_logic;
    options.run_structure_validation_before_conversion = request.validate_logic;
    options.date_check_mode = request.date_check_mode;
    options.save_processed_output = request.save_processed_output;

    pipeline_workflow_.RunConverter(request.input_path, options);
    return {.ok = true, .error_message = ""};
  } catch (const std::exception& exception) {
    return core_api_failure::BuildOperationFailure("RunConvert", exception);
  } catch (...) {
    return core_api_failure::BuildOperationFailure("RunConvert");
  }
}

auto PipelineApi::RunIngest(const IngestRequest& request) -> OperationAck {
  try {
    pipeline_workflow_.RunIngest(request.input_path, request.date_check_mode,
                                 request.save_processed_output,
                                 request.ingest_mode);
    return {.ok = true, .error_message = ""};
  } catch (const std::exception& exception) {
    return core_api_failure::BuildOperationFailure("RunIngest", exception);
  } catch (...) {
    return core_api_failure::BuildOperationFailure("RunIngest");
  }
}

auto PipelineApi::RunIngestSyncStatusQuery(
    const IngestSyncStatusRequest& request) -> IngestSyncStatusOutput {
  try {
    return pipeline_workflow_.RunIngestSyncStatusQuery(request);
  } catch (const std::exception& exception) {
    return {.ok = false,
            .items = {},
            .error_message = core_api_failure::BuildErrorMessage(
                "RunIngestSyncStatusQuery", exception.what())};
  } catch (...) {
    return {.ok = false,
            .items = {},
            .error_message = core_api_failure::BuildErrorMessage(
                "RunIngestSyncStatusQuery", "Unknown error.")};
  }
}

auto PipelineApi::ClearIngestSyncStatus() -> OperationAck {
  try {
    pipeline_workflow_.ClearIngestSyncStatus();
    return {.ok = true, .error_message = ""};
  } catch (const std::exception& exception) {
    return core_api_failure::BuildOperationFailure("ClearIngestSyncStatus",
                                                   exception);
  } catch (...) {
    return core_api_failure::BuildOperationFailure("ClearIngestSyncStatus");
  }
}

auto PipelineApi::RunImport(const ImportRequest& request) -> OperationAck {
  try {
    pipeline_workflow_.RunDatabaseImport(request.processed_path);
    return {.ok = true, .error_message = ""};
  } catch (const std::exception& exception) {
    return core_api_failure::BuildOperationFailure("RunImport", exception);
  } catch (...) {
    return core_api_failure::BuildOperationFailure("RunImport");
  }
}

auto PipelineApi::RunValidateStructure(const ValidateStructureRequest& request)
    -> OperationAck {
  try {
    pipeline_workflow_.RunValidateStructure(request.input_path);
    return {.ok = true, .error_message = ""};
  } catch (const std::exception& exception) {
    return core_api_failure::BuildOperationFailure("RunValidateStructure",
                                                   exception);
  } catch (...) {
    return core_api_failure::BuildOperationFailure("RunValidateStructure");
  }
}

auto PipelineApi::RunValidateLogic(const ValidateLogicRequest& request)
    -> OperationAck {
  try {
    pipeline_workflow_.RunValidateLogic(request.input_path,
                                        request.date_check_mode);
    return {.ok = true, .error_message = ""};
  } catch (const std::exception& exception) {
    return core_api_failure::BuildOperationFailure("RunValidateLogic",
                                                   exception);
  } catch (...) {
    return core_api_failure::BuildOperationFailure("RunValidateLogic");
  }
}

auto PipelineApi::RunRecordActivityAtomically(
    const RecordActivityAtomicallyRequest& request)
    -> RecordActivityAtomicallyResponse {
  try {
    return pipeline_workflow_.RunRecordActivityAtomically(request);
  } catch (const std::exception& exception) {
    return {.ok = false,
            .message = core_api_failure::BuildErrorMessage(
                "RunRecordActivityAtomically", exception.what()),
            .operation_id = "",
            .warnings = {},
            .rollback_failed = false,
            .retained_transaction_root = std::nullopt};
  } catch (...) {
    return {.ok = false,
            .message = core_api_failure::BuildErrorMessage(
                "RunRecordActivityAtomically", "Unknown error."),
            .operation_id = "",
            .warnings = {},
            .rollback_failed = false,
            .retained_transaction_root = std::nullopt};
  }
}

}  // namespace tracer::core::application::use_cases
