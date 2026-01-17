// reports/daily/formatters/latex/DayTexUtils.hpp
#ifndef REPORTS_DAILY_FORMATTERS_LATEX_DAY_TEX_UTILS_HPP_
#define REPORTS_DAILY_FORMATTERS_LATEX_DAY_TEX_UTILS_HPP_

#include <sstream>
#include <memory>
#include "reports/shared/model/DailyReportData.hpp"
#include "reports/daily/formatters/latex/DayTexConfig.hpp"

namespace DayTexUtils {

    void display_header(std::stringstream& ss, const DailyReportData& data, const std::shared_ptr<DayTexConfig>& config);
    
    // [核心修改] display_statistics 声明已被移除
    
    void display_detailed_activities(std::stringstream& ss, const DailyReportData& data, const std::shared_ptr<DayTexConfig>& config);

} // namespace DayTexUtils

#endif // REPORTS_DAILY_FORMATTERS_LATEX_DAY_TEX_UTILS_HPP_