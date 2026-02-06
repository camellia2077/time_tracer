// reports/range/formatters/latex/range_tex_utils.cpp
#include "reports/range/formatters/latex/range_tex_utils.hpp"

#include <string>
#include <vector>

#include "reports/shared/formatters/latex/tex_common_utils.hpp"
#include "reports/shared/formatters/latex/tex_utils.hpp"
#include "reports/shared/utils/format/report_string_utils.hpp"
#include "reports/shared/utils/format/time_format.hpp"

namespace RangeTexUtils {

void DisplaySummary(std::stringstream& report_stream,

                     const RangeReportData& data,
                     const std::shared_ptr<RangeTexConfig>& config) {
  std::string title =
      FormatTitleTemplate(config->GetTitleTemplate(), data);
  std::string title_content = TexUtils::EscapeLatex(title);

  TexCommonUtils::RenderTitle(report_stream, title_content,
                               config->GetReportTitleFontSize());

  if (data.actual_days > 0) {
    std::vector<TexCommonUtils::SummaryItem> items = {
        {config->GetTotalTimeLabel(),
         TexUtils::EscapeLatex(
             TimeFormatDuration(data.total_duration, data.actual_days))},
        {config->GetActualDaysLabel(), std::to_string(data.actual_days)}};

    TexCommonUtils::RenderSummaryList(report_stream, items,
                                        config->GetListTopSepPt(),
                                        config->GetListItemSepEx());
  }
}

}  // namespace RangeTexUtils
