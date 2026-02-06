// validator/txt/rules/line_rules.cpp
#include "validator/txt/rules/line_rules.hpp"

#include <algorithm>
#include <iostream>

#include "common/utils/string_utils.hpp"

namespace validator::txt {

LineRules::LineRules(const ConverterConfig& config) : config_(config) {
  // [Fix] 直接访问 public 成员变量，不再使用 Getter
  const auto& text_map = config.text_mapping;
  for (const auto& pair : text_map) {
    valid_event_keywords_.insert(pair.first);
  }

  const auto& dur_text_map = config.text_duration_mapping;
  for (const auto& pair : dur_text_map) {
    valid_event_keywords_.insert(pair.first);
  }

  const auto& wake_vec = config.wake_keywords;
  wake_keywords_.insert(wake_vec.begin(), wake_vec.end());

  const auto& top_map = config.top_parent_mapping;
  for (const auto& pair : top_map) {
    valid_event_keywords_.insert(pair.first);
  }

  // 这部分是对运行时注入的配置进行处理，保持不变
  for (const auto& pair : config.initial_top_parents) {
    valid_event_keywords_.insert(pair.first);
  }
}

auto LineRules::is_year(const std::string& line) -> bool {
  constexpr size_t kYearStringLength = 5;
  if (line.length() != kYearStringLength || line[0] != 'y') {
    return false;
  }
  return std::all_of(line.begin() + 1, line.end(), ::isdigit);
}

auto LineRules::is_date(const std::string& line) -> bool {
  constexpr size_t kDateStringLength = 4;
  return line.length() == kDateStringLength &&
         std::ranges::all_of(line, ::isdigit);
}

auto LineRules::is_remark(const std::string& line) const -> bool {
  // [Fix] 直接访问 public 成员变量
  const std::string& prefix = config_.remark_prefix;

  if (prefix.empty() || !line.starts_with(prefix)) {
    return false;
  }
  return !Trim(line.substr(prefix.length())).empty();
}

auto LineRules::is_valid_event_line(const std::string& line, int line_number,
                                    std::set<Error>& errors) const -> bool {
  constexpr size_t kMinimumEventLineLength = 5;
  constexpr size_t kTimePrefixLength = 4;
  constexpr int kMaxHours = 23;
  constexpr int kMaxMinutes = 59;

  if (line.length() < kMinimumEventLineLength ||
      !std::all_of(line.begin(), line.begin() + kTimePrefixLength, ::isdigit)) {
    return false;
  }
  try {
    int hours = std::stoi(line.substr(0, 2));
    int minutes = std::stoi(line.substr(2, 2));
    if (hours > kMaxHours || minutes > kMaxMinutes) {
      return false;
    }

    std::string remaining_line = line.substr(kTimePrefixLength);
    std::string description;

    size_t comment_pos = std::string::npos;
    constexpr std::array<const char*, 3> kDelimiters = {"//", "#", ";"};
    for (const char* delim : kDelimiters) {
      size_t pos = remaining_line.find(delim);
      if (pos != std::string::npos &&
          (comment_pos == std::string::npos || pos < comment_pos)) {
        comment_pos = pos;
      }
    }

    description = Trim(remaining_line.substr(0, comment_pos));
    if (description.empty()) {
      return false;
    }

    if (!wake_keywords_.contains(description) &&
        !valid_event_keywords_.contains(description)) {
      errors.insert({line_number,
                     "Unrecognized activity '" + description +
                         "'. Please check spelling or update config file.",
                     ErrorType::kUnrecognizedActivity});
    }
    return true;
  } catch (const std::exception&) {
    return false;
  }
}

}  // namespace validator::txt
