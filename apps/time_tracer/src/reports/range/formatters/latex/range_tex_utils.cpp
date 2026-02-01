// reports/range/formatters/latex/range_tex_utils.cpp
#include "range_tex_utils.hpp"

#include <string>
#include <vector>

#include "reports/shared/formatters/latex/tex_common_utils.hpp"
#include "reports/shared/formatters/latex/tex_utils.hpp"
#include "reports/shared/utils/format/report_string_utils.hpp"
#include "reports/shared/utils/format/time_format.hpp"

namespace RangeTexUtils {

void display_summary(std::stringstream& report_stream,
                     const RangeReportData& data,
                     const std::shared_ptr<RangeTexConfig>& config) {
  std::string title =
      format_title_template(config->get_title_template(), data);
  std::string title_content = TexUtils::escape_latex(title);

  TexCommonUtils::render_title(report_stream, title_content,
                               config->get_report_title_font_size());

  if (data.actual_days > 0) {
    std::vector<TexCommonUtils::SummaryItem> items = {
        {config->get_total_time_label(),
         TexUtils::escape_latex(
             time_format_duration(data.total_duration, data.actual_days))},
        {config->get_actual_days_label(), std::to_string(data.actual_days)}};

    TexCommonUtils::render_summary_list(report_stream, items,
                                        config->get_list_top_sep_pt(),
                                        config->get_list_item_sep_ex());
  }
}

}  // namespace RangeTexUtils
