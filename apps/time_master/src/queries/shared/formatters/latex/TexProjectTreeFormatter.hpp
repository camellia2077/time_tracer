// queries/shared/formatters/latex/TexProjectTreeFormatter.hpp
#ifndef TEX_PROJECT_TREE_FORMATTER_HPP
#define TEX_PROJECT_TREE_FORMATTER_HPP

#include <string>
#include "common/utils/ProjectTree.hpp"

namespace TexUtils::Internal {

/**
 * @brief (内部实现) 将项目树格式化为 LaTeX 字符串。
 * * 这是 format_project_tree 的具体实现，被从 TexUtils.cpp 中分离出来。
 */
std::string format_project_tree_impl(
    const ProjectTree& tree,
    long long total_duration,
    int avg_days,
    int category_title_font_size,
    double list_top_sep_pt,
    double list_item_sep_ex
);

} // namespace TexUtils::Internal

#endif // TEX_PROJECT_TREE_FORMATTER_HPP