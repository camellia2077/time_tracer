// queries/shared/formatters/latex/TexProjectTreeFormatter.cpp
#include "TexProjectTreeFormatter.hpp"
#include "TexUtils.hpp" // 需要 escape_latex
#include "queries/shared/utils/format/TimeFormat.hpp"
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <iomanip>
#include <format>

namespace { // 匿名命名空间

void generate_sorted_tex_output(
    std::stringstream& ss,
    const ProjectNode& node,
    int avg_days,
    const std::string& itemize_options
) {
    if (node.children.empty()) {
        return;
    }

    std::vector<std::pair<std::string, ProjectNode>> sorted_children;
    for (const auto& pair : node.children) {
        sorted_children.push_back(pair);
    }
    std::sort(sorted_children.begin(), sorted_children.end(), [](const auto& a, const auto& b) {
        return a.second.duration > b.second.duration;
    });

    ss << "\\begin{itemize}" << itemize_options << "\n";

    for (const auto& pair : sorted_children) {
        const std::string& name = pair.first;
        const ProjectNode& child_node = pair.second;

        if (child_node.duration > 0 || !child_node.children.empty()) {
            ss << "    \\item " << TexUtils::escape_latex(name) << ": "
               << TexUtils::escape_latex(time_format_duration(child_node.duration, avg_days));

            if (!child_node.children.empty()) {
                ss << "\n";
                generate_sorted_tex_output(ss, child_node, avg_days, itemize_options);
            }
            ss << "\n";
        }
    }

    ss << "\\end{itemize}\n";
}

} // 匿名命名空间结束


namespace TexUtils::Internal {

std::string format_project_tree_impl(
    const ProjectTree& tree,
    long long total_duration,
    int avg_days,
    int category_title_font_size,
    double list_top_sep_pt,
    double list_item_sep_ex
) {
    /*
        [中文注释] 设计决策说明
        ========================
        为什么 TexUtils 不像 TypstUtils 或 MarkdownUtils 那样直接调用 ProjectTreeFormatter?

        1.  格式化方式的根本不同:
            - Markdown 和 Typst: 它们的列表层级关系仅通过简单的“行前缩进”来表达。
              通用遍历器 `ProjectTreeFormatter` 可以很好地处理这种情况，因为它只需要在
              递归的每一层增加缩进即可。
            - LaTeX: 它的嵌套列表需要使用 `\begin{itemize}` 和 `\end{itemize}`
              这样的环境块将一组 `\item` 包裹起来。每一层嵌套都需要一个新的环境块。

        2.  ProjectTreeFormatter 的局限性:
            - `ProjectTreeFormatter` 及其 `IFormattingStrategy` 接口的设计是“无状态”的，
              它只提供了“如何格式化单个节点”的策略，但没有提供“在开始遍历子节点之前”
              和“结束遍历子节点之后”的回调接口。
            - 这意味着我们无法通过这个通用接口来插入必需的 `\begin{itemize}` 和
              `\end{itemize}` 命令。

        3.  结论:
            为了生成语法正确的 LaTeX 代码，我们不能使用通用的 `ProjectTreeFormatter`。
            因此，我们选择在这里维护一个专门为 LaTeX 定制的递归实现 (`generate_sorted_tex_output`)，
            以确保对列表环境的精确控制。
    */
    std::stringstream ss;
    ss << std::fixed << std::setprecision(1);

    std::vector<std::pair<std::string, ProjectNode>> sorted_top_level;
    for (const auto& pair : tree) {
        sorted_top_level.push_back(pair);
    }
    std::sort(sorted_top_level.begin(), sorted_top_level.end(), [](const auto& a, const auto& b) {
        return a.second.duration > b.second.duration;
    });

    std::string itemize_options = std::format("[topsep={}pt, itemsep={}ex]",
        list_top_sep_pt,
        list_item_sep_ex
    );

    for (const auto& pair : sorted_top_level) {
        const std::string& category_name = pair.first;
        const ProjectNode& category_node = pair.second;
        double percentage = (total_duration > 0) ? (static_cast<double>(category_node.duration) / total_duration * 100.0) : 0.0;

        ss << "{";
        ss << "\\fontsize{" << category_title_font_size << "}{" << category_title_font_size * 1.2 << "}\\selectfont";
        ss << "\\section*{" << TexUtils::escape_latex(category_name) << ": "
           << TexUtils::escape_latex(time_format_duration(category_node.duration, avg_days))
           << " (" << percentage << "\\%)}";
        ss << "}\n";

        generate_sorted_tex_output(ss, category_node, avg_days, itemize_options);
        ss << "\n";
    }

    return ss.str();
}

} // namespace TexUtils::Internal