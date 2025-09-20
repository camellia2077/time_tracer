// queries/shared/formatters/latex/TexUtils.cpp
#include "TexUtils.hpp"
#include "TexProjectTreeFormatter.hpp" // [新增] 引入新的实现模块
#include <sstream>
#include <string>
#include <map>

namespace TexUtils {

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

// [核心修改] 函数实现被移至 TexProjectTreeFormatter.cpp
// 此处只保留接口，并将调用委托给新的实现函数
std::string format_project_tree(
    const ProjectTree& tree,
    long long total_duration,
    int avg_days,
    int category_title_font_size,
    double list_top_sep_pt,
    double list_item_sep_ex
) {
    return Internal::format_project_tree_impl(
        tree,
        total_duration,
        avg_days,
        category_title_font_size,
        list_top_sep_pt,
        list_item_sep_ex
    );
}

} // namespace TexUtils