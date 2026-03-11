class WorkflowHandler : public IWorkflowHandler {
 public:
  using ConverterConfigProviderPtr =
      std::shared_ptr<tracer_core::application::ports::IConverterConfigProvider>;
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
  using ValidationIssueReporterPtr =
      std::shared_ptr<tracer_core::application::ports::IValidationIssueReporter>;

  WorkflowHandler(fs::path output_root_path,
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
  auto RunValidateStructure(const std::string& source_path) -> void override;
  auto RunValidateLogic(const std::string& source_path,
                        DateCheckMode date_check_mode) -> void override;

 private:
  fs::path output_root_path_;
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
};
