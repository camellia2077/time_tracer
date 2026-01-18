// reports/daily/formatters/typst/day_typ_utils.hpp
#ifndef REPORTS_DAILY_FORMATTERS_TYPST_DAY_TYP_UTILS_HPP_
#define REPORTS_DAILY_FORMATTERS_TYPST_DAY_TYP_UTILS_HPP_

#include <sstream>
#include <memory>
#include "reports/shared/model/daily_report_data.hpp"
#include "reports/daily/formatters/typst/day_typ_config.hpp"


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

#endif // REPORTS_DAILY_FORMATTERS_TYPST_DAY_TYP_UTILS_HPP_