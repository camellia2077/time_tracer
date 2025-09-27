// reports/daily/formatters/typ/DayTypUtils.hpp
#ifndef DAY_TYP_UTILS_HPP
#define DAY_TYP_UTILS_HPP

#include <sstream>
#include <memory>
#include "reports/shared/data/DailyReportData.hpp"
#include "reports/daily/formatters/typ/DayTypConfig.hpp"
// 声明了所有用于格式化报告不同部分的辅助函数

namespace DayTypUtils {

    /**
     * @brief 显示报告的头部信息。
     */
    void display_header(std::stringstream& ss, const DailyReportData& data, const std::shared_ptr<DayTypConfig>& config);



    /**
     * @brief 显示详细的活动记录。
     */
    void display_detailed_activities(std::stringstream& ss, const DailyReportData& data, const std::shared_ptr<DayTypConfig>& config);

} // namespace DayTypUtils

#endif // DAY_TYP_UTILS_HPP