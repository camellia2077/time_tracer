// application/workflow_handler.hpp
#ifndef APPLICATION_WORKFLOW_HANDLER_H_
#define APPLICATION_WORKFLOW_HANDLER_H_

#include <filesystem>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "application/interfaces/i_workflow_handler.hpp"
#include "application/ports/i_converter_config_provider.hpp"
#include "application/ports/i_database_health_checker.hpp"
#include "application/ports/i_ingest_input_provider.hpp"
#include "application/ports/i_processed_data_loader.hpp"
#include "application/ports/i_processed_data_storage.hpp"
#include "application/ports/i_time_sheet_repository.hpp"
#include "application/ports/i_validation_issue_reporter.hpp"
#include "domain/model/daily_log.hpp"
#include "domain/types/app_options.hpp"
#include "domain/types/date_check_mode.hpp"
#include "domain/types/ingest_mode.hpp"

namespace fs = std::filesystem;

class WorkflowHandler : public IWorkflowHandler {
 public:
  WorkflowHandler(
      fs::path output_root_path,
      std::shared_ptr<time_tracer::application::ports::IProcessedDataLoader>
          processed_data_loader,
      std::shared_ptr<time_tracer::application::ports::ITimeSheetRepository>
          time_sheet_repository,
      std::shared_ptr<time_tracer::application::ports::IDatabaseHealthChecker>
          database_health_checker,
      std::shared_ptr<time_tracer::application::ports::IConverterConfigProvider>
          converter_config_provider,
      std::shared_ptr<time_tracer::application::ports::IIngestInputProvider>
          ingest_input_provider,
      std::shared_ptr<time_tracer::application::ports::IProcessedDataStorage>
          processed_data_storage,
      std::shared_ptr<time_tracer::application::ports::IValidationIssueReporter>
          validation_issue_reporter);
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
  auto RunValidateStructure(const std::string& source_path) -> void override;
  auto RunValidateLogic(const std::string& source_path,
                        DateCheckMode date_check_mode) -> void override;

 private:
  fs::path output_root_path_;
  std::shared_ptr<time_tracer::application::ports::IProcessedDataLoader>
      processed_data_loader_;
  std::shared_ptr<time_tracer::application::ports::ITimeSheetRepository>
      time_sheet_repository_;
  std::shared_ptr<time_tracer::application::ports::IDatabaseHealthChecker>
      database_health_checker_;
  std::shared_ptr<time_tracer::application::ports::IConverterConfigProvider>
      converter_config_provider_;
  std::shared_ptr<time_tracer::application::ports::IIngestInputProvider>
      ingest_input_provider_;
  std::shared_ptr<time_tracer::application::ports::IProcessedDataStorage>
      processed_data_storage_;
  std::shared_ptr<time_tracer::application::ports::IValidationIssueReporter>
      validation_issue_reporter_;

  auto RunDatabaseImportFromMemoryReplacingMonth(
      const std::map<std::string, std::vector<DailyLog>>& data_map, int year,
      int month) -> void;
};

#endif  // APPLICATION_WORKFLOW_HANDLER_H_
