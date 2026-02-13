// adapters/input/parser/text_parser.cpp
#include "application/parser/text_parser.hpp"

#include <algorithm>
#include <array>
#include <cctype>
#include <iostream>
#include <stdexcept>
#include <string_view>

#include "shared/types/ansi_colors.hpp"
#include "shared/utils/string_utils.hpp"

namespace {
constexpr size_t kYearMarkerLength = 5;
constexpr char kYearMarkerPrefix = 'y';
constexpr size_t kDayMarkerLength = 4;
constexpr size_t kMonthDigitsLength = 2;
constexpr size_t kDayDigitsLength = 2;
constexpr size_t kMonthStartOffset = 0;
constexpr size_t kDayStartOffset = 2;
constexpr size_t kTimeDigitsLength = 4;
constexpr size_t kTimeHourOffset = 0;
constexpr size_t kTimeHourLength = 2;
constexpr size_t kTimeMinuteOffset = 2;
constexpr size_t kTimeMinuteLength = 2;
constexpr int kMaxHour = 23;
constexpr int kMaxMinute = 59;
constexpr size_t kRemarkDelimiterCount = 3;
// Inline remark delimiters. Example: "1026math //note" -> remark "note".
constexpr std::array<std::string_view, kRemarkDelimiterCount>
    kRemarkDelimiters = {"//", "#", ";"};

[[nodiscard]] auto IsAsciiDigit(char value) -> bool {
  return value >= '0' && value <= '9';
}

auto FormatTime(const std::string& time_str_hhmm) -> std::string {
  return (time_str_hhmm.length() == kTimeDigitsLength)
             ? time_str_hhmm.substr(kTimeHourOffset, kTimeHourLength) + ":" +
                   time_str_hhmm.substr(kTimeMinuteOffset, kTimeMinuteLength)
             : time_str_hhmm;
}

[[noreturn]] void ThrowParseError(int line_number, const std::string& line,
                                  const std::string& message) {
  throw std::runtime_error("Parse error at line " +
                           std::to_string(line_number) + ": " + message +
                           " => '" + line + "'");
}
}  // namespace

TextParser::TextParser(const ConverterConfig& config)
    : config_(config), wake_keywords_(config.wake_keywords) {}

auto TextParser::Parse(std::istream& input_stream,
                       std::function<void(DailyLog&)> on_new_day,
                       std::string_view source_file) -> void {
  DailyLog current_day;
  std::string line;
  std::string current_year_prefix;
  int line_number = 0;

  while (std::getline(input_stream, line)) {
    ++line_number;
    line = Trim(line);
    if (line.empty()) {
      continue;
    }

    if (IsYearMarker(line)) {
      current_year_prefix = line.substr(1);
      continue;
    }

    if (current_year_prefix.empty()) {
      std::cerr
          << time_tracer::common::colors::kYellow << "Warning: Skipping line '"
          << line
          << "' because a year header (e.g., y2025) has not been found yet."
          << time_tracer::common::colors::kReset << std::endl;
      continue;
    }

    if (IsNewDayMarker(line)) {
      if (!current_day.date.empty()) {
        on_new_day(current_day);
      }
      current_day.Clear();
      current_day.date = current_year_prefix + "-" +
                         line.substr(kMonthStartOffset, kMonthDigitsLength) +
                         "-" + line.substr(kDayStartOffset, kDayDigitsLength);
      current_day.source_span =
          SourceSpan{.file_path = std::string(source_file),
                     .line_start = line_number,
                     .line_end = line_number,
                     .column_start = 1,
                     .column_end = static_cast<int>(line.length()),
                     .raw_text = line};

    } else {
      ParseLine(line, line_number, current_day, source_file);
    }
  }
  if (!current_day.date.empty()) {
    on_new_day(current_day);
  }
}

