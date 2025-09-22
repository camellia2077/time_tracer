// queries/shared/formatters/latex/TexUtils.cpp
#include "TexUtils.hpp"
#include "queries/shared/formatters/base/ProjectTreeFormatter.hpp"
#include "queries/shared/utils/format/TimeFormat.hpp"
#include <sstream>
#include <string>
#include <map>
#include <memory>
#include <format>
#include <iomanip>

namespace TexUtils {

namespace { // 匿名命名空间

class LatexFormattingStrategy : public reporting::IFormattingStrategy {
public:
    LatexFormattingStrategy(int category_font_size, double top_sep, double item_sep)
        : m_category_font_size(category_font_size),
          m_itemize_options(std::format("[topsep={}pt, itemsep={}ex]", top_sep, item_sep))
    {}

    std::string format_category_header(
        const std::string& category_name,
        const std::string& formatted_duration,
        double percentage) const override
    {
        std::stringstream ss;
        ss << "{";
        ss << "\\fontsize{" << m_category_font_size << "}{" << m_category_font_size * 1.2 << "}\\selectfont";
        ss << "\\section*{" << TexUtils::escape_latex(category_name) << ": "
           << TexUtils::escape_latex(formatted_duration)
           << " (" << std::fixed << std::setprecision(1) << percentage << "\\%)}";
        ss << "}\n";
        return ss.str();
    }

    std::string format_tree_node(
        const std::string& project_name,
        const std::string& formatted_duration,
        int /*indent_level*/) const override
    {
        // indent_level is not used in LaTeX as itemize handles nesting
        return "    \\item " + TexUtils::escape_latex(project_name) + ": " +
               TexUtils::escape_latex(formatted_duration) + "\n";
    }

    std::string start_children_list() const override {
        return "\\begin{itemize}" + m_itemize_options + "\n";
    }

    std::string end_children_list() const override {
        return "\\end{itemize}\n";
    }

private:
    int m_category_font_size;
    std::string m_itemize_options;
};

} // 匿名命名空间结束

std::string get_tex_preamble(
    const std::string& main_font,
    const std::string& cjk_main_font,
    int font_size,
    double margin_in,
    const std::map<std::string, std::string>& keyword_colors)
{
    std::stringstream ss;
    ss << "\\documentclass[" << font_size << "pt]{extarticle}\n";
    ss << "\\usepackage[a4paper, margin=" << margin_in << "in]{geometry}\n";
    ss << "\\usepackage[dvipsnames]{xcolor}\n";
    ss << "\\usepackage{enumitem}\n";
    ss << "\\usepackage{fontspec}\n";
    ss << "\\usepackage{ctex}\n";
    ss << "\\usepackage{titlesec}\n\n";

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

std::string format_project_tree(
    const ProjectTree& tree,
    long long total_duration,
    int avg_days,
    int category_title_font_size,
    double list_top_sep_pt,
    double list_item_sep_ex
) {
    auto strategy = std::make_unique<LatexFormattingStrategy>(
        category_title_font_size, list_top_sep_pt, list_item_sep_ex);
    reporting::ProjectTreeFormatter formatter(std::move(strategy));
    return formatter.format_project_tree(tree, total_duration, avg_days);
}

} // namespace TexUtils