// validator/txt/rules/LineRules.cpp
#include "LineRules.hpp"

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

bool LineRules::is_year(const std::string& line) {
  if (line.length() != 5 || line[0] != 'y') {
    return false;
  }
  return std::all_of(line.begin() + 1, line.end(), ::isdigit);
}

bool LineRules::is_date(const std::string& line) {
  return line.length() == 4 && std::ranges::all_of(line, ::isdigit);
}

auto LineRules::is_remark(const std::string& line) const -> bool {
  // [Fix] 直接访问 public 成员变量
  const std::string& prefix = config_.remark_prefix;

  if (prefix.empty() || !line.starts_with(prefix)) {
    return false;
  }
  return !trim(line.substr(prefix.length())).empty();
}

auto LineRules::is_valid_event_line(const std::string& line, int line_number,
                                    std::set<Error>& errors) const -> bool {
  if (line.length() < 5 ||
      !std::all_of(line.begin(), line.begin() + 4, ::isdigit)) {
    return false;
  }
  try {
    int hh = std::stoi(line.substr(0, 2));
    int mm = std::stoi(line.substr(2, 2));
    if (hh > 23 || mm > 59) {
      return false;
    }

    std::string remaining_line = line.substr(4);
    std::string description;

    size_t comment_pos = std::string::npos;
    const char* delimiters[] = {"//", "#", ";"};
    for (const char* delim : delimiters) {
      size_t pos = remaining_line.find(delim);
      if (pos != std::string::npos &&
          (comment_pos == std::string::npos || pos < comment_pos)) {
        comment_pos = pos;
      }
    }

    description = trim(remaining_line.substr(0, comment_pos));
    if (description.empty()) {
      return false;
    }

    if (!wake_keywords_.contains(description) &&
        !valid_event_keywords_.contains(description)) {
      errors.insert({line_number,
                     "Unrecognized activity '" + description +
                         "'. Please check spelling or update config file.",
                     ErrorType::UnrecognizedActivity});
    }
    return true;
  } catch (const std::exception&) {
    return false;
  }
}

}  // namespace validator::txt
