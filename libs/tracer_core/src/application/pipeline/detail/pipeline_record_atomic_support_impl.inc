// Atomic record helpers for TXT candidate transaction, validation, ingest, and rollback.

struct ResolvedRecordPaths {
  fs::path runtime_root;
  fs::path input_root;
  fs::path cache_root;
  fs::path target_txt_path;
  std::string target_relative_path;
};

struct AtomicTransactionWorkspace {
  fs::path root;
  fs::path candidate_txt_path;
  fs::path backup_txt_path;
  std::string operation_id;
};

using RunSingleTxtReplaceMonthIngestFn =
    std::function<void(const std::string&, DateCheckMode)>;

[[nodiscard]] auto ReadCanonicalTextFile(const fs::path& path) -> std::string {
  if (!fs::exists(path)) {
    throw std::runtime_error("File not found: " + path.string());
  }
  std::ifstream file(path, std::ios::in | std::ios::binary);
  if (!file.is_open()) {
    throw std::runtime_error("Unable to open file for reading: " +
                             path.string());
  }

  std::ostringstream buffer;
  buffer << file.rdbuf();
  if (file.bad()) {
    throw std::runtime_error("Error occurred while reading file: " +
                             path.string());
  }
  return modtext::RequireCanonicalText(buffer.str(), path.string());
}

auto WriteCanonicalTextFile(const fs::path& path, std::string_view content)
    -> void {
  const std::string canonical =
      modtext::RequireCanonicalText(content, path.string());
  std::ofstream file(path, std::ios::out | std::ios::trunc | std::ios::binary);
  if (!file.is_open()) {
    throw std::runtime_error("Unable to open file for writing: " +
                             path.string());
  }
  file.write(canonical.data(), static_cast<std::streamsize>(canonical.size()));
  if (file.fail()) {
    throw std::runtime_error("Error occurred while writing file: " +
                             path.string());
  }
}

[[nodiscard]] auto BuildRuntimeRoots(const fs::path& output_root)
    -> ResolvedRecordPaths {
  const fs::path runtime_root = fs::absolute(output_root).parent_path();
  if (runtime_root.empty()) {
    throw std::runtime_error("Cannot resolve runtime root from output root.");
  }
  return {
      .runtime_root = runtime_root,
      .input_root = runtime_root / "input",
      .cache_root = runtime_root / "cache",
      .target_txt_path = {},
      .target_relative_path = "",
  };
}

[[nodiscard]] auto HasTxtExtension(const fs::path& path) -> bool {
  return ToLowerAsciiCopy(path.extension().string()) == ".txt";
}

[[nodiscard]] auto ResolveTxtPathInsideInputRoot(const fs::path& input_root,
                                                 std::string_view raw_path)
    -> std::pair<fs::path, std::string> {
  const fs::path canonical_root = fs::weakly_canonical(input_root);
  const fs::path requested_path =
      fs::path(Trim(std::string(raw_path))).is_absolute()
          ? fs::path(Trim(std::string(raw_path)))
          : canonical_root / fs::path(Trim(std::string(raw_path)));
  const fs::path canonical_target = fs::weakly_canonical(requested_path);
  const fs::path relative = canonical_target.lexically_relative(canonical_root);
  const std::string relative_generic = relative.generic_string();
  if (relative.empty() || relative_generic.starts_with("..")) {
    throw std::invalid_argument(
        "preferred_txt_path must stay inside runtime input root.");
  }
  if (!HasTxtExtension(canonical_target)) {
    throw std::invalid_argument("preferred_txt_path must point to a TXT file.");
  }
  return {canonical_target, relative_generic};
}

[[nodiscard]] auto ResolveRecordTargetPaths(
    const fs::path& output_root, const ParsedIsoDate& logical_date,
    std::string_view preferred_txt_path) -> ResolvedRecordPaths {
  ResolvedRecordPaths paths = BuildRuntimeRoots(output_root);
  const std::string default_relative =
      std::format("{0:04d}/{0:04d}-{1:02d}.txt", logical_date.year,
                  logical_date.month);

  if (!preferred_txt_path.empty()) {
    const auto [canonical_target, relative_target] =
        ResolveTxtPathInsideInputRoot(paths.input_root, preferred_txt_path);
    if (relative_target != default_relative) {
      throw std::invalid_argument(
          "preferred_txt_path month does not match target_date_iso month " +
          logical_date.month_key + ".");
    }
    if (!fs::exists(canonical_target) || !fs::is_regular_file(canonical_target)) {
      throw std::invalid_argument(
          "preferred_txt_path does not exist as a regular TXT file.");
    }
    paths.target_txt_path = canonical_target;
    paths.target_relative_path = relative_target;
    return paths;
  }

  paths.target_txt_path = fs::weakly_canonical(paths.input_root) / default_relative;
  paths.target_relative_path = default_relative;
  return paths;
}

