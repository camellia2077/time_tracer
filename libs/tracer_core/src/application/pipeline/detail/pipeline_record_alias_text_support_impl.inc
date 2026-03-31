// Atomic record helpers for alias validation and TXT day-block text assembly.
// Keep this block isolated so record input semantics can be tested independently.

struct ParsedIsoDate {
  int year = 0;
  int month = 0;
  int day = 0;
  std::string iso_date;
  std::string month_key;
  std::string day_marker;
};

struct CandidateRecordBuildResult {
  std::string canonical_text;
  std::string event_time;
  std::optional<int> gap_from_previous_minutes;
  bool month_file_created = false;
  std::vector<std::string> warnings;
};

constexpr std::string_view kIncompleteDayWarning =
    "Warning: this day currently has fewer than 2 authored events, so some "
    "intervals may not be computable yet.";
constexpr std::string_view kOvernightContinuationWarning =
    "Warning: possible overnight continuation; the first event of this day is "
    "not wake-related, so no sleep activity will be auto-generated.";

[[nodiscard]] auto IsDigitsOnly(std::string_view value) -> bool {
  return !value.empty() &&
         std::ranges::all_of(value, [](const char ch) -> bool {
           return std::isdigit(static_cast<unsigned char>(ch)) != 0;
         });
}

[[nodiscard]] auto IsLeapYear(const int year) -> bool {
  return (year % 400 == 0) || ((year % 4 == 0) && (year % 100 != 0));
}

[[nodiscard]] auto DaysInMonth(const int year, const int month) -> int {
  switch (month) {
    case 1:
    case 3:
    case 5:
    case 7:
    case 8:
    case 10:
    case 12:
      return 31;
    case 4:
    case 6:
    case 9:
    case 11:
      return 30;
    case 2:
      return IsLeapYear(year) ? 29 : 28;
    default:
      return 0;
  }
}

[[nodiscard]] auto ParseIsoDate(std::string_view raw_target_date)
    -> ParsedIsoDate {
  const std::string trimmed = Trim(std::string(raw_target_date));
  if (trimmed.size() != 10U || trimmed[4] != '-' || trimmed[7] != '-' ||
      !IsDigitsOnly(trimmed.substr(0, 4)) ||
      !IsDigitsOnly(trimmed.substr(5, 2)) ||
      !IsDigitsOnly(trimmed.substr(8, 2))) {
    throw std::invalid_argument("target_date_iso must use YYYY-MM-DD.");
  }

  const int year = std::stoi(trimmed.substr(0, 4));
  const int month = std::stoi(trimmed.substr(5, 2));
  const int day = std::stoi(trimmed.substr(8, 2));
  const int max_day = DaysInMonth(year, month);
  if (month < 1 || month > 12 || day < 1 || day > max_day) {
    throw std::invalid_argument("target_date_iso is not a valid calendar date.");
  }

  ParsedIsoDate parsed{};
  parsed.year = year;
  parsed.month = month;
  parsed.day = day;
  parsed.iso_date = std::format("{:04d}-{:02d}-{:02d}", year, month, day);
  parsed.month_key = std::format("{:04d}-{:02d}", year, month);
  parsed.day_marker = std::format("{:02d}{:02d}", month, day);
  return parsed;
}

[[nodiscard]] auto BuildMonthTemplate(const ParsedIsoDate& logical_date)
    -> std::string {
  return std::format("y{:04d}\nm{:02d}\n\n", logical_date.year,
                     logical_date.month);
}

[[nodiscard]] auto NormalizeLineInput(std::string_view raw) -> std::string {
  std::string cleaned;
  cleaned.reserve(raw.size());
  for (const char ch : raw) {
    if (ch == '\n' || ch == '\r') {
      cleaned.push_back(' ');
    } else {
      cleaned.push_back(ch);
    }
  }
  return Trim(cleaned);
}

[[nodiscard]] auto NormalizeActivityNameForRecord(std::string_view raw)
    -> std::string {
  std::string cleaned = NormalizeLineInput(raw);
  return cleaned;
}

[[nodiscard]] auto NormalizeRemarkForRecord(std::string_view raw)
    -> std::string {
  return NormalizeLineInput(raw);
}

