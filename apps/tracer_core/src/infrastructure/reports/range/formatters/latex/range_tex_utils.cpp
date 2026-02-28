// infrastructure/reports/range/formatters/latex/range_tex_utils.cpp
#include "infrastructure/reports/range/formatters/latex/range_tex_utils.hpp"

#include <string>
#include <vector>

#include "infrastructure/reports/shared/formatters/latex/tex_common_utils.hpp"
#include "infrastructure/reports/shared/formatters/latex/tex_utils.hpp"
#include "infrastructure/reports/shared/utils/format/report_string_utils.hpp"
#include "infrastructure/reports/shared/utils/format/time_format.hpp"

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
        {config->GetActualDaysLabel(), std::to_string(data.actual_days)},
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

    TexCommonUtils::RenderSummaryList(report_stream, items,
                                      config->GetListTopSepPt(),
                                      config->GetListItemSepEx());
  }
}

}  // namespace RangeTexUtils
