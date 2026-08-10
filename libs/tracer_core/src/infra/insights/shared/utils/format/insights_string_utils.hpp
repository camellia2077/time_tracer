// infra/insights/shared/utils/format/insights_string_utils.hpp
#ifndef INFRASTRUCTURE_INSIGHTS_SHARED_UTILS_FORMAT_INSIGHTS_STRING_UTILS_H_
#define INFRASTRUCTURE_INSIGHTS_SHARED_UTILS_FORMAT_INSIGHTS_STRING_UTILS_H_

#include <string>

#include "domain/insights/models/range_insights_data.hpp"
#include "infra/insights/shared/api/shared_api.hpp"

/**
 * @brief 将表示布尔值的字符串 ("0" 或 "1")
 * 转换为文本形式。

 * *
 * @param value
 * 要转换的字符串，期望为 "0" 或 "1"。
 * @return 输入为 "1"
 * 返回 "true"，否则返回 "false"。
 */
INSIGHTS_SHARED_API auto BoolToString(const std::string& value) -> std::string;

/**
 * @brief 替换字符串中所有匹配的子串。
 */
INSIGHTS_SHARED_API auto ReplaceAll(std::string str, const std::string& from,
                                   const std::string& replacement_str)
    -> std::string;

/**
 * @brief [新增] 为列表项中的多行文本进行格式化。
 * @param text 原始多行文本（包含 \n）。
 * @param indent_spaces 换行后需要补充的空格数量，用于保持 Markdown/Typst
 * 的缩进对齐。
 * @param line_suffix 可选，每行结尾添加的字符（例如 LaTeX 需要 "\\"）。
 * @return 格式化后的字符串。
 */
INSIGHTS_SHARED_API auto FormatMultilineForList(
    const std::string& text, int indent_spaces,
    const std::string& line_suffix = "") -> std::string;

/**
 * @brief 使用模板渲染 Range 标题。
 * 支持占位符: {range_label},
 * {start_date}, {end_date}, {requested_days},
 * {year_month}, {days_to_query}
 *
 */
INSIGHTS_SHARED_API auto FormatTitleTemplate(std::string title_template,
                                            const RangeInsightsData& data)
    -> std::string;

/**
 * @brief Format a count with percentage text, e.g. "3 (12.50%)".
 * If
 * total_count <= 0, returns the count only.
 *
 * @param count Current count
 * value.
 * @param total_count Denominator for percentage calculation.
 *
 * @param percent_suffix Percentage symbol (e.g. "%" or "\\%").
 */
INSIGHTS_SHARED_API auto FormatCountWithPercentage(
    int count, int total_count, const std::string& percent_suffix = "%")
    -> std::string;

INSIGHTS_SHARED_API auto FormatCountWithAverage(int count, int total_days)
    -> std::string;

INSIGHTS_SHARED_API auto FormatBooleanCountLabel(std::string label, int count)
    -> std::string;

#endif  // INFRASTRUCTURE_INSIGHTS_SHARED_UTILS_FORMAT_INSIGHTS_STRING_UTILS_H_
