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

auto format_title_template(std::string title_template,
                           const RangeReportData& data) -> std::string {
  const std::string requested_days =
      (data.requested_days > 0) ? std::to_string(data.requested_days) : "";

  title_template =
      replace_all(title_template, "{range_label}", data.range_label);
  title_template =
      replace_all(title_template, "{start_date}", data.start_date);
  title_template = replace_all(title_template, "{end_date}", data.end_date);
  title_template =
      replace_all(title_template, "{requested_days}", requested_days);

  // Backward-compatible aliases
  title_template =
      replace_all(title_template, "{year_month}", data.range_label);
  title_template =
      replace_all(title_template, "{days_to_query}", requested_days);

  return title_template;
}
