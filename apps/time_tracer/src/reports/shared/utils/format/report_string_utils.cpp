// reports/shared/utils/format/report_string_utils.cpp
#include "report_string_utils.hpp"

auto replace_all(std::string str, const std::string& from,
                 const std::string& to) -> std::string {
  size_t start_pos = 0;
  while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
    str.replace(start_pos, from.length(), to);
    start_pos += to.length();
  }
  return str;
}

auto format_multiline_for_list(const std::string& text, int indent_spaces,
                               const std::string& line_suffix) -> std::string {
  if (text.empty()) {
    return "";
  }

  std::string indent(indent_spaces, ' ');
  // 将 "\n" 替换为 "后缀 + \n + 缩进"
  std::string replacement = line_suffix + "\n" + indent;

  return replace_all(text, "\n", replacement);
}