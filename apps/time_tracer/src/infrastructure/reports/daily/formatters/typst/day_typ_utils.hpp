// infrastructure/reports/daily/formatters/typst/day_typ_utils.hpp
#ifndef REPORTS_DAILY_FORMATTERS_TYPST_DAY_TYP_UTILS_H_
#define REPORTS_DAILY_FORMATTERS_TYPST_DAY_TYP_UTILS_H_

#include <memory>
#include <string>

#include "domain/reports/models/daily_report_data.hpp"
#include "infrastructure/reports/daily/formatters/typst/day_typ_config.hpp"

namespace DayTypUtils {

/**
 * @brief 显示报告的头部信息。
 */
void DisplayHeader(std::string& report_stream, const DailyReportData& data,
                   const std::shared_ptr<DayTypConfig>& config);

/**
 * @brief 显示详细的活动记录。
 */
void DisplayDetailedActivities(std::string& report_stream,
                               const DailyReportData& data,
                               const std::shared_ptr<DayTypConfig>& config);

}  // namespace DayTypUtils

#endif  // REPORTS_DAILY_FORMATTERS_TYPST_DAY_TYP_UTILS_H_
