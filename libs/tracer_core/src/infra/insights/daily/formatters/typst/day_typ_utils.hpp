// infra/insights/daily/formatters/typst/day_typ_utils.hpp
#ifndef INFRASTRUCTURE_INSIGHTS_DAILY_FORMATTERS_TYPST_DAY_TYP_UTILS_H_
#define INFRASTRUCTURE_INSIGHTS_DAILY_FORMATTERS_TYPST_DAY_TYP_UTILS_H_

#include <memory>
#include <string>

#include "domain/insights/models/daily_insights_data.hpp"
#include "infra/insights/daily/formatters/typst/day_typ_config.hpp"

namespace DayTypUtils {

/**
 * @brief 显示报告的头部信息。
 */
void DisplayHeader(std::string& insights_stream, const DailyInsightsData& data,
                   const std::shared_ptr<DayTypConfig>& config);

/**
 * @brief 显示详细的活动记录。
 */
void DisplayDetailedActivities(std::string& insights_stream,
                               const DailyInsightsData& data,
                               const std::shared_ptr<DayTypConfig>& config);

}  // namespace DayTypUtils

#endif  // INFRASTRUCTURE_INSIGHTS_DAILY_FORMATTERS_TYPST_DAY_TYP_UTILS_H_
