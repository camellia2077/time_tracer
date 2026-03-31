#ifndef APPLICATION_PIPELINE_PIPELINE_WORKFLOW_HPP_
#define APPLICATION_PIPELINE_PIPELINE_WORKFLOW_HPP_

#include <filesystem>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "application/pipeline/i_pipeline_workflow.hpp"
#include "application/ports/pipeline/i_converter_config_provider.hpp"
#include "application/ports/pipeline/i_database_health_checker.hpp"
#include "application/ports/pipeline/i_ingest_input_provider.hpp"
#include "application/ports/pipeline/i_processed_data_loader.hpp"
#include "application/ports/pipeline/i_processed_data_storage.hpp"
#include "application/ports/pipeline/i_time_sheet_repository.hpp"
#include "application/ports/pipeline/i_validation_issue_reporter.hpp"

namespace tracer::core::application::pipeline {

class PipelineWorkflow final : public IPipelineWorkflow {
 public:
  using ConverterConfigProviderPtr = std::shared_ptr<
      tracer_core::application::ports::IConverterConfigProvider>;
  using DatabaseHealthCheckerPtr =
      std::shared_ptr<tracer_core::application::ports::IDatabaseHealthChecker>;
  using IngestInputProviderPtr =
      std::shared_ptr<tracer_core::application::ports::IIngestInputProvider>;
  using ProcessedDataLoaderPtr =
      std::shared_ptr<tracer_core::application::ports::IProcessedDataLoader>;
  using ProcessedDataStoragePtr =
      std::shared_ptr<tracer_core::application::ports::IProcessedDataStorage>;
  using TimeSheetRepositoryPtr =
      std::shared_ptr<tracer_core::application::ports::ITimeSheetRepository>;
  using ValidationIssueReporterPtr = std::shared_ptr<
      tracer_core::application::ports::IValidationIssueReporter>;

  PipelineWorkflow(std::filesystem::path output_root_path,
                   ProcessedDataLoaderPtr processed_data_loader,
                   TimeSheetRepositoryPtr time_sheet_repository,
                   DatabaseHealthCheckerPtr database_health_checker,
                   ConverterConfigProviderPtr converter_config_provider,
                   IngestInputProviderPtr ingest_input_provider,
                   ProcessedDataStoragePtr processed_data_storage,
                   ValidationIssueReporterPtr validation_issue_reporter);
  ~PipelineWorkflow() override;

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
  auto RunRecordActivityAtomically(
      const tracer_core::core::dto::RecordActivityAtomicallyRequest& request)
      -> tracer_core::core::dto::RecordActivityAtomicallyResponse override;
  auto InstallActiveConverterConfig(
      const std::string& source_main_config_path,
      const std::string& target_main_config_path) -> void override;

 private:
  std::filesystem::path output_root_path_;
  ProcessedDataLoaderPtr processed_data_loader_;
  TimeSheetRepositoryPtr time_sheet_repository_;
  DatabaseHealthCheckerPtr database_health_checker_;
  ConverterConfigProviderPtr converter_config_provider_;
  IngestInputProviderPtr ingest_input_provider_;
  ProcessedDataStoragePtr processed_data_storage_;
  ValidationIssueReporterPtr validation_issue_reporter_;

  auto RunDatabaseImportFromMemoryReplacingMonth(
      const std::map<std::string, std::vector<DailyLog>>& data_map, int year,
      int month) -> void;
  auto RunDatabaseImportFromMemoryReplacingAll(
      const std::map<std::string, std::vector<DailyLog>>& data_map) -> void;
};

}  // namespace tracer::core::application::pipeline

using PipelineWorkflow = tracer::core::application::pipeline::PipelineWorkflow;

#endif  // APPLICATION_PIPELINE_PIPELINE_WORKFLOW_HPP_
