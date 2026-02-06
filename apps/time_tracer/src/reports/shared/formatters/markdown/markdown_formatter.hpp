// reports/shared/formatters/markdown/markdown_formatter.hpp
#ifndef REPORTS_SHARED_FORMATTERS_MARKDOWN_MARKDOWN_FORMATTER_H_
#define REPORTS_SHARED_FORMATTERS_MARKDOWN_MARKDOWN_FORMATTER_H_

#include <string>

#include "reports/data/model/project_tree.hpp"
#include "reports/shared/api/shared_api.hpp"

namespace MarkdownFormatter {

/**
 * @brief 将项目树格式化为 Markdown 字符串。
 *
 * @param tree 要格式化的项目树。
 * @param total_duration 所有项目的总时长，用于计算百分比。
 * @param avg_days 用于计算平均时长的天数。
 * @return 格式化后的 Markdown 字符串。
 */
// [修正] 添加 reporting:: 命名空间前缀
// Public API: keep parameter order and naming for ABI compatibility.
// NOLINTBEGIN(bugprone-easily-swappable-parameters)
REPORTS_SHARED_API auto FormatProjectTree(const reporting::ProjectTree& tree,
                                          long long total_duration,
                                          int avg_days) -> std::string;
// NOLINTEND(bugprone-easily-swappable-parameters)

}  // namespace MarkdownFormatter

#endif  // REPORTS_SHARED_FORMATTERS_MARKDOWN_MARKDOWN_FORMATTER_H_
