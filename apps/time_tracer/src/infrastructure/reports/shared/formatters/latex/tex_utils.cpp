// infrastructure/reports/shared/formatters/latex/tex_utils.cpp
#include "infrastructure/reports/shared/formatters/latex/tex_utils.hpp"

#include <format>
#include <iomanip>
#include <map>
#include <memory>
#include <sstream>
#include <string>

#include "infrastructure/reports/shared/formatters/base/project_tree_formatter.hpp"
#include "infrastructure/reports/shared/utils/format/time_format.hpp"

namespace TexUtils {

namespace {

class LatexFormattingStrategy : public reporting::IFormattingStrategy {
 public:
  // NOLINTBEGIN(bugprone-easily-swappable-parameters)
  LatexFormattingStrategy(int category_font_size, double top_sep,
                          double item_sep)
      : m_category_font_size_(category_font_size),
        m_itemize_options_(
            std::format("[topsep={}pt, itemsep={}ex]", top_sep, item_sep)) {}
  // NOLINTEND(bugprone-easily-swappable-parameters)

  [[nodiscard]] auto FormatCategoryHeader(const std::string& category_name,
                                          const std::string& formatted_duration,
                                          double percentage) const
      -> std::string override {
    std::stringstream output_ss;
    constexpr double kFontSizeMultiplier = 1.2;
    output_ss << "{";
    output_ss << "\\fontsize{" << m_category_font_size_ << "}{"
              << m_category_font_size_ * kFontSizeMultiplier << "}\\selectfont";

    // [核心修改] 将 \section* 改为 \subsection*
    // 这样它们就会成为 "Project Breakdown" 的子章节
    output_ss << "\\subsection*{" << TexUtils::EscapeLatex(category_name)
              << ": " << TexUtils::EscapeLatex(formatted_duration) << " ("
              << std::fixed << std::setprecision(1) << percentage << "\\%)}";

    output_ss << "}\n";
    return output_ss.str();
  }

  [[nodiscard]] auto FormatTreeNode(const std::string& project_name,
                                    const std::string& formatted_duration,
                                    int /*indent_level*/) const
      -> std::string override {
    // indent_level is not used in LaTeX as itemize handles nesting
    return "    \\item " + TexUtils::EscapeLatex(project_name) + ": " +
           TexUtils::EscapeLatex(formatted_duration) + "\n";
  }

  [[nodiscard]] auto StartChildrenList() const -> std::string override {
    return "\\begin{itemize}" + m_itemize_options_ + "\n";
  }

  [[nodiscard]] auto EndChildrenList() const -> std::string override {
    return "\\end{itemize}\n";
  }

 private:
  int m_category_font_size_;
  std::string m_itemize_options_;
};

}  // namespace

// Public API: keep parameter order and naming for ABI compatibility.
// NOLINTBEGIN(bugprone-easily-swappable-parameters)
auto GetTexPreamble(const std::string& main_font,
                    const std::string& cjk_main_font, int font_size,
                    double margin_in,
                    const std::map<std::string, std::string>& keyword_colors)
    -> std::string {
  std::stringstream output_ss;
  output_ss << "\\documentclass[" << font_size << "pt]{extarticle}\n";
  output_ss << "\\usepackage[a4paper, margin=" << margin_in
            << "in]{geometry}\n";
  output_ss << "\\usepackage[dvipsnames]{xcolor}\n";
  output_ss << "\\usepackage{enumitem}\n";
  output_ss << "\\usepackage{fontspec}\n";
  output_ss << "\\usepackage{ctex}\n";
  output_ss << "\\usepackage{titlesec}\n\n";

  output_ss << "\\titleformat{\\section}{\\normalfont\\bfseries}{}{0em}{}\n";
  output_ss
      << "\\titleformat{\\subsection}{\\normalfont\\bfseries}{}{0em}{}\n\n";

  if (!keyword_colors.empty()) {
    for (const auto& pair : keyword_colors) {
      std::string hex_color = pair.second;
      if (!hex_color.empty() && hex_color[0] == '#') {
        hex_color = hex_color.substr(1);
      }
      output_ss << "\\definecolor{" << pair.first << "color}{HTML}{"
                << hex_color << "}\n";
    }
  }

  output_ss << "\n";
  // 添加关于如何使用绝对路径字体的注释，方便用户修改
  output_ss << "% To use an absolute font path, comment out the following two "
               "lines "
               "and uncomment the two after.\n";
  output_ss << "%\\setmainfont[Path=C:/your/font/path/]{MiSansVF.ttf}\n";
  output_ss << "%\\setCJKmainfont[Path=C:/your/font/path/]{MiSansVF.ttf}\n\n";

  output_ss << "\\setmainfont{" << main_font << "}\n";
  output_ss << "\\setCJKmainfont{" << cjk_main_font << "}\n\n";
  output_ss << "\\begin{document}\n\n";
  return output_ss.str();
}
// NOLINTEND(bugprone-easily-swappable-parameters)

auto GetTexPostfix() -> std::string {
  return "\n\\end{document}\n";
}

auto EscapeLatex(const std::string& input) -> std::string {
  std::string output;
  output.reserve(input.size());
  for (char current_char : input) {
    switch (current_char) {
      case '&':
        output += "\\&";
        break;
      case '%':
        output += "\\%";
        break;
      case '$':
        output += "\\$";
        break;
      case '#':
        output += "\\#";
        break;
      case '_':
        output += "\\_";
        break;
      case '{':
        output += "\\{";
        break;
      case '}':
        output += "\\}";
        break;
      case '~':
        output += "\\textasciitilde{}";
        break;
      case '^':
        output += "\\textasciicircum{}";
        break;
      case '\\':
        output += "\\textbackslash{}";
        break;
      default:
        output += current_char;
        break;
    }
  }
  return output;
}

// Public API: keep parameter order and naming for ABI compatibility.
// NOLINTBEGIN(bugprone-easily-swappable-parameters)
auto FormatProjectTree(
    const reporting::ProjectTree& tree,  // [修改] 加上 reporting:: 命名空间
    long long total_duration, int avg_days, int category_title_font_size,
    double list_top_sep_pt, double list_item_sep_ex) -> std::string {
  auto strategy = std::make_unique<LatexFormattingStrategy>(
      category_title_font_size, list_top_sep_pt, list_item_sep_ex);
  reporting::ProjectTreeFormatter formatter(std::move(strategy));
  return formatter.FormatProjectTree(tree, total_duration, avg_days);
}
// NOLINTEND(bugprone-easily-swappable-parameters)

}  // namespace TexUtils
