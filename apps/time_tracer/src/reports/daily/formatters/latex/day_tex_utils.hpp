// reports/daily/formatters/latex/day_tex_utils.hpp
#ifndef REPORTS_DAILY_FORMATTERS_LATEX_DAY_TEX_UTILS_H_
#define REPORTS_DAILY_FORMATTERS_LATEX_DAY_TEX_UTILS_H_

#include <memory>
#include <sstream>

#include "reports/daily/formatters/latex/day_tex_config.hpp"
#include "reports/data/model/daily_report_data.hpp"

namespace DayTexUtils {

void display_header(std::stringstream& report_stream,
                    const DailyReportData& data,
                    const std::shared_ptr<DayTexConfig>& config);

void display_detailed_activities(std::stringstream& report_stream,
                                 const DailyReportData& data,
                                 const std::shared_ptr<DayTexConfig>& config);

}  // namespace DayTexUtils

#endif  // REPORTS_DAILY_FORMATTERS_LATEX_DAY_TEX_UTILS_H_
