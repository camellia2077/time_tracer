// Atomic authored day-remark update. The TXT file is the source of truth;
// successful completion re-ingests the affected month so the database stays
// synchronized with the authored content.

[[nodiscard]] auto BuildUpdatedDayRemarkContent(
    const UpdateDayRemarkAtomicallyRequest& request,
    const std::string& official_content, const ParsedIsoDate& logical_date)
    -> std::string {
  const auto resolved = txt_day_block::ResolveDayBlock({
      .content = official_content,
      .day_marker = logical_date.day_marker,
      .selected_month = logical_date.month_key,
  });
  if (!resolved.ok || !resolved.found) {
    throw std::invalid_argument("target day block was not found.");
  }

  const auto lines = SplitRemarkBody(resolved.day_body);
  std::size_t first_activity_index = lines.size();
  for (std::size_t index = 0; index < lines.size(); ++index) {
    if (TryParseRemarkEventLine(lines[index]).has_value()) {
      first_activity_index = index;
      break;
    }
  }

  const std::string normalized_remark = NormalizeRemarkForRecord(request.remark);
  const auto remark_lines = SplitRemarkBody(normalized_remark);
  std::vector<std::string> updated_lines;
  updated_lines.reserve(lines.size() + remark_lines.size());

  // Day remarks are only the leading // lines before the first activity.
  // Continuation lines after an activity remain activity remarks.
  for (std::size_t index = 0; index < first_activity_index; ++index) {
    if (!IsContinuationLine(lines[index])) {
      updated_lines.push_back(lines[index]);
    }
  }
  for (const auto& remark_line : remark_lines) {
    updated_lines.push_back("// " + remark_line);
  }
  for (std::size_t index = first_activity_index; index < lines.size(); ++index) {
    updated_lines.push_back(lines[index]);
  }

  std::string updated_body;
  for (const auto& line : updated_lines) {
    updated_body += line;
    updated_body.push_back('\n');
  }
  return txt_day_block::ReplaceDayBlock({
             .content = official_content,
             .day_marker = logical_date.day_marker,
             .edited_day_body = updated_body,
         })
      .updated_content;
}

auto RunUpdateDayRemarkAtomicallySupport(
    const UpdateDayRemarkAtomicallyRequest& request,
    const fs::path& output_root_path,
    std::shared_ptr<app_ports::IConverterConfigProvider>
        converter_config_provider,
    std::shared_ptr<app_ports::IValidationIssueReporter>
        validation_issue_reporter,
    const RunSingleTxtReplaceMonthIngestFn& run_single_txt_replace_month_ingest)
    -> UpdateDayRemarkAtomicallyResponse {
  try {
    const ParsedIsoDate logical_date = ParseIsoDate(request.target_date_iso);
    const auto paths = ResolveRecordTargetPaths(
        output_root_path, logical_date, request.preferred_txt_path);
    const std::string official_content = ReadCanonicalTextFile(paths.target_txt_path);
    EnsureTargetMonthMatchesLogicalDate(official_content, logical_date,
                                        paths.target_txt_path);
    const std::string candidate_content =
        BuildUpdatedDayRemarkContent(request, official_content, logical_date);

    const auto workspace = CreateAtomicTransactionWorkspace(paths.cache_root);
    bool retain_workspace = false;
    try {
      WriteCanonicalTextFile(workspace.candidate_txt_path, candidate_content);
      static_cast<void>(RunManualValidationPipeline(
          output_root_path, converter_config_provider->LoadConverterConfig(),
          validation_issue_reporter, IngestInputModel{
              .source_id = paths.target_txt_path.string(),
              .source_label = paths.target_txt_path.string(),
              .content = candidate_content,
          },
          request.date_check_mode));
      WriteCanonicalTextFile(workspace.backup_txt_path, official_content);
      WriteCanonicalTextFile(paths.target_txt_path, candidate_content);
      try {
        run_single_txt_replace_month_ingest(paths.target_txt_path.string(),
                                             request.date_check_mode);
      } catch (const std::exception& ingest_error) {
        try {
          RestoreOfficialTxt(paths.target_txt_path, workspace.backup_txt_path,
                             true);
        } catch (...) {
          retain_workspace = true;
          return {.ok = false,
                  .message = std::string("day remark ingest failed and TXT rollback failed: ") +
                             ingest_error.what(),
                  .operation_id = workspace.operation_id,
                  .rollback_failed = true,
                  .retained_transaction_root = workspace.root.string()};
        }
        const UpdateDayRemarkAtomicallyResponse response{
            .ok = false,
            .message = std::string("day remark ingest failed; TXT rolled back: ") +
                       ingest_error.what(),
            .operation_id = workspace.operation_id};
        CleanupTransactionWorkspace(workspace.root);
        return response;
      }
      const UpdateDayRemarkAtomicallyResponse response{
          .ok = true,
          .message = "day remark updated and database re-ingested",
          .operation_id = workspace.operation_id};
      CleanupTransactionWorkspace(workspace.root);
      return response;
    } catch (...) {
      if (!retain_workspace) {
        CleanupTransactionWorkspace(workspace.root);
      }
      throw;
    }
  } catch (const std::exception& error) {
    return {.ok = false, .message = error.what()};
  }
}
