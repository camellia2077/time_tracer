// infrastructure/reports/shared/formatters/latex/tex_utils.cpp
#include "infrastructure/reports/shared/formatters/latex/tex_utils.hpp"

#include <map>
#include <memory>
#include <string>

#include "infrastructure/reports/shared/formatters/base/project_tree_formatter.hpp"
#include "infrastructure/reports/shared/formatters/latex/tex_common_utils.hpp"
#include "infrastructure/reports/shared/utils/format/time_format.hpp"

namespace TexUtils {

namespace {
constexpr int kDecimalBase = 10;
constexpr std::size_t kDecimalOutputReserve = 24;
constexpr std::size_t kCategoryHeaderReservePadding = 64;
constexpr std::size_t kTexPreambleReservePadding = 512;

auto FormatCompactNumber(double value) -> std::string {
  std::string output = std::to_string(value);
  while (!output.empty() && output.back() == '0') {
    output.pop_back();
  }
  if (!output.empty() && output.back() == '.') {
    output.pop_back();
  }
  if ((output == "-0") || output.empty()) {
    return "0";
  }
  return output;
}

auto FormatOneDecimal(double value) -> std::string {
  const auto kScaled = static_cast<long long>(
      (value >= 0.0) ? ((value * 10.0) + 0.5) : ((value * 10.0) - 0.5));
  long long abs_scaled = (kScaled < 0) ? -kScaled : kScaled;
  const auto kWholePart = abs_scaled / kDecimalBase;
  const auto kFractionalPart = abs_scaled % kDecimalBase;

  std::string output;
  output.reserve(kDecimalOutputReserve);
  if (kScaled < 0) {
    output.push_back('-');
  }
  output += std::to_string(kWholePart);
  output.push_back('.');
  output += std::to_string(kFractionalPart);
  return output;
}

class LatexFormattingStrategy : public reporting::IFormattingStrategy {
 public:
  // NOLINTBEGIN(bugprone-easily-swappable-parameters)
  LatexFormattingStrategy(int category_font_size, double top_sep,
                          double item_sep)
      : m_category_font_size_(category_font_size),
        m_itemize_options_(
            TexCommonUtils::BuildCompactListOptions(top_sep, item_sep)) {}
  // NOLINTEND(bugprone-easily-swappable-parameters)

  [[nodiscard]] auto FormatCategoryHeader(const std::string& category_name,
                                          const std::string& formatted_duration,
                                          double percentage) const
      -> std::string override {
    constexpr int kLineHeightTenthsMultiplier = 12;
    constexpr int kLineHeightTenthsDivisor = 10;
    const int kLineHeightTenths =
        m_category_font_size_ * kLineHeightTenthsMultiplier;

    std::string output;
    output.reserve(category_name.size() + formatted_duration.size() +
                   kCategoryHeaderReservePadding);
    output += "{";
    output += "\\fontsize{";
    output += std::to_string(m_category_font_size_);
    output += "}{";
    output += std::to_string(kLineHeightTenths / kLineHeightTenthsDivisor);
    output += ".";
    output += std::to_string(kLineHeightTenths % kLineHeightTenthsDivisor);
    output += "}\\selectfont";

    // [核心修改] 将 \section* 改为 \subsection*
    // 这样它们就会成为 "Project Breakdown" 的子章节
    output += "\\subsection*{";
    output += TexUtils::EscapeLatex(category_name);
    output += ": ";
    output += TexUtils::EscapeLatex(formatted_duration);
    output += " (";
    output += FormatOneDecimal(percentage);
    output += "\\%)}";

    output += "}\n";
    return output;
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
  std::string output;
  output.reserve(main_font.size() + cjk_main_font.size() +
                 kTexPreambleReservePadding);
  output += "\\documentclass[";
  output += std::to_string(font_size);
  output += "pt]{extarticle}\n";
  output += "\\usepackage[a4paper, margin=";
  output += FormatCompactNumber(margin_in);
  output += "in]{geometry}\n";
  output += "\\usepackage[dvipsnames]{xcolor}\n";
  output += "\\usepackage{enumitem}\n";
  output += "\\usepackage{fontspec}\n";
  output += "\\usepackage{ctex}\n";
  output += "\\usepackage{titlesec}\n\n";

  output += "\\titleformat{\\section}{\\normalfont\\bfseries}{}{0em}{}\n";
  output += "\\titleformat{\\subsection}{\\normalfont\\bfseries}{}{0em}{}\n\n";

  if (!keyword_colors.empty()) {
    for (const auto& pair : keyword_colors) {
      std::string hex_color = pair.second;
      if (!hex_color.empty() && hex_color[0] == '#') {
        hex_color = hex_color.substr(1);
      }
      output += "\\definecolor{";
      output += pair.first;
      output += "color}{HTML}{";
      output += hex_color;
      output += "}\n";
    }
  }

  output += "\n";
  // 添加关于如何使用绝对路径字体的注释，方便用户修改
  output +=
      "% To use an absolute font path, comment out the following two "
      "lines and uncomment the two after.\n";
  output += "%\\setmainfont[Path=C:/your/font/path/]{MiSansVF.ttf}\n";
  output += "%\\setCJKmainfont[Path=C:/your/font/path/]{MiSansVF.ttf}\n\n";

  output += "\\setmainfont{";
  output += main_font;
  output += "}\n";
  output += "\\setCJKmainfont{";
  output += cjk_main_font;
  output += "}\n\n";
  output += "\\begin{document}\n\n";
  return output;
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

auto FormatProjectTree(const TtProjectTreeNodeV1* nodes, uint32_t node_count,
                       long long total_duration, int avg_days,
                       int category_title_font_size, double list_top_sep_pt,
                       double list_item_sep_ex) -> std::string {
  auto strategy = std::make_unique<LatexFormattingStrategy>(
      category_title_font_size, list_top_sep_pt, list_item_sep_ex);
  reporting::ProjectTreeFormatter formatter(std::move(strategy));
  return formatter.FormatProjectTree(nodes, node_count, total_duration,
                                     avg_days);
}
// NOLINTEND(bugprone-easily-swappable-parameters)

}  // namespace TexUtils