auto EnsureTargetMonthMatchesLogicalDate(const std::string& content,
                                         const ParsedIsoDate& logical_date,
                                         const fs::path& target_path) -> void {
  const auto target_month = TryParseSingleTxtTargetMonthFromContent(content);
  if (!target_month.has_value()) {
    return;
  }
  if (target_month->year != logical_date.year ||
      target_month->month != logical_date.month) {
    throw std::invalid_argument("Target TXT header month " +
                                target_month->month_key +
                                " does not match target_date_iso month " +
                                logical_date.month_key + ": " +
                                target_path.string());
  }
}

[[nodiscard]] auto CreateAtomicTransactionWorkspace(const fs::path& cache_root)
    -> AtomicTransactionWorkspace {
  std::error_code io_error;
  fs::create_directories(cache_root, io_error);
  if (io_error) {
    throw std::runtime_error("Failed to prepare cache root: " +
                             cache_root.string() + " | " + io_error.message());
  }

  static std::atomic<unsigned long long> counter{0};
  const auto sequence = counter.fetch_add(1) + 1;
  const auto now_ms =
      std::chrono::duration_cast<std::chrono::milliseconds>(
          std::chrono::system_clock::now().time_since_epoch())
          .count();
  const std::string operation_id =
      std::format("record_activity_txn_{}_{}", now_ms, sequence);
  const fs::path root = cache_root / operation_id;
  fs::create_directories(root, io_error);
  if (io_error) {
    throw std::runtime_error("Failed to create transaction root: " +
                             root.string() + " | " + io_error.message());
  }
  return {
      .root = root,
      .candidate_txt_path = root / "candidate.txt",
      .backup_txt_path = root / "official_backup.txt",
      .operation_id = operation_id,
  };
}

auto CleanupTransactionWorkspace(const fs::path& root) -> void {
  if (root.empty()) {
    return;
  }
  std::error_code io_error;
  fs::remove_all(root, io_error);
}

[[nodiscard]] auto BuildRecordSuccessMessage(
    const ParsedIsoDate& logical_date, const fs::path& target_path,
    const std::optional<int>& gap_from_previous_minutes,
    const bool month_file_created,
    const std::vector<std::string>& warnings) -> std::string {
  const auto format_elapsed = [](const int minutes) -> std::string {
    const int hours = minutes / 60;
    const int mins = minutes % 60;
    return std::format("{:02d}:{:02d}", hours, mins);
  };

  std::vector<std::string> lines;
  lines.push_back("record: ok");
  lines.push_back("logical_date: " + logical_date.iso_date);
  lines.push_back("target_file: " + target_path.string());
  // Success message surfaces activity spacing using `gap_from_previous`.
  // It is intentionally a derived display field (current vs previous event),
  // not a raw write clock field.
  if (gap_from_previous_minutes.has_value()) {
    lines.push_back("gap_from_previous: " +
                    format_elapsed(*gap_from_previous_minutes));
  } else {
    lines.push_back("gap_from_previous: n/a");
  }
  if (month_file_created) {
    lines.push_back("month_file_created: true");
  }
  for (const auto& warning : warnings) {
    if (warning.starts_with("Warning:")) {
      lines.push_back(warning);
    } else {
      lines.push_back("warning: " + warning);
    }
  }
  lines.push_back("sync: ok");

  std::ostringstream stream;
  for (std::size_t index = 0; index < lines.size(); ++index) {
    if (index != 0U) {
      stream << '\n';
    }
    stream << lines[index];
  }
  return stream.str();
}

