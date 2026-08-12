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
#include "application/ports/pipeline/i_ingest_runtime_repository.hpp"
#include "application/ports/pipeline/i_processed_data_loader.hpp"
#include "application/ports/pipeline/i_processed_data_storage.hpp"
#include "application/ports/pipeline/i_time_sheet_write_repository.hpp"
#include "application/ports/pipeline/i_validation_issue_reporter.hpp"

namespace tracer::core::application::pipeline {

class PipelineIngestService;

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
  using TimeSheetWriteRepositoryPtr = std::shared_ptr<
      tracer_core::application::ports::ITimeSheetWriteRepository>;
  using IngestRuntimeRepositoryPtr = std::shared_ptr<
      tracer_core::application::ports::IIngestRuntimeRepository>;
  using ValidationIssueReporterPtr = std::shared_ptr<
      tracer_core::application::ports::IValidationIssueReporter>;

  PipelineWorkflow(std::filesystem::path output_root_path,
                   ProcessedDataLoaderPtr processed_data_loader,
                   TimeSheetWriteRepositoryPtr time_sheet_write_repository,
                   IngestRuntimeRepositoryPtr ingest_runtime_repository,
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
                 bool save_processed, IngestMode ingest_mode) -> void override;
  auto RunIngestSyncStatusQuery(
      const tracer_core::core::dto::IngestSyncStatusRequest& request)
      -> tracer_core::core::dto::IngestSyncStatusOutput override;
  auto ClearIngestSyncStatus() -> void override;
  auto RunIngestReplacingAll(const std::string& source_path,
                             DateCheckMode date_check_mode, bool save_processed)
      -> void override;
  auto RunValidateStructure(const std::string& source_path) -> void override;
  auto RunValidateLogic(const std::string& source_path,
                        DateCheckMode date_check_mode) -> void override;
  auto RunRecordActivityAtomically(
      const tracer_core::core::dto::RecordActivityAtomicallyRequest& request)
      -> tracer_core::core::dto::RecordActivityAtomicallyResponse override;
  auto RunUpdateActivityRemarkAtomically(
      const tracer_core::core::dto::UpdateActivityRemarkAtomicallyRequest&
          request)
      -> tracer_core::core::dto::UpdateActivityRemarkAtomicallyResponse
      override;
  auto RunUpdateDayRemarkAtomically(
      const tracer_core::core::dto::UpdateDayRemarkAtomicallyRequest& request)
      -> tracer_core::core::dto::UpdateDayRemarkAtomicallyResponse override;
  auto RunDefaultTxtDayMarker(
      const tracer_core::core::dto::DefaultTxtDayMarkerRequest& request)
      -> tracer_core::core::dto::DefaultTxtDayMarkerResponse override;
  auto RunResolveTxtDayBlock(
      const tracer_core::core::dto::ResolveTxtDayBlockRequest& request)
      -> tracer_core::core::dto::ResolveTxtDayBlockResponse override;
  auto RunReplaceTxtDayBlock(
      const tracer_core::core::dto::ReplaceTxtDayBlockRequest& request)
      -> tracer_core::core::dto::ReplaceTxtDayBlockResponse override;
  auto RunResolveTxtDayEdit(
      const tracer_core::core::dto::ResolveTxtDayEditRequest& request)
      -> tracer_core::core::dto::ResolveTxtDayEditResponse override;
  auto RunApplyTxtDayEdit(
      const tracer_core::core::dto::ApplyTxtDayEditRequest& request)
      -> tracer_core::core::dto::ApplyTxtDayEditResponse override;
  auto RunConvertTxtActivityNames(
      const tracer_core::core::dto::ConvertTxtActivityNamesRequest& request)
      -> tracer_core::core::dto::ConvertTxtActivityNamesResponse override;
  auto RunReplaceTxtCanonicalActivityNames(
      const tracer_core::core::dto::ReplaceTxtCanonicalActivityNamesRequest&
          request)
      -> tracer_core::core::dto::ReplaceTxtCanonicalActivityNamesResponse
      override;
  auto RunReplaceTxtAliasActivityNames(
      const tracer_core::core::dto::ReplaceTxtAliasActivityNamesRequest&
          request)
      -> tracer_core::core::dto::ReplaceTxtAliasActivityNamesResponse override;
  auto InstallActiveConverterConfig(
      const ActiveConverterConfigInstallRequest& request) -> void override;

 private:
  std::filesystem::path output_root_path_;
  ProcessedDataLoaderPtr processed_data_loader_;
  TimeSheetWriteRepositoryPtr time_sheet_write_repository_;
  IngestRuntimeRepositoryPtr ingest_runtime_repository_;
  DatabaseHealthCheckerPtr database_health_checker_;
  ConverterConfigProviderPtr converter_config_provider_;
  IngestInputProviderPtr ingest_input_provider_;
  ProcessedDataStoragePtr processed_data_storage_;
  ValidationIssueReporterPtr validation_issue_reporter_;
  std::unique_ptr<PipelineIngestService> ingest_service_;

  auto RunDatabaseImportFromMemoryReplacingMonth(
      const std::map<std::string, std::vector<DailyLog>>& data_map, int year,
      int month) -> void;
  auto RunDatabaseImportFromMemoryReplacingAll(
      const std::map<std::string, std::vector<DailyLog>>& data_map) -> void;
};

}  // namespace tracer::core::application::pipeline

using PipelineWorkflow = tracer::core::application::pipeline::PipelineWorkflow;

#endif  // APPLICATION_PIPELINE_PIPELINE_WORKFLOW_HPP_
