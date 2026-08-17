// infra/insights/range/formatters/latex/range_tex_utils.cpp
#include "infra/insights/range/formatters/latex/range_tex_utils.hpp"

#include <string>
#include <vector>

#include "infra/insights/shared/formatters/latex/tex_common_utils.hpp"
#include "infra/insights/shared/formatters/latex/tex_utils.hpp"
#include "infra/insights/shared/utils/format/insights_string_utils.hpp"
#include "infra/insights/shared/utils/format/status_statistics_format.hpp"
#include "infra/insights/shared/utils/format/time_format.hpp"

namespace RangeTexUtils {

void DisplaySummary(std::string& insights_stream,

                    const RangeInsightsData& data,
                    const std::shared_ptr<RangeTexConfig>& config) {
  std::string title_content =
      TexUtils::EscapeLatex(config->GetSummarySectionLabel());

  TexCommonUtils::RenderTitle(insights_stream, title_content,
                              config->GetInsightsTitleFontSize());

  std::vector<TexCommonUtils::SummaryItem> items = {
      {config->GetPeriodLabel(),
       TexUtils::EscapeLatex(data.start_date + " - " + data.end_date)}};
  if (data.actual_days > 0) {
    items.insert(
        items.end(),
        {{config->GetTotalTimeLabel(),
          TexUtils::EscapeLatex(TimeFormatDuration(
              data.activity.total_duration_seconds, data.actual_days))},
         {config->GetActivityCountLabel(),
          FormatCountWithAverage(data.activity.occurrence_count,
                                 data.requested_days)},
         {config->GetActualDaysLabel(), std::to_string(data.actual_days)}});
  }
  TexCommonUtils::RenderSummaryList(insights_stream, items,
                                    config->GetListTopSepPt(),
                                    config->GetListItemSepEx());
  if (data.actual_days > 0) {
    if (!data.statuses.empty()) {
      TexCommonUtils::RenderTitle(
          insights_stream,
          TexUtils::EscapeLatex(config->GetCustomSectionLabel()),
          config->GetCategoryTitleFontSize(), true);
      std::vector<TexCommonUtils::SummaryItem> statuses;
      statuses.reserve(data.statuses.size());
      for (const auto& status : data.statuses) {
        statuses.emplace_back(
            status.label, TexUtils::EscapeLatex(FormatStatusStatistics(
                              status.occurrence_count, status.total_duration,
                              config->GetStatusCountUnit())));
      }
      TexCommonUtils::RenderSummaryList(insights_stream, statuses,
                                        config->GetListTopSepPt(),
                                        config->GetListItemSepEx());
    }
  }
}

}  // namespace RangeTexUtils
