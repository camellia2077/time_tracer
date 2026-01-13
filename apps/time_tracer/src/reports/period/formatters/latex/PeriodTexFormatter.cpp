// reports/period/formatters/latex/PeriodTexUtils.cpp
#include "PeriodTexUtils.hpp"
#include <iomanip>
#include <format>
#include <vector>
#include "reports/shared/utils/format/TimeFormat.hpp"
#include "reports/shared/formatters/latex/TexUtils.hpp"
#include "reports/shared/formatters/latex/TexCommonUtils.hpp" // [新增] 引入通用工具

namespace PeriodTexUtils {

void display_summary(std::stringstream& ss, const PeriodReportData& data, const std::shared_ptr<PeriodTexConfig>& config) {
    // 1. 构建标题内容
    // 格式示例: "Report for last 30 days (2023-01-01 - 2023-01-31)"
    std::string title_content = std::format("{} {} {} ({} {} {})",
       config->get_report_title_prefix(),
       data.days_to_query,
       config->get_report_title_days(),
       TexUtils::escape_latex(data.start_date),
       config->get_report_title_date_separator(),
       TexUtils::escape_latex(data.end_date)
    );
    
    // 使用通用工具渲染标题
    TexCommonUtils::render_title(ss, title_content, config->get_report_title_font_size());

    // 2. 渲染摘要列表
    if (data.actual_days > 0) {
        std::vector<TexCommonUtils::SummaryItem> items = {
            // 注意保持原有的顺序：先 Total Time，后 Actual Days
            {config->get_total_time_label(), TexUtils::escape_latex(time_format_duration(data.total_duration, data.actual_days))},
            {config->get_actual_days_label(), std::to_string(data.actual_days)}
        };

        // 使用通用工具渲染列表
        TexCommonUtils::render_summary_list(
            ss, 
            items, 
            config->get_list_top_sep_pt(), 
            config->get_list_item_sep_ex()
        );
    }
}

} // namespace PeriodTexUtils