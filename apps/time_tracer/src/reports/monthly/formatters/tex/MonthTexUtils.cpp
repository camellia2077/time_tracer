// reports/monthly/formatters/tex/MonthTexUtils.cpp
#include "MonthTexUtils.hpp"
#include <iomanip>
#include <format>
#include "reports/shared/utils/format/TimeFormat.hpp"
#include "reports/shared/formatters/latex/TexUtils.hpp"

namespace MonthTexUtils {

void display_summary(std::stringstream& ss, const MonthlyReportData& data, const std::shared_ptr<MonthTexConfig>& config) {
    // [修正] 直接使用
    std::string title_month = data.year_month;
    
    int title_size = config->get_report_title_font_size();
    ss << "{";
    ss << "\\fontsize{" << title_size << "}{" << title_size * 1.2 << "}\\selectfont";
    // 记得转义，虽然日期里通常没有特殊字符，但保持习惯一致性
    ss << "\\section*{" << config->get_report_title() << " " << TexUtils::escape_latex(title_month) << "}";
    ss << "}\n\n";

    if (data.actual_days > 0) {
        std::string compact_list_options = std::format("[topsep={}pt, itemsep={}ex]", 
            config->get_list_top_sep_pt(), 
            config->get_list_item_sep_ex()
        );
        ss << "\\begin{itemize}" << compact_list_options << "\n";
        ss << "    \\item \\textbf{" << config->get_actual_days_label() << "}: " << data.actual_days << "\n";
        ss << "    \\item \\textbf{" << config->get_total_time_label()  << "}: " << TexUtils::escape_latex(time_format_duration(data.total_duration, data.actual_days)) << "\n";
        ss << "\\end{itemize}\n\n";
    }
}

} // namespace MonthTexUtils