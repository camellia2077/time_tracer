// infra/insights/monthly/formatters/latex/month_tex_utils.cpp
#include "infra/insights/monthly/formatters/latex/month_tex_utils.hpp"

#include "infra/insights/shared/formatters/latex/tex_common_utils.hpp"
#include "infra/insights/shared/formatters/latex/tex_utils.hpp"
#include "infra/insights/shared/utils/format/insights_string_utils.hpp"
#include "infra/insights/shared/utils/format/time_format.hpp"

namespace MonthTexUtils {

namespace {
auto FormatRatio(int count, int total_days) -> std::string {
  return FormatCountWithPercentage(count, total_days, "\\%");
}
}  // namespace

void DisplayHeader(std::string& insights_stream, const MonthlyInsightsData& data,
                   const std::shared_ptr<MonthTexConfig>& config) {
  // 1. 渲染标题
  std::string title = FormatTitleTemplate(config->GetTitleTemplate(), data);
  std::string title_content = TexUtils::EscapeLatex(title);

  TexCommonUtils::RenderTitle(insights_stream, title_content,
                              config->GetInsightsTitleFontSize());

  // 2. 渲染摘要列表
  if (data.actual_days > 0) {
    std::vector<TexCommonUtils::SummaryItem> meta_data = {
        {config->GetActualDaysLabel(), std::to_string(data.actual_days)},
        {config->GetTotalTimeLabel(),
         TexUtils::EscapeLatex(
             TimeFormatDuration(data.total_duration, data.actual_days))},
        {config->GetActivityCountLabel(),
         FormatCountWithAverage(data.matched_record_count,
                                data.requested_days)},
        {FormatBooleanCountLabel(config->GetStatusDaysLabel(),
                                 data.status_true_days),
         FormatRatio(data.status_true_days, data.actual_days)},
        {FormatBooleanCountLabel(config->GetExerciseDaysLabel(),
                                 data.exercise_true_days),
         FormatRatio(data.exercise_true_days, data.actual_days)},
        {FormatBooleanCountLabel(config->GetCardioDaysLabel(),
                                 data.cardio_true_days),
         FormatRatio(data.cardio_true_days, data.actual_days)},
        {FormatBooleanCountLabel(config->GetAnaerobicDaysLabel(),
                                 data.anaerobic_true_days),
         FormatRatio(data.anaerobic_true_days, data.actual_days)}};

    TexCommonUtils::RenderSummaryList(insights_stream, meta_data,
                                      config->GetListTopSepPt(),
                                      config->GetListItemSepEx());
  }
}

}  // namespace MonthTexUtils
