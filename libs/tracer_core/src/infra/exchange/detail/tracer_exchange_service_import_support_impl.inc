struct ImportTransactionPaths {
  fs::path transaction_root;
  fs::path extracted_root;
  fs::path effective_text_root;
  fs::path backup_config_root;
  fs::path backup_text_root;
};

auto BuildImportTransactionPaths(const fs::path& runtime_work_root,
                                 const fs::path& input_path)
    -> ImportTransactionPaths {
  const std::string stem = input_path.stem().empty()
                               ? input_path.filename().string()
                               : input_path.stem().string();
  const fs::path transaction_root =
      BuildScopedStagingDir(runtime_work_root, "import", stem);
  return {
      .transaction_root = transaction_root,
      .extracted_root = transaction_root / "decoded",
      .effective_text_root = transaction_root / "effective_text_root",
      .backup_config_root = transaction_root / "backup" / "config",
      .backup_text_root = transaction_root / "backup" / "text",
  };
}

auto EnsureTransactionRootCreated(const fs::path& transaction_root) -> void {
  std::error_code io_error;
  fs::create_directories(transaction_root, io_error);
  if (io_error) {
    throw std::runtime_error(
        "Failed to create tracer exchange transaction root: " +
        transaction_root.string() + " | " + io_error.message());
  }
}

auto EmitImportTransactionProgress(
    const app_dto::TracerExchangeProgressObserver& progress_observer,
    std::string_view phase, std::size_t phase_index, std::size_t phase_count,
    std::string_view current_item, std::size_t done_count,
    std::size_t total_count, const fs::path& input_root_path,
    const fs::path& output_root_path, const fs::path& current_input_path = {},
    const fs::path& current_output_path = {}) -> void {
  if (!progress_observer) {
    return;
  }

  app_dto::TracerExchangeProgressSnapshot snapshot{};
  snapshot.input_root_path = input_root_path;
  snapshot.output_root_path = output_root_path;
  snapshot.current_input_path = current_input_path;
  snapshot.current_output_path = current_output_path;
  snapshot.current_item = std::string(current_item);
  snapshot.current_group_label = std::string(current_item);
  snapshot.phase_index = phase_index;
  snapshot.phase_count = phase_count;
  snapshot.done_count = done_count;
  snapshot.total_count = total_count;
  snapshot.group_index = phase_index;
  snapshot.group_count = phase_count;
  snapshot.file_index_in_group = done_count;
  snapshot.file_count_in_group = total_count;
  snapshot.current_file_index = done_count;
  snapshot.total_files = total_count;
  snapshot.current_file_done_bytes = done_count;
  snapshot.current_file_total_bytes = total_count;
  snapshot.overall_done_bytes = phase_index;
  snapshot.overall_total_bytes = phase_count;
  snapshot.is_encrypt_operation = false;
  snapshot.phase = std::string(phase);
  if (progress_observer(snapshot) ==
      app_dto::TracerExchangeProgressControl::kCancel) {
    throw std::runtime_error("Tracer exchange import cancelled.");
  }
}

auto TryRollbackImportTransaction(
    app_workflow::IWorkflowHandler& workflow_handler,
    const fs::path& active_text_root, const fs::path& backup_text_root,
    const fs::path& backup_config_root,
    const fs::path& active_converter_main_config_path,
    const std::vector<ImportedPayloadFile>& imported_payloads,
    bool text_root_updated, bool config_applied) -> std::string {
  try {
    if (text_root_updated) {
      RestoreManagedTextFiles(active_text_root, backup_text_root,
                              imported_payloads);
    }
    if (config_applied) {
      workflow_handler.InstallActiveConverterConfig({
          .source_main_config_path =
              (backup_config_root / fs::path(exchange_pkg::kConverterMainPath))
                  .string(),
          .target_main_config_path =
              active_converter_main_config_path.string(),
      });
    }
  } catch (const std::exception& rollback_error) {
    return rollback_error.what();
  }
  return {};
}

auto BuildSuccessfulImportResult(
    const exchange_pkg::DecodedTracerExchangePackage& package,
    const std::vector<ImportedPayloadFile>& imported_payloads,
    const std::map<std::string, fs::path>& managed_month_files,
    const fs::path& transaction_root, const std::error_code& cleanup_error)
    -> app_dto::TracerExchangeImportResult {
  app_dto::TracerExchangeImportResult result{};
  result.ok = true;
  result.source_root_name = package.manifest.source_root_name;
  result.payload_file_count =
      static_cast<std::uint64_t>(imported_payloads.size());
  const std::uint64_t overlap_count = static_cast<std::uint64_t>(std::count_if(
      imported_payloads.begin(), imported_payloads.end(),
      [&](const ImportedPayloadFile& imported_payload) {
        return managed_month_files.contains(imported_payload.month_key);
      }));
  result.replaced_month_count =
      static_cast<std::uint64_t>(imported_payloads.size());
  result.preserved_month_count =
      static_cast<std::uint64_t>(managed_month_files.size()) - overlap_count;
  result.rebuilt_month_count =
      result.replaced_month_count + result.preserved_month_count;
  result.text_root_updated = true;
  result.config_applied = true;
  result.database_rebuilt = true;
  if (cleanup_error) {
    result.backup_retained_root = transaction_root;
    result.backup_cleanup_error = cleanup_error.message();
  }
  result.error_message.clear();
  return result;
}

auto BuildFailedImportResult(const fs::path& transaction_root,
                             std::string_view failure_message,
                             std::string_view rollback_error_message)
    -> app_dto::TracerExchangeImportResult {
  app_dto::TracerExchangeImportResult failure_result{};
  failure_result.ok = false;
  failure_result.text_root_updated = false;
  failure_result.config_applied = false;
  failure_result.database_rebuilt = false;
  failure_result.retained_failure_root = transaction_root;
  failure_result.error_message =
      rollback_error_message.empty()
          ? std::string(failure_message)
          : std::string(failure_message) +
                " | rollback_failed: " + std::string(rollback_error_message);
  return failure_result;
}
