// reports/monthly/formatters/latex/month_tex_utils.cpp
#include "reports/monthly/formatters/latex/month_tex_utils.hpp"
#include <iomanip>
#include <format>
#include "reports/shared/utils/format/time_format.hpp"
#include "reports/shared/formatters/latex/tex_utils.hpp"
#include "reports/shared/formatters/latex/tex_common_utils.hpp"
#include "reports/shared/utils/date_utils.hpp" // Added for date_utils::get_month_string

namespace MonthTexUtils {

void DisplaySummary(std::stringstream& report_stream, const MonthlyReportData& data, const std::shared_ptr<MonthTexConfig>& config) {
    // 1. 渲染标题
    std::string title_month = date_utils::get_month_string(data.year, data.month);

    std::string title_content = config->GetReportTitle() + " " + TexUtils::EscapeLatex(title_month);
    
    TexCommonUtils::RenderTitle(report_stream, title_content, config->GetReportTitleFontSize());

    // 2. 渲染摘要列表
    if (data.actual_days > 0) {
        std::vector<TexCommonUtils::SummaryItem> meta_data = {
            {config->GetActualDaysLabel(), std::to_string(data.actual_days)},
            {config->GetTotalTimeLabel(),  TexUtils::EscapeLatex(TimeFormatDuration(data.total_duration, data.actual_days))}
        };

        TexCommonUtils::RenderSummaryList(
            report_stream, 
            meta_data, 
            config->GetListTopSepPt(), 
            config->GetListItemSepEx()
        );
    }
}

} // namespace MonthTexUtils