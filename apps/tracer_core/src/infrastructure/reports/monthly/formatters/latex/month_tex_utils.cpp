// infrastructure/reports/monthly/formatters/latex/month_tex_utils.cpp
#include "infrastructure/reports/monthly/formatters/latex/month_tex_utils.hpp"

#include "infrastructure/reports/shared/formatters/latex/tex_common_utils.hpp"
#include "infrastructure/reports/shared/formatters/latex/tex_utils.hpp"
#include "infrastructure/reports/shared/utils/format/report_string_utils.hpp"
#include "infrastructure/reports/shared/utils/format/time_format.hpp"

namespace MonthTexUtils {

namespace {
auto FormatRatio(int count, int total_days) -> std::string {
  return FormatCountWithPercentage(count, total_days, "\\%");
}
}  // namespace

void DisplayHeader(std::string& report_stream, const MonthlyReportData& data,
                   const std::shared_ptr<MonthTexConfig>& config) {
  // 1. 渲染标题
  std::string title = FormatTitleTemplate(config->GetTitleTemplate(), data);
  std::string title_content = TexUtils::EscapeLatex(title);

  TexCommonUtils::RenderTitle(report_stream, title_content,
                              config->GetReportTitleFontSize());

  // 2. 渲染摘要列表
  if (data.actual_days > 0) {
    std::vector<TexCommonUtils::SummaryItem> meta_data = {
        {config->GetActualDaysLabel(), std::to_string(data.actual_days)},
        {config->GetTotalTimeLabel(),
         TexUtils::EscapeLatex(
             TimeFormatDuration(data.total_duration, data.actual_days))},
        {config->GetStatusDaysLabel(),
         FormatRatio(data.status_true_days, data.actual_days)},
        {config->GetSleepDaysLabel(),
         FormatRatio(data.sleep_true_days, data.actual_days)},
        {config->GetExerciseDaysLabel(),
         FormatRatio(data.exercise_true_days, data.actual_days)},
        {config->GetCardioDaysLabel(),
         FormatRatio(data.cardio_true_days, data.actual_days)},
        {config->GetAnaerobicDaysLabel(),
         FormatRatio(data.anaerobic_true_days, data.actual_days)}};

    TexCommonUtils::RenderSummaryList(report_stream, meta_data,
                                      config->GetListTopSepPt(),
                                      config->GetListItemSepEx());
  }
}

}  // namespace MonthTexUtils
