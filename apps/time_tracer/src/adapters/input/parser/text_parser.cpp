// adapters/input/parser/text_parser.cpp
#include "text_parser.hpp"

#include <algorithm>
#include <cctype>
#include <iostream>
#include <regex>
#include <stdexcept>

#include "common/ansi_colors.hpp"
#include "common/utils/string_utils.hpp"

namespace {
auto FormatTime(const std::string& timeStrHHMM) -> std::string {
  return (timeStrHHMM.length() == 4)
             ? timeStrHHMM.substr(0, 2) + ":" + timeStrHHMM.substr(2, 2)
             : timeStrHHMM;
}

[[noreturn]] void ThrowParseError(int line_number,
                                 const std::string& line,
                                 const std::string& message) {
  throw std::runtime_error("Parse error at line " + std::to_string(line_number) +
                           ": " + message + " => '" + line + "'");
}

const std::regex kEventPattern(R"(^(\d{2})(\d{2})(.*)$)");
}  // namespace

TextParser::TextParser(const ConverterConfig& config)
    : config_(config),
      // [??] ???? public ??
      wake_keywords_(config.wake_keywords) {}

void TextParser::parse(std::istream& inputStream,
                       std::function<void(DailyLog&)> onNewDay) {
  DailyLog current_day;
  std::string line;
  std::string current_year_prefix;
  int line_number = 0;

  while (std::getline(inputStream, line)) {
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
        onNewDay(current_day);
      }
      current_day.clear();
      current_day.date = current_year_prefix + "-" + line.substr(0, 2) + "-" +
                         line.substr(2, 2);

    } else {
      parseLine(line, line_number, current_day);
    }
  }
  if (!current_day.date.empty()) {
    onNewDay(current_day);
  }
}

bool TextParser::isYearMarker(const std::string& line) {
  if (line.length() != 5 || line[0] != 'y') {
    return false;
  }
  return std::all_of(line.begin() + 1, line.end(), ::isdigit);
}

bool TextParser::isNewDayMarker(const std::string& line) {
  return line.length() == 4 && std::ranges::all_of(line, ::isdigit);
}

void TextParser::parseLine(const std::string& line,
                           int line_number,
                           DailyLog& currentDay) const {
  // [??] ???? public ?? config_.remark_prefix
  const std::string& remark_prefix = config_.remark_prefix;

  if (!remark_prefix.empty() && line.starts_with(remark_prefix)) {
    if (!currentDay.date.empty()) {
      currentDay.generalRemarks.push_back(line.substr(remark_prefix.length()));
    }
    return;
  }

  if (currentDay.date.empty()) {
    ThrowParseError(line_number, line, "Event line appears before date");
  }

  std::smatch match;
  if (!std::regex_match(line, match, kEventPattern) || match.size() != 4) {
    ThrowParseError(line_number, line, "Invalid event line format");
  }

  int hh = 0;
  int mm = 0;
  try {
    hh = std::stoi(match[1].str());
    mm = std::stoi(match[2].str());
  } catch (const std::exception&) {
    ThrowParseError(line_number, line, "Failed to parse time");
  }

  if (hh > 23 || mm > 59) {
    ThrowParseError(line_number, line, "Time out of range");
  }

  std::string time_str = match[1].str() + match[2].str();
  std::string remaining_line = match[3].str();

  std::string desc;
  std::string remark;

  size_t comment_pos = std::string::npos;
  const char* delimiters[] = {"//", "#", ";"};
  for (const char* delim : delimiters) {
    size_t pos = remaining_line.find(delim);
    if (pos != std::string::npos) {
      if (comment_pos == std::string::npos || pos < comment_pos) {
        comment_pos = pos;
      }
    }
  }

  if (comment_pos != std::string::npos) {
    desc = trim(remaining_line.substr(0, comment_pos));
    size_t delim_len =
        (remaining_line.substr(comment_pos, 2) == "//") ? 2 : 1;
    remark = trim(remaining_line.substr(comment_pos + delim_len));
  } else {
    desc = trim(remaining_line);
  }

  if (desc.empty()) {
    ThrowParseError(line_number, line, "Missing activity description");
  }

  // [??] wake_keywords_ ??? vector??? std::find
  bool is_wake = false;
  for (const auto& kw : wake_keywords_) {
    if (kw == desc) {
      is_wake = true;
      break;
    }
  }

  if (is_wake) {
    if (currentDay.getupTime.empty()) {
      currentDay.getupTime = FormatTime(time_str);
    }
  } else {
    if (currentDay.getupTime.empty() && currentDay.rawEvents.empty()) {
      currentDay.isContinuation = true;
    }
  }
  currentDay.rawEvents.push_back({time_str, desc, remark});
}
