// Atomic authored-remark update. This first implementation deliberately
// reuses the existing full single-month ingest path; the public contract can
// later keep the same shape while the implementation gains an incremental DB
// fast path.

struct RemarkEventMatch {
  std::size_t line_index = 0;
  std::size_t delimiter_index = std::string::npos;
};

[[nodiscard]] auto FindRemarkDelimiter(std::string_view line)
    -> std::size_t {
  return line.find("//");
}

[[nodiscard]] auto IsContinuationLine(std::string_view line) -> bool {
  return Trim(std::string(line)).starts_with("//");
}

[[nodiscard]] auto TryParseRemarkEventLine(std::string_view line)
    -> std::optional<std::tuple<std::string, std::string, std::string,
                                std::size_t>> {
  const std::string trimmed = Trim(std::string(line));
  const bool has_six_digit_start =
      trimmed.size() >= 6U && IsDigitsOnly(trimmed.substr(0, 6));
  const std::size_t start_time_length = has_six_digit_start ? 6U : 4U;
  if (trimmed.size() < start_time_length + 1U ||
      !IsDigitsOnly(trimmed.substr(0, start_time_length))) {
    return std::nullopt;
  }

  std::string start_time = trimmed.substr(0, start_time_length);
  std::string end_time;
  std::size_t payload_start = start_time_length;
  if (trimmed.size() >= start_time_length + 7U &&
             trimmed[start_time_length] == '-' &&
             IsDigitsOnly(trimmed.substr(start_time_length + 1U, 6))) {
    end_time = trimmed.substr(start_time_length + 1U, 6);
    payload_start = start_time_length + 7U;
  } else if (trimmed.size() >= start_time_length + 5U &&
             trimmed[start_time_length] == '-' &&
             IsDigitsOnly(trimmed.substr(start_time_length + 1U, 4))) {
    end_time = trimmed.substr(start_time_length + 1U, 4);
    payload_start = start_time_length + 5U;
  }
  if (payload_start >= trimmed.size()) {
    return std::nullopt;
  }

  const std::string_view payload(trimmed.data() + payload_start,
                                 trimmed.size() - payload_start);
  const std::size_t delimiter_index = FindRemarkDelimiter(payload);
  const std::string activity = Trim(std::string(
      payload.substr(0, delimiter_index == std::string::npos
                              ? payload.size()
                              : delimiter_index)));
  if (activity.empty()) {
    return std::nullopt;
  }
  return std::tuple<std::string, std::string, std::string, std::size_t>{
      std::move(start_time), std::move(end_time), std::move(activity),
      delimiter_index == std::string::npos
          ? std::string::npos
          : payload_start + delimiter_index};
}

[[nodiscard]] auto SplitRemarkBody(std::string_view body)
    -> std::vector<std::string> {
  std::vector<std::string> lines;
  std::string current;
  for (const char character : body) {
    if (character == '\n') {
      if (!current.empty()) {
        lines.push_back(std::move(current));
      }
      current.clear();
    } else {
      current.push_back(character);
    }
  }
  if (!current.empty()) {
    lines.push_back(std::move(current));
  }
  return lines;
}

[[nodiscard]] auto BuildUpdatedRemarkContent(
    const UpdateActivityRemarkAtomicallyRequest& request,
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

  constexpr long long kDailySequenceBase = 1000000;
  if (request.logical_id <= 0) {
    throw std::invalid_argument("logical_id must be positive.");
  }
  const long long sequence = request.logical_id % kDailySequenceBase;
  if (sequence <= 0) {
    throw std::invalid_argument("logical_id does not contain a valid activity sequence.");
  }

  auto lines = SplitRemarkBody(resolved.day_body);
  std::optional<RemarkEventMatch> match;
  long long activity_sequence = 0;
  for (std::size_t index = 0; index < lines.size(); ++index) {
    const auto parsed = TryParseRemarkEventLine(lines[index]);
    if (!parsed.has_value()) {
      continue;
    }
    ++activity_sequence;
    if (activity_sequence == sequence) {
      match = RemarkEventMatch{.line_index = index,
                               .delimiter_index = FindRemarkDelimiter(lines[index])};
      break;
    }
  }
  if (!match.has_value()) {
    throw std::invalid_argument("target activity event was not found.");
  }

  const std::string normalized_remark = NormalizeRemarkForRecord(request.remark);
  std::string updated_line = lines[match->line_index];
  const std::size_t delimiter_index = FindRemarkDelimiter(updated_line);
  if (delimiter_index != std::string::npos) {
    updated_line = updated_line.substr(0, delimiter_index);
    while (!updated_line.empty() &&
           std::isspace(static_cast<unsigned char>(updated_line.back())) != 0) {
      updated_line.pop_back();
    }
  }
  if (!normalized_remark.empty()) {
    updated_line += " // ";
  }

  std::vector<std::string> updated_lines;
  updated_lines.reserve(lines.size() + 4U);
  bool removing_matched_continuations = false;
  for (std::size_t index = 0; index < lines.size(); ++index) {
    if (index == match->line_index) {
      removing_matched_continuations = true;
      if (normalized_remark.empty()) {
        updated_lines.push_back(std::move(updated_line));
        continue;
      }

      auto remark_lines = SplitRemarkBody(normalized_remark);
      updated_line += remark_lines.front();
      updated_lines.push_back(std::move(updated_line));
      for (std::size_t remark_index = 1; remark_index < remark_lines.size();
           ++remark_index) {
        updated_lines.push_back("// " + remark_lines[remark_index]);
      }
      continue;
    }

    if (removing_matched_continuations) {
      if (TryParseRemarkEventLine(lines[index]).has_value()) {
        removing_matched_continuations = false;
      } else if (IsContinuationLine(lines[index])) {
        continue;
      }
    }
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

auto RunUpdateActivityRemarkAtomicallySupport(
    const UpdateActivityRemarkAtomicallyRequest& request,
    const fs::path& output_root_path,
    std::shared_ptr<app_ports::IConverterConfigProvider>
        converter_config_provider,
    std::shared_ptr<app_ports::IValidationIssueReporter>
        validation_issue_reporter,
    const RunSingleTxtReplaceMonthIngestFn& run_single_txt_replace_month_ingest)
    -> UpdateActivityRemarkAtomicallyResponse {
  try {
    const ParsedIsoDate logical_date = ParseIsoDate(request.target_date_iso);
    const auto paths = ResolveRecordTargetPaths(
        output_root_path, logical_date, request.preferred_txt_path);
    const std::string official_content = ReadCanonicalTextFile(paths.target_txt_path);
    EnsureTargetMonthMatchesLogicalDate(official_content, logical_date,
                                        paths.target_txt_path);
    const std::string candidate_content =
        BuildUpdatedRemarkContent(request, official_content, logical_date);

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
                  .message = std::string("remark ingest failed and TXT rollback failed: ") +
                             ingest_error.what(),
                  .operation_id = workspace.operation_id,
                  .rollback_failed = true,
                  .retained_transaction_root = workspace.root.string()};
        }
        const UpdateActivityRemarkAtomicallyResponse response{
            .ok = false,
            .message = std::string("remark ingest failed; TXT rolled back: ") +
                       ingest_error.what(),
            .operation_id = workspace.operation_id};
        CleanupTransactionWorkspace(workspace.root);
        return response;
      }
      const UpdateActivityRemarkAtomicallyResponse response{
          .ok = true,
          .message = "activity remark updated and database re-ingested",
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
