// reports/daily/formatters/statistics/LatexStrategy.hpp
#ifndef LATEX_STRATEGY_HPP
#define LATEX_STRATEGY_HPP

#include "IStatStrategy.hpp"
#include "reports/daily/formatters/tex/DayTexConfig.hpp"
#include "reports/shared/formatters/latex/TexUtils.hpp"
#include <format>
#include <vector>
#include <string>

class LatexStrategy : public IStatStrategy {
public:
    explicit LatexStrategy(const std::shared_ptr<DayTexConfig>& config) : config_(config) {}

    /**
     * @brief [核心修正] 修正 std::format 字符串以生成正确的 LaTeX 命令。
     */
    std::string format_header(const std::string& title) const override {
        int category_size = config_->get_category_title_font_size();
        return std::format("{{ \\fontsize{{{}}}{{{}}}\\selectfont \\subsection*{{{}}}}}\n\n",
            category_size, category_size * 1.2, title);
    }

    std::string format_main_item(const std::string& label, const std::string& value) const override {
        return std::format("    \\item \\textbf{{{}}}: {}", TexUtils::escape_latex(label), TexUtils::escape_latex(value));
    }

    std::string format_sub_item(const std::string& label, const std::string& value) const override {
        return std::format("        \\item \\textbf{{{}}}: {}", TexUtils::escape_latex(label), TexUtils::escape_latex(value));
    }

    std::string build_output(const std::vector<std::string>& lines) const override {
        if (lines.empty()) {
            return "";
        }

        std::string compact_list_options = std::format("[topsep={}pt, itemsep={}ex]",
            config_->get_list_top_sep_pt(), config_->get_list_item_sep_ex());
        
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

#endif // LATEX_STRATEGY_HPP