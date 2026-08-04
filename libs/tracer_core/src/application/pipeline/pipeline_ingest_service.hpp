#ifndef APPLICATION_PIPELINE_PIPELINE_INGEST_SERVICE_H_
#define APPLICATION_PIPELINE_PIPELINE_INGEST_SERVICE_H_

#include <filesystem>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "application/ports/pipeline/i_converter_config_provider.hpp"
#include "application/ports/pipeline/i_database_health_checker.hpp"
#include "application/ports/pipeline/i_ingest_input_provider.hpp"
#include "application/ports/pipeline/i_ingest_runtime_repository.hpp"
#include "application/ports/pipeline/i_processed_data_loader.hpp"
#include "application/ports/pipeline/i_processed_data_storage.hpp"
#include "application/ports/pipeline/i_time_sheet_write_repository.hpp"
#include "application/ports/pipeline/i_validation_issue_reporter.hpp"
#include "domain/types/date_check_mode.hpp"
#include "domain/types/ingest_mode.hpp"

struct DailyLog;

namespace tracer::core::application::pipeline {

class PipelineIngestService final {
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

  PipelineIngestService(std::filesystem::path output_root_path,
                        ProcessedDataLoaderPtr processed_data_loader,
                        TimeSheetWriteRepositoryPtr time_sheet_write_repository,
                        IngestRuntimeRepositoryPtr ingest_runtime_repository,
                        DatabaseHealthCheckerPtr database_health_checker,
                        ConverterConfigProviderPtr converter_config_provider,
                        IngestInputProviderPtr ingest_input_provider,
                        ProcessedDataStoragePtr processed_data_storage,
                        ValidationIssueReporterPtr validation_issue_reporter);
  ~PipelineIngestService();

  auto RunDatabaseImport(const std::string& processed_path_str) -> void;
  auto RunDatabaseImportFromMemory(
      const std::map<std::string, std::vector<DailyLog>>& data_map) -> void;
  auto RunDatabaseImportFromMemoryReplacingAll(
      const std::map<std::string, std::vector<DailyLog>>& data_map) -> void;
  auto RunDatabaseImportFromMemoryReplacingMonth(
      const std::map<std::string, std::vector<DailyLog>>& data_map, int year,
      int month) -> void;

  auto RunIngest(const std::string& source_path, DateCheckMode date_check_mode,
                 bool save_processed, IngestMode ingest_mode) -> void;
  auto RunIngestReplacingAll(const std::string& source_path,
                             DateCheckMode date_check_mode, bool save_processed)
      -> void;

  [[nodiscard]] auto RunIngestSyncStatusQuery(
      const tracer_core::core::dto::IngestSyncStatusRequest& request)
      -> tracer_core::core::dto::IngestSyncStatusOutput;
  auto ClearIngestSyncStatus() -> void;

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
};

}  // namespace tracer::core::application::pipeline

#endif  // APPLICATION_PIPELINE_PIPELINE_INGEST_SERVICE_H_
