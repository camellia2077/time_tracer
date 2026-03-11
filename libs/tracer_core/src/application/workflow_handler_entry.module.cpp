module;

#include <filesystem>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "domain/types/date_check_mode.hpp"
#include "domain/types/ingest_mode.hpp"

namespace fs = std::filesystem;

struct AppOptions;
struct DailyLog;

namespace tracer_core::application::ports {
class IConverterConfigProvider;
class IDatabaseHealthChecker;
class IIngestInputProvider;
class IProcessedDataLoader;
class IProcessedDataStorage;
class ITimeSheetRepository;
class IValidationIssueReporter;
}  // namespace tracer_core::application::ports

module tracer.core.application.workflow.handler;

import tracer.core.application.workflow.interface;
import tracer.core.application.pipeline.orchestrator;
import tracer.core.domain.types.app_options;
import tracer.core.domain.types.date_check_mode;
import tracer.core.domain.ports.diagnostics;

namespace app_ports = tracer_core::application::ports;
namespace app_pipeline = tracer::core::application::pipeline;
using tracer::core::domain::types::AppOptions;
using tracer::core::domain::types::DateCheckMode;
namespace modports = tracer::core::domain::ports;

using app_pipeline::PipelineOrchestrator;

namespace workflow_handler_internal {
[[nodiscard]] auto BuildPipelineFailureMessage(std::string_view base_message)
    -> std::string;
}  // namespace workflow_handler_internal

namespace {

[[nodiscard]] auto BuildStructureValidationOptions(
    const std::string& source_path) -> AppOptions {
  AppOptions options;
  options.input_path = source_path;
  options.validate_structure = true;
  options.convert = false;
  options.validate_logic = false;
  options.save_processed_output = false;
  options.date_check_mode = DateCheckMode::kNone;
  return options;
}

[[nodiscard]] auto BuildLogicValidationOptions(const std::string& source_path,
                                               DateCheckMode date_check_mode)
    -> AppOptions {
  AppOptions options;
  options.input_path = source_path;
  options.validate_structure = false;
  options.convert = true;
  options.validate_logic = true;
  options.save_processed_output = false;
  options.date_check_mode = date_check_mode;
  return options;
}

auto RunPipelineOrThrow(PipelineOrchestrator& pipeline,
                        const AppOptions& options,
                        std::string_view failure_message) -> void {
  if (!pipeline.Run(options)) {
    throw std::runtime_error(
        workflow_handler_internal::BuildPipelineFailureMessage(
            failure_message));
  }
}

}  // namespace

namespace tracer::core::application::workflow {

WorkflowHandler::WorkflowHandler(
    fs::path output_root_path,
    ProcessedDataLoaderPtr processed_data_loader,
    TimeSheetRepositoryPtr time_sheet_repository,
    DatabaseHealthCheckerPtr database_health_checker,
    ConverterConfigProviderPtr converter_config_provider,
    IngestInputProviderPtr ingest_input_provider,
    ProcessedDataStoragePtr processed_data_storage,
    ValidationIssueReporterPtr validation_issue_reporter)
    : output_root_path_(std::move(output_root_path)),
      processed_data_loader_(std::move(processed_data_loader)),
      time_sheet_repository_(std::move(time_sheet_repository)),
      database_health_checker_(std::move(database_health_checker)),
      converter_config_provider_(std::move(converter_config_provider)),
      ingest_input_provider_(std::move(ingest_input_provider)),
      processed_data_storage_(std::move(processed_data_storage)),
      validation_issue_reporter_(std::move(validation_issue_reporter)) {
  if (!processed_data_loader_ || !time_sheet_repository_ ||
      !database_health_checker_ || !converter_config_provider_ ||
      !ingest_input_provider_ || !processed_data_storage_ ||
      !validation_issue_reporter_) {
    throw std::invalid_argument(
        "WorkflowHandler dependencies must not be null.");
  }
}

WorkflowHandler::~WorkflowHandler() = default;

auto WorkflowHandler::RunConverter(const std::string& input_path,
                                   const AppOptions& options) -> void {
  PipelineOrchestrator pipeline(output_root_path_, converter_config_provider_,
                                ingest_input_provider_,
                                processed_data_storage_,
                                validation_issue_reporter_);
  if (!pipeline.Run(options)) {
    throw std::runtime_error("Converter Pipeline Failed.");
  }
}

auto WorkflowHandler::RunValidateStructure(const std::string& source_path)
    -> void {
  modports::ClearBufferedDiagnostics();
  const AppOptions kOptions = BuildStructureValidationOptions(source_path);

  PipelineOrchestrator pipeline(output_root_path_, converter_config_provider_,
                                ingest_input_provider_,
                                processed_data_storage_,
                                validation_issue_reporter_);
  RunPipelineOrThrow(pipeline, kOptions,
                     "Validate structure pipeline failed.");
}

auto WorkflowHandler::RunValidateLogic(const std::string& source_path,
                                       DateCheckMode date_check_mode) -> void {
  modports::ClearBufferedDiagnostics();
  const AppOptions kOptions =
      BuildLogicValidationOptions(source_path, date_check_mode);

  PipelineOrchestrator pipeline(output_root_path_, converter_config_provider_,
                                ingest_input_provider_,
                                processed_data_storage_,
                                validation_issue_reporter_);
  RunPipelineOrThrow(pipeline, kOptions,
                     "Validate logic pipeline failed.");
}

}  // namespace tracer::core::application::workflow
