// reports/daily/formatters/tex/DayTexUtils.hpp
#ifndef DAY_TEX_UTILS_HPP
#define DAY_TEX_UTILS_HPP

#include <sstream>
#include <memory>
#include "reports/shared/data/DailyReportData.hpp"
#include "reports/daily/formatters/tex/DayTexConfig.hpp"

namespace DayTexUtils {

    /**
     * @brief 显示报告的头部信息。
     */
    void display_header(std::stringstream& ss, const DailyReportData& data, const std::shared_ptr<DayTexConfig>& config);

    /**
     * @brief 显示统计数据部分。
     */
    void display_statistics(std::stringstream& ss, const DailyReportData& data, const std::shared_ptr<DayTexConfig>& config);

    /**
     * @brief 显示详细的活动记录。
     */
    void display_detailed_activities(std::stringstream& ss, const DailyReportData& data, const std::shared_ptr<DayTexConfig>& config);

} // namespace DayTexUtils

#endif // DAY_TEX_UTILS_HPP