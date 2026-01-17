// cli/impl/utils/arg_utils.hpp
#ifndef CLI_IMPL_UTILS_ARG_UTILS_HPP_
#define CLI_IMPL_UTILS_ARG_UTILS_HPP_

#include <string>
#include <vector>
#include <sstream>
#include <stdexcept>
#include <algorithm> // for std::find if needed
#include "reports/shared/types/ReportFormat.hpp"
#include "validator/common/ValidatorUtils.hpp" // DateCheckMode definition

class ArgUtils {
public:
    // 解析报表格式 (处理逗号分隔字符串，如 "md,tex")
    static std::vector<ReportFormat> parse_report_formats(const std::string& format_str) {
        std::vector<ReportFormat> formats;
        std::stringstream ss(format_str);
        std::string segment;

        while (std::getline(ss, segment, ',')) {
            // 简单的去空格处理
            segment.erase(0, segment.find_first_not_of(" \t\n\r"));
            segment.erase(segment.find_last_not_of(" \t\n\r") + 1);

            if (segment == "md" || segment == "markdown") {
                formats.push_back(ReportFormat::Markdown);
            } else if (segment == "tex" || segment == "latex") {
                formats.push_back(ReportFormat::LaTeX);
            } else if (segment == "typ" || segment == "typst") {
                formats.push_back(ReportFormat::Typ);
            } else {
                throw std::runtime_error("Unsupported format: '" + segment + "'");
            }
        }
        
        if (formats.empty()) return {ReportFormat::Markdown};
        return formats;
    }

    // 解析日期检查模式
    static DateCheckMode parse_date_check_mode(const std::string& mode_str) {
        if (mode_str == "continuity") {
            return DateCheckMode::Continuity;
        } else if (mode_str == "full" || mode_str == "strict") {
            return DateCheckMode::Full;
        } else if (mode_str == "none" || mode_str == "off") {
            return DateCheckMode::None;
        } else {
            throw std::runtime_error("Invalid date check mode: '" + mode_str + "'");
        }
    }
};

#endif // CLI_IMPL_UTILS_ARG_UTILS_HPP_