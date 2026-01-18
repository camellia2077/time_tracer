// reports/daily/formatters/latex/day_tex_utils.cpp
#include "day_tex_utils.hpp"
#include <iomanip>
#include <vector>
#include <format>
#include "reports/shared/utils/format/bool_to_string.hpp"
#include "reports/shared/utils/format/time_format.hpp"
#include "reports/shared/utils/format/report_string_utils.hpp"
#include "reports/shared/formatters/latex/tex_utils.hpp"
#include "reports/shared/formatters/latex/tex_common_utils.hpp"

namespace DayTexUtils {

void display_header(std::stringstream& ss, const DailyReportData& data, const std::shared_ptr<DayTexConfig>& config) {
    // 1. 渲染标题
    std::string title_content = config->get_report_title() + " " + TexUtils::escape_latex(data.date);
    TexCommonUtils::render_title(ss, title_content, config->get_report_title_font_size());

    // 2. 准备列表数据
    // 处理多行备注：先转义，再添加 LaTeX 换行符 (\\)
    std::string safe_remark = TexUtils::escape_latex(data.metadata.remark);
    std::string formatted_remark = format_multiline_for_list(safe_remark, 0, "\\\\");

    std::vector<TexCommonUtils::SummaryItem> items = {
        {config->get_date_label(),       TexUtils::escape_latex(data.date)},
        {config->get_total_time_label(), TexUtils::escape_latex(time_format_duration(data.total_duration))},
        {config->get_status_label(),     TexUtils::escape_latex(bool_to_string(data.metadata.status))},
        {config->get_sleep_label(),      TexUtils::escape_latex(bool_to_string(data.metadata.sleep))},
        {config->get_exercise_label(),   TexUtils::escape_latex(bool_to_string(data.metadata.exercise))},
        {config->get_getup_time_label(), TexUtils::escape_latex(data.metadata.getup_time)},
        {config->get_remark_label(),     formatted_remark}
    };

    // 3. 渲染列表
    TexCommonUtils::render_summary_list(ss, items, config->get_list_top_sep_pt(), config->get_list_item_sep_ex());
}

void display_detailed_activities(std::stringstream& ss, const DailyReportData& data, const std::shared_ptr<DayTexConfig>& config) {
    if (data.detailed_records.empty()) {
        return;
    }

    // [优化] 使用通用函数渲染子标题
    TexCommonUtils::render_title(
        ss, 
        config->get_all_activities_label(), 
        config->get_category_title_font_size(), 
        true // is_subsection
    );

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
            
            std::string safe_activity_remark = TexUtils::escape_latex(record.activityRemark.value());
            std::string formatted_activity_remark = format_multiline_for_list(safe_activity_remark, 0, "\\\\");

            ss << "        \\item \\textbf{" << config->get_activity_remark_label() << "}: "
               << formatted_activity_remark << "\n";
            ss << "    \\end{itemize}\n";
        }
    }
    ss << "\\end{itemize}\n\n";
}

} // namespace DayTexUtils