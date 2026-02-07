// api/cli/impl/utils/arg_utils.hpp
#ifndef CLI_IMPL_UTILS_ARG_UTILS_H_
#define CLI_IMPL_UTILS_ARG_UTILS_H_

#include <algorithm>  // for std::find if needed
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "domain/logic/validator/common/validator_utils.hpp"  // DateCheckMode definition
#include "domain/reports/types/report_format.hpp"

class ArgUtils {
 public:
  // 解析报表格式 (处理逗号分隔字符串，如 "md,tex")
  [[nodiscard]] static auto ParseReportFormats(const std::string& format_str)
      -> std::vector<ReportFormat> {
    std::vector<ReportFormat> formats;
    std::stringstream report_stream(format_str);
    std::string segment;

    while (std::getline(report_stream, segment, ',')) {
      // 简单的去空格处理
      segment.erase(0, segment.find_first_not_of(" \t\n\r"));
      segment.erase(segment.find_last_not_of(" \t\n\r") + 1);

      if (segment == "md" || segment == "markdown") {
        formats.push_back(ReportFormat::kMarkdown);
      } else if (segment == "tex" || segment == "latex") {
        formats.push_back(ReportFormat::kLaTeX);
      } else if (segment == "typ" || segment == "typst") {
        formats.push_back(ReportFormat::kTyp);
      } else {
        throw std::runtime_error("Unsupported format: '" + segment + "'");
      }
    }

    if (formats.empty()) {
      return {ReportFormat::kMarkdown};
    }

    return formats;
  }

  // 解析数字列表 (如 "1,3,7")
  [[nodiscard]] static auto ParseNumberList(const std::string& input)
      -> std::vector<int> {
    std::vector<int> numbers;
    std::stringstream input_stream(input);
    std::string segment;
    while (std::getline(input_stream, segment, ',')) {
      segment.erase(0, segment.find_first_not_of(" \t\n\r"));
      segment.erase(segment.find_last_not_of(" \t\n\r") + 1);
      if (!segment.empty()) {
        try {
          numbers.push_back(std::stoi(segment));
        } catch (const std::exception&) {
          throw std::runtime_error("Invalid number in list: '" + segment + "'");
        }
      }
    }
    return numbers;
  }

  // 解析日期检查模式
  [[nodiscard]] static auto ParseDateCheckMode(const std::string& mode_str)
      -> DateCheckMode {
    if (mode_str == "continuity") {
      return DateCheckMode::kContinuity;
    }
    if (mode_str == "full" || mode_str == "strict") {
      return DateCheckMode::kFull;
    }
    if (mode_str == "none" || mode_str == "off") {
      return DateCheckMode::kNone;
    }
    throw std::runtime_error("Invalid date check mode: '" + mode_str + "'");
  }
};

#endif  // CLI_IMPL_UTILS_ARG_UTILS_H_