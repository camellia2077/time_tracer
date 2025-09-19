// queries/daily/formatters/typ/DayTypUtils.hpp
#ifndef DAY_TYP_UTILS_HPP
#define DAY_TYP_UTILS_HPP

#include <sstream>
#include <memory>
#include "queries/shared/data/DailyReportData.hpp"
#include "queries/daily/formatters/typ/DayTypConfig.hpp"
// 声明了所有用于格式化报告不同部分的辅助函数

namespace DayTypUtils {

    /**
     * @brief 显示报告的头部信息。
     */
    void display_header(std::stringstream& ss, const DailyReportData& data, const std::shared_ptr<DayTypConfig>& config);

    /**
     * @brief 显示统计信息。
     */
    void display_statistics(std::stringstream& ss, const DailyReportData& data, const std::shared_ptr<DayTypConfig>& config);

    /**
     * @brief 显示详细的活动记录。
     */
    void display_detailed_activities(std::stringstream& ss, const DailyReportData& data, const std::shared_ptr<DayTypConfig>& config);

    /**
     * @brief 显示项目时间的分解报告。
     */
    void display_project_breakdown(std::stringstream& ss, const DailyReportData& data, const std::shared_ptr<DayTypConfig>& config);

} // namespace DayTypUtils

#endif // DAY_TYP_UTILS_HPP