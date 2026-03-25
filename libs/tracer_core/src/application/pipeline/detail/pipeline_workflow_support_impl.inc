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

void PersistBufferedDiagnosticsReportIfEnabled() {
  const std::string report_destination =
      modports::GetErrorReportDestinationLabel();
  if (report_destination.empty() || report_destination == "disabled") {
    return;
  }

  const std::string diagnostic_summary =
      modports::GetBufferedDiagnosticsSummary(
          modports::DiagnosticSeverity::kError, 8);
  if (diagnostic_summary.empty()) {
    return;
  }

  static_cast<void>(modports::AppendErrorReport(diagnostic_summary + "\n"));
}

[[nodiscard]] auto BuildPipelineFailureMessage(std::string_view base_message)
    -> std::string {
  std::string message(base_message);
  const std::string report_destination =
      modports::GetErrorReportDestinationLabel();
  const std::string diagnostic_summary =
      modports::GetBufferedDiagnosticsSummary(
          modports::DiagnosticSeverity::kError, 8);
  if (!report_destination.empty() && report_destination != "disabled") {
    message += "\nFull error report: " + report_destination;
  }
  if (!diagnostic_summary.empty()) {
    message += "\nRecent diagnostics:\n" + diagnostic_summary;
  }
  return message;
}

auto ThrowIfImportTaskFailed(const ImportStats& stats,
                             std::string_view default_message) -> void {
  if (stats.db_open_success && stats.transaction_success) {
    return;
  }

  throw std::runtime_error(stats.error_message.empty()
                               ? std::string(default_message)
                               : stats.error_message);
}

[[nodiscard]] auto BuildStructureValidationOptions(
    const std::string& source_path) -> AppOptions {
  AppOptions options;
  options.input_path = source_path;
  options.validate_structure = true;
  options.convert = false;
  options.validate_logic = false;
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
  options.save_processed_output = false;
  options.date_check_mode = date_check_mode;
  return options;
}

[[nodiscard]] auto BuildIngestOptions(const std::string& source_path,
                                      DateCheckMode date_check_mode,
                                      bool save_processed) -> AppOptions {
  AppOptions options;
  options.input_path = source_path;
  options.validate_structure = true;
  options.convert = true;
  options.validate_logic = true;
  options.date_check_mode = date_check_mode;
  options.save_processed_output = save_processed;
  return options;
}

auto RunPipelineOrThrow(PipelineOrchestrator& pipeline,
                        const AppOptions& options,
                        std::string_view failure_message) -> void {
  if (!pipeline.Run(options)) {
    PersistBufferedDiagnosticsReportIfEnabled();
    throw std::runtime_error(BuildPipelineFailureMessage(failure_message));
  }
}

auto PrintImportStats(const ImportStats& stats, std::string_view title)
    -> void {
  std::ostringstream stream;
  stream << "\n--- " << title << " Report ---";
  runtime_bridge::LogInfo(stream.str());

  if (!stats.db_open_success) {
    runtime_bridge::LogError(
        "[Fatal] DB Error: " +
        (stats.error_message.empty() ? "Unknown" : stats.error_message));
    return;
  }

  if (!stats.transaction_success) {
    runtime_bridge::LogError("[Fatal] Transaction Failed: " +
                             stats.error_message);
    return;
  }

  const bool has_skipped =
      (stats.skipped_days > 0U) || (stats.skipped_records > 0U);
  const bool has_legacy_failed = !stats.failed_files.empty();

  if (!has_legacy_failed && !has_skipped) {
    runtime_bridge::LogInfo("[Success] Processed " +
                            std::to_string(stats.successful_files) + " items.");
  } else {
    std::ostringstream summary;
    summary << "[Partial] Days=" << stats.successful_days << "/"
            << stats.total_days << ", Records=" << stats.successful_records
            << "/" << stats.total_records;
    if (has_legacy_failed) {
      summary << ", LegacyFailedFiles=" << stats.failed_files.size();
    }
    runtime_bridge::LogWarn(summary.str());

    for (const auto& failed_file : stats.failed_files) {
      runtime_bridge::LogError("  Failed: " + failed_file);
    }
  }

  std::ostringstream breakdown;
  breakdown << "Breakdown: days(total=" << stats.total_days
            << ", success=" << stats.successful_days
            << ", skipped=" << stats.skipped_days
            << "), records(total=" << stats.total_records
            << ", success=" << stats.successful_records
            << ", skipped=" << stats.skipped_records << ")";
  runtime_bridge::LogInfo(breakdown.str());

  if (!stats.reason_buckets.empty()) {
    runtime_bridge::LogInfo("Reason Buckets:");
    for (const auto& [reason, count] : stats.reason_buckets) {
      runtime_bridge::LogInfo("  - " + reason + ": " + std::to_string(count));
    }
  }

  const double total_time =
      stats.parsing_duration_s + stats.db_insertion_duration_s;

  std::ostringstream timing;
  timing << std::fixed << std::setprecision(3)
         << "Timing: Parse=" << stats.parsing_duration_s
         << "s, Insert=" << stats.db_insertion_duration_s
         << "s, Total=" << total_time << "s";
  runtime_bridge::LogInfo(timing.str());

  if (stats.replaced_month.has_value()) {
    runtime_bridge::LogInfo("Replace scope: month=" + *stats.replaced_month);
  }
}