auto RestoreOfficialTxt(const fs::path& target_txt_path,
                        const fs::path& backup_txt_path,
                        const bool target_previously_existed) -> void {
  std::error_code io_error;
  if (target_previously_existed) {
    fs::copy_file(backup_txt_path, target_txt_path,
                  fs::copy_options::overwrite_existing, io_error);
    if (io_error) {
      throw std::runtime_error("Failed to restore official TXT from backup: " +
                               target_txt_path.string() + " | " +
                               io_error.message());
    }
    return;
  }

  if (fs::exists(target_txt_path)) {
    fs::remove(target_txt_path, io_error);
    if (io_error || fs::exists(target_txt_path)) {
      throw std::runtime_error("Failed to remove newly created TXT during rollback: " +
                               target_txt_path.string() +
                               (io_error ? " | " + io_error.message() : ""));
    }
  }
}

[[nodiscard]] auto RunManualValidationPipeline(
    const fs::path& output_root, const ConverterConfig& converter_config,
    std::shared_ptr<app_ports::IValidationIssueReporter> validation_issue_reporter,
    const IngestInputModel& input, const DateCheckMode date_check_mode)
    -> PipelineSession {
  modports::ClearBufferedDiagnostics();
  const ScopedErrorReportWriterOverride disable_error_reports(nullptr);

  PipelineSession session(output_root);
  session.config.input_root = fs::path(input.source_id);
  session.config.date_check_mode = date_check_mode;
  session.config.structure_validation_blocks_conversion = true;
  session.config.save_processed_output = false;
  session.state.validation_issue_reporter =
      std::move(validation_issue_reporter);
  session.state.converter_config = converter_config;
  session.state.ingest_inputs = {input};

  runtime_bridge::LogInfo("\n--- Atomic Record Candidate Validation Started ---");
  runtime_bridge::LogInfo(
      "[STEP] Step: Pre-validating File Structure before Conversion...");
  if (!StructureValidationStage::Execute(session)) {
    throw std::runtime_error(
        BuildPipelineFailureMessage("Atomic record structure validation failed."));
  }

  if (!ConversionStage::Execute(session)) {
    throw std::runtime_error(
        BuildPipelineFailureMessage("Atomic record conversion failed."));
  }
  if (!CrossMonthLinkStage::Execute(session)) {
    throw std::runtime_error(
        BuildPipelineFailureMessage("Atomic record cross-month link failed."));
  }
  runtime_bridge::LogInfo("[STEP] Step: Validating Business Logic...");
  if (!LogicValidationStage::Execute(session)) {
    throw std::runtime_error(
        BuildPipelineFailureMessage("Atomic record logic validation failed."));
  }

  runtime_bridge::LogInfo(
      "--- Atomic Record Candidate Validation Finished Successfully ---");
  return session;
}

