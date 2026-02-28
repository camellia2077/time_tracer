// infrastructure/reports/daily/formatters/statistics/latex_strategy.cpp
#include "infrastructure/reports/daily/formatters/statistics/latex_strategy.hpp"

#include "infrastructure/reports/shared/formatters/latex/tex_common_utils.hpp"
#include "infrastructure/reports/shared/formatters/latex/tex_utils.hpp"

LatexStrategy::LatexStrategy(const std::shared_ptr<DayTexConfig>& config)
    : config_(config) {}

auto LatexStrategy::FormatHeader(const std::string& title) const
    -> std::string {
  constexpr std::size_t kHeaderReservePadding = 64;
  const int kCategorySize = config_->GetCategoryTitleFontSize();
  const int kLineHeightSize = (kCategorySize * 12 + 5) / 10;

  std::string header;
  header.reserve(title.size() + kHeaderReservePadding);
  header += "{ \\fontsize{";
  header += std::to_string(kCategorySize);
  header += "}{";
  header += std::to_string(kLineHeightSize);
  header += "}\\selectfont \\subsection*{";
  header += title;
  header += "}}\n\n";
  return header;
}

auto LatexStrategy::FormatMainItem(const std::string& label,
                                   const std::string& value) const
    -> std::string {
  std::string item = "    \\item \\textbf{";
  item += TexUtils::EscapeLatex(label);
  item += "}: ";
  item += TexUtils::EscapeLatex(value);
  return item;
}

auto LatexStrategy::FormatSubItem(const std::string& label,
                                  const std::string& value) const
    -> std::string {
  std::string item = "        \\item \\textbf{";
  item += TexUtils::EscapeLatex(label);
  item += "}: ";
  item += TexUtils::EscapeLatex(value);
  return item;
}

auto LatexStrategy::BuildOutput(const std::vector<std::string>& lines) const
    -> std::string {
  if (lines.empty()) {
    return "";
  }

  const std::string kCompactListOptions =
      TexCommonUtils::BuildCompactListOptions(config_->GetListTopSepPt(),
                                              config_->GetListItemSepEx());

  std::string result = "\\begin{itemize}" + kCompactListOptions + "\n";
  bool in_sublist = false;

  for (const auto& line : lines) {
    const bool kIsSubItem = line.starts_with("        ");

    if (kIsSubItem && !in_sublist) {
      result += "    \\begin{itemize}" + kCompactListOptions + "\n";
      in_sublist = true;
    } else if (!kIsSubItem && in_sublist) {
      result += "    \\end{itemize}\n";
      in_sublist = false;
    }
    result += line;
    result += "\n";
  }

  if (in_sublist) {
    result += "    \\end{itemize}\n";
  }

  result += "\\end{itemize}\n\n";
  return result;
}
