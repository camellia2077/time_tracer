// reports/shared/formatters/latex/tex_common_utils.cpp
#include "reports/shared/formatters/latex/tex_common_utils.hpp"

#include <format>
#include <map>  // Added for std::map

namespace TexCommonUtils {

void RenderTitle(std::stringstream& output_ss, const std::string& content,
                   int font_size, bool is_subsection) {
  constexpr double kFontSizeMultiplier = 1.2;
  output_ss << "{";
  output_ss << "\\fontsize{" << font_size << "}{"
            << static_cast<double>(font_size) * kFontSizeMultiplier
            << "}\\selectfont";
  output_ss << (is_subsection ? "\\subsection*{" : "\\section*{") << content
            << "}";
  output_ss << "}\n\n";
}

void RenderSummaryList(std::stringstream& output_ss,
                         const std::vector<SummaryItem>& items,
                         double top_sep_pt, double item_sep_ex) {
  if (items.empty()) {
    return;
  }

  std::string compact_list_options =
      std::format("[topsep={}pt, itemsep={}ex]", top_sep_pt, item_sep_ex);

  output_ss << "\\begin{itemize}" << compact_list_options << "\n";
  for (const auto& item : items) {
    output_ss << "    \\item \\textbf{" << item.label << "}: " << item.value
              << "\n";
  }
  output_ss << "\\end{itemize}\n\n";
}

}  // namespace TexCommonUtils