auto TextParser::IsYearMarker(const std::string& line) -> bool {
  if (line.length() != kYearMarkerLength || line[0] != kYearMarkerPrefix) {
    return false;
  }
  return std::ranges::all_of(
      line.substr(1), [](char value) -> bool { return IsAsciiDigit(value); });
}

auto TextParser::IsNewDayMarker(const std::string& line) -> bool {
  return line.length() == kDayMarkerLength &&
         std::ranges::all_of(
             line, [](char value) -> bool { return IsAsciiDigit(value); });
}

auto TextParser::ExtractRemark(std::string_view remaining_line)
    -> RemarkResult {
  size_t comment_pos = std::string::npos;
  std::string_view chosen_delimiter;

  for (std::string_view delimiter : kRemarkDelimiters) {
    size_t pos = remaining_line.find(delimiter);
    if (pos != std::string::npos) {
      if (comment_pos == std::string::npos || pos < comment_pos) {
        comment_pos = pos;
        chosen_delimiter = delimiter;
      }
    }
  }

  if (comment_pos != std::string::npos) {
    std::string description =
        Trim(std::string(remaining_line.substr(0, comment_pos)));
    std::string remark = Trim(std::string(
        remaining_line.substr(comment_pos + chosen_delimiter.length())));
    return {.description = description, .remark = remark};
  }

  return {.description = Trim(std::string(remaining_line)), .remark = ""};
}

auto TextParser::ProcessEventContext(DailyLog& current_day,
                                     EventInput input) const

    -> bool {
  bool is_wake = false;
  for (const auto& keyword : wake_keywords_) {
    if (keyword == input.description) {
      is_wake = true;
      break;
    }
  }

  if (is_wake) {
    if (current_day.getupTime.empty()) {
      current_day.getupTime = FormatTime(std::string(input.time_str_hhmm));
    }

  } else {
    if (current_day.getupTime.empty() && current_day.rawEvents.empty()) {
      current_day.isContinuation = true;
    }
  }
  return is_wake;
}

auto TextParser::ParseLine(const std::string& line, int line_number,
                           DailyLog& current_day,
                           std::string_view source_file) const -> void {
  const std::string& remark_prefix = config_.remark_prefix;

  if (!remark_prefix.empty() && line.starts_with(remark_prefix)) {
    if (!current_day.date.empty()) {
      current_day.generalRemarks.push_back(line.substr(remark_prefix.length()));
    }
    return;
  }

  if (current_day.date.empty()) {
    ThrowParseError(line_number, line, "Event line appears before date");
  }

  if (line.length() < kTimeDigitsLength ||
      !std::ranges::all_of(
          line.substr(0, kTimeDigitsLength),
          [](char value) -> bool { return IsAsciiDigit(value); })) {
    ThrowParseError(line_number, line, "Invalid event line format");
  }

  const int kHour =
      ((line[kTimeHourOffset] - '0') * 10) + (line[kTimeHourOffset + 1] - '0');
  const int kMinute = ((line[kTimeMinuteOffset] - '0') * 10) +
                      (line[kTimeMinuteOffset + 1] - '0');

  if (kHour > kMaxHour || kMinute > kMaxMinute) {
    ThrowParseError(line_number, line, "Time out of range");
  }

  std::string time_str_hhmm = line.substr(0, kTimeDigitsLength);
  RemarkResult remark_data = ExtractRemark(line.substr(kTimeDigitsLength));

  if (remark_data.description.empty()) {
    ThrowParseError(line_number, line, "Missing activity description");
  }

  ProcessEventContext(current_day, {.description = remark_data.description,
                                    .time_str_hhmm = time_str_hhmm});

  current_day.rawEvents.push_back(
      {time_str_hhmm, remark_data.description, remark_data.remark,
       SourceSpan{.file_path = std::string(source_file),
                  .line_start = line_number,
                  .line_end = line_number,
                  .column_start = 1,
                  .column_end = static_cast<int>(line.length()),
                  .raw_text = line}});
}
