// reports/shared/formatters/latex/tex_common_utils.cpp
#include "tex_common_utils.hpp"
#include <format>

namespace TexCommonUtils {

void render_title(std::stringstream& ss, const std::string& content, int font_size, bool is_subsection) {
    ss << "{";
    ss << "\\fontsize{" << font_size << "}{" << font_size * 1.2 << "}\\selectfont";
    ss << (is_subsection ? "\\subsection*{" : "\\section*{") << content << "}";
    ss << "}\n\n";
}

void render_summary_list(std::stringstream& ss, const std::vector<SummaryItem>& items, double top_sep_pt, double item_sep_ex) {
    if (items.empty()) {
        return;
    }

    std::string compact_list_options = std::format("[topsep={}pt, itemsep={}ex]", 
        top_sep_pt, 
        item_sep_ex
    );

    ss << "\\begin{itemize}" << compact_list_options << "\n";
    for (const auto& item : items) {
        ss << "    \\item \\textbf{" << item.label << "}: " << item.value << "\n";
    }
    ss << "\\end{itemize}\n\n";
}

} // namespace TexCommonUtils