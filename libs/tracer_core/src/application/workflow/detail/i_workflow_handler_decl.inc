class IWorkflowHandler {
 public:
  virtual ~IWorkflowHandler() = default;

  virtual auto RunConverter(const std::string& input_path,
                            const AppOptions& options) -> void = 0;
  virtual auto RunDatabaseImport(const std::string& processed_path_str)
      -> void = 0;
  virtual auto RunDatabaseImportFromMemory(
      const std::map<std::string, std::vector<DailyLog>>& data_map) -> void = 0;
  virtual auto RunIngest(const std::string& source_path,
                         DateCheckMode date_check_mode,
                         bool save_processed = false,
                         IngestMode ingest_mode = IngestMode::kStandard)
      -> void = 0;

  virtual auto RunValidateStructure(const std::string& source_path) -> void = 0;
  virtual auto RunValidateLogic(const std::string& source_path,
                                DateCheckMode date_check_mode) -> void = 0;
};
