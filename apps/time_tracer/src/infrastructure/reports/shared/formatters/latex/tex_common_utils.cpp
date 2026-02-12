// infrastructure/reports/shared/formatters/latex/tex_common_utils.cpp
#include "infrastructure/reports/shared/formatters/latex/tex_common_utils.hpp"

#include <string>

namespace TexCommonUtils {

namespace {
constexpr int kLineHeightScale = 10;

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

}  // namespace

auto BuildCompactListOptions(double top_sep_pt, double item_sep_ex)
    -> std::string {
  std::string options = "[topsep=";
  options += FormatCompactNumber(top_sep_pt);
  options += "pt, itemsep=";
  options += FormatCompactNumber(item_sep_ex);
  options += "ex]";
  return options;
}

void RenderTitle(std::string& output_ss, const std::string& content,
                 int font_size, bool is_subsection) {
  const int kLineHeightTenths = font_size * 12;

  output_ss += "{";
  output_ss += "\\fontsize{";
  output_ss += std::to_string(font_size);
  output_ss += "}{";
  output_ss += std::to_string(kLineHeightTenths / kLineHeightScale);
  output_ss += ".";
  output_ss += std::to_string(kLineHeightTenths % kLineHeightScale);
  output_ss += "}\\selectfont";
  output_ss += is_subsection ? "\\subsection*{" : "\\section*{";
  output_ss += content;
  output_ss += "}";
  output_ss += "}\n\n";
}

void RenderSummaryList(std::string& output_ss,
                       const std::vector<SummaryItem>& items, double top_sep_pt,
                       double item_sep_ex) {
  if (items.empty()) {
    return;
  }

  std::string compact_list_options =
      BuildCompactListOptions(top_sep_pt, item_sep_ex);

  output_ss += "\\begin{itemize}";
  output_ss += compact_list_options;
  output_ss += "\n";
  for (const auto& item : items) {
    output_ss += "    \\item \\textbf{";
    output_ss += item.label;
    output_ss += "}: ";
    output_ss += item.value;
    output_ss += "\n";
  }
  output_ss += "\\end{itemize}\n\n";
}

}  // namespace TexCommonUtils
