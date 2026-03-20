#include <exception>
#include <stdexcept>
#include <utility>

#include "application/dto/core_requests.hpp"
#include "application/dto/core_responses.hpp"
#include "application/ports/i_data_query_service.hpp"
#include "application/ports/i_report_dto_formatter.hpp"
#include "application/ports/i_report_export_writer.hpp"
#include "application/ports/i_tracer_exchange_service.hpp"
#include "application/use_cases/tracer_core_api.hpp"

import tracer.core.application.use_cases.helpers;
import tracer.core.application.workflow.interface;
import tracer.core.domain.types.app_options;

using namespace tracer_core::core::dto;
using tracer::core::domain::types::AppOptions;
namespace core_api_helpers = tracer::core::application::use_cases::helpers;

namespace tracer::core::application::use_cases {

TracerCoreApi::TracerCoreApi(
    ::tracer::core::application::workflow::IWorkflowHandler& workflow_handler,
    IReportHandler& report_handler,
    ProjectRepositoryPtr project_repository,
    DataQueryServicePtr data_query_service,
    ReportDataQueryServicePtr report_data_query_service,
    ReportDtoFormatterPtr report_dto_formatter,
    ReportExportWriterPtr report_export_writer,
    TracerExchangeServicePtr tracer_exchange_service)
    : workflow_handler_(workflow_handler),
      report_handler_(report_handler),
      project_repository_(std::move(project_repository)),
      data_query_service_(std::move(data_query_service)),
      kReport_(std::move(report_data_query_service)),
      report_dto_formatter_(std::move(report_dto_formatter)),
      report_export_writer_(std::move(report_export_writer)),
      tracer_exchange_service_(std::move(tracer_exchange_service)) {
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
    options.run_structure_validation_before_conversion =
        request.validate_logic;
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

}  // namespace tracer::core::application::use_cases