[[nodiscard]] auto NormalizeForComparison(std::string_view raw)
    -> std::string {
  std::string normalized;
  normalized.reserve(raw.size());
  for (const char ch : raw) {
    if (std::isspace(static_cast<unsigned char>(ch)) != 0) {
      continue;
    }
    normalized.push_back(
        static_cast<char>(std::tolower(static_cast<unsigned char>(ch))));
  }
  return normalized;
}

[[nodiscard]] auto IsWakeKeywordActivity(
    std::string_view activity_name,
    const std::vector<std::string>& wake_keywords) -> bool {
  const std::string normalized = NormalizeForComparison(activity_name);
  if (normalized.empty()) {
    return false;
  }
  for (const auto& wake_keyword : wake_keywords) {
    if (NormalizeForComparison(wake_keyword) == normalized) {
      return true;
    }
  }
  return false;
}

[[nodiscard]] auto ExtractEventTimeToken(std::string_view line)
    -> std::optional<std::string> {
  const auto first_non_space = line.find_first_not_of(" \t");
  if (first_non_space == std::string_view::npos ||
      first_non_space + 4U > line.size()) {
    return std::nullopt;
  }
  const std::string_view hhmm = line.substr(first_non_space, 4);
  if (!IsDigitsOnly(hhmm)) {
    return std::nullopt;
  }
  return std::string(hhmm);
}

[[nodiscard]] auto ExtractActivityName(std::string_view line) -> std::string {
  const auto hhmm = ExtractEventTimeToken(line);
  if (!hhmm.has_value()) {
    return "";
  }

  const auto first_non_space = line.find_first_not_of(" \t");
  if (first_non_space == std::string_view::npos) {
    return "";
  }
  std::string raw_body = Trim(std::string(line.substr(first_non_space + 4U)));
  if (raw_body.empty()) {
    return "";
  }

  std::size_t cut_at = raw_body.size();
  for (const std::string_view separator : {"//", "#", ";"}) {
    const std::size_t index = raw_body.find(separator);
    if (index != std::string::npos && index < cut_at) {
      cut_at = index;
    }
  }
  return Trim(raw_body.substr(0, cut_at));
}

[[nodiscard]] auto IsDayMarkerLine(std::string_view line) -> bool {
  const std::string trimmed = Trim(std::string(line));
  return trimmed.size() == 4U && IsDigitsOnly(trimmed);
}

[[nodiscard]] auto FindDayBlockEnd(const std::vector<std::string>& lines,
                                   const int block_start) -> int {
  for (int index = block_start + 1; index < static_cast<int>(lines.size());
       ++index) {
    if (IsDayMarkerLine(lines[static_cast<std::size_t>(index)])) {
      return index;
    }
  }
  return static_cast<int>(lines.size());
}

[[nodiscard]] auto FindFirstActivityName(const std::vector<std::string>& lines,
                                         const int block_start,
                                         const int block_end)
    -> std::optional<std::string> {
  for (int index = block_start + 1; index < block_end; ++index) {
    std::string activity =
        ExtractActivityName(lines[static_cast<std::size_t>(index)]);
    if (!activity.empty()) {
      return activity;
    }
  }
  return std::nullopt;
}

[[nodiscard]] auto CountAuthoredEventsInDayBlock(
    const std::vector<std::string>& lines, const int block_start,
    const int block_end) -> int {
  int count = 0;
  for (int index = block_start + 1; index < block_end; ++index) {
    if (!ExtractActivityName(lines[static_cast<std::size_t>(index)]).empty()) {
      ++count;
    }
  }
  return count;
}

[[nodiscard]] auto FindLastValidEventTimeToken(
    const std::vector<std::string>& lines, const int block_start,
    const int block_end) -> std::optional<std::string> {
  for (int index = block_end - 1; index > block_start; --index) {
    const auto hhmm =
        ExtractEventTimeToken(lines[static_cast<std::size_t>(index)]);
    if (hhmm.has_value() &&
        record_time_order::TryParseHhmmToMinutes(*hhmm).has_value()) {
      return hhmm;
    }
  }
  return std::nullopt;
}

