// reports/shared/utils/format/ReportStringUtils.hpp
#ifndef REPORTS_SHARED_UTILS_FORMAT_REPORT_STRING_UTILS_HPP_
#define REPORTS_SHARED_UTILS_FORMAT_REPORT_STRING_UTILS_HPP_

#include <string>
#include "reports/shared/api/shared_api.hpp"

/**
 * @brief 替换字符串中所有匹配的子串。
 */
REPORTS_SHARED_API std::string replace_all(std::string str, const std::string& from, const std::string& to);

/**
 * @brief [新增] 为列表项中的多行文本进行格式化。
 * @param text 原始多行文本（包含 \n）。
 * @param indent_spaces 换行后需要补充的空格数量，用于保持 Markdown/Typst 的缩进对齐。
 * @param line_suffix 可选，每行结尾添加的字符（例如 LaTeX 需要 "\\"）。
 * @return 格式化后的字符串。
 */
REPORTS_SHARED_API std::string format_multiline_for_list(
    const std::string& text, 
    int indent_spaces, 
    const std::string& line_suffix = ""
);

#endif // REPORTS_SHARED_UTILS_FORMAT_REPORT_STRING_UTILS_HPP_