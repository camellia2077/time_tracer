// reports/period/formatters/tex/PeriodTexUtils.cpp
#include "PeriodTexUtils.hpp"
#include <iomanip>
#include <format>
#include "reports/shared/utils/format/TimeFormat.hpp"
#include "reports/shared/formatters/latex/TexUtils.hpp"

namespace PeriodTexUtils {

void display_summary(std::stringstream& ss, const PeriodReportData& data, const std::shared_ptr<PeriodTexConfig>& config) {
    int title_size = config->get_report_title_font_size();
    ss << "{";
    ss << "\\fontsize{" << title_size << "}{" << title_size * 1.2 << "}\\selectfont";
    ss << "\\section*{"
       << config->get_report_title_prefix() << " " << data.days_to_query << " "
       << config->get_report_title_days() << " ("
       << TexUtils::escape_latex(data.start_date) << " " << config->get_report_title_date_separator() << " "
       << TexUtils::escape_latex(data.end_date) << ")}";
    ss << "}\n\n";

    if (data.actual_days > 0) {
        std::string compact_list_options = std::format("[topsep={}pt, itemsep={}ex]", 
            config->get_list_top_sep_pt(), 
            config->get_list_item_sep_ex()
        );
        ss << "\\begin{itemize}" << compact_list_options << "\n";
        ss << "    \\item \\textbf{" << config->get_total_time_label() << "}: "
           << TexUtils::escape_latex(time_format_duration(data.total_duration, data.actual_days)) << "\n";
        ss << "    \\item \\textbf{" << config->get_actual_days_label() << "}: "
           << data.actual_days << "\n";
        ss << "\\end{itemize}\n\n";
    }
}

} // namespace PeriodTexUtils