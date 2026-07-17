// application/parser/text_parser.cpp
#include "application/parser/text_parser.hpp"
#include "shared/utils/ide_location_formatter.hpp"

#include <algorithm>
#include <array>
#include <cctype>
#include <stdexcept>
#include <string_view>

#include "application/runtime_bridge/logger.hpp"

import tracer.core.shared.string_utils;

using tracer::core::shared::ide_location::BuildIdeLocationPrefix;
using tracer::core::shared::string_utils::Trim;

namespace {
constexpr size_t kYearMarkerLength = 5;
constexpr char kYearMarkerPrefix = 'y';
constexpr size_t kMonthMarkerLength = 3;
constexpr char kMonthMarkerPrefix = 'm';
constexpr size_t kDayMarkerLength = 5;
constexpr char kDayMarkerPrefix = 'd';
constexpr size_t kDayMonthStartOffset = 1;
constexpr size_t kDayMonthDigitsLength = 2;
constexpr size_t kDayDigitsLength = 2;
constexpr size_t kDayStartOffset = 3;
constexpr size_t kLegacyTimeDigitsLength = 4;
constexpr size_t kCanonicalTimeDigitsLength = 6;
constexpr char kIntervalSeparator = '-';
constexpr size_t kTimeHourOffset = 0;
constexpr size_t kTimeHourLength = 2;
constexpr size_t kTimeMinuteOffset = 2;
constexpr size_t kTimeMinuteLength = 2;
constexpr size_t kTimeSecondOffset = 4;
constexpr size_t kTimeSecondLength = 2;
constexpr int kMaxHour = 23;
constexpr int kMaxMinute = 59;
constexpr int kMaxSecond = 59;
constexpr int kMinMonth = 1;
constexpr int kMaxMonth = 12;
constexpr size_t kRemarkDelimiterCount = 3;
// Inline remark delimiters. Example: "1026math //note" -> remark "note".
constexpr std::array<std::string_view, kRemarkDelimiterCount>
    kRemarkDelimiters = {"//", "#", ";"};

[[nodiscard]] auto IsAsciiDigit(char value) -> bool {
  return value >= '0' && value <= '9';
}

auto FormatTime(const std::string& time_str_hhmmss) -> std::string {
  return (time_str_hhmmss.length() == kCanonicalTimeDigitsLength)
             ? time_str_hhmmss.substr(kTimeHourOffset, kTimeHourLength) + ":" +
                   time_str_hhmmss.substr(kTimeMinuteOffset, kTimeMinuteLength) +
                   ":" +
                   time_str_hhmmss.substr(kTimeSecondOffset, kTimeSecondLength)
             : time_str_hhmmss;
}

[[noreturn]] void ThrowParseError(std::string_view source_file, int line_number,
                                  const std::string& line,
                                  const std::string& message) {
  std::string prefix = BuildIdeLocationPrefix(source_file, line_number);
  if (prefix.empty()) {
    prefix = "unknown location: ";
  }
  throw std::runtime_error(prefix + "Parse error: " + message + " => '" + line +
                           "'");
}

[[nodiscard]] auto NormalizeTimeToHhmmss(std::string_view authored_time)
    -> std::optional<std::string> {
  if ((authored_time.length() != kLegacyTimeDigitsLength &&
       authored_time.length() != kCanonicalTimeDigitsLength) ||
      !std::ranges::all_of(authored_time,
                           [](char value) -> bool { return IsAsciiDigit(value); })) {
    return std::nullopt;
  }

  const int hour =
      ((authored_time[kTimeHourOffset] - '0') * 10) +
      (authored_time[kTimeHourOffset + 1] - '0');
  const int minute = ((authored_time[kTimeMinuteOffset] - '0') * 10) +
                     (authored_time[kTimeMinuteOffset + 1] - '0');
  if (hour > kMaxHour || minute > kMaxMinute) {
    return std::nullopt;
  }
  if (authored_time.length() == kCanonicalTimeDigitsLength) {
    const int second = ((authored_time[kTimeSecondOffset] - '0') * 10) +
                       (authored_time[kTimeSecondOffset + 1] - '0');
    if (second > kMaxSecond) {
      return std::nullopt;
    }
    return std::string(authored_time);
  }
  return std::string(authored_time) + "00";
}

[[nodiscard]] auto UsesSixDigitTime(std::string_view line) -> bool {
  return line.length() >= kCanonicalTimeDigitsLength &&
         std::ranges::all_of(line.substr(0, kCanonicalTimeDigitsLength),
                             [](char value) -> bool {
                               return IsAsciiDigit(value);
                             });
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
  std::string current_month_prefix;
  int line_number = 0;

  while (std::getline(input_stream, line)) {
    ++line_number;
    line = Trim(line);
    if (line.empty()) {
      continue;
    }

    if (IsYearMarker(line)) {
      current_year_prefix = line.substr(1);
      current_month_prefix.clear();
      continue;
    }

    if (IsMonthMarker(line)) {
      if (current_year_prefix.empty()) {
        tracer_core::application::runtime_bridge::LogWarn(
            "Warning: Skipping line '" + line +
            "' because a year header (e.g., y2025) has not been found yet.");
        continue;
      }
      current_month_prefix = line.substr(1);
      continue;
    }

    if (current_year_prefix.empty()) {
      tracer_core::application::runtime_bridge::LogWarn(
          "Warning: Skipping line '" + line +
          "' because a year header (e.g., y2025) has not been found yet.");
      continue;
    }

    if (IsNewDayMarker(line)) {
      if (current_month_prefix.empty()) {
        ThrowParseError(source_file, line_number, line,
                        "Date found before month header (mMM)");
      }
      if (!current_day.date.empty()) {
        on_new_day(current_day);
      }
      current_day.Clear();
      const std::string kMonthPrefix =
          line.substr(kDayMonthStartOffset, kDayMonthDigitsLength);
      if (kMonthPrefix != current_month_prefix) {
        ThrowParseError(source_file, line_number, line,
                        "Date month '" + kMonthPrefix +
                            "' does not match month header '" +
                            current_month_prefix + "'");
      }
      current_day.date = current_year_prefix + "-" + kMonthPrefix + "-" +
                         line.substr(kDayStartOffset, kDayDigitsLength);
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

auto TextParser::IsMonthMarker(const std::string& line) -> bool {
  if (line.length() != kMonthMarkerLength || line[0] != kMonthMarkerPrefix) {
    return false;
  }
  if (!std::ranges::all_of(line.substr(1), [](char value) -> bool {
        return IsAsciiDigit(value);
      })) {
    return false;
  }
  int month_value = std::stoi(line.substr(1));
  return month_value >= kMinMonth && month_value <= kMaxMonth;
}

auto TextParser::IsNewDayMarker(const std::string& line) -> bool {
  return line.length() == kDayMarkerLength && line[0] == kDayMarkerPrefix &&
         std::ranges::all_of(
             line.substr(1),
             [](char value) -> bool { return IsAsciiDigit(value); });
}

auto TextParser::ExtractRemark(std::string_view remaining_line)
    -> RemarkResult {
  size_t comment_pos = std::string::npos;
  std::string_view chosen_delimiter;

  // Multiple inline remark markers are allowed; pick the earliest one so
  // "0232foo //a #b" keeps description as "foo" and remark as "a #b".
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

  const bool is_first_semantic_event =
      current_day.getupTime.empty() && current_day.rawEvents.empty();

  if (is_wake) {
    // Wake keywords define the day-level wake anchor, not a sleep activity.
    // Parser ownership stops at establishing first-event day semantics.
    // Only the first semantic event may establish that anchor here; later wake
    // keywords are not rejected in parser, but must be rejected later by
    // logic validation and must not redefine the day.
    if (input.kind == RawEventKind::Point && is_first_semantic_event) {
      current_day.getupTime = FormatTime(std::string(input.end_time_str_hhmm));
    }

    return is_wake;
  }

  // A first point event without wake semantics needs previous-day context to
  // close its leading segment. A first interval event is self-contained and
  // must not inherit a previous-day boundary.
  if (input.kind == RawEventKind::Point && is_first_semantic_event) {
    current_day.isContinuation = true;
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
    ThrowParseError(source_file, line_number, line,
                    "Event line appears before date");
  }

  if (line.length() < kLegacyTimeDigitsLength ||
      !std::ranges::all_of(
          line.substr(0, kLegacyTimeDigitsLength),
          [](char value) -> bool { return IsAsciiDigit(value); })) {
    ThrowParseError(source_file, line_number, line,
                    "Invalid event line format");
  }

  RawEventKind event_kind = RawEventKind::Point;
  std::optional<std::string> start_time_hhmmss;
  std::string end_time_hhmmss;
  std::string_view event_payload;
  const size_t time_length = UsesSixDigitTime(line) ? kCanonicalTimeDigitsLength
                                                     : kLegacyTimeDigitsLength;
  if (line.length() > time_length && line[time_length] == kIntervalSeparator) {
    const size_t end_offset = time_length + 1U;
    if (line.length() < end_offset + time_length) {
      ThrowParseError(source_file, line_number, line, "Invalid event line format");
    }
    const auto start_time = NormalizeTimeToHhmmss(
        std::string_view(line).substr(0, time_length));
    const auto end_time = NormalizeTimeToHhmmss(
        std::string_view(line).substr(end_offset, time_length));
    if (!start_time.has_value() || !end_time.has_value()) {
      ThrowParseError(source_file, line_number, line, "Time out of range");
    }
    event_kind = RawEventKind::Interval;
    start_time_hhmmss = *start_time;
    end_time_hhmmss = *end_time;
    event_payload = std::string_view(line).substr(end_offset + time_length);
  } else {
    const auto end_time = NormalizeTimeToHhmmss(
        std::string_view(line).substr(0, time_length));
    if (!end_time.has_value()) {
      ThrowParseError(source_file, line_number, line, "Time out of range");
    }
    end_time_hhmmss = *end_time;
    event_payload = std::string_view(line).substr(time_length);
  }

  RemarkResult remark_data = ExtractRemark(event_payload);

  if (remark_data.description.empty()) {
    ThrowParseError(source_file, line_number, line,
                    "Missing activity description");
  }

  std::optional<std::string_view> start_time_view;
  if (start_time_hhmmss.has_value()) {
    start_time_view = *start_time_hhmmss;
  }

  ProcessEventContext(current_day, {.kind = event_kind,
                                    .description = remark_data.description,
                                    .start_time_str_hhmm = start_time_view,
                                    .end_time_str_hhmm = end_time_hhmmss});

  RawEvent raw_event;
  raw_event.kind = event_kind;
  raw_event.startTimeStr = std::move(start_time_hhmmss);
  raw_event.endTimeStr = std::move(end_time_hhmmss);
  raw_event.description = std::move(remark_data.description);
  raw_event.remark = std::move(remark_data.remark);
  raw_event.source_span = SourceSpan{.file_path = std::string(source_file),
                                     .line_start = line_number,
                                     .line_end = line_number,
                                     .column_start = 1,
                                     .column_end = static_cast<int>(line.length()),
                                     .raw_text = line};
  current_day.rawEvents.push_back(std::move(raw_event));
}
