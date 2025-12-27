// reports/daily/formatters/tex/DayTexUtils.hpp
#ifndef DAY_TEX_UTILS_HPP
#define DAY_TEX_UTILS_HPP

#include <sstream>
#include <memory>
#include "reports/shared/data/DailyReportData.hpp"
#include "reports/daily/formatters/tex/DayTexConfig.hpp"

namespace DayTexUtils {

    void display_header(std::stringstream& ss, const DailyReportData& data, const std::shared_ptr<DayTexConfig>& config);
    
    // [核心修改] display_statistics 声明已被移除
    
    void display_detailed_activities(std::stringstream& ss, const DailyReportData& data, const std::shared_ptr<DayTexConfig>& config);

} // namespace DayTexUtils

#endif // DAY_TEX_UTILS_HPP