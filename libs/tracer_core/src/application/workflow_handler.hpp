#ifndef APPLICATION_WORKFLOW_HANDLER_H_
#define APPLICATION_WORKFLOW_HANDLER_H_

#include "application/interfaces/i_workflow_handler.hpp"
#include "application/pipeline/pipeline_workflow.hpp"

namespace tracer::core::application::workflow {

class WorkflowHandler final : public IWorkflowHandler {
 public:
  using ConverterConfigProviderPtr =
      pipeline::PipelineWorkflow::ConverterConfigProviderPtr;
  using DatabaseHealthCheckerPtr =
      pipeline::PipelineWorkflow::DatabaseHealthCheckerPtr;
  using IngestInputProviderPtr =
      pipeline::PipelineWorkflow::IngestInputProviderPtr;
  using ProcessedDataLoaderPtr =
      pipeline::PipelineWorkflow::ProcessedDataLoaderPtr;
  using ProcessedDataStoragePtr =
      pipeline::PipelineWorkflow::ProcessedDataStoragePtr;
  using TimeSheetRepositoryPtr =
      pipeline::PipelineWorkflow::TimeSheetRepositoryPtr;
  using ValidationIssueReporterPtr =
      pipeline::PipelineWorkflow::ValidationIssueReporterPtr;

  WorkflowHandler(std::filesystem::path output_root_path,
                  ProcessedDataLoaderPtr processed_data_loader,
                  TimeSheetRepositoryPtr time_sheet_repository,
                  DatabaseHealthCheckerPtr database_health_checker,
                  ConverterConfigProviderPtr converter_config_provider,
                  IngestInputProviderPtr ingest_input_provider,
                  ProcessedDataStoragePtr processed_data_storage,
                  ValidationIssueReporterPtr validation_issue_reporter);
  ~WorkflowHandler() override;

  auto RunConverter(const std::string& input_path, const AppOptions& options)
      -> void override;
  auto RunDatabaseImport(const std::string& processed_path_str)
      -> void override;
  auto RunDatabaseImportFromMemory(
      const std::map<std::string, std::vector<DailyLog>>& data_map)
      -> void override;
  auto RunIngest(const std::string& source_path, DateCheckMode date_check_mode,
                 bool save_processed = false,
                 IngestMode ingest_mode = IngestMode::kStandard)
      -> void override;
  auto RunIngestSyncStatusQuery(
      const tracer_core::core::dto::IngestSyncStatusRequest& request)
      -> tracer_core::core::dto::IngestSyncStatusOutput override;
  auto ClearIngestSyncStatus() -> void override;
  auto RunIngestReplacingAll(const std::string& source_path,
                             DateCheckMode date_check_mode,
                             bool save_processed = false) -> void override;
  auto RunValidateStructure(const std::string& source_path) -> void override;
  auto RunValidateLogic(const std::string& source_path,
                        DateCheckMode date_check_mode) -> void override;
  auto InstallActiveConverterConfig(
      const std::string& source_main_config_path,
      const std::string& target_main_config_path) -> void override;

 private:
  pipeline::PipelineWorkflow impl_;
};

}  // namespace tracer::core::application::workflow

using WorkflowHandler = tracer::core::application::workflow::WorkflowHandler;

#endif  // APPLICATION_WORKFLOW_HANDLER_H_