[[nodiscard]] auto IsStrictlyAfter(std::string_view candidate_time,
                                   std::string_view baseline_time,
                                   const TimeOrderMode mode) -> bool {
  // Design intent:
  // 1) Keep strict "candidate must be later" semantics for all modes.
  // 2) Never hardcode specific HHmm pairs (for example "0009 > 2058").
  // 3) Ordering is derived only from selected mode + generic cutoff mapping.
  return record_time_order::IsStrictlyAfter(candidate_time, baseline_time,
                                            mode);
}

[[nodiscard]] auto HasDuplicateInDayBlock(
    const std::vector<std::string>& lines, const int block_start,
    const int block_end, std::string_view event_time,
    std::string_view normalized_activity) -> bool {
  const std::string normalized_target =
      NormalizeForComparison(normalized_activity);
  if (normalized_target.empty()) {
    return false;
  }

  for (int index = block_start + 1; index < block_end; ++index) {
    const auto line_time =
        ExtractEventTimeToken(lines[static_cast<std::size_t>(index)]);
    if (!line_time.has_value() || *line_time != event_time) {
      continue;
    }

    const std::string line_activity =
        ExtractActivityName(lines[static_cast<std::size_t>(index)]);
    if (NormalizeForComparison(line_activity) == normalized_target) {
      return true;
    }
  }
  return false;
}

[[nodiscard]] auto SplitLines(std::string_view content)
    -> std::vector<std::string> {
  std::vector<std::string> lines;
  std::stringstream stream{std::string(content)};
  std::string line;
  while (std::getline(stream, line)) {
    lines.push_back(line);
  }
  return lines;
}

[[nodiscard]] auto JoinLinesCanonical(const std::vector<std::string>& lines)
    -> std::string {
  std::ostringstream stream;
  for (const auto& line : lines) {
    stream << line << '\n';
  }
  return modtext::RequireCanonicalText(stream.str(), "record_activity_candidate");
}

[[nodiscard]] auto BuildRawEventLine(std::string_view hhmm,
                                     std::string_view activity,
                                     std::string_view remark) -> std::string {
  if (remark.empty()) {
    return std::string(hhmm) + std::string(activity);
  }
  return std::string(hhmm) + std::string(activity) + " // " +
         std::string(remark);
}

[[nodiscard]] auto ResolveAuthoredEventCompletenessWarning(
    const std::vector<std::string>& lines, const int block_start,
    const int block_end, const std::vector<std::string>& wake_keywords)
    -> std::optional<std::string> {
  if (CountAuthoredEventsInDayBlock(lines, block_start, block_end) >= 2) {
    return std::nullopt;
  }

  const auto first_activity = FindFirstActivityName(lines, block_start,
                                                    block_end);
  if (first_activity.has_value() &&
      !wake_keywords.empty() &&
      !IsWakeKeywordActivity(*first_activity, wake_keywords)) {
    return std::string(kOvernightContinuationWarning);
  }
  return std::string(kIncompleteDayWarning);
}

[[nodiscard]] auto GetCurrentLocalHhmm() -> std::string {
  const auto now = std::chrono::system_clock::now();
  const std::time_t now_time = std::chrono::system_clock::to_time_t(now);
  std::tm local_time{};
#if defined(_WIN32)
  localtime_s(&local_time, &now_time);
#else
  localtime_r(&now_time, &local_time);
#endif
  return std::format("{:02d}{:02d}", local_time.tm_hour, local_time.tm_min);
}

[[nodiscard]] auto ValidateRecordActivityName(
    std::string_view raw_activity_name, const ConverterConfig& config)
    -> std::string {
  // Design intent:
  // 1) Atomic record validates authorable event tokens: alias_mapping.toml LEFT
  //    keys plus converter-config wake_keywords.
  // 2) On success, TXT keeps the user-entered key text unchanged (no key->value rewrite here).
  // 3) Alias key->value mapping happens later during full TXT parse/ingest into DB.
  // This prevents record-time behavior from diverging from the TXT-first authoring model.
  const std::string normalized = NormalizeActivityNameForRecord(raw_activity_name);
  if (normalized.empty()) {
    throw std::invalid_argument("Activity name is empty.");
  }
  if (config.text_mapping.contains(normalized) ||
      IsWakeKeywordActivity(normalized, config.wake_keywords)) {
    return normalized;
  }
  throw std::invalid_argument(
      "Activity name must match configured authorable event token "
      "(alias key or wake keyword): " +
      normalized);
}

