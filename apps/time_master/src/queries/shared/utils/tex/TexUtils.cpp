// queries/shared/utils/TexUtils.cpp
#include "TexUtils.hpp"
#include <sstream>

namespace TexUtils {

std::string get_tex_preamble(
    const std::string& main_font,
    const std::string& cjk_main_font,
    const std::map<std::string, std::string>& keyword_colors)
{
    std::stringstream ss;
    ss << "\\documentclass{article}\n";
    ss << "\\usepackage[a4paper, margin=1in]{geometry}\n";
    ss << "\\usepackage[dvipsnames]{xcolor}\n";
    ss << "\\usepackage{enumitem}\n";
    ss << "\\usepackage{fontspec}\n";
    ss << "\\usepackage{ctex}\n";

    // 如果提供了颜色定义，则添加它们
    if (!keyword_colors.empty()) {
        for (const auto& pair : keyword_colors) {
            ss << "\\definecolor{" << pair.first << "color}{HTML}{" << pair.second << "}\n";
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

} // namespace TexUtils