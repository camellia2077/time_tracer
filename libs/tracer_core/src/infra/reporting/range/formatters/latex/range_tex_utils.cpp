// infra/reporting/range/formatters/latex/range_tex_utils.cpp
#include "infra/reporting/range/formatters/latex/range_tex_utils.hpp"

#include <string>
#include <vector>

#include "infra/reporting/shared/formatters/latex/tex_common_utils.hpp"
#include "infra/reporting/shared/formatters/latex/tex_utils.hpp"
#include "infra/reporting/shared/utils/format/report_string_utils.hpp"
#include "infra/reporting/shared/utils/format/time_format.hpp"

namespace RangeTexUtils {

namespace {
auto FormatRatio(int count, int total_days) -> std::string {
  return FormatCountWithPercentage(count, total_days, "\\%");
}
}  // namespace

void DisplaySummary(std::string& report_stream,

                    const RangeReportData& data,
                    const std::shared_ptr<RangeTexConfig>& config) {
  std::string title = FormatTitleTemplate(config->GetTitleTemplate(), data);
  std::string title_content = TexUtils::EscapeLatex(title);

  TexCommonUtils::RenderTitle(report_stream, title_content,
                              config->GetReportTitleFontSize());

  if (data.actual_days > 0) {
    std::vector<TexCommonUtils::SummaryItem> items = {
      {config->GetTotalTimeLabel(),
       TexUtils::EscapeLatex(
           TimeFormatDuration(data.total_duration, data.actual_days))},
      {config->GetActivityCountLabel(),
       FormatCountWithAverage(data.matched_record_count, data.requested_days)},
        {config->GetActualDaysLabel(), std::to_string(data.actual_days)},
        {FormatBooleanCountLabel(config->GetStatusDaysLabel(),
                                 data.status_true_days),
         FormatRatio(data.status_true_days, data.actual_days)},
        {FormatBooleanCountLabel(config->GetWakeAnchorDaysLabel(),
                                 data.wake_anchor_days),
         FormatRatio(data.wake_anchor_days, data.actual_days)},
        {FormatBooleanCountLabel(config->GetExerciseDaysLabel(),
                                 data.exercise_true_days),
         FormatRatio(data.exercise_true_days, data.actual_days)},
        {FormatBooleanCountLabel(config->GetCardioDaysLabel(),
                                 data.cardio_true_days),
         FormatRatio(data.cardio_true_days, data.actual_days)},
        {FormatBooleanCountLabel(config->GetAnaerobicDaysLabel(),
                                 data.anaerobic_true_days),
         FormatRatio(data.anaerobic_true_days, data.actual_days)}};

    TexCommonUtils::RenderSummaryList(report_stream, items,
                                      config->GetListTopSepPt(),
                                      config->GetListItemSepEx());
  }
}

}  // namespace RangeTexUtils
