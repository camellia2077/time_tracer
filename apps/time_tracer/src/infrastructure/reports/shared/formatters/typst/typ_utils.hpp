// infrastructure/reports/shared/formatters/typst/typ_utils.hpp
#ifndef REPORTS_SHARED_FORMATTERS_TYPST_TYP_UTILS_H_
#define REPORTS_SHARED_FORMATTERS_TYPST_TYP_UTILS_H_

#include <string>

#include "domain/reports/models/project_tree.hpp"
#include "infrastructure/reports/shared/api/shared_api.hpp"

namespace TypUtils {

/**
 * @brief 将项目树格式化为 Typst 字符串。
 *
 * @param tree 要格式化的项目树。
 * @param total_duration 所有项目的总时长，用于计算百分比。
 * @param avg_days 用于计算平均时长的天数。
 * @param category_title_font 类别标题字体。
 * @param category_title_font_size 类别标题字体大小。
 * @return 格式化后的 Typst 字符串。
 */
// [修正] 添加 reporting:: 命名空间前缀
// Public API: keep parameter order and naming for ABI compatibility.
// NOLINTBEGIN(bugprone-easily-swappable-parameters)
REPORTS_SHARED_API auto FormatProjectTree(
    const reporting::ProjectTree& tree, long long total_duration, int avg_days,
    const std::string& category_title_font, int category_title_font_size)
    -> std::string;
// NOLINTEND(bugprone-easily-swappable-parameters)

}  // namespace TypUtils

#endif  // REPORTS_SHARED_FORMATTERS_TYPST_TYP_UTILS_H_
