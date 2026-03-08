class PipelineOrchestrator {
 public:
  PipelineOrchestrator(
      fs::path output_root,
      std::shared_ptr<tracer_core::application::ports::IConverterConfigProvider>
          converter_config_provider,
      std::shared_ptr<tracer_core::application::ports::IIngestInputProvider>
          ingest_input_provider,
      std::shared_ptr<tracer_core::application::ports::IProcessedDataStorage>
          processed_data_storage,
      std::shared_ptr<tracer_core::application::ports::IValidationIssueReporter>
          validation_issue_reporter);

  [[nodiscard]] auto Run(const AppOptions& options)
      -> std::optional<PipelineSession>;

 private:
  fs::path output_root_;
  std::shared_ptr<tracer_core::application::ports::IConverterConfigProvider>
      converter_config_provider_;
  std::shared_ptr<tracer_core::application::ports::IIngestInputProvider>
      ingest_input_provider_;
  std::shared_ptr<tracer_core::application::ports::IProcessedDataStorage>
      processed_data_storage_;
  std::shared_ptr<tracer_core::application::ports::IValidationIssueReporter>
      validation_issue_reporter_;
};
