// application/workflow_handler_entry.cpp
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

#include "application/pipeline/pipeline_manager.hpp"
#include "application/workflow_handler.hpp"
#include "domain/ports/diagnostics.hpp"

using namespace core::pipeline;
namespace app_ports = tracer_core::application::ports;

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

auto RunPipelineOrThrow(PipelineManager& pipeline, const std::string& source,
                        const AppOptions& options,
                        std::string_view failure_message) -> void {
  if (!pipeline.Run(source, options)) {
    throw std::runtime_error(
        workflow_handler_internal::BuildPipelineFailureMessage(
            failure_message));
  }
}

}  // namespace

WorkflowHandler::WorkflowHandler(
    fs::path output_root_path,
    std::shared_ptr<app_ports::IProcessedDataLoader> processed_data_loader,
    std::shared_ptr<app_ports::ITimeSheetRepository> time_sheet_repository,
    std::shared_ptr<app_ports::IDatabaseHealthChecker> database_health_checker,
    std::shared_ptr<app_ports::IConverterConfigProvider>
        converter_config_provider,
    std::shared_ptr<app_ports::IIngestInputProvider> ingest_input_provider,
    std::shared_ptr<app_ports::IProcessedDataStorage> processed_data_storage,
    std::shared_ptr<app_ports::IValidationIssueReporter>
        validation_issue_reporter)
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
  PipelineManager pipeline(output_root_path_, converter_config_provider_,
                           ingest_input_provider_, processed_data_storage_,
                           validation_issue_reporter_);
  if (!pipeline.Run(input_path, options)) {
    throw std::runtime_error("Converter Pipeline Failed.");
  }
}

auto WorkflowHandler::RunValidateStructure(const std::string& source_path)
    -> void {
  tracer_core::domain::ports::ClearBufferedDiagnostics();
  const AppOptions kOptions = BuildStructureValidationOptions(source_path);

  PipelineManager pipeline(output_root_path_, converter_config_provider_,
                           ingest_input_provider_, processed_data_storage_,
                           validation_issue_reporter_);
  RunPipelineOrThrow(pipeline, source_path, kOptions,
                     "Validate structure pipeline failed.");
}

auto WorkflowHandler::RunValidateLogic(const std::string& source_path,
                                       DateCheckMode date_check_mode) -> void {
  tracer_core::domain::ports::ClearBufferedDiagnostics();
  const AppOptions kOptions =
      BuildLogicValidationOptions(source_path, date_check_mode);

  PipelineManager pipeline(output_root_path_, converter_config_provider_,
                           ingest_input_provider_, processed_data_storage_,
                           validation_issue_reporter_);
  RunPipelineOrThrow(pipeline, source_path, kOptions,
                     "Validate logic pipeline failed.");
}
