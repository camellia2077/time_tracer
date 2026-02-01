// adapters/input/parser/text_parser.cpp
#include "text_parser.hpp"

#include <algorithm>
#include <array>
#include <cctype>
#include <iostream>
#include <regex>
#include <stdexcept>
#include <string_view>

#include "common/ansi_colors.hpp"
#include "common/utils/string_utils.hpp"

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
constexpr size_t kEventMatchSize = 4;
constexpr size_t kDoubleSlashLength = 2;
constexpr size_t kSingleCharDelimiterLength = 1;
constexpr std::string_view kDoubleSlashDelimiter = "//";
constexpr size_t kRemarkDelimiterCount = 3;
// Inline remark delimiters. Example: "1026math //note" -> remark "note".
constexpr std::array<std::string_view, kRemarkDelimiterCount>
    kRemarkDelimiters = {"//", "#", ";"};

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

const std::regex kEventPattern(R"(^(\d{2})(\d{2})(.*)$)");
}  // namespace

TextParser::TextParser(const ConverterConfig& config)
    : config_(config), wake_keywords_(config.wake_keywords) {}

void TextParser::parse(std::istream& input_stream,
                       std::function<void(DailyLog&)> on_new_day) {
  DailyLog current_day;
  std::string line;
  std::string current_year_prefix;
  int line_number = 0;

  while (std::getline(input_stream, line)) {
    ++line_number;
    line = trim(line);
    if (line.empty()) {
      continue;
    }

    if (isYearMarker(line)) {
      current_year_prefix = line.substr(1);
      continue;
    }

    if (current_year_prefix.empty()) {
      std::cerr
          << YELLOW_COLOR << "Warning: Skipping line '" << line
          << "' because a year header (e.g., y2025) has not been found yet."
          << RESET_COLOR << std::endl;
      continue;
    }

    if (isNewDayMarker(line)) {
      if (!current_day.date.empty()) {
        on_new_day(current_day);
      }
      current_day.clear();
      current_day.date = current_year_prefix + "-" +
                         line.substr(kMonthStartOffset, kMonthDigitsLength) +
                         "-" + line.substr(kDayStartOffset, kDayDigitsLength);

    } else {
      parseLine(line, line_number, current_day);
    }
  }
  if (!current_day.date.empty()) {
    on_new_day(current_day);
  }
}

auto TextParser::isYearMarker(const std::string& line) -> bool {
  if (line.length() != kYearMarkerLength || line[0] != kYearMarkerPrefix) {
    return false;
  }
  return std::all_of(line.begin() + 1, line.end(), ::isdigit);
}

auto TextParser::isNewDayMarker(const std::string& line) -> bool {
  return line.length() == kDayMarkerLength &&
         std::ranges::all_of(line, ::isdigit);
}

void TextParser::parseLine(const std::string& line, int line_number,
                           DailyLog& current_day) const {
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

  std::smatch match;
  if (!std::regex_match(line, match, kEventPattern) ||
      match.size() != kEventMatchSize) {
    ThrowParseError(line_number, line, "Invalid event line format");
  }

  int hour = 0;
  int minute = 0;
  try {
    hour = std::stoi(match[1].str());
    minute = std::stoi(match[2].str());
  } catch (const std::exception&) {
    ThrowParseError(line_number, line, "Failed to parse time");
  }

  if (hour > kMaxHour || minute > kMaxMinute) {
    ThrowParseError(line_number, line, "Time out of range");
  }

  std::string time_str_hhmm = match[1].str() + match[2].str();
  std::string remaining_line = match[3].str();

  std::string description;
  std::string remark_text;

  size_t comment_pos = std::string::npos;
  for (std::string_view delimiter : kRemarkDelimiters) {
    size_t pos = remaining_line.find(delimiter);
    if (pos != std::string::npos) {
      if (comment_pos == std::string::npos || pos < comment_pos) {
        comment_pos = pos;
      }
    }
  }

  if (comment_pos != std::string::npos) {
    description = trim(remaining_line.substr(0, comment_pos));
    size_t delimiter_length =
        (remaining_line.substr(comment_pos, kDoubleSlashLength) ==
         kDoubleSlashDelimiter)
            ? kDoubleSlashLength
            : kSingleCharDelimiterLength;
    remark_text = trim(remaining_line.substr(comment_pos + delimiter_length));
  } else {
    description = trim(remaining_line);
  }

  if (description.empty()) {
    ThrowParseError(line_number, line, "Missing activity description");
  }

  bool is_wake = false;
  for (const auto& keyword : wake_keywords_) {
    if (keyword == description) {
      is_wake = true;
      break;
    }
  }

  if (is_wake) {
    if (current_day.getupTime.empty()) {
      current_day.getupTime = FormatTime(time_str_hhmm);
    }
  } else {
    if (current_day.getupTime.empty() && current_day.rawEvents.empty()) {
      current_day.isContinuation = true;
    }
  }
  current_day.rawEvents.push_back({time_str_hhmm, description, remark_text});
}
