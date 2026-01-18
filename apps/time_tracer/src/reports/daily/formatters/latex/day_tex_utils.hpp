// reports/daily/formatters/latex/day_tex_utils.hpp
#ifndef REPORTS_DAILY_FORMATTERS_LATEX_DAY_TEX_UTILS_HPP_
#define REPORTS_DAILY_FORMATTERS_LATEX_DAY_TEX_UTILS_HPP_

#include <sstream>
#include <memory>
#include "reports/shared/model/daily_report_data.hpp"
#include "reports/daily/formatters/latex/day_tex_config.hpp"

namespace DayTexUtils {

    void display_header(std::stringstream& ss, const DailyReportData& data, const std::shared_ptr<DayTexConfig>& config);
    
    void display_detailed_activities(std::stringstream& ss, const DailyReportData& data, const std::shared_ptr<DayTexConfig>& config);

} // namespace DayTexUtils

#endif // REPORTS_DAILY_FORMATTERS_LATEX_DAY_TEX_UTILS_HPP_