#include "application/workflow_handler.hpp"

namespace tracer::core::application::workflow {

WorkflowHandler::WorkflowHandler(
    std::filesystem::path output_root_path,
    ProcessedDataLoaderPtr processed_data_loader,
    TimeSheetRepositoryPtr time_sheet_repository,
    DatabaseHealthCheckerPtr database_health_checker,
    ConverterConfigProviderPtr converter_config_provider,
    IngestInputProviderPtr ingest_input_provider,
    ProcessedDataStoragePtr processed_data_storage,
    ValidationIssueReporterPtr validation_issue_reporter)
    : impl_(std::move(output_root_path), std::move(processed_data_loader),
            std::move(time_sheet_repository),
            std::move(database_health_checker),
            std::move(converter_config_provider),
            std::move(ingest_input_provider), std::move(processed_data_storage),
            std::move(validation_issue_reporter)) {}

WorkflowHandler::~WorkflowHandler() = default;

auto WorkflowHandler::RunConverter(const std::string& input_path,
                                   const AppOptions& options) -> void {
  impl_.RunConverter(input_path, options);
}

auto WorkflowHandler::RunDatabaseImport(const std::string& processed_path_str)
    -> void {
  impl_.RunDatabaseImport(processed_path_str);
}

auto WorkflowHandler::RunDatabaseImportFromMemory(
    const std::map<std::string, std::vector<DailyLog>>& data_map) -> void {
  impl_.RunDatabaseImportFromMemory(data_map);
}

auto WorkflowHandler::RunIngest(const std::string& source_path,
                                DateCheckMode date_check_mode,
                                bool save_processed, IngestMode ingest_mode)
    -> void {
  impl_.RunIngest(source_path, date_check_mode, save_processed, ingest_mode);
}

auto WorkflowHandler::RunIngestReplacingAll(const std::string& source_path,
                                            DateCheckMode date_check_mode,
                                            bool save_processed) -> void {
  impl_.RunIngestReplacingAll(source_path, date_check_mode, save_processed);
}

auto WorkflowHandler::RunValidateStructure(const std::string& source_path)
    -> void {
  impl_.RunValidateStructure(source_path);
}

auto WorkflowHandler::RunValidateLogic(const std::string& source_path,
                                       DateCheckMode date_check_mode) -> void {
  impl_.RunValidateLogic(source_path, date_check_mode);
}

}  // namespace tracer::core::application::workflow
