// reports/daily/formatters/statistics/latex_strategy.hpp
#ifndef REPORTS_DAILY_FORMATTERS_STATISTICS_LATEX_STRATEGY_H_
#define REPORTS_DAILY_FORMATTERS_STATISTICS_LATEX_STRATEGY_H_

#include <format>
#include <string>
#include <vector>

#include "reports/daily/formatters/latex/day_tex_config.hpp"
#include "reports/daily/formatters/statistics/i_stat_strategy.hpp"
#include "reports/shared/formatters/latex/tex_utils.hpp"

class LatexStrategy : public IStatStrategy {
 public:
  explicit LatexStrategy(const std::shared_ptr<DayTexConfig>& config)
      : config_(config) {}

  /**
   * @brief [核心修正] 修正 std::format 字符串以生成正确的 LaTeX 命令。
   */
  std::string FormatHeader(const std::string& title) const override {
    int category_size = config_->GetCategoryTitleFontSize();
    return std::format(
        "{{ \\fontsize{{{}}}{{{}}}\\selectfont \\subsection*{{{}}}}}\n\n",
        category_size, category_size * 1.2, title);
  }

  std::string FormatMainItem(const std::string& label,
                             const std::string& value) const override {
    return std::format("    \\item \\textbf{{{}}}: {}",
                       TexUtils::EscapeLatex(label),
                       TexUtils::EscapeLatex(value));
  }

  std::string FormatSubItem(const std::string& label,
                            const std::string& value) const override {
    return std::format("        \\item \\textbf{{{}}}: {}",
                       TexUtils::EscapeLatex(label),
                       TexUtils::EscapeLatex(value));
  }

  std::string BuildOutput(
      const std::vector<std::string>& lines) const override {
    if (lines.empty()) {
      return "";
    }

    std::string compact_list_options =
        std::format("[topsep={}pt, itemsep={}ex]", config_->GetListTopSepPt(),
                    config_->GetListItemSepEx());

    std::string result = "\\begin{itemize}" + compact_list_options + "\n";
    bool in_sublist = false;

    for (const auto& line : lines) {
      bool is_sub_item = (line.rfind("        ", 0) == 0);

      if (is_sub_item && !in_sublist) {
        result += "    \\begin{itemize}" + compact_list_options + "\n";
        in_sublist = true;
      } else if (!is_sub_item && in_sublist) {
        result += "    \\end{itemize}\n";
        in_sublist = false;
      }
      result += line + "\n";
    }

    if (in_sublist) {
      result += "    \\end{itemize}\n";
    }

    result += "\\end{itemize}\n\n";
    return result;
  }

 private:
  std::shared_ptr<DayTexConfig> config_;
};

#endif  // REPORTS_DAILY_FORMATTERS_STATISTICS_LATEX_STRATEGY_H_