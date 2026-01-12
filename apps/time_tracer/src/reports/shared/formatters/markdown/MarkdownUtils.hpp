// reports/shared/formatters/markdown/MarkdownUtils.hpp
#ifndef MARKDOWN_UTILS_HPP
#define MARKDOWN_UTILS_HPP

#include <string>
#include "reports/shared/model/ProjectTree.hpp"
#include "reports/shared/shared_api.hpp" // <--- [新增]

namespace MarkdownUtils {

/**
 * @brief 将项目树格式化为 Markdown 字符串。
 *
 * @param tree 要格式化的项目树。
 * @param total_duration 所有项目的总时长，用于计算百分比。
 * @param avg_days 用于计算平均时长的天数。
 * @return 格式化后的 Markdown 字符串。
 */
REPORTS_SHARED_API std::string format_project_tree(const ProjectTree& tree, long long total_duration, int avg_days); // <--- [修改]

} // namespace MarkdownUtils

#endif // MARKDOWN_UTILS_HPP