namespace {

struct SingleTxtTargetMonth {
  int year = 0;
  int month = 0;
  std::string month_key;
  std::string month_start_date;
};

struct PreviousMonthInfo {
  int year = 0;
  int month = 0;
  std::string month_key;
};

struct PreviousMonthFileCandidate {
  fs::path path;
  std::string content;
};

struct ActivityTailCandidate {
  std::string date;
  std::string end_time;
  long long end_timestamp = 0;
};

[[nodiscard]] auto ToLowerAsciiCopy(std::string value) -> std::string {
  for (char& ch : value) {
    ch = static_cast<char>(
        std::tolower(static_cast<unsigned char>(ch)));  // ASCII-only
  }
  return value;
}

[[nodiscard]] auto BuildMonthKey(const int year, const int month)
    -> std::string {
  return std::format("{:04d}-{:02d}", year, month);
}

[[nodiscard]] auto TryBuildPreviousMonthInfo(const int year, const int month)
    -> std::optional<PreviousMonthInfo> {
  if (month < 1 || month > 12) {
    return std::nullopt;
  }

  int previous_year = year;
  int previous_month = month - 1;
  if (previous_month == 0) {
    previous_month = 12;
    --previous_year;
  }

  return PreviousMonthInfo{
      .year = previous_year,
      .month = previous_month,
      .month_key = BuildMonthKey(previous_year, previous_month),
  };
}

[[nodiscard]] auto TryParseSingleTxtTargetMonthFromContent(
    std::string_view content) -> std::optional<SingleTxtTargetMonth> {
  constexpr std::size_t kYearLineLength = 5;
  constexpr int kMaxMonth = 12;
  std::stringstream stream{std::string(content)};
  std::string line;
  std::optional<int> year;
  std::optional<int> month;

  while (std::getline(stream, line)) {
    const std::string trimmed = Trim(line);
    if (trimmed.empty()) {
      continue;
    }

    if (!year.has_value() && trimmed.size() == kYearLineLength &&
        trimmed[0] == 'y') {
      try {
        year = std::stoi(trimmed.substr(1));
      } catch (const std::exception&) {
        return std::nullopt;
      }
      continue;
    }

    if (year.has_value() && !month.has_value() && trimmed.size() == 3 &&
        trimmed[0] == 'm') {
      try {
        const int kValue = std::stoi(trimmed.substr(1));
        if (kValue < 1 || kValue > kMaxMonth) {
          return std::nullopt;
        }
        month = kValue;
      } catch (const std::exception&) {
        return std::nullopt;
      }
      continue;
    }
  }

  if (!year.has_value() || !month.has_value()) {
    return std::nullopt;
  }

  const std::string kMonthKey = BuildMonthKey(*year, *month);
  return SingleTxtTargetMonth{
      .year = *year,
      .month = *month,
      .month_key = kMonthKey,
      .month_start_date = kMonthKey + "-01",
  };
}

[[nodiscard]] auto TryResolveSingleTxtTargetMonth(
    const PipelineSession& context) -> std::optional<SingleTxtTargetMonth> {
  if (context.state.ingest_inputs.size() != 1U) {
    return std::nullopt;
  }
  return TryParseSingleTxtTargetMonthFromContent(
      context.state.ingest_inputs.front().content);
}

[[nodiscard]] auto IsSingleMonthConsistent(
    const std::map<std::string, std::vector<DailyLog>>& processed_data,
    std::string_view month_key) -> bool {
  constexpr size_t kMonthKeyLength = 7;
  for (const auto& [key, days] : processed_data) {
    if (key != month_key) {
      return false;
    }
    for (const auto& day : days) {
      if (day.date.size() < kMonthKeyLength ||
          day.date.substr(0, kMonthKeyLength) != month_key) {
        return false;
      }
    }
  }
  return true;
}

[[nodiscard]] auto TryReadTextFile(const fs::path& path)
    -> std::optional<std::string> {
  std::ifstream input(path, std::ios::in | std::ios::binary);
  if (!input.is_open()) {
    return std::nullopt;
  }

  std::ostringstream buffer;
  buffer << input.rdbuf();
  if (input.bad()) {
    return std::nullopt;
  }

  const std::string raw_text = buffer.str();
  const auto canonical = modtext::Canonicalize(raw_text, path.string());
  if (!canonical.ok) {
    runtime_bridge::LogWarn(
        "Skipping sibling TXT fallback due to invalid text: " +
        canonical.error_message);
    return std::nullopt;
  }
  return canonical.text;
}

[[nodiscard]] auto CollectPreviousMonthSiblingTxtFiles(
    const fs::path& source_path, const PreviousMonthInfo& previous_month)
    -> std::vector<PreviousMonthFileCandidate> {
  std::vector<PreviousMonthFileCandidate> candidates;

  std::error_code error;
  const fs::path kSourceParent = source_path.parent_path();
  if (kSourceParent.empty() || !fs::exists(kSourceParent, error) ||
      !fs::is_directory(kSourceParent, error)) {
    return candidates;
  }

  fs::directory_iterator iterator(kSourceParent, error);
  if (error) {
    return candidates;
  }

  for (const fs::directory_entry& entry : iterator) {
    if (!entry.is_regular_file()) {
      continue;
    }

    const std::string kExtension =
        ToLowerAsciiCopy(entry.path().extension().string());
    if (kExtension != ".txt") {
      continue;
    }

    std::error_code eq_error;
    if (fs::equivalent(entry.path(), source_path, eq_error)) {
      continue;
    }

    const auto kContentOpt = TryReadTextFile(entry.path());
    if (!kContentOpt.has_value()) {
      continue;
    }

    const auto kMonthOpt =
        TryParseSingleTxtTargetMonthFromContent(*kContentOpt);
    if (!kMonthOpt.has_value()) {
      continue;
    }
    if (kMonthOpt->year != previous_month.year ||
        kMonthOpt->month != previous_month.month) {
      continue;
    }

    candidates.push_back(PreviousMonthFileCandidate{.path = entry.path(),
                                                    .content = *kContentOpt});
  }

  std::ranges::sort(candidates,
                    [](const PreviousMonthFileCandidate& left,
                       const PreviousMonthFileCandidate& right) -> bool {
                      return left.path.string() < right.path.string();
                    });
  return candidates;
}

[[nodiscard]] auto IsBetterTailCandidate(const ActivityTailCandidate& candidate,
                                         const ActivityTailCandidate& best)
    -> bool {
  if (candidate.date != best.date) {
    return candidate.date > best.date;
  }
  if (candidate.end_timestamp != best.end_timestamp) {
    return candidate.end_timestamp > best.end_timestamp;
  }
  return candidate.end_time > best.end_time;
}

[[nodiscard]] auto TryExtractLatestTailFromProcessedData(
    const std::map<std::string, std::vector<DailyLog>>& processed_data,
    std::string_view month_key)
    -> std::optional<app_ports::PreviousActivityTail> {
  std::optional<ActivityTailCandidate> best;

  for (const auto& [key, days] : processed_data) {
    if (key != month_key) {
      continue;
    }

    for (const auto& day : days) {
      for (const auto& activity : day.processedActivities) {
        if (activity.end_time_str.empty()) {
          continue;
        }
        ActivityTailCandidate candidate{
            .date = day.date,
            .end_time = activity.end_time_str,
            .end_timestamp = activity.end_timestamp,
        };
        if (!best.has_value() || IsBetterTailCandidate(candidate, *best)) {
          best = std::move(candidate);
        }
      }
    }
  }

  if (!best.has_value()) {
    return std::nullopt;
  }
  return app_ports::PreviousActivityTail{
      .date = best->date,
      .end_time = best->end_time,
  };
}

[[nodiscard]] auto TryGetPreviousTailFromSiblingTxt(
    const PipelineSession& context, const SingleTxtTargetMonth& target_month)
    -> std::optional<app_ports::PreviousActivityTail> {
  if (context.state.ingest_inputs.size() != 1U) {
    return std::nullopt;
  }

  const auto kPreviousMonth =
      TryBuildPreviousMonthInfo(target_month.year, target_month.month);
  if (!kPreviousMonth.has_value()) {
    return std::nullopt;
  }

  const auto& input = context.state.ingest_inputs.front();
  if (input.source_id.empty()) {
    return std::nullopt;
  }

  const fs::path kSourcePath(input.source_id);
  if (kSourcePath.empty()) {
    return std::nullopt;
  }

  auto candidates =
      CollectPreviousMonthSiblingTxtFiles(kSourcePath, *kPreviousMonth);
  if (candidates.empty()) {
    return std::nullopt;
  }

  if (candidates.size() > 1U) {
    runtime_bridge::LogWarn(
        "[LogLinker] Found multiple sibling TXT files for previous month " +
        kPreviousMonth->month_key +
        ". Fallback uses lexicographically first path: " +
        candidates.front().path.string());
  }

  const auto& selected = candidates.front();
  LogProcessor processor(context.state.converter_config);
  auto conversion_result =
      processor.ProcessSourceContent(selected.path.string(), selected.content);
  if (!conversion_result.success || conversion_result.processed_data.empty()) {
    runtime_bridge::LogWarn(
        "[LogLinker] Failed to convert sibling TXT fallback: " +
        selected.path.string());
    return std::nullopt;
  }

  if (!IsSingleMonthConsistent(conversion_result.processed_data,
                               kPreviousMonth->month_key)) {
    runtime_bridge::LogWarn(
        "[LogLinker] Sibling TXT fallback is not single-month consistent: " +
        selected.path.string());
    return std::nullopt;
  }

  const auto kTail = TryExtractLatestTailFromProcessedData(
      conversion_result.processed_data, kPreviousMonth->month_key);
  if (!kTail.has_value()) {
    runtime_bridge::LogWarn(
        "[LogLinker] Sibling TXT fallback has no activity tail: " +
        selected.path.string());
    return std::nullopt;
  }

  runtime_bridge::LogInfo(
      "[LogLinker] Loaded previous tail from sibling TXT: " +
      selected.path.string());
  return kTail;
}

[[nodiscard]] auto ResolvePreviousTailForReplaceMonth(
    const PipelineSession& context, const SingleTxtTargetMonth& target_month,
    const std::map<std::string, std::vector<DailyLog>>& processed_data,
    app_ports::ITimeSheetRepository& repository)
    -> std::optional<app_ports::PreviousActivityTail> {
  if (processed_data.empty()) {
    return std::nullopt;
  }

  auto previous_tail = repository.TryGetLatestActivityTailBeforeDate(
      target_month.month_start_date);
  if (previous_tail.has_value()) {
    return previous_tail;
  }
  return TryGetPreviousTailFromSiblingTxt(context, target_month);
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

}  // namespace
