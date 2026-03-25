namespace {

class ScopedErrorReportWriterOverride final {
 public:
  explicit ScopedErrorReportWriterOverride(
      std::shared_ptr<modports::IErrorReportWriter> writer)
      : previous_writer_(modports::GetErrorReportWriter()) {
    modports::SetErrorReportWriter(std::move(writer));
  }

  ScopedErrorReportWriterOverride(const ScopedErrorReportWriterOverride&) =
      delete;
  auto operator=(const ScopedErrorReportWriterOverride&)
      -> ScopedErrorReportWriterOverride& = delete;

  ~ScopedErrorReportWriterOverride() {
    modports::SetErrorReportWriter(previous_writer_);
  }

 private:
  std::shared_ptr<modports::IErrorReportWriter> previous_writer_;
};

[[nodiscard]] auto BuildStructureValidationOptions(
    const std::string& source_path) -> AppOptions {
  AppOptions options;
  options.input_path = source_path;
  options.validate_structure = true;
  options.convert = false;
  options.validate_logic = false;
  // Validate commands are intentionally terminal-first checks. We do not
  // persist processed output or validation report files for them; users are
  // expected to fix issues directly from the current terminal diagnostics.
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
  options.run_structure_validation_before_conversion = true;
  // `validate logic` reuses the conversion pipeline for checks, but it keeps
  // the same terminal-first contract as `validate structure`: no processed
  // output or local validation report files are written for validate runs.
  options.save_processed_output = false;
  options.date_check_mode = date_check_mode;
  return options;
}

auto RunPipelineOrThrow(PipelineOrchestrator& pipeline,
                        const AppOptions& options,
                        std::string_view failure_message) -> void {
  if (!pipeline.Run(options)) {
    throw std::runtime_error(
        workflow_handler_internal::BuildPipelineFailureMessage(
            failure_message));
  }
}

}  // namespace

namespace tracer::core::application::workflow {

WorkflowHandler::WorkflowHandler(
    fs::path output_root_path,
    WorkflowHandler::ProcessedDataLoaderPtr processed_data_loader,
    WorkflowHandler::TimeSheetRepositoryPtr time_sheet_repository,
    WorkflowHandler::DatabaseHealthCheckerPtr database_health_checker,
    WorkflowHandler::ConverterConfigProviderPtr converter_config_provider,
    WorkflowHandler::IngestInputProviderPtr ingest_input_provider,
    WorkflowHandler::ProcessedDataStoragePtr processed_data_storage,
    WorkflowHandler::ValidationIssueReporterPtr validation_issue_reporter)
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
  PipelineOrchestrator pipeline(output_root_path_, converter_config_provider_,
                                ingest_input_provider_, processed_data_storage_,
                                validation_issue_reporter_);
  if (!pipeline.Run(options)) {
    throw std::runtime_error("Converter Pipeline Failed.");
  }
}

auto WorkflowHandler::RunValidateStructure(const std::string& source_path)
    -> void {
  modports::ClearBufferedDiagnostics();
  const AppOptions kOptions = BuildStructureValidationOptions(source_path);
  // Keep validate runs terminal-only: suppress per-run error-report files and
  // rely on the diagnostics already emitted to the active sink/terminal.
  const ScopedErrorReportWriterOverride kDisableErrorReports(nullptr);

  PipelineOrchestrator pipeline(output_root_path_, converter_config_provider_,
                                ingest_input_provider_, processed_data_storage_,
                                validation_issue_reporter_);
  RunPipelineOrThrow(pipeline, kOptions, "Validate structure pipeline failed.");
}

auto WorkflowHandler::RunValidateLogic(const std::string& source_path,
                                       DateCheckMode date_check_mode) -> void {
  modports::ClearBufferedDiagnostics();
  const AppOptions kOptions =
      BuildLogicValidationOptions(source_path, date_check_mode);
  // Keep validate runs terminal-only: suppress per-run error-report files and
  // rely on the diagnostics already emitted to the active sink/terminal.
  const ScopedErrorReportWriterOverride kDisableErrorReports(nullptr);

  PipelineOrchestrator pipeline(output_root_path_, converter_config_provider_,
                                ingest_input_provider_, processed_data_storage_,
                                validation_issue_reporter_);
  RunPipelineOrThrow(pipeline, kOptions, "Validate logic pipeline failed.");
}

}  // namespace tracer::core::application::workflow
