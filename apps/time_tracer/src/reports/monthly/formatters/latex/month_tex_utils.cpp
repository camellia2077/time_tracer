// reports/monthly/formatters/latex/month_tex_utils.cpp
#include "month_tex_utils.hpp"
#include <iomanip>
#include <format>
#include "reports/shared/utils/format/time_format.hpp"
#include "reports/shared/formatters/latex/tex_utils.hpp"
#include "reports/shared/formatters/latex/tex_common_utils.hpp"

namespace MonthTexUtils {

void display_summary(std::stringstream& ss, const MonthlyReportData& data, const std::shared_ptr<MonthTexConfig>& config) {
    // 1. 渲染标题
    std::string title_month = data.year_month;
    std::string title_content = config->get_report_title() + " " + TexUtils::escape_latex(title_month);
    
    TexCommonUtils::render_title(ss, title_content, config->get_report_title_font_size());

    // 2. 渲染摘要列表
    if (data.actual_days > 0) {
        std::vector<TexCommonUtils::SummaryItem> items = {
            {config->get_actual_days_label(), std::to_string(data.actual_days)},
            {config->get_total_time_label(),  TexUtils::escape_latex(time_format_duration(data.total_duration, data.actual_days))}
        };

        TexCommonUtils::render_summary_list(
            ss, 
            items, 
            config->get_list_top_sep_pt(), 
            config->get_list_item_sep_ex()
        );
    }
}

} // namespace MonthTexUtils