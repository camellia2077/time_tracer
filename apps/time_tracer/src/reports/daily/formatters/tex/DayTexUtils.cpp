// reports/daily/formatters/tex/DayTexUtils.cpp
#include "DayTexUtils.hpp"
#include <iomanip>
#include <vector>
#include <format>
#include "reports/shared/utils/format/BoolToString.hpp"
#include "reports/shared/utils/format/TimeFormat.hpp"
#include "reports/shared/utils/format/ReportStringUtils.hpp"
#include "reports/shared/formatters/latex/TexUtils.hpp"

namespace DayTexUtils {

void display_header(std::stringstream& ss, const DailyReportData& data, const std::shared_ptr<DayTexConfig>& config) {
    int title_size = config->get_report_title_font_size();
    ss << "{";
    ss << "\\fontsize{" << title_size << "}{" << title_size * 1.2 << "}\\selectfont";
    ss << "\\section*{" << config->get_report_title() << " " << TexUtils::escape_latex(data.date) << "}";
    ss << "}\n\n";

    std::string compact_list_options = std::format("[topsep={}pt, itemsep={}ex]",
        config->get_list_top_sep_pt(),
        config->get_list_item_sep_ex()
    );

    ss << "\\begin{itemize}" << compact_list_options << "\n";
    ss << "    \\item \\textbf{" << config->get_date_label()      << "}: " << TexUtils::escape_latex(data.date) << "\n";
    ss << "    \\item \\textbf{" << config->get_total_time_label() << "}: " << TexUtils::escape_latex(time_format_duration(data.total_duration)) << "\n";
    ss << "    \\item \\textbf{" << config->get_status_label()    << "}: " << TexUtils::escape_latex(bool_to_string(data.metadata.status)) << "\n";
    ss << "    \\item \\textbf{" << config->get_sleep_label()     << "}: " << TexUtils::escape_latex(bool_to_string(data.metadata.sleep)) << "\n";
    ss << "    \\item \\textbf{" << config->get_exercise_label()  << "}: " << TexUtils::escape_latex(bool_to_string(data.metadata.exercise)) << "\n";
    ss << "    \\item \\textbf{" << config->get_getup_time_label() << "}: " << TexUtils::escape_latex(data.metadata.getup_time) << "\n";
    ss << "    \\item \\textbf{" << config->get_remark_label()    << "}: " << TexUtils::escape_latex(data.metadata.remark) << "\n";
    ss << "\\end{itemize}\n\n";
}

void display_detailed_activities(std::stringstream& ss, const DailyReportData& data, const std::shared_ptr<DayTexConfig>& config) {
    if (data.detailed_records.empty()) {
        return;
    }

    int category_size = config->get_category_title_font_size();
    ss << "{";
    ss << "\\fontsize{" << category_size << "}{" << category_size * 1.2 << "}\\selectfont";
    ss << "\\subsection*{" << config->get_all_activities_label() << "}";
    ss << "}\n\n";

    std::string compact_list_options = std::format("[topsep={}pt, itemsep={}ex]",
        config->get_list_top_sep_pt(),
        config->get_list_item_sep_ex()
    );
    ss << "\\begin{itemize}" << compact_list_options << "\n";

    for (const auto& record : data.detailed_records) {
        std::string project_path = replace_all(record.project_path, "_", config->get_activity_connector());
        std::string base_string = TexUtils::escape_latex(record.start_time) + " - " +
                                  TexUtils::escape_latex(record.end_time) + " (" +
                                  TexUtils::escape_latex(time_format_duration(record.duration_seconds)) +
                                  "): " + TexUtils::escape_latex(project_path);

        std::string colorized_string = base_string;

        for (const auto& pair : config->get_keyword_colors()) {
            if (record.project_path.find(pair.first) != std::string::npos) {
                colorized_string = "\\textcolor{" + pair.first + "color}{" + base_string + "}";
                break;
            }
        }

        ss << "    \\item " << colorized_string << "\n";

        if (record.activityRemark.has_value()) {
            ss << "    \\begin{itemize}" << compact_list_options << "\n";
            ss << "        \\item \\textbf{" << config->get_activity_remark_label() << "}: "
               << TexUtils::escape_latex(record.activityRemark.value()) << "\n";
            ss << "    \\end{itemize}\n";
        }
    }
    ss << "\\end{itemize}\n\n";
}

} // namespace DayTexUtils