[[nodiscard]] auto BuildCandidateRecordContent(
    std::string_view existing_content, const ParsedIsoDate& logical_date,
    std::string_view record_activity_name, std::string_view remark,
    const TimeOrderMode time_order_mode,
    const std::vector<std::string>& wake_keywords)
    -> CandidateRecordBuildResult {
  // Record Activity starts from one synthesized event line, but core validation semantics are
  // defined for a whole month TXT. Build the full candidate content in memory first so the new
  // line is judged in document context before official TXT is modified.
  std::vector<std::string> lines = SplitLines(existing_content);
  const std::string event_time = GetCurrentLocalHhmm();
  const std::string event_line =
      BuildRawEventLine(event_time, record_activity_name, remark);

  const auto block_start_it = std::ranges::find_if(
      lines, [&](const std::string& line) -> bool {
        return Trim(line) == logical_date.day_marker;
      });

  std::vector<std::string> warnings;
  if (block_start_it == lines.end()) {
    if (!lines.empty() && !lines.back().empty()) {
      lines.push_back("");
    }
    lines.push_back(logical_date.day_marker);
    lines.push_back(event_line);
    const int block_start = static_cast<int>(lines.size()) - 2;
    const auto completeness_warning = ResolveAuthoredEventCompletenessWarning(
        lines, block_start, static_cast<int>(lines.size()), wake_keywords);
    if (completeness_warning.has_value()) {
      warnings.push_back(*completeness_warning);
    }
    return {
        .canonical_text = JoinLinesCanonical(lines),
        .event_time = event_time,
        .gap_from_previous_minutes = std::nullopt,
        .month_file_created = false,
        .warnings = warnings,
    };
  }

  const int block_start =
      static_cast<int>(std::distance(lines.begin(), block_start_it));
  const int block_end = FindDayBlockEnd(lines, block_start);
  if (HasDuplicateInDayBlock(lines, block_start, block_end, event_time,
                             record_activity_name)) {
    warnings.push_back(
        "Possible duplicate record: same HHmm and activity already exists in this day.");
  }

  const auto last_event_time =
      FindLastValidEventTimeToken(lines, block_start, block_end);
  std::optional<int> gap_from_previous_minutes = std::nullopt;
  if (last_event_time.has_value() &&
      !IsStrictlyAfter(event_time, *last_event_time, time_order_mode)) {
    throw std::invalid_argument(
        "Record rejected: new time " + event_time +
        " must be later than last event time " + *last_event_time +
        " in day " + logical_date.day_marker +
        ". Use DAY/ALL editor for backfill edits.");
  }
  if (last_event_time.has_value()) {
    // `gap_from_previous_minutes` is a presentation metric for Record success
    // summary: it measures the delta between current event time and previous
    // event time in the same day block, under the selected time-order mode.
    // This does not participate in persistence format or validation outcome.
    const auto candidate_minutes =
        record_time_order::TryParseHhmmToMinutes(event_time);
    const auto baseline_minutes =
        record_time_order::TryParseHhmmToMinutes(*last_event_time);
    if (candidate_minutes.has_value() && baseline_minutes.has_value()) {
      const int mapped_candidate =
          record_time_order::MapMinutesForTimeOrderMode(*candidate_minutes,
                                                        time_order_mode);
      const int mapped_baseline =
          record_time_order::MapMinutesForTimeOrderMode(*baseline_minutes,
                                                        time_order_mode);
      const int delta_minutes = mapped_candidate - mapped_baseline;
      if (delta_minutes > 0) {
        gap_from_previous_minutes = delta_minutes;
      }
    }
  }

  lines.insert(lines.begin() + block_end, event_line);
  const int updated_block_end = FindDayBlockEnd(lines, block_start);
  const auto completeness_warning = ResolveAuthoredEventCompletenessWarning(
      lines, block_start, updated_block_end, wake_keywords);
  if (completeness_warning.has_value()) {
    warnings.push_back(*completeness_warning);
  }

  return {
      .canonical_text = JoinLinesCanonical(lines),
      .event_time = event_time,
      .gap_from_previous_minutes = gap_from_previous_minutes,
      .month_file_created = false,
      .warnings = warnings,
  };
}
