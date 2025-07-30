// query/format/tex/ProjectBreakdownTexFormat.cpp (NEW FILE)
#include "ProjectBreakdownTexFormat.h"
#include "common/common_utils.h" // For time_format_duration
#include <vector>
#include <algorithm>
#include <iomanip>

/**
 * @brief 转义 LaTeX 中的特殊字符。
 */
std::string ProjectBreakdownTexFormat::escape_latex(const std::string& input) const {
    std::string output;
    output.reserve(input.size());
    for (const char c : input) {
        switch (c) {
            case '&':  output += "\\&";        break;
            case '%':  output += "\\%";        break;
            case '$':  output += "\\$";        break;
            case '#':  output += "\\#";        break;
            case '_':  output += "\\_";        break;
            case '{':  output += "\\{";        break;
            case '}':  output += "\\}";        break;
            case '~':  output += "\\textasciitilde{}"; break;
            case '^':  output += "\\textasciicircum{}"; break;
            case '\\': output += "\\textbackslash{}"; break;
            default:   output += c;            break;
        }
    }
    return output;
}

/**
 * @brief 主要的格式化函数。
 */
std::string ProjectBreakdownTexFormat::format(const ProjectTree& tree, long long total_duration, int avg_days) const {
    std::stringstream ss;
    ss << std::fixed << std::setprecision(1);

    // 1. 对顶层类别按时间降序排序
    std::vector<std::pair<std::string, ProjectNode>> sorted_top_level;
    for (const auto& pair : tree) {
        sorted_top_level.push_back(pair);
    }
    std::sort(sorted_top_level.begin(), sorted_top_level.end(), [](const auto& a, const auto& b) {
        return a.second.duration > b.second.duration;
    });

    // 2. 遍历并生成 LaTeX 格式的输出
    for (const auto& pair : sorted_top_level) {
        const std::string& category_name = pair.first;
        const ProjectNode& category_node = pair.second;
        double percentage = (total_duration > 0) ? (static_cast<double>(category_node.duration) / total_duration * 100.0) : 0.0;

        // 使用 \section* 来创建不带编号的章节标题
        ss << "\\section*{" << escape_latex(category_name) << ": "
           << escape_latex(time_format_duration(category_node.duration, avg_days))
           << " (" << percentage << "\\%)}\n";

        // 调用递归函数处理子节点
        generate_sorted_output(ss, category_node, avg_days);
        ss << "\n"; // 在每个大类后添加一些间距
    }

    return ss.str();
}

/**
 * @brief 递归生成 itemize 列表。
 */
void ProjectBreakdownTexFormat::generate_sorted_output(std::stringstream& ss, const ProjectNode& node, int avg_days) const {
    if (node.children.empty()) {
        return;
    }

    // 对子节点按时间排序
    std::vector<std::pair<std::string, ProjectNode>> sorted_children;
    for (const auto& pair : node.children) {
        sorted_children.push_back(pair);
    }
    std::sort(sorted_children.begin(), sorted_children.end(), [](const auto& a, const auto& b) {
        return a.second.duration > b.second.duration;
    });

    // 开始一个 itemize 环境
    ss << "\\begin{itemize}\n";

    for (const auto& pair : sorted_children) {
        const std::string& name = pair.first;
        const ProjectNode& child_node = pair.second;

        if (child_node.duration > 0 || !child_node.children.empty()) {
            // 每个项目是一个 \item
            ss << "    \\item " << escape_latex(name) << ": "
               << escape_latex(time_format_duration(child_node.duration, avg_days));

            // 如果还有子节点，则递归调用以创建嵌套列表
            if (!child_node.children.empty()) {
                ss << "\n";
                generate_sorted_output(ss, child_node, avg_days);
            }
            ss << "\n";
        }
    }

    // 结束 itemize 环境
    ss << "\\end{itemize}\n";
}