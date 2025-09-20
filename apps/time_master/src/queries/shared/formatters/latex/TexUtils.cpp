// queries/shared/formatters/latex/TexUtils.cpp
#include "TexUtils.hpp"
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <iomanip>
#include <format>
#include "queries/shared/utils/format/TimeFormat.hpp"

namespace { // 匿名命名空间

/**
 * @brief 递归函数，用于生成排序后的 LaTeX itemize 列表。
 * @param ss 字符串流，用于构建输出。
 * @param node 当前正在处理的 ProjectNode。
 * @param avg_days 用于计算平均值的天数。
 * @param itemize_options 传递给 itemize 环境的选项。
 */
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

namespace TexUtils {

std::string get_tex_preamble(
    const std::string& main_font,
    const std::string& cjk_main_font,
    int font_size,
    double margin_in,
    const std::map<std::string, std::string>& keyword_colors)
{
    std::stringstream ss;
    // [FIX] Corrected the order: \documentclass must be the first command.
    ss << "\\documentclass[" << font_size << "pt]{extarticle}\n"; // Using extarticle from extsizes
    ss << "\\usepackage[a4paper, margin=" << margin_in << "in]{geometry}\n";
    ss << "\\usepackage[dvipsnames]{xcolor}\n";
    ss << "\\usepackage{enumitem}\n";
    ss << "\\usepackage{fontspec}\n";
    ss << "\\usepackage{ctex}\n";
    ss << "\\usepackage{titlesec}\n\n";

    // Customize section formatting to be independent of default styles
    ss << "\\titleformat{\\section}{\\normalfont\\bfseries}{}{0em}{}\n";
    ss << "\\titleformat{\\subsection}{\\normalfont\\bfseries}{}{0em}{}\n\n";

    if (!keyword_colors.empty()) {
        for (const auto& pair : keyword_colors) {
            std::string hex_color = pair.second;
            if (!hex_color.empty() && hex_color[0] == '#') {
                hex_color = hex_color.substr(1);
            }
            ss << "\\definecolor{" << pair.first << "color}{HTML}{" << hex_color << "}\n";
        }
    }

    ss << "\n";
    ss << "\\setmainfont{" << main_font << "}\n";
    ss << "\\setCJKmainfont{" << cjk_main_font << "}\n\n";
    ss << "\\begin{document}\n\n";
    return ss.str();
}

std::string get_tex_postfix() {
    return "\n\\end{document}\n";
}

std::string escape_latex(const std::string& input) {
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

// [新增]
std::string format_project_tree(
    const ProjectTree& tree,
    long long total_duration,
    int avg_days,
    int category_title_font_size,
    double list_top_sep_pt,
    double list_item_sep_ex
) {
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
        ss << "\\section*{" << escape_latex(category_name) << ": "
           << escape_latex(time_format_duration(category_node.duration, avg_days))
           << " (" << percentage << "\\%)}";
        ss << "}\n";

        generate_sorted_tex_output(ss, category_node, avg_days, itemize_options);
        ss << "\n";
    }

    return ss.str();
}


} // namespace TexUtils