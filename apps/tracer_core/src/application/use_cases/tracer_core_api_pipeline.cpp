// application/use_cases/tracer_core_api_pipeline.cpp
#include <exception>
#include <stdexcept>
#include <utility>

#include "application/interfaces/i_report_handler.hpp"
#include "application/interfaces/i_workflow_handler.hpp"
#include "application/ports/i_data_query_service.hpp"
#include "application/ports/i_report_data_query_service.hpp"
#include "application/ports/i_report_dto_formatter.hpp"
#include "application/ports/i_report_export_writer.hpp"
#include "application/use_cases/tracer_core_api.hpp"
#include "application/use_cases/tracer_core_api_helpers.hpp"

using namespace tracer_core::core::dto;
namespace core_api_helpers =
    tracer_core::application::use_cases::core_api_helpers;

TracerCoreApi::TracerCoreApi(
    IWorkflowHandler& workflow_handler, IReportHandler& report_handler,
    std::shared_ptr<IProjectRepository> project_repository,
    std::shared_ptr<tracer_core::application::ports::IDataQueryService>
        data_query_service,
    std::shared_ptr<tracer_core::application::ports::IReportDataQueryService>
        report_data_query_service,
    std::shared_ptr<tracer_core::application::ports::IReportDtoFormatter>
        report_dto_formatter,
    std::shared_ptr<tracer_core::application::ports::IReportExportWriter>
        report_export_writer)
    : workflow_handler_(workflow_handler),
      report_handler_(report_handler),
      project_repository_(std::move(project_repository)),
      data_query_service_(std::move(data_query_service)),
      kReport_(std::move(report_data_query_service)),
      report_dto_formatter_(std::move(report_dto_formatter)),
      report_export_writer_(std::move(report_export_writer)) {
  if (!project_repository_) {
    throw std::invalid_argument("project_repository must not be null.");
  }
  if (!data_query_service_) {
    throw std::invalid_argument("data_query_service must not be null.");
  }
}

auto TracerCoreApi::RunConvert(const ConvertRequest& request) -> OperationAck {
  try {
    AppOptions options;
    options.input_path = request.input_path;
    options.convert = true;
    options.validate_structure = request.validate_structure;
    options.validate_logic = request.validate_logic;
    options.date_check_mode = request.date_check_mode;
    options.save_processed_output = request.save_processed_output;

    workflow_handler_.RunConverter(request.input_path, options);
    return {.ok = true, .error_message = ""};
  } catch (const std::exception& exception) {
    return core_api_helpers::BuildOperationFailure("RunConvert", exception);
  } catch (...) {
    return core_api_helpers::BuildOperationFailure("RunConvert");
  }
}

auto TracerCoreApi::RunIngest(const IngestRequest& request) -> OperationAck {
  try {
    workflow_handler_.RunIngest(request.input_path, request.date_check_mode,
                                request.save_processed_output,
                                request.ingest_mode);
    return {.ok = true, .error_message = ""};
  } catch (const std::exception& exception) {
    return core_api_helpers::BuildOperationFailure("RunIngest", exception);
  } catch (...) {
    return core_api_helpers::BuildOperationFailure("RunIngest");
  }
}

auto TracerCoreApi::RunImport(const ImportRequest& request) -> OperationAck {
  try {
    workflow_handler_.RunDatabaseImport(request.processed_path);
    return {.ok = true, .error_message = ""};
  } catch (const std::exception& exception) {
    return core_api_helpers::BuildOperationFailure("RunImport", exception);
  } catch (...) {
    return core_api_helpers::BuildOperationFailure("RunImport");
  }
}

auto TracerCoreApi::RunValidateStructure(
    const ValidateStructureRequest& request) -> OperationAck {
  try {
    workflow_handler_.RunValidateStructure(request.input_path);
    return {.ok = true, .error_message = ""};
  } catch (const std::exception& exception) {
    return core_api_helpers::BuildOperationFailure("RunValidateStructure",
                                                   exception);
  } catch (...) {
    return core_api_helpers::BuildOperationFailure("RunValidateStructure");
  }
}

auto TracerCoreApi::RunValidateLogic(const ValidateLogicRequest& request)
    -> OperationAck {
  try {
    workflow_handler_.RunValidateLogic(request.input_path,
                                       request.date_check_mode);
    return {.ok = true, .error_message = ""};
  } catch (const std::exception& exception) {
    return core_api_helpers::BuildOperationFailure("RunValidateLogic",
                                                   exception);
  } catch (...) {
    return core_api_helpers::BuildOperationFailure("RunValidateLogic");
  }
}
