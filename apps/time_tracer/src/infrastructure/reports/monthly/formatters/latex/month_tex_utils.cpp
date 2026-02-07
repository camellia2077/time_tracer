// infrastructure/reports/monthly/formatters/latex/month_tex_utils.cpp
#include "infrastructure/reports/monthly/formatters/latex/month_tex_utils.hpp"

#include <format>
#include <iomanip>

#include "infrastructure/reports/shared/formatters/latex/tex_common_utils.hpp"
#include "infrastructure/reports/shared/formatters/latex/tex_utils.hpp"
#include "infrastructure/reports/shared/utils/format/time_format.hpp"

namespace MonthTexUtils {

void DisplayHeader(std::stringstream& report_stream,
                   const MonthlyReportData& data,
                   const std::shared_ptr<MonthTexConfig>& config) {
  // 1. 渲染标题
  std::string title_content =
      config->GetReportTitle() + " " + TexUtils::EscapeLatex(data.range_label);

  TexCommonUtils::RenderTitle(report_stream, title_content,
                              config->GetReportTitleFontSize());

  // 2. 渲染摘要列表
  if (data.actual_days > 0) {
    std::vector<TexCommonUtils::SummaryItem> meta_data = {
        {config->GetActualDaysLabel(), std::to_string(data.actual_days)},
        {config->GetTotalTimeLabel(),
         TexUtils::EscapeLatex(
             TimeFormatDuration(data.total_duration, data.actual_days))}};

    TexCommonUtils::RenderSummaryList(report_stream, meta_data,
                                      config->GetListTopSepPt(),
                                      config->GetListItemSepEx());
  }
}

}  // namespace MonthTexUtils
