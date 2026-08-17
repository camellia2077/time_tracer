// infra/insights/daily/formatters/typst/daily_typ_utils.hpp
#ifndef INFRASTRUCTURE_INSIGHTS_DAILY_FORMATTERS_TYPST_DAILY_TYP_UTILS_H_
#define INFRASTRUCTURE_INSIGHTS_DAILY_FORMATTERS_TYPST_DAILY_TYP_UTILS_H_

#include <memory>
#include <string>

#include "domain/insights/models/daily_insights_data.hpp"
#include "infra/insights/daily/formatters/typst/daily_typ_formatter_config.hpp"

namespace DailyTypUtils {

/**
 * @brief 显示报告的头部信息。
 */
void DisplayHeader(std::string& insights_stream, const DailyInsightsData& data,
                   const std::shared_ptr<DailyTypFormatterConfig>& config);

/**
 * @brief 显示详细的活动记录。
 */
void DisplayDetailedActivities(
    std::string& insights_stream, const DailyInsightsData& data,
    const std::shared_ptr<DailyTypFormatterConfig>& config);

}  // namespace DailyTypUtils

#endif  // INFRASTRUCTURE_INSIGHTS_DAILY_FORMATTERS_TYPST_DAILY_TYP_UTILS_H_