auto RunRecordActivityAtomicallySupport(
    const RecordActivityAtomicallyRequest& request,
    const fs::path& output_root_path,
    app_ports::IConverterConfigProvider& converter_config_provider,
    std::shared_ptr<app_ports::IValidationIssueReporter> validation_issue_reporter,
    const RunSingleTxtReplaceMonthIngestFn& run_single_txt_replace_month_ingest)
    -> RecordActivityAtomicallyResponse {
  try {
    const ParsedIsoDate logical_date = ParseIsoDate(request.target_date_iso);
    const ConverterConfig converter_config =
        converter_config_provider.LoadConverterConfig();
    const std::string record_activity_name =
        ValidateRecordActivityName(request.raw_activity_name, converter_config);
    const std::string normalized_remark =
        NormalizeRemarkForRecord(request.remark);

    ResolvedRecordPaths resolved_paths = ResolveRecordTargetPaths(
        output_root_path, logical_date, request.preferred_txt_path);
    std::error_code io_error;
    fs::create_directories(resolved_paths.input_root, io_error);
    if (io_error) {
      throw std::runtime_error("Failed to prepare input root: " +
                               resolved_paths.input_root.string() + " | " +
                               io_error.message());
    }

    const bool target_previously_existed =
        fs::exists(resolved_paths.target_txt_path);
    const std::string official_content = target_previously_existed
                                             ? ReadCanonicalTextFile(
                                                   resolved_paths.target_txt_path)
                                             : BuildMonthTemplate(logical_date);
    if (target_previously_existed) {
      EnsureTargetMonthMatchesLogicalDate(official_content, logical_date,
                                          resolved_paths.target_txt_path);
    }

    // Upper layer chooses explicit time_order_mode. This layer only applies
    // generic mode semantics and must not infer/override mode on its own.
    CandidateRecordBuildResult candidate_build = BuildCandidateRecordContent(
        official_content, logical_date, record_activity_name,
        normalized_remark, request.time_order_mode,
        converter_config.wake_keywords);
    candidate_build.month_file_created = !target_previously_existed;

    AtomicTransactionWorkspace workspace =
        CreateAtomicTransactionWorkspace(resolved_paths.cache_root);
    bool retain_workspace = false;
    RecordActivityAtomicallyResponse response{};
    bool response_ready = false;
    try {
      WriteCanonicalTextFile(workspace.candidate_txt_path,
                             candidate_build.canonical_text);
      const std::string candidate_content =
          ReadCanonicalTextFile(workspace.candidate_txt_path);

      IngestInputModel validation_input{
          .source_id = resolved_paths.target_txt_path.string(),
          .source_label = resolved_paths.target_txt_path.string(),
          .content = candidate_content,
      };
      static_cast<void>(RunManualValidationPipeline(
          output_root_path, converter_config, validation_issue_reporter,
          validation_input, request.date_check_mode));

      if (target_previously_existed) {
        WriteCanonicalTextFile(workspace.backup_txt_path, official_content);
      }

      fs::create_directories(resolved_paths.target_txt_path.parent_path(),
                             io_error);
      if (io_error) {
        throw std::runtime_error("Failed to prepare target TXT directory: " +
                                 resolved_paths.target_txt_path.parent_path().string() +
                                 " | " + io_error.message());
      }
      WriteCanonicalTextFile(resolved_paths.target_txt_path, candidate_content);

      try {
        run_single_txt_replace_month_ingest(
            resolved_paths.target_txt_path.string(), request.date_check_mode);
      } catch (const std::exception& ingest_error) {
        try {
          RestoreOfficialTxt(resolved_paths.target_txt_path,
                             workspace.backup_txt_path,
                             target_previously_existed);
        } catch (const std::exception& rollback_error) {
          retain_workspace = true;
          response = {.ok = false,
                      .message = std::string("Atomic record ingest failed and TXT rollback also failed. ingest=") +
                                 ingest_error.what() + " | rollback=" +
                                 rollback_error.what(),
                      .operation_id = workspace.operation_id,
                      .warnings = candidate_build.warnings,
                      .rollback_failed = true,
                      .retained_transaction_root = workspace.root.string()};
          response_ready = true;
        }

        if (!response_ready) {
          response = {.ok = false,
                      .message = std::string("Atomic record ingest failed. Official TXT has been rolled back: ") +
                                 ingest_error.what(),
                      .operation_id = workspace.operation_id,
                      .warnings = candidate_build.warnings,
                      .rollback_failed = false,
                      .retained_transaction_root = std::nullopt};
          response_ready = true;
        }
      }

      if (!response_ready) {
        response = {.ok = true,
                    .message = BuildRecordSuccessMessage(
                        logical_date, resolved_paths.target_txt_path,
                        candidate_build.gap_from_previous_minutes,
                        candidate_build.month_file_created,
                        candidate_build.warnings),
                    .operation_id = workspace.operation_id,
                    .warnings = candidate_build.warnings,
                    .rollback_failed = false,
                    .retained_transaction_root = std::nullopt};
        response_ready = true;
      }
    } catch (const std::exception& error) {
      if (!retain_workspace) {
        CleanupTransactionWorkspace(workspace.root);
      }
      return {.ok = false,
              .message = error.what(),
              .operation_id = workspace.operation_id,
              .warnings = candidate_build.warnings,
              .rollback_failed = false,
              .retained_transaction_root = std::nullopt};
    } catch (...) {
      if (!retain_workspace) {
        CleanupTransactionWorkspace(workspace.root);
      }
      return {.ok = false,
              .message = "Atomic record activity failed unexpectedly.",
              .operation_id = workspace.operation_id,
              .warnings = candidate_build.warnings,
              .rollback_failed = false,
              .retained_transaction_root = std::nullopt};
    }
    if (!retain_workspace) {
      CleanupTransactionWorkspace(workspace.root);
    }
    return response;
  } catch (const std::exception& error) {
    return {.ok = false,
            .message = error.what(),
            .operation_id = "",
            .warnings = {},
            .rollback_failed = false,
            .retained_transaction_root = std::nullopt};
  }
}
