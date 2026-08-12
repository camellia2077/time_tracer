#include "application/use_cases/pipeline_api.hpp"

#include <optional>

#include "application/use_cases/core_api_failure.hpp"

import tracer.core.domain.types.app_options;

namespace tracer::core::application::use_cases {

using tracer::core::domain::types::AppOptions;
using tracer_core::core::dto::ConvertRequest;
using tracer_core::core::dto::ConvertTxtActivityNamesRequest;
using tracer_core::core::dto::ConvertTxtActivityNamesResponse;
using tracer_core::core::dto::DefaultTxtDayMarkerRequest;
using tracer_core::core::dto::DefaultTxtDayMarkerResponse;
using tracer_core::core::dto::ImportRequest;
using tracer_core::core::dto::IngestRequest;
using tracer_core::core::dto::IngestSyncStatusOutput;
using tracer_core::core::dto::IngestSyncStatusRequest;
using tracer_core::core::dto::OperationAck;
using tracer_core::core::dto::RecordActivityAtomicallyRequest;
using tracer_core::core::dto::RecordActivityAtomicallyResponse;
using tracer_core::core::dto::ReplaceTxtAliasActivityNamesRequest;
using tracer_core::core::dto::ReplaceTxtAliasActivityNamesResponse;
using tracer_core::core::dto::ReplaceTxtCanonicalActivityNamesRequest;
using tracer_core::core::dto::ReplaceTxtCanonicalActivityNamesResponse;
using tracer_core::core::dto::ReplaceTxtDayBlockRequest;
using tracer_core::core::dto::ReplaceTxtDayBlockResponse;
using tracer_core::core::dto::ResolveTxtDayEditRequest;
using tracer_core::core::dto::ResolveTxtDayEditResponse;
using tracer_core::core::dto::ResolveTxtDayBlockRequest;
using tracer_core::core::dto::ResolveTxtDayBlockResponse;
using tracer_core::core::dto::ApplyTxtDayEditRequest;
using tracer_core::core::dto::ApplyTxtDayEditResponse;
using tracer_core::core::dto::UpdateActivityRemarkAtomicallyRequest;
using tracer_core::core::dto::UpdateActivityRemarkAtomicallyResponse;
using tracer_core::core::dto::UpdateDayRemarkAtomicallyRequest;
using tracer_core::core::dto::UpdateDayRemarkAtomicallyResponse;
using tracer_core::core::dto::ValidateLogicRequest;
using tracer_core::core::dto::ValidateStructureRequest;
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

auto PipelineApi::RunDefaultTxtDayMarker(
    const DefaultTxtDayMarkerRequest& request) -> DefaultTxtDayMarkerResponse {
  try {
    return pipeline_workflow_.RunDefaultTxtDayMarker(request);
  } catch (const std::exception& exception) {
    return {.ok = false,
            .normalized_day_marker = "",
            .error_message = core_api_failure::BuildErrorMessage(
                "RunDefaultTxtDayMarker", exception.what())};
  } catch (...) {
    return {.ok = false,
            .normalized_day_marker = "",
            .error_message = core_api_failure::BuildErrorMessage(
                "RunDefaultTxtDayMarker", "Unknown error.")};
  }
}

auto PipelineApi::RunResolveTxtDayBlock(
    const ResolveTxtDayBlockRequest& request) -> ResolveTxtDayBlockResponse {
  try {
    return pipeline_workflow_.RunResolveTxtDayBlock(request);
  } catch (const std::exception& exception) {
    return {.ok = false,
            .normalized_day_marker = "",
            .found = false,
            .is_marker_valid = false,
            .can_save = false,
            .day_body = "",
            .day_content_iso_date = std::nullopt,
            .error_message = core_api_failure::BuildErrorMessage(
                "RunResolveTxtDayBlock", exception.what())};
  } catch (...) {
    return {.ok = false,
            .normalized_day_marker = "",
            .found = false,
            .is_marker_valid = false,
            .can_save = false,
            .day_body = "",
            .day_content_iso_date = std::nullopt,
            .error_message = core_api_failure::BuildErrorMessage(
                "RunResolveTxtDayBlock", "Unknown error.")};
  }
}

auto PipelineApi::RunReplaceTxtDayBlock(
    const ReplaceTxtDayBlockRequest& request) -> ReplaceTxtDayBlockResponse {
  try {
    return pipeline_workflow_.RunReplaceTxtDayBlock(request);
  } catch (const std::exception& exception) {
    return {.ok = false,
            .normalized_day_marker = "",
            .found = false,
            .is_marker_valid = false,
            .updated_content = request.content,
            .error_message = core_api_failure::BuildErrorMessage(
                "RunReplaceTxtDayBlock", exception.what())};
  } catch (...) {
    return {.ok = false,
            .normalized_day_marker = "",
            .found = false,
            .is_marker_valid = false,
            .updated_content = request.content,
            .error_message = core_api_failure::BuildErrorMessage(
                "RunReplaceTxtDayBlock", "Unknown error.")};
  }
}

auto PipelineApi::RunResolveTxtDayEdit(
    const ResolveTxtDayEditRequest& request) -> ResolveTxtDayEditResponse {
  try {
    return pipeline_workflow_.RunResolveTxtDayEdit(request);
  } catch (const std::exception& exception) {
    return {.ok = false,
            .normalized_day_marker = "",
            .found = false,
            .is_marker_valid = false,
            .can_save = false,
            .day_remark = "",
            .events = {},
            .day_content_iso_date = std::nullopt,
            .error_message = core_api_failure::BuildErrorMessage(
                "RunResolveTxtDayEdit", exception.what())};
  } catch (...) {
    return {.ok = false,
            .normalized_day_marker = "",
            .found = false,
            .is_marker_valid = false,
            .can_save = false,
            .day_remark = "",
            .events = {},
            .day_content_iso_date = std::nullopt,
            .error_message = core_api_failure::BuildErrorMessage(
                "RunResolveTxtDayEdit", "Unknown error.")};
  }
}

auto PipelineApi::RunApplyTxtDayEdit(
    const ApplyTxtDayEditRequest& request) -> ApplyTxtDayEditResponse {
  try {
    return pipeline_workflow_.RunApplyTxtDayEdit(request);
  } catch (const std::exception& exception) {
    return {.ok = false,
            .normalized_day_marker = "",
            .found = false,
            .is_marker_valid = false,
            .updated_content = request.content,
            .error_message = core_api_failure::BuildErrorMessage(
                "RunApplyTxtDayEdit", exception.what())};
  } catch (...) {
    return {.ok = false,
            .normalized_day_marker = "",
            .found = false,
            .is_marker_valid = false,
            .updated_content = request.content,
            .error_message = core_api_failure::BuildErrorMessage(
                "RunApplyTxtDayEdit", "Unknown error.")};
  }
}

auto PipelineApi::RunUpdateActivityRemarkAtomically(
    const UpdateActivityRemarkAtomicallyRequest& request)
    -> UpdateActivityRemarkAtomicallyResponse {
  try {
    return pipeline_workflow_.RunUpdateActivityRemarkAtomically(request);
  } catch (const std::exception& exception) {
    return {.ok = false,
            .message = core_api_failure::BuildErrorMessage(
                "RunUpdateActivityRemarkAtomically", exception.what())};
  } catch (...) {
    return {.ok = false,
            .message = core_api_failure::BuildErrorMessage(
                "RunUpdateActivityRemarkAtomically", "Unknown error.")};
  }
}

auto PipelineApi::RunUpdateDayRemarkAtomically(
    const UpdateDayRemarkAtomicallyRequest& request)
    -> UpdateDayRemarkAtomicallyResponse {
  try {
    return pipeline_workflow_.RunUpdateDayRemarkAtomically(request);
  } catch (const std::exception& exception) {
    return {.ok = false,
            .message = core_api_failure::BuildErrorMessage(
                "RunUpdateDayRemarkAtomically", exception.what())};
  } catch (...) {
    return {.ok = false,
            .message = core_api_failure::BuildErrorMessage(
                "RunUpdateDayRemarkAtomically", "Unknown error.")};
  }
}

auto PipelineApi::RunConvertTxtActivityNames(
    const ConvertTxtActivityNamesRequest& request)
    -> ConvertTxtActivityNamesResponse {
  try {
    return pipeline_workflow_.RunConvertTxtActivityNames(request);
  } catch (const std::exception& exception) {
    return {.ok = false,
            .converted_content = request.content,
            .error_message = core_api_failure::BuildErrorMessage(
                "RunConvertTxtActivityNames", exception.what())};
  } catch (...) {
    return {.ok = false,
            .converted_content = request.content,
            .error_message = core_api_failure::BuildErrorMessage(
                "RunConvertTxtActivityNames", "Unknown error.")};
  }
}

auto PipelineApi::RunReplaceTxtCanonicalActivityNames(
    const ReplaceTxtCanonicalActivityNamesRequest& request)
    -> ReplaceTxtCanonicalActivityNamesResponse {
  try {
    return pipeline_workflow_.RunReplaceTxtCanonicalActivityNames(request);
  } catch (const std::exception& exception) {
    return {.ok = false,
            .updated_content = request.content,
            .error_message = core_api_failure::BuildErrorMessage(
                "RunReplaceTxtCanonicalActivityNames", exception.what())};
  } catch (...) {
    return {.ok = false,
            .updated_content = request.content,
            .error_message = core_api_failure::BuildErrorMessage(
                "RunReplaceTxtCanonicalActivityNames", "Unknown error.")};
  }
}

auto PipelineApi::RunReplaceTxtAliasActivityNames(
    const ReplaceTxtAliasActivityNamesRequest& request)
    -> ReplaceTxtAliasActivityNamesResponse {
  try {
    return pipeline_workflow_.RunReplaceTxtAliasActivityNames(request);
  } catch (const std::exception& exception) {
    return {.ok = false,
            .updated_content = request.content,
            .error_message = core_api_failure::BuildErrorMessage(
                "RunReplaceTxtAliasActivityNames", exception.what())};
  } catch (...) {
    return {.ok = false,
            .updated_content = request.content,
            .error_message = core_api_failure::BuildErrorMessage(
                "RunReplaceTxtAliasActivityNames", "Unknown error.")};
  }
}

}  // namespace tracer::core::application::use_cases
