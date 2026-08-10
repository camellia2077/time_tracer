// infra/insights/monthly/formatters/latex/month_tex_utils.cpp
#include "infra/insights/monthly/formatters/latex/month_tex_utils.hpp"

#include "infra/insights/shared/formatters/latex/tex_common_utils.hpp"
#include "infra/insights/shared/formatters/latex/tex_utils.hpp"
#include "infra/insights/shared/utils/format/insights_string_utils.hpp"
#include "infra/insights/shared/utils/format/status_statistics_format.hpp"
#include "infra/insights/shared/utils/format/time_format.hpp"

namespace MonthTexUtils {

void DisplayHeader(std::string& insights_stream, const MonthlyInsightsData& data,
                   const std::shared_ptr<MonthTexConfig>& config) {
  // 1. 渲染标题
  std::string title_content =
      TexUtils::EscapeLatex(config->GetSummarySectionLabel());

  TexCommonUtils::RenderTitle(insights_stream, title_content,
                              config->GetInsightsTitleFontSize());

  // 2. 渲染摘要列表
  std::vector<TexCommonUtils::SummaryItem> meta_data = {
      {config->GetPeriodLabel(),
       TexUtils::EscapeLatex(data.start_date + " - " + data.end_date)}};
  if (data.actual_days > 0) {
    meta_data.insert(meta_data.end(), {
        {config->GetActualDaysLabel(), std::to_string(data.actual_days)},
        {config->GetTotalTimeLabel(),
         TexUtils::EscapeLatex(
             TimeFormatDuration(data.total_duration, data.actual_days))},
        {config->GetActivityCountLabel(),
         FormatCountWithAverage(data.matched_record_count,
                                data.requested_days)}});
  }
  TexCommonUtils::RenderSummaryList(insights_stream, meta_data,
                                    config->GetListTopSepPt(),
                                    config->GetListItemSepEx());
  if (data.actual_days > 0) {
    if (!data.statuses.empty()) {
      TexCommonUtils::RenderTitle(
          insights_stream, TexUtils::EscapeLatex(config->GetCustomSectionLabel()),
          config->GetCategoryTitleFontSize(), true);
      std::vector<TexCommonUtils::SummaryItem> statuses;
      statuses.reserve(data.statuses.size());
      for (const auto& status : data.statuses) {
        statuses.emplace_back(status.label, TexUtils::EscapeLatex(
            FormatStatusStatistics(status.occurrence_count, status.total_duration,
                                   config->GetStatusCountUnit())));
      }
      TexCommonUtils::RenderSummaryList(insights_stream, statuses,
                                        config->GetListTopSepPt(),
                                        config->GetListItemSepEx());
    }
  }
}

}  // namespace MonthTexUtils
