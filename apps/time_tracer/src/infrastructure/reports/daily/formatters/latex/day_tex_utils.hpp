// infrastructure/reports/daily/formatters/latex/day_tex_utils.hpp
#ifndef REPORTS_DAILY_FORMATTERS_LATEX_DAY_TEX_UTILS_H_
#define REPORTS_DAILY_FORMATTERS_LATEX_DAY_TEX_UTILS_H_

#include <memory>
#include <string>

#include "domain/reports/models/daily_report_data.hpp"
#include "infrastructure/reports/daily/formatters/latex/day_tex_config.hpp"

namespace DayTexUtils {

void DisplayHeader(std::string& report_stream, const DailyReportData& data,
                   const std::shared_ptr<DayTexConfig>& config);

void DisplayDetailedActivities(std::string& report_stream,
                               const DailyReportData& data,
                               const std::shared_ptr<DayTexConfig>& config);

}  // namespace DayTexUtils

#endif  // REPORTS_DAILY_FORMATTERS_LATEX_DAY_TEX_UTILS_H